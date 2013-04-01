//
//  parser.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/30.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "core.h"

#define YES		1
#define NO		0

void v6502_parseError(const char *fmt, ...) {
	
}

v6502_opcode v6502_opcodeForStringAndMode(const char *string, v6502_address_mode mode) {	
	if (!strncmp(string, "brk", 3)) {
		return v6502_opcode_brk;
	}
	if (!strncmp(string, "ora", 3)) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_ora_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_ora_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_ora_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_ora_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_ora_absx;
			case v6502_address_mode_absolute_y:
				return v6502_opcode_ora_absy;
			case v6502_address_mode_indirect_x:
				return v6502_opcode_ora_indx;
			case v6502_address_mode_indirect_y:
				return v6502_opcode_ora_indy;
			default:
				v6502_fault("Bad address mode for operation ORA");
				return v6502_opcode_nop;
		}
	}
	if (!strncmp(string, "nop", 3)) {
		return v6502_opcode_nop;
	}
	
	char exception[50];
	snprintf(exception, 50, "Unknown Opcode - %s", string);
	v6502_fault(exception);
	return v6502_opcode_nop;
}

static int _valueLengthInChars(const char *string) {
	int i;
	for (i = 0; string[i] && (isdigit(string[i]) || (string[i] >= 'a' && string[i] <= 'f')); i++);
	
	return i;
}

void v6502_valueForString(uint8_t *high, uint8_t *low, char *wide, const char *string) {
	char workString[6];
	uint16_t result;
	int base;
	
	// Remove all whitespace, #'s, *'s, and parenthesis, also, truncate at comma
	int i = 0;
	for (const char *cur = string; *cur; cur++) {
		if (!isspace(*cur) && *cur != '#' && *cur !='*' && *cur !='(' && *cur !=')') {
			workString[i++] = *cur;
		}
		if (*cur == ',') {
			break;
		}
	}
	workString[i] = '\0';
	
	// Check first char to determine base
	switch (workString[0]) {
		case '$': { // Hex
			base = 16;
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 2) ? YES : NO;
			}
		} break;
		case '%': { // Binary
			base = 2;
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 8) ? YES : NO;
			}
		} break;
		case '0': { // Octal
			base = 8;
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 3) ? YES : NO;
			}
		} break;
		default: { // Decimal
			base = 10;
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 3) ? YES : NO;
			}
		} break;
	}
	
	result = strtol(workString + 1, NULL, base);

	if (result > 0xFF && wide) {
		// Octal and decimal split digits
		*wide = YES;
	}
	if (low) {
		*low = result & 0xFF;
	}

	if (high) {
		*high = result >> 8;
	}
}

static v6502_address_mode _incrementModeByFoundRegister(v6502_address_mode mode, const char *cur) {
	/* This relies on the fact that the enum is always ordered in normal, x, y. */
	
	cur = strchr(cur, 'x');
	if (cur) {
		return mode + 1;
	}
	
	cur = strchr(cur, 'y');
	if (cur) {
		return mode + 2;
	}
	
	return mode;
}

v6502_address_mode v6502_addressModeForLine(const char *string) {
	/* 
	 √ OPC			....	implied
	 √ OPC A		....	Accumulator
	 √ OPC #BB		....	immediate
	 √ OPC HHLL		....	absolute
	 √ OPC HHLL,X	....	absolute, X-indexed
	 √ OPC HHLL,Y	....	absolute, Y-indexed
	 √ OPC *LL		....	zeropage
	 √ OPC *LL,X	....	zeropage, X-indexed
	 √ OPC *LL,Y	....	zeropage, Y-indexed
	 √ OPC (BB,X)	....	X-indexed, indirect
	 √ OPC (LL),Y	....	indirect, Y-indexed
	 √ OPC (HHLL)	....	indirect
	 √ OPC BB		....	relative
	 */
	
	const char *cur;
	
	// Skip opcode and whitespace to find first argument
	for (cur = string + 3; isspace(*cur); cur++) {
		if (*cur == '\0' || *cur == ';') {
			return v6502_address_mode_implied;
		}
	}
	
	// Check first character of argument, and byte length
	switch (*cur) {
		case 'a': // Accumulator (normalized)
			return v6502_address_mode_accumulator;
		case '#': // Immediate
			return v6502_address_mode_immediate;
		case '*': { // Zeropage
			return _incrementModeByFoundRegister(v6502_address_mode_zeropage, cur);
		} break;
		case '(': { // Indirect
			return _incrementModeByFoundRegister(v6502_address_mode_indirect, cur);
		} break;
		default: { // Relative or Absolute
			// TODO: Better byte length determination, this doesn't tell shit
			char wide;
			v6502_valueForString(NULL, NULL, &wide, cur);
			if (wide) {
				return _incrementModeByFoundRegister(v6502_address_mode_absolute, cur);
			}
			else {
				return v6502_address_mode_relative;
			}
		} break;
	}
	
	v6502_parseError("Unknown address mode for line - %s", string);
	return -1;
}

void v6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len) {
	v6502_address_mode mode;
	v6502_opcode opcode;
	uint8_t low, high;
	char string[len];
	
	// Normalize text (all lowercase) and copy into a non-const string
	// Perhaps this should collapse whitespace as well?
	int i;
	for(i = 0; line[i]; i++){
		string[i] = tolower(line[i]);
	}
	string[i] = '\0';
	
	// Determine address mode
	mode = v6502_addressModeForLine(string);
	
	/* TODO: Make none of this rely on the operation being the first 3 chars every time */
	// Determine opcode, based on entire line
	opcode = v6502_opcodeForStringAndMode(string, mode);
	
	// Determine operands
	v6502_valueForString(&high, &low, NULL, string + 3);
	
	// Execute whole instruction
	v6502_execute(cpu, opcode, low, high);
}
