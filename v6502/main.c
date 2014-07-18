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
#include "log.h"
#include "parser.h"
#include "error.h"
#include "reverse.h"
#include "linectl.h"
#include "breakpoint.h"
#include "textmode.h"

#define MAX_COMMAND_LEN			80
#define MAX_INSTRUCTION_LEN		32
#define DISASSEMBLY_COUNT		10
#define MEMORY_SIZE				0xFFFF
#define ROM_LOAD_LOCATION		0x600

static int verbose;
static volatile sig_atomic_t interrupt;
static int resist;
static v6502_cpu *cpu;
static v6502_breakpoint_list *breakpoint_list;
static v6502_textmode_video *video;

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
		mem->bytes[ROM_LOAD_LOCATION + (offset++)] = byte;
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
	
	// Step once if we are starting from a breakpoint, so that we don't hit it again
	if (v6502_breakpointIsInList(breakpoint_list, cpu->pc)) {
		if (verbose) {
			printSingleInstruction(cpu, cpu->pc);
		}
		v6502_step(cpu);
	}

	textMode_refreshVideo(video);
	resist = YES;
	do {
		if (v6502_breakpointIsInList(breakpoint_list, cpu->pc)) {
			printf("Hit breakpoint at 0x%02x.\n", cpu->pc);
			return;
		}
		
		if (verbose) {
			printSingleInstruction(cpu, cpu->pc);
		}
		v6502_step(cpu);
	} while (!(cpu->sr & v6502_cpu_status_break) && !interrupt);
	resist = NO;
	
	textMode_rest(video);
	
	if (cpu->sr & v6502_cpu_status_break) {
		printf("Encountered 'brk' at 0x%02x.\n", cpu->pc - 1);
	}
	
	if (interrupt) {
		printf("Received interrupt, CPU halted.\n");
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
		printf("breakpoint <addr>   Toggles a breakpoint at the specified address. If no address is spefied, lists all breakpoints.\n"
			   "cpu                 Displays the current state of the CPU.\n"
			   "disassemble <addr>  Disassemble %d instructions starting at a given address, or the program counter if no address is specified.\n"
			   "help                Displays this help.\n"
			   "iv <type> <addr>    Sets the interrupt vector of the type specified (of nmi, reset, interrupt) to the given address. If no address is specified, then the vector value is output.\n"
			   "load <file>         Load binary image into memory at 0x0600.\n"
			   "nmi                 Sends a non-maskable interrupt to the CPU.\n"
			   "peek <addr>         Dumps the memory at and around a given address.\n"
			   "poke <addr> <value> Sets the location in memory to the value specified.\n"
			   "quit                Exits v6502.\n"
			   "run                 Contunuously steps the cpu until a 'brk' instruction is encountered.\n"
			   "reset               Resets the CPU.\n"
			   "mreset              Zeroes all memory.\n"
			   "step                Forcibly steps the CPU once.\n"
			   "verbose             Toggle verbose mode; prints each instruction as they are executed when running.\n", DISASSEMBLY_COUNT);
		return YES;
	}
	if (compareCommand(command, "breakpoint")) {
		command = trimheadtospc(command, len);

		if(command[0]) {
			command++;
			// Get address
			uint16_t address = as6502_valueForString(NULL, command);
			
			// Toggle breakpoint
			if (v6502_breakpointIsInList(breakpoint_list, address)) {
				v6502_removeBreakpointFromList(breakpoint_list, address);
				printf("Removed breakpoint 0x%04x.\n", address);
			}
			else {
				v6502_addBreakpointToList(breakpoint_list, address);
				printf("Added breakpoint 0x%04x.\n", address);
			}
		}
		else {
			v6502_printBreakpointList(breakpoint_list);
		}

		return YES;
	}
	if (compareCommand(command, "cpu")) {
		v6502_printCpuState(cpu);
		return YES;
	}
	if (compareCommand(command, "nmi")) {
		v6502_nmi(cpu);
		return YES;
	}
	if (compareCommand(command, "iv")) {
		command = trimheadtospc(command, len);
		command++;
		
		uint16_t vector_address = 0;
		
		// Determine IV address based on vector type
		if (compareCommand(command, "nmi")) {
			vector_address = v6502_memoryVectorNMILow;
		}
		else if (compareCommand(command, "reset")) {
			vector_address = v6502_memoryVectorResetLow;
		}
		else if (compareCommand(command, "interrupt")) {
			vector_address = v6502_memoryVectorInterruptLow;
		}
		else {
			printf("Unknown vector type. Valid types are nmi, reset, interrupt.\n");
			return YES;
		}
		
		// Set or display IV based on what was set
		command = trimheadtospc(command, len);
		
		if (command[0]) {
			command++;

			uint8_t low, high;
			as6502_byteValuesForString(&high, &low, NULL, command);

			v6502_write(cpu->memory, vector_address, low);
			v6502_write(cpu->memory, vector_address + 1, high);
		}
		else {
			// Low first, little endian
			uint16_t value = v6502_read(cpu->memory, vector_address, NO) | (v6502_read(cpu->memory, vector_address + 1, NO) << 8);
			printf("0x%04x\n", value);
		}
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
		
		uint16_t start = as6502_valueForString(NULL, command);
		
		// Make sure we don't go out of bounds either direction
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
	if (compareCommand(command, "poke")) {
		command = trimheadtospc(command, len);
		command++;
		
		// Make sure we don't go out of bounds either direction
		uint16_t address = as6502_valueForString(NULL, command);
		uint8_t value;
		
		command = trimheadtospc(command, len);
		command++;
		as6502_byteValuesForString(NULL, &value, NULL, command);
		
		// Make sure we don't go out of bounds either direction
		if (address <= 0x10) {
			address = 0x00;
		}
		else if (address >= cpu->memory->size - 0x30) {
			address = cpu->memory->size - 0x30;
		}
		else {
			address -= 0x10;
		}
		
		v6502_write(cpu->memory, address, value);
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
	
	printf("Creating 1 virtual CPU...\n");
	cpu = v6502_createCPU();
	cpu->fault_callback = fault;
	
	printf("Allocating %dk of virtual memory...\n", (MEMORY_SIZE + 1) / 1024);
	cpu->memory = v6502_createMemory(MEMORY_SIZE);
	
	// Check for a binary as an argument; if so, load and run it
	if (argc > 1) {
		const char *filename = argv[argc - 1];
		printf("Loading binary image \"%s\" into memory...\n", filename);
		loadProgram(cpu->memory, filename);
		
		// Make sure the reset vector is the same
		v6502_write(cpu->memory, v6502_memoryVectorResetLow, ROM_LOAD_LOCATION & 0xFF);
		v6502_write(cpu->memory, v6502_memoryVectorResetHigh, ROM_LOAD_LOCATION >> 8);
	}
	
	printf("Resetting CPU...\n");
	v6502_reset(cpu);

	/* An empty breakpoint list must be created even before first run, since 
	 * breakpoint checks are made during all calls to run()
	 */
	breakpoint_list = v6502_createBreakpointList();

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
	
	textMode_destroy(video);
	v6502_destroyBreakpointList(breakpoint_list);
	history_end(hist);
	el_end(el);
	free(command);
	printf("\n");
    return EXIT_SUCCESS;
}

