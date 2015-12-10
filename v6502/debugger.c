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
#include <ctype.h> // isdigit
#include <as6502/linectl.h>
#include <as6502/parser.h>
#include <dis6502/reverse.h>

#include "debugger.h"
#include "log.h"
#include "breakpoint.h"

#define DISASSEMBLY_COUNT		10
#define MAX_LINE_LEN			80

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

void v6502_runDebuggerScript(v6502_cpu *cpu, FILE *file, v6502_breakpoint_list *breakpoint_list, as6502_symbol_table *table, v6502_debuggerRunCallback runCallback, int *verbose) {
	char line[MAX_LINE_LEN];
	while (fgets(line, MAX_LINE_LEN, file)) {
		v6502_handleDebuggerCommand(cpu, line, MAX_LINE_LEN, breakpoint_list, table, runCallback, verbose);
	}
}

int v6502_compareDebuggerCommand(const char * command, size_t len, const char * literal) {
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

/** Returns YES if handled */
int v6502_handleDebuggerCommand(v6502_cpu *cpu, char *command, size_t len, v6502_breakpoint_list *breakpoint_list, as6502_symbol_table *table, v6502_debuggerRunCallback runCallback, int *verbose) {
	if (v6502_compareDebuggerCommand(command, len, "help")) {
		printf("breakpoint <addr>   Toggles a breakpoint at the specified address. If no address is spefied, lists all breakpoints.\n"
			   "cpu                 Displays the current state of the CPU.\n"
			   "disassemble <addr>  Disassemble %d instructions starting at a given address, or the program counter if no address is specified.\n"
			   "help                Displays this help.\n"
			   "iv <type> <addr>    Sets the interrupt vector of the type specified (of nmi, reset, interrupt) to the given address. If no address is specified, then the vector value is output.\n"
			   "label <name> <addr> Define a new label for automatic symbolication during disassembly.\n"
			   "load <file> <addr>  Load binary image into memory at the address specified. If no address is specified, then the reset vector is used.\n"
			   "nmi                 Sends a non-maskable interrupt to the CPU.\n"
			   "peek <addr>         Dumps the memory at and around a given address.\n"
			   "poke <addr> <value> Sets the location in memory to the value specified.\n"
			   "quit                Exits v6502.\n"
			   "run                 Contunuously steps the cpu until a 'brk' instruction is encountered.\n"
			   "reset               Resets the CPU.\n"
			   "mreset              Zeroes all memory.\n"
			   "script              Load a script of debugger commands.\n"
			   "step                Forcibly steps the CPU once.\n"
			   "symbols             Print the entire symbol table as it currently exists.\n"
			   "var <name> <addr>   Define a new variable for automatic symbolication during disassembly.\n"
			   "verbose             Toggle verbose mode; prints each instruction as they are executed when running.\n", DISASSEMBLY_COUNT);
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "breakpoint")) {
		command = trimheadtospc(command, len);
		
		if(command[0]) {
			command++;
			// Get address
			uint16_t address;
			
			// Direct address or symbol name
			if (isdigit(command[0]) || command[0] == '$') {
				address = as6502_valueForString(NULL, command);
			}
			else {
				address = as6502_addressForLabel(table, command);
			}
			
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
	if (v6502_compareDebuggerCommand(command, len, "cpu")) {
		v6502_printCpuState(stderr, cpu);
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "nmi")) {
		v6502_nmi(cpu);
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "iv")) {
		command = trimheadtospc(command, len);
		command++;
		
		uint16_t vector_address = 0;
		
		// Determine IV address based on vector type
		if (v6502_compareDebuggerCommand(command, len, "nmi")) {
			vector_address = v6502_memoryVectorNMILow;
		}
		else if (v6502_compareDebuggerCommand(command, len, "reset")) {
			vector_address = v6502_memoryVectorResetLow;
		}
		else if (v6502_compareDebuggerCommand(command, len, "interrupt")) {
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
	if (v6502_compareDebuggerCommand(command, len, "load")) {
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
	
	as6502_symbol_type symbolType = as6502_symbol_type_unknown; // Initialized for lint
	int makeSymbol = 0;
	if (v6502_compareDebuggerCommand(command, len, "label")) {
		symbolType = as6502_symbol_type_label;
		makeSymbol++;
	}
	else if (v6502_compareDebuggerCommand(command, len, "var")) {
		symbolType = as6502_symbol_type_variable;
		makeSymbol++;
	}
	
	if (makeSymbol) {
		if (!table) {
			return YES;
		}
		
		// Extract name
		const char *c2 = command;
		command = trimheadtospc(command, len) + 1;
		size_t sLen = strnspc(command, len - (c2 - command)) - command;
		char *name = malloc(sLen + 1);
		memcpy(name, command, sLen);
		name[sLen] = '\0';
		
		command = trimheadtospc(command, len);
		uint16_t address = as6502_valueForString(NULL, command);
		
		as6502_addSymbolToTable(table, 0, name, address, symbolType);
		free(name);
		return YES;
	}
	
	if (v6502_compareDebuggerCommand(command, len, "disassemble")) {
		command = trimheadtospc(command, len);
		uint16_t start = cpu->pc;
		int isFunction = NO;

		if (command[0]) {
			command++;

			// Direct address or symbol name
			if (isdigit(command[0]) || command[0] == '$') {
				start = as6502_valueForString(NULL, command);
			}
			else {
				start = as6502_addressForLabel(table, command);
				isFunction = YES;
			}
		}
		
		for (int i = 0; i < DISASSEMBLY_COUNT; i++) {
			start += dis6502_printAnnotatedInstruction(stderr, cpu, start, table);
			if (isFunction && v6502_read(cpu->memory, start, NO) == v6502_opcode_rts) {
				dis6502_printAnnotatedInstruction(stderr, cpu, start, table);
				break;
			}
		}
		
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "step")) {
		as6502_symbol *symbol = as6502_symbolForAddress(table, cpu->pc);
		if (symbol) {
			fprintf(stderr, "0x%02x: %s:\n", symbol->address, symbol->name);
		}
		dis6502_printAnnotatedInstruction(stderr, cpu, cpu->pc, table);
		v6502_step(cpu);
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "script")) {
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
		
		FILE *file = fopen(filename, "r");
		v6502_runDebuggerScript(cpu, file, breakpoint_list, table, runCallback, verbose);
		fclose(file);
		
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "symbols")) {
		as6502_printSymbolTable(table);
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "peek")) {
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
	if (v6502_compareDebuggerCommand(command, len, "poke")) {
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
	if (v6502_compareDebuggerCommand(command, len, "jmp")) {
		command = trimheadtospc(command, len);
		command++;
		
		uint16_t address = as6502_valueForString(NULL, command);
		cpu->pc = address;
		
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "quit")) {
		v6502_destroyMemory(cpu->memory);
		v6502_destroyCPU(cpu);
		
		exit(EXIT_SUCCESS);
		return NO;
	}
	if (v6502_compareDebuggerCommand(command, len, "run")) {
		runCallback(cpu);
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "reset")) {
		v6502_reset(cpu);
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "mreset")) {
		bzero(cpu->memory->bytes, cpu->memory->size * sizeof(uint8_t));
		return YES;
	}
	if (v6502_compareDebuggerCommand(command, len, "verbose")) {
		printf("Verbose mode %s.\n", *verbose ? "disabled" : "enabled");
		*verbose ^= 1;
		return YES;
	}
	return NO;
}
