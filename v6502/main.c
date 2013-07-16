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

#include "cpu.h"
#include "mem.h"
#include "core.h"
#include "parser.h"
#include "error.h"

#define MAX_COMMAND_LEN		80

static void fault(void *ctx, const char *error) {
	fprintf(stderr, "fault: ");
	fprintf(stderr, "%s", error);
	if (error[strlen(error)] != '\n') {
		fprintf(stderr, "\n");
	}
}

static void popArg(char *str, size_t len) {
	char *space = strchr(str, ' ');
	if (!space) {
		return;
	}
	space++;

	size_t diff = space - str;
	strncpy(str, space, MAX_COMMAND_LEN - diff);
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
	
	fclose(f);
}

static void run(v6502_cpu *cpu) {
	cpu->sr &= ~v6502_cpu_status_break;
	do {
		v6502_step(cpu);
	} while (!(cpu->sr & v6502_cpu_status_break));
	printf("Encountered 'brk' at 0x%02x\n", cpu->pc - 1);
}

/** 0 is success, 1 is failure */
static int handleDebugCommand(v6502_cpu *cpu, char *command) {
	if (!strncmp(command, "help", 4)) {
		printf("!cpu\t\t\tDisplays the current state of the CPU.\n"
			   "!help\t\t\tDisplays this help.\n"
			   "!peek <addr>\tDumps the memory at and around a given address.\n"
			   "!quit\t\t\tExits v6502.\n"
			   "!run\t\t\tContunuously steps the cpu until a 'brk' instruction is encountered.\n"
			   "!step\t\t\tSteps the CPU once.\n"
			   "Anything not starting with an exclamation point is interpreted as a assembly instruction.\n");
		return 0;
	}
	if (!strncmp(command, "cpu", 3)) {
		v6502_printCpuState(cpu);
		return 0;
	}
	if (!strncmp(command, "step", 4)) {
		v6502_step(cpu);
		return 0;
	}
	if (!strncmp(command, "peek", 4)) {
		popArg(command, MAX_COMMAND_LEN);
		
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
	printf("Unknown Command - %s\n", command);
	return 0;
}

int main(int argc, const char * argv[])
{
	currentFileName = "v6502";
	
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
		
		printf("(0x%x) ", cpu->pc);
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

