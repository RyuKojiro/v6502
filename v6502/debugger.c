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
#include <ctype.h> // isdigit
#include <as6502/linectl.h>
#include <as6502/parser.h>
#include <dis6502/reverse.h>

#include "debugger.h"
#include "log.h"
#include "breakpoint.h"

#define DISASSEMBLY_COUNT		10
#define MAX_ARG_LEN				23

#define XSTRINGIFY(a)			# a
#define STRINGIFY(a)			XSTRINGIFY(a)

#define regeq(a, b)	(!strncasecmp(a, b, sizeof(a)))

#define DEBUGGER_COMMAND_LIST(_) \
	_(breakpoint) \
	_(cpu) \
	_(disassemble) \
	_(help) \
	_(iv) \
	_(label) \
	_(load) \
	_(nmi) \
	_(peek) \
	_(poke) \
	_(quit) \
	_(run) \
	_(register) \
	_(reset) \
	_(mreset) \
	_(script) \
	_(step) \
	_(symbols) \
	_(var) \
	_(verbose)

#define ARRAY_MEMBER(a)			XSTRINGIFY(a),
static const char *_debuggerCommands[] = {
	DEBUGGER_COMMAND_LIST(ARRAY_MEMBER)
};

#define ENUM_MEMBER(a)			v6502_debuggerCommand_ ## a,
typedef enum {
	DEBUGGER_COMMAND_LIST(ENUM_MEMBER)
	v6502_debuggerCommand_NONE
} v6502_debuggerCommand;

static const char *_debuggerCommandArguments[] = {
	"<addr>",
	NULL,
	"<addr>",
	NULL,
	"<type> <addr>",
	"<name> <addr>",
	"<file> <addr>",
	NULL,
	"<addr>",
	"<addr> <value>",
	NULL,
	NULL,
	"<reg> <value>",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"<name> <addr>",
	NULL
};

static const char *_debuggerHelp[] = {
	"Toggles a breakpoint at the specified address. If no address is spefied, lists all breakpoints.",
	"Displays the current state of the CPU.",
	"Disassemble " STRINGIFY(DISASSEMBLY_COUNT) " instructions starting at a given address, or the program counter if no address is specified.",
	"Displays this help.",
	"Sets the interrupt vector of the type specified (of nmi, reset, interrupt) to the given address. If no address is specified, then the vector value is output.",
	"Define a new label for automatic symbolication during disassembly.",
	"Load binary image into memory at the address specified. If no address is specified, then the reset vector is used.",
	"Sends a non-maskable interrupt to the CPU.",
	"Dumps the memory at and around a given address.",
	"Sets the location in memory to the value specified.",
	"Exits v6502.",
	"Contunuously steps the cpu until a 'brk' instruction is encountered.",
	"Sets the value of the specified register.",
	"Resets the CPU.",
	"Zeroes all memory.",
	"Load a script of debugger commands.",
	"Forcibly steps the CPU once.",
	"Print the entire symbol table as it currently exists.",
	"Define a new variable for automatic symbolication during disassembly.",
	"Toggle verbose mode; prints each instruction as they are executed when running."
};

static v6502_debuggerCommand v6502_debuggerCommandParse(const char *command, size_t len) {
	for (int i = 0; i < v6502_debuggerCommand_NONE; i++) {
		if (v6502_compareDebuggerCommand(command, len, _debuggerCommands[i])) {
			return i;
		}
	}
	return v6502_debuggerCommand_NONE;
}

int v6502_loadFileAtAddress(v6502_memory *mem, const char *fname, uint16_t address) {
	FILE *f = fopen(fname, "r");
	
	if (!f) {
		fprintf(stderr, "Could not open \"%s\" for reading!\n", fname);
		return NO;
	}
	
	uint8_t byte;
	uint16_t offset = 0;
	
	while (fread(&byte, 1, 1, f)) {
		mem->bytes[address + (offset++)] = byte;
	}
	
	fprintf(stderr, "Loaded %u bytes at %#x.\n", offset, address);
	
	fclose(f);

	return YES;
}

void v6502_runDebuggerScript(v6502_cpu *cpu, FILE *file, v6502_breakpoint_list *breakpoint_list, as6502_symbol_table *table, v6502_debuggerRunCallback runCallback, int *verbose) {
	char *line = NULL;
	size_t cap = 0;
	ssize_t len;
	while ((len = getline(&line, &cap, file))) {
		v6502_handleDebuggerCommand(cpu, line, len, breakpoint_list, table, runCallback, verbose);
	}
}

static void _makeSymbolOfType(const char *command, size_t len, as6502_symbol_table *table, as6502_symbol_type symbolType) {
	// Extract name
	const char *c2 = command;
	command = trimheadtospc(command, len) + 1;
	size_t sLen = strnspc(command, len - (c2 - command)) - command;
	char *name = strndup(command, sLen);

	command = trimheadtospc(command, len);
	uint16_t address = as6502_valueForString(NULL, command, len - (c2 - command));

	as6502_addSymbolToTable(table, 0, name, address, symbolType);
	free(name);
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

unsigned char v6502_completeDebuggerCommand(EditLine *e, int ch) {
	const LineInfo *lineInfo = el_line(e);
	size_t len = lineInfo->cursor - lineInfo->buffer;
	int multiMatch = NO;

	// Find how many possible matches there are
	int matches = 0;
	const char *lastMatch = NULL;
	for (int i = 0; i < v6502_debuggerCommand_NONE; i++) {
		if (v6502_compareDebuggerCommand(lineInfo->buffer, len, _debuggerCommands[i])) {
			// Multiple matches? Dump them all!
			if (matches) {
				if (matches == 1) {
					printf("\n");
				}
				printf("%s\t", lastMatch);
				multiMatch = YES;
			}

			matches++;
			lastMatch = _debuggerCommands[i];
		}
	}

	if (multiMatch) {
		printf("%s\n", lastMatch);
		return CC_REDISPLAY;
	}

	// Only one match? Complete it
	if (matches == 1 && len < strlen(lastMatch)) {
		el_insertstr(e, lastMatch + len);
		el_insertstr(e, " ");
	}

	return CC_REFRESH;
}

/** Returns YES if handled */
int v6502_handleDebuggerCommand(v6502_cpu *cpu, char *command, size_t len, v6502_breakpoint_list *breakpoint_list, as6502_symbol_table *table, v6502_debuggerRunCallback runCallback, int *verbose) {
	// Make a backup for length calculation
	const char *_command = command;

	// jmp is a special override
	if (v6502_compareDebuggerCommand(command, len, "jmp")) {
		command = trimheadtospc(command, len);
		command++;

		uint16_t address = as6502_valueForString(NULL, command, len - (_command - command));
		cpu->pc = address;

		return YES;
	}

	v6502_debuggerCommand cmd = v6502_debuggerCommandParse(command, len);

	switch (cmd) {
		case v6502_debuggerCommand_help: {
			for (size_t i = 0; i < (sizeof(_debuggerCommands) / sizeof(const char *)); i++) {
				if (_debuggerCommandArguments[i]) {
					char concat[MAX_ARG_LEN];
					snprintf(concat, MAX_ARG_LEN, "%s %s", _debuggerCommands[i], _debuggerCommandArguments[i]);
					printf("%-23s %s\n", concat, _debuggerHelp[i]);
				}
				else {
					printf("%-23s %s\n", _debuggerCommands[i], _debuggerHelp[i]);
				}
			}
			return YES;
		}
		case v6502_debuggerCommand_breakpoint: {
			command = trimheadtospc(command, len);

			if(command[0]) {
				command++;
				// Get address
				uint16_t address;

				// Direct address or symbol name
				if (isdigit(command[0]) || command[0] == '$') {
					address = as6502_valueForString(NULL, command, len - (_command - command));
				}
				else {
					address = as6502_addressForLabel(table, command);
				}

				// Toggle breakpoint
				if (v6502_breakpointIsInList(breakpoint_list, address)) {
					v6502_removeBreakpointFromList(breakpoint_list, address);
					printf("Removed breakpoint %#04x.\n", address);
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
		case v6502_debuggerCommand_cpu: {
			v6502_printCpuState(stderr, cpu);
			return YES;
		}
		case v6502_debuggerCommand_nmi: {
			v6502_nmi(cpu);
			return YES;
		}
		case v6502_debuggerCommand_iv: {
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
				as6502_byteValuesForString(&high, &low, NULL, command, len - (_command - command));

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
		case v6502_debuggerCommand_load: {
			command = trimheadtospc(command, len);

			// Make sure we have at least one argument
			if (!command[0]) {
				printf("You must specify a file to load.\n");
				return YES;
			}
			// Bump past space
			command++;

			// We have at least one argument, so extract the filename
			size_t fLen = strnspc(command, len - (_command - command)) - command;
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
				as6502_byteValuesForString(&high, &low, NULL, command, len - (_command - command));
				addr = (high << 8) | low;
			}
			else {
				addr = v6502_read(cpu->memory, v6502_memoryVectorResetLow, NO) | (v6502_read(cpu->memory, v6502_memoryVectorResetHigh, NO) << 8);
			}

			v6502_loadFileAtAddress(cpu->memory, filename, addr);
			free(filename);
			
			return YES;
		}
		case v6502_debuggerCommand_label: {
			if (!table) {
				return YES;
			}

			_makeSymbolOfType(command, len, table, as6502_symbol_type_label);

			return YES;
		}
		case v6502_debuggerCommand_var: {
			if (!table) {
				return YES;
			}

			_makeSymbolOfType(command, len, table, as6502_symbol_type_variable);

			return YES;
		}
		case v6502_debuggerCommand_step: {
			dis6502_printAnnotatedInstruction(stderr, cpu, cpu->pc, table);
			v6502_step(cpu);
			return YES;
		}
		case v6502_debuggerCommand_disassemble: {
			command = trimheadtospc(command, len);
			uint16_t start = cpu->pc;
			int isFunction = NO;

			if (command[0]) {
				command++;

				// Direct address or symbol name
				if (isdigit(command[0]) || command[0] == '$') {
					start = as6502_valueForString(NULL, command, len - (_command - command));
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
		case v6502_debuggerCommand_script: {
			command = trimheadtospc(command, len);

			// Make sure we have at least one argument
			if (!command[0]) {
				printf("You must specify a file to load.\n");
				return YES;
			}
			// Bump past space
			command++;

			// We have at least one argument, so extract the filename
			size_t fLen = strnspc(command, len - (_command - command)) - command;
			char *filename = malloc(fLen + 1);
			memcpy(filename, command, fLen);
			filename[fLen] = '\0';

			FILE *file = fopen(filename, "r");
			free(filename);
			v6502_runDebuggerScript(cpu, file, breakpoint_list, table, runCallback, verbose);
			fclose(file);
			
			return YES;
		}
		case v6502_debuggerCommand_symbols: {
			as6502_printSymbolTable(table);
			return YES;
		}
		case v6502_debuggerCommand_peek: {
			command = trimheadtospc(command, len);
			command++;

			uint16_t start = as6502_valueForString(NULL, command, len - (_command - command));

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
		case v6502_debuggerCommand_poke: {
			command = trimheadtospc(command, len);
			command++;

			// Make sure we don't go out of bounds either direction
			uint16_t address = as6502_valueForString(NULL, command, len - (_command - command));
			uint8_t value;

			command = trimheadtospc(command, len);
			command++;
			as6502_byteValuesForString(NULL, &value, NULL, command, len - (_command - command));

			// Make sure we don't go out of bounds either direction
			if (address >= cpu->memory->size) {
				printf("Cannot poke memory that is out of bounds.\n");
				return YES;
			}

			v6502_write(cpu->memory, address, value);
			return YES;
		}
		// TODO: jmp
		case v6502_debuggerCommand_quit: {
			v6502_destroyMemory(cpu->memory);
			v6502_destroyCPU(cpu);

			exit(EXIT_SUCCESS);
			return NO;
		}
		case v6502_debuggerCommand_run: {
			runCallback(cpu);
			return YES;
		}
		case v6502_debuggerCommand_reset: {
			v6502_reset(cpu);
			return YES;
		}
		case v6502_debuggerCommand_register: {
			// Extract register name
			command = trimheadtospc(command, len) + 1;
			size_t sLen = strnspc(command, len - (_command - command)) - command;
			char *name = malloc(sLen + 1);
			memcpy(name, command, sLen);
			name[sLen] = '\0';

			// Extract value
			command = trimheadtospc(command, len);
			uint16_t value = as6502_valueForString(NULL, command, len - (_command - command));

			int valid = 0;
			// PC is a special case, in that it is 16-bit
			if (regeq("pc", name)) {
				cpu->pc = value;
				printf("%s -> 0x%04x\n", name, value);
				free(name);
				return YES;
			}

			// Chop off anything above 8-bits for the remaining registers
			value &= 0xFF;

			if (regeq("a", name) || regeq("ac", name)) {
				cpu->ac = value;
				valid++;
			}
			else if (regeq("x", name)) {
				cpu->x = value;
				valid++;
			}
			else if (regeq("y", name)) {
				cpu->y = value;
				valid++;
			}
			else if (regeq("sp", name)) {
				cpu->sp = value;
				valid++;
			}
			else if (regeq("sr", name)) {
				cpu->sr = value;
				valid++;
			}

			if (valid) {
				printf("%s -> 0x%02x\n", name, value);
			}
			else {
				printf("Invalid register '%s'\n", name);
			}
			free(name);
			return YES;
		}
		case v6502_debuggerCommand_mreset: {
			bzero(cpu->memory->bytes, cpu->memory->size * sizeof(uint8_t));
			return YES;
		}
		case v6502_debuggerCommand_verbose: {
			printf("Verbose mode %s.\n", *verbose ? "disabled" : "enabled");
			*verbose ^= 1;
			return YES;
		}
		default:
			return NO;
	}
}
