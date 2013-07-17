//
//  main.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

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
	as6502_stringForInstruction(instruction, MAX_INSTRUCTION_LEN, cpu->memory->bytes[address], cpu->memory->bytes[address + 2], cpu->memory->bytes[address + 1]);
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

static int compareCommand(const char * restrict command, const char * restrict literal) {
	size_t len = strlen(command);
	for (size_t i = 0; i < len; i++) {
		if (command[i] != literal[i] && command[i] != '\n' && command[i] != '\0') {
			return NO;
		}
	}
	return YES;
}

/** return YES if handled */
static int handleDebugCommand(v6502_cpu *cpu, char *command) {
	if (compareCommand(command, "help")) {
		printf("cpu         Displays the current state of the CPU.\n"
			   "dis <addr>  Disassemble %d instructions starting at a given address, or the program counter if no address is specified.\n"
			   "help        Displays this help.\n"
			   "load <file> Load binary image into memory at 0x0600.\n"
			   "peek <addr> Dumps the memory at and around a given address.\n"
			   "quit        Exits v6502.\n"
			   "run         Contunuously steps the cpu until a 'brk' instruction is encountered.\n"
			   "reset       Resets the CPU.\n"
			   "mreset      Zeroes all memory.\n"
			   "step        Forcibly steps the CPU once.\n"
			   "v           Toggle verbose mode; prints each instruction as they are executed when running.\n", DISASSEMBLY_COUNT);
		return YES;
	}
	if (compareCommand(command, "cpu")) {
		v6502_printCpuState(cpu);
		return YES;
	}
	if (compareCommand(command, "load")) {
		command = trimheadtospc(command);
		
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
	if (compareCommand(command, "dis")) {
		command = trimheadtospc(command);
		
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
		command = trimheadtospc(command);
		
		if (command[0]) {
			command++;
		}
		else {
			return YES;
		}
		
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
		return 0;
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
	if (compareCommand(command, "v")) {
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

int main(int argc, const char * argv[])
{
	currentFileName = "v6502";
	
	signal(SIGINT, handleSignal);
	
	printf("Creating 1 virtual CPU…\n");
	v6502_cpu *cpu = v6502_createCPU();
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
	
	char command[MAX_COMMAND_LEN];
	for (;;) {
		currentLineNum++;
		
		printf("(0x%04x) ", cpu->pc);
		fflush(stdout);
		
		fgets(command, MAX_COMMAND_LEN, stdin);
		
		if (command[0] == '\n') {
			continue;
		}

		if (handleDebugCommand(cpu, command)) {
			continue;
		}
		else if (command[0] != ';') {
			as6502_executeAsmLineOnCPU(cpu, command, strlen(command));
		}
	}

	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	
    return EXIT_FAILURE;
}

