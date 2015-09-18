//
//  debugger.c
//  v6502
//
//  Created by Daniel Loffgren on 9/17/15.
//  Copyright (c) 2015 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <as6502/linectl.h>
#include <as6502/parser.h>
#include <dis6502/reverse.h>

#include "debugger.h"
#include "log.h"
#include "breakpoint.h"

#define DISASSEMBLY_COUNT		10

void v6502_loadFileAtAddress(v6502_memory *mem, const char *fname, uint16_t address) {
	FILE *f = fopen(fname, "r");
	
	if (!f) {
		fprintf(stderr, "Could not read from \"%s\"!\n", fname);
		return;
	}
	
	uint8_t byte;
	uint16_t offset = 0;
	
	while (fread(&byte, 1, 1, f)) {
		mem->bytes[address + (offset++)] = byte;
	}
	
	fprintf(stderr, "Loaded %u bytes at 0x%x.\n", offset, address);
	
	fclose(f);
}

static int compareCommand(const char * command, size_t len, const char * literal) {
	char *cmd = malloc(len);
	strncpy(cmd, command, len);
	
	trimgreedytaild(cmd);
	
	for (size_t i = 0; i < len && cmd[i]; i++) {
		if (cmd[i] != literal[i]) {
			free(cmd);
			return NO;
		}
	}
	free(cmd);
	return YES;
}

/** return YES if handled */
int v6502_handleDebuggerCommand(v6502_cpu *cpu, char *command, size_t len, v6502_breakpoint_list *breakpoint_list, v6502_debuggerRunCallback runCallback, int *verbose) {
	if (compareCommand(command, len, "help")) {
		printf("breakpoint <addr>   Toggles a breakpoint at the specified address. If no address is spefied, lists all breakpoints.\n"
			   "cpu                 Displays the current state of the CPU.\n"
			   "disassemble <addr>  Disassemble %d instructions starting at a given address, or the program counter if no address is specified.\n"
			   "help                Displays this help.\n"
			   "iv <type> <addr>    Sets the interrupt vector of the type specified (of nmi, reset, interrupt) to the given address. If no address is specified, then the vector value is output.\n"
			   "load <file> <addr>  Load binary image into memory at the address specified. If no address is specified, then the reset vector is used.\n"
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
	if (compareCommand(command, len, "breakpoint")) {
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
	if (compareCommand(command, len, "cpu")) {
		v6502_printCpuState(stderr, cpu);
		return YES;
	}
	if (compareCommand(command, len, "nmi")) {
		v6502_nmi(cpu);
		return YES;
	}
	if (compareCommand(command, len, "iv")) {
		command = trimheadtospc(command, len);
		command++;
		
		uint16_t vector_address = 0;
		
		// Determine IV address based on vector type
		if (compareCommand(command, len, "nmi")) {
			vector_address = v6502_memoryVectorNMILow;
		}
		else if (compareCommand(command, len, "reset")) {
			vector_address = v6502_memoryVectorResetLow;
		}
		else if (compareCommand(command, len, "interrupt")) {
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
	if (compareCommand(command, len, "load")) {
		const char *c2 = command;
		command = trimheadtospc(command, len);
		
		// Make sure we have at least one argument
		if (!command[0]) {
			printf("You must specify a file to load.\n");
			return YES;
		}
		// Bump past space
		command++;
		
		// We have at least one argument, so extract the filename
		size_t fLen = strnspc(command, len - (c2 - command)) - command;
		char *filename = malloc(fLen + 1);
		memcpy(filename, command, fLen);
		filename[fLen] = '\0';
		
		// Next arg
		command = trimheadtospc(command, len);
		
		// Now check for a load address, or fall back to the reset vector
		uint16_t addr;
		if(command[0]) {
			command++;
			
			uint8_t low, high;
			as6502_byteValuesForString(&high, &low, NULL, command);
			addr = (high << 8) | low;
		}
		else {
			addr = v6502_read(cpu->memory, v6502_memoryVectorResetLow, NO) | (v6502_read(cpu->memory, v6502_memoryVectorResetHigh, NO) << 8);
		}
		
		v6502_loadFileAtAddress(cpu->memory, filename, addr);
		free(filename);
		
		return YES;
	}
	if (compareCommand(command, len, "disassemble")) {
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
			start += dis6502_printAnnotatedInstruction(stderr, cpu, start);
		}
		
		return YES;
	}
	if (compareCommand(command, len, "step")) {
		dis6502_printAnnotatedInstruction(stderr, cpu, cpu->pc);
		v6502_step(cpu);
		return YES;
	}
	if (compareCommand(command, len, "peek")) {
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
	if (compareCommand(command, len, "poke")) {
		command = trimheadtospc(command, len);
		command++;
		
		// Make sure we don't go out of bounds either direction
		uint16_t address = as6502_valueForString(NULL, command);
		uint8_t value;
		
		command = trimheadtospc(command, len);
		command++;
		as6502_byteValuesForString(NULL, &value, NULL, command);
		
		// Make sure we don't go out of bounds either direction
		if (address >= cpu->memory->size) {
			printf("Cannot poke memory that is out of bounds.\n");
			return YES;
		}
		
		v6502_write(cpu->memory, address, value);
		return YES;
	}
	if (compareCommand(command, len, "jmp")) {
		command = trimheadtospc(command, len);
		command++;
		
		uint16_t address = as6502_valueForString(NULL, command);
		cpu->pc = address;
		
		return YES;
	}
	if (compareCommand(command, len, "quit")) {
		v6502_destroyMemory(cpu->memory);
		v6502_destroyCPU(cpu);
		
		exit(EXIT_SUCCESS);
		return NO;
	}
	if (compareCommand(command, len, "run")) {
		runCallback(cpu);
		return YES;
	}
	if (compareCommand(command, len, "reset")) {
		v6502_reset(cpu);
		return YES;
	}
	if (compareCommand(command, len, "mreset")) {
		bzero(cpu->memory->bytes, cpu->memory->size * sizeof(uint8_t));
		return YES;
	}
	if (compareCommand(command, len, "verbose")) {
		printf("Verbose mode %s.\n", *verbose ? "disabled" : "enabled");
		*verbose ^= 1;
		return YES;
	}
	return NO;
}
