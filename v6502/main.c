/*
 * Copyright (c) 2013 Daniel Loffgren
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <histedit.h>

#include <as6502/parser.h>
#include <as6502/error.h>
#include <dis6502/reverse.h>
#include <as6502/linectl.h>

#include "cpu.h"
#include "mem.h"
#include "log.h"
#include "breakpoint.h"
#include "textmode.h"
#include "debugger.h"

#define MEMORY_SIZE				0xFFFF
#define DEFAULT_RESET_VECTOR	0x0600

static int verbose;
static volatile sig_atomic_t interrupt;
static int resist;
static v6502_cpu *cpu;
static v6502_breakpoint_list *breakpoint_list;
static v6502_textmode_video *video;
static as6502_symbol_table *table;

static void fault(void *ctx, const char *error) {
	(void)ctx;
	
	fprintf(stderr, "fault: ");
	fprintf(stderr, "%s", error);
	if (error[strlen(error)] != '\n') {
		fprintf(stderr, "\n");
	}
}

static void run(v6502_cpu *cpu) {
	cpu->sr &= ~v6502_cpu_status_break;
	interrupt = 0;
	
	// Step once if we are starting from a breakpoint, so that we don't hit it again
	if (v6502_breakpointIsInList(breakpoint_list, cpu->pc)) {
		if (verbose) {
			dis6502_printAnnotatedInstruction(stderr, cpu, cpu->pc, table);
		}
		v6502_step(cpu);
	}

	textMode_refreshVideo(video);
	resist = YES;
	do {
		if (v6502_breakpointIsInList(breakpoint_list, cpu->pc)) {
			printf("Hit breakpoint at %#02x.\n", cpu->pc);
			return;
		}
		
		if (verbose) {
			dis6502_printAnnotatedInstruction(stderr, cpu, cpu->pc, table);
		}
		v6502_step(cpu);
	} while (!(cpu->sr & v6502_cpu_status_break) && !interrupt);
	resist = NO;
	
	textMode_rest(video);
	
	if (cpu->sr & v6502_cpu_status_break) {
		printf("Encountered 'brk' at %#02x.\n", cpu->pc - 1);
	}
	
	if (interrupt) {
		printf("Received interrupt, CPU halted.\n");
	}
}

static void handleSignal(int signal) {
	if (signal == SIGINT) {
		interrupt++;
	}
	
	if (!resist) {
		exit(EXIT_SUCCESS);
	}
}

static const char * prompt() {
	static char prompt[10];
	snprintf(prompt, 10, "(%#04x) ", cpu->pc);
	return prompt;
}

int main(int argc, const char * argv[])
{
	currentFileName = "v6502";
	
	signal(SIGINT, handleSignal);
	
	printf("Creating 1 virtual CPU...\n");
	cpu = v6502_createCPU();
	cpu->fault_callback = fault;
	
	printf("Allocating %dk of virtual memory...\n", (MEMORY_SIZE + 1) / 1024);
	cpu->memory = v6502_createMemory(MEMORY_SIZE);
	
	// Check for a binary as an argument; if so, load and run it
	if (argc > 1) {
		const char *filename = argv[argc - 1];
		printf("Loading binary image \"%s\" into memory...\n", filename);
		v6502_loadFileAtAddress(cpu->memory, filename, DEFAULT_RESET_VECTOR);
	}
	
	// Set the reset vector
	v6502_write(cpu->memory, v6502_memoryVectorResetLow, DEFAULT_RESET_VECTOR & 0xFF);
	v6502_write(cpu->memory, v6502_memoryVectorResetHigh, DEFAULT_RESET_VECTOR >> 8);
	
	printf("Resetting CPU...\n");
	v6502_reset(cpu);

	/* An empty breakpoint list must be created even before first run, since 
	 * breakpoint checks are made during all calls to run()
	 */
	breakpoint_list = v6502_createBreakpointList();

	// An empty symbol table is allocated, since they are dynamically created
	table = as6502_createSymbolTable();
	
	printf("Starting Text Mode Video...\n");
	video = textMode_create(cpu->memory);
	
	printf("Running...\n");
	run(cpu);
	
	int commandLen;
	HistEvent ev;
	History *hist = history_init();
	history(hist, &ev, H_SETSIZE, 100);
	
	EditLine *el = el_init(currentFileName, stdin, stdout, stderr);
	el_set(el, EL_PROMPT, &prompt);
	el_set(el, EL_SIGNAL, SIGWINCH);
	el_set(el, EL_EDITOR, "emacs");
	el_set(el, EL_HIST, history, hist);
	el_set(el, EL_ADDFN, "tab-complete", "Tab completion", v6502_completeDebuggerCommand);
	el_set(el, EL_BIND, "\t", "tab-complete", NULL);

	char *command = NULL;
	while (!feof(stdin)) {
		currentLineNum++;
		
		const char *in = el_gets(el, &commandLen);
		currentLineText = in;

		if (!in) {
			break;
		}
		
		history(hist, &ev, H_ENTER, in);
		command = realloc(command, commandLen + 1);
		memcpy(command, in, commandLen);
		
		// Trim newline, always the last char
		command[commandLen - 1] = '\0';
		
		if (command[0] == '\0') {
			continue;
		}

		if (v6502_handleDebuggerCommand(cpu, command, commandLen, breakpoint_list, table, run, &verbose)) {
			continue;
		}
		else if (command[0] != ';') {
			as6502_executeAsmLineOnCPU(cpu, command, strlen(command));
		}
	}
	
	textMode_destroy(video);
	as6502_destroySymbolTable(table);
	v6502_destroyBreakpointList(breakpoint_list);
	history_end(hist);
	el_end(el);
	free(command);
	printf("\n");
    return EXIT_SUCCESS;
}

