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

/** 0 is success, 1 is exit */
static int handleDebugCommand(v6502_cpu *cpu, char *command) {
	if (!strncmp(command, "help", 4)) {
		printf("!cpu         Displays the current state of the CPU.\n"
			   "!dis <addr>  Disassemble ten instructions starting at a given address, or the program counter if no address is specified.\n"
			   "!help        Displays this help.\n"
			   "!load <file> Load binary image into memory at 0x0600.\n"
			   "!peek <addr> Dumps the memory at and around a given address.\n"
			   "!quit        Exits v6502.\n"
			   "!run         Contunuously steps the cpu until a 'brk' instruction is encountered.\n"
			   "!reset       Resets the CPU.\n"
			   "!mreset      Zeroes all memory.\n"
			   "!step        Forcibly steps the CPU once.\n"
			   "!v           Toggle verbose mode; prints each instruction as they are executed when running.\n"
			   "Anything not starting with an exclamation point is interpreted as a assembly instruction.\n");
		return 0;
	}
	if (!strncmp(command, "cpu", 3)) {
		v6502_printCpuState(cpu);
		return 0;
	}
	if (!strncmp(command, "load", 4)) {
		command = trimheadtospc(command);
		
		if (command[0]) {
			command++;
		}
		else {
			return 0;
		}
		
		trimtaild(command);
		loadProgram(cpu->memory, command);
		return 0;
	}
	if (!strncmp(command, "dis", 3)) {
		command = trimheadtospc(command);
		
		if (command[0]) {
			command++;
		}
		else {
			return 0;
		}
		
		uint8_t high, low;
		uint16_t start = cpu->pc;
		if (command[0] && command[0] != '\n') {
			as6502_byteValuesForString(&high, &low, NULL, command);
			start = (high << 8) | low;
		}
		
		for (int i = 0; i < 10; i++) {
			start += printSingleInstruction(cpu, start);
		}
		
		return 0;
	}
	if (!strncmp(command, "step", 4)) {
		printSingleInstruction(cpu, cpu->pc);
		v6502_step(cpu);
		return 0;
	}
	if (!strncmp(command, "peek", 4)) {
		command = trimheadtospc(command);
		
		if (command[0]) {
			command++;
		}
		else {
			return 0;
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
	if (!strncmp(command, "quit", 4)) {
		return 1;
	}
	if (!strncmp(command, "run", 3)) {
		run(cpu);
		return 0;
	}
	if (!strncmp(command, "reset", 5)) {
		v6502_reset(cpu);
		return 0;
	}
	if (!strncmp(command, "mreset", 5)) {
		bzero(cpu->memory->bytes, cpu->memory->size * sizeof(uint8_t));
		return 0;
	}
	if (!strncmp(command, "v", 1)) {
		printf("Verbose mode %s.\n", verbose ? "disabled" : "enabled");
		verbose ^= 1;
		return 0;
	}
	printf("Unknown Command - %s\n", command);
	return 0;
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
	
	printf("Allocating 64k of virtual memory…\n");
	cpu->memory = v6502_createMemory(0xFFFF);
	
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
		
		// getInput(command, MAX_COMMAND_LEN);
		fgets(command, MAX_COMMAND_LEN, stdin);
		
		if (command[0] == '\n') {
			continue;
		}

		if (command[0] == '!') {
			if (handleDebugCommand(cpu, command + 1)) {
				break;
			}
		}
		else if (command[0] != ';') {
			as6502_executeAsmLineOnCPU(cpu, command, strlen(command));
		}
	}
	
	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	
    return 0;
}

