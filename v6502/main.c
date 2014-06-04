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
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <histedit.h>

#include "cpu.h"
#include "mem.h"
#include "core.h"
#include "parser.h"
#include "error.h"
#include "reverse.h"
#include "linectl.h"

#define MAX_COMMAND_LEN			80
#define MAX_INSTRUCTION_LEN		32
#define DISASSEMBLY_COUNT		10
#define MEMORY_SIZE				0xFFFF

static int verbose;
static volatile sig_atomic_t interrupt;
static int resist;
static v6502_cpu *cpu;

static void fault(void *ctx, const char *error) {
	fprintf(stderr, "fault: ");
	fprintf(stderr, "%s", error);
	if (error[strlen(error)] != '\n') {
		fprintf(stderr, "\n");
	}
}

static void loadProgram(v6502_memory *mem, const char *fname) {
	FILE *f = fopen(fname, "r");
	
	if (!f) {
		fprintf(stderr, "Could not read from \"%s\"!\n", fname);
		return;
	}
	
	uint8_t byte;
	uint16_t offset = 0;
	
	while (fread(&byte, 1, 1, f)) {
		mem->bytes[0x600 + (offset++)] = byte;
	}
	
	printf("Loaded %u bytes.\n", offset);
	
	fclose(f);
}

static int printSingleInstruction(v6502_cpu *cpu, uint16_t address) {
	char instruction[MAX_INSTRUCTION_LEN];
	int instructionLength;
	dis6502_stringForInstruction(instruction, MAX_INSTRUCTION_LEN, cpu->memory->bytes[address], cpu->memory->bytes[address + 2], cpu->memory->bytes[address + 1]);
	instructionLength = v6502_instructionLengthForOpcode(cpu->memory->bytes[address]);
	
	printf("0x%04x: ", address);
	
	switch (instructionLength) {
		case 1: {
			printf("%02x      ", cpu->memory->bytes[address]);
		} break;
		case 2: {
			printf("%02x %02x   ", cpu->memory->bytes[address], cpu->memory->bytes[address + 1]);
		} break;
		case 3: {
			printf("%02x %02x %02x", cpu->memory->bytes[address], cpu->memory->bytes[address + 1], cpu->memory->bytes[address + 2]);
		} break;
		default: {
			printf("        ");
		} break;
	}
	
	printf(" - %s\n", instruction);
	
	return instructionLength;
}

static void run(v6502_cpu *cpu) {
	cpu->sr &= ~v6502_cpu_status_break;
	interrupt = 0;
	
	resist = YES;
	do {
		if (verbose) {
			printSingleInstruction(cpu, cpu->pc);
		}
		v6502_step(cpu);
	} while (!(cpu->sr & v6502_cpu_status_break) && !interrupt);
	resist = NO;
	
	if (cpu->sr & v6502_cpu_status_break) {
		printf("Encountered 'brk' at 0x%02x.\n", cpu->pc - 1);
	}
	
	if (interrupt) {
		printf("Recieved interrupt, CPU halted.\n");
	}
}

static int compareCommand(const char * command, const char * literal) {
	char cmd[MAX_COMMAND_LEN];
	strncpy(cmd, command, MAX_COMMAND_LEN);
	
	trimgreedytaild(cmd);
	
	size_t len = strlen(cmd);
	for (size_t i = 0; i < len && cmd[i]; i++) {
		if (cmd[i] != literal[i]) {
			return NO;
		}
	}
	return YES;
}

/** return YES if handled */
static int handleDebugCommand(v6502_cpu *cpu, char *command, size_t len) {
	if (compareCommand(command, "help")) {
		printf("cpu                 Displays the current state of the CPU.\n"
			   "disassemble <addr>  Disassemble %d instructions starting at a given address, or the program counter if no address is specified.\n"
			   "help                Displays this help.\n"
			   "load <file>         Load binary image into memory at 0x0600.\n"
			   "peek <addr>         Dumps the memory at and around a given address.\n"
			   "quit                Exits v6502.\n"
			   "run                 Contunuously steps the cpu until a 'brk' instruction is encountered.\n"
			   "reset               Resets the CPU.\n"
			   "mreset              Zeroes all memory.\n"
			   "step                Forcibly steps the CPU once.\n"
			   "verbose             Toggle verbose mode; prints each instruction as they are executed when running.\n", DISASSEMBLY_COUNT);
		return YES;
	}
	if (compareCommand(command, "cpu")) {
		v6502_printCpuState(cpu);
		return YES;
	}
	if (compareCommand(command, "load")) {
		command = trimheadtospc(command, len);
		
		if (command[0]) {
			command++;
		}
		else {
			return YES;
		}
		
		trimtaild(command);
		loadProgram(cpu->memory, command);
		return YES;
	}
	if (compareCommand(command, "disassemble")) {
		command = trimheadtospc(command, len);
		
		if (command[0]) {
			command++;
		}
		else {
			return YES;
		}
		
		uint8_t high, low;
		uint16_t start = cpu->pc;
		if (command[0] && command[0] != '\n') {
			as6502_byteValuesForString(&high, &low, NULL, command);
			start = (high << 8) | low;
		}
		
		for (int i = 0; i < DISASSEMBLY_COUNT; i++) {
			start += printSingleInstruction(cpu, start);
		}
		
		return YES;
	}
	if (compareCommand(command, "step")) {
		printSingleInstruction(cpu, cpu->pc);
		v6502_step(cpu);
		return YES;
	}
	if (compareCommand(command, "peek")) {
		command = trimheadtospc(command, len);
		command++;
		
		// Make sure we don't go out of bounds either direction
		uint8_t high, low;
		as6502_byteValuesForString(&high, &low, NULL, command);
		uint16_t start = (high << 8) | low;
		
		if (start <= 0x10) {
			start = 0x00;
		}
		else if (start >= cpu->memory->size - 0x30) {
			start = cpu->memory->size - 0x30;
		}
		else {
			start -= 0x10;
		}
		
		v6502_printMemoryRange(cpu->memory, start, 0x30);
		return YES;
	}
	if (compareCommand(command, "quit")) {
		v6502_destroyMemory(cpu->memory);
		v6502_destroyCPU(cpu);
		
		exit(EXIT_SUCCESS);
		return NO;
	}
	if (compareCommand(command, "run")) {
		run(cpu);
		return YES;
	}
	if (compareCommand(command, "reset")) {
		v6502_reset(cpu);
		return YES;
	}
	if (compareCommand(command, "mreset")) {
		bzero(cpu->memory->bytes, cpu->memory->size * sizeof(uint8_t));
		return YES;
	}
	if (compareCommand(command, "verbose")) {
		printf("Verbose mode %s.\n", verbose ? "disabled" : "enabled");
		verbose ^= 1;
		return YES;
	}
	return NO;
}

void handleSignal(int signal) {
	if (signal == SIGINT) {
		interrupt++;
	}
	
	if (!resist) {
		exit(EXIT_SUCCESS);
	}
}

const char * prompt() {
	static char prompt[10];
	snprintf(prompt, 10, "(0x%04x) ", cpu->pc);
	return prompt;
}

int main(int argc, const char * argv[])
{
	currentFileName = "v6502";
	
	signal(SIGINT, handleSignal);
	
	printf("Creating 1 virtual CPU…\n");
	cpu = v6502_createCPU();
	cpu->fault_callback = fault;
	
	printf("Allocating %dk of virtual memory…\n", (MEMORY_SIZE + 1) / 1024);
	cpu->memory = v6502_createMemory(MEMORY_SIZE);
	
	printf("Resetting CPU…\n");
	v6502_reset(cpu);

	// Check for a binary as an argument; if so, load and run it
	if (argc > 1) {
		const char *filename = argv[argc - 1];
		printf("Loading binary image \"%s\" into memory…\n", filename);
		loadProgram(cpu->memory, filename);
	}
	
	printf("Running…\n");
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
	
	char *command = NULL;
	while (!feof(stdin)) {
		currentLineNum++;
		
		const char *in = el_gets(el, &commandLen);
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

		if (handleDebugCommand(cpu, command, MAX_COMMAND_LEN)) {
			continue;
		}
		else if (command[0] != ';') {
			as6502_executeAsmLineOnCPU(cpu, command, strlen(command));
		}
	}
	
	history_end(hist);
	el_end(el);
	free(command);
	printf("\n");
    return EXIT_SUCCESS;
}

