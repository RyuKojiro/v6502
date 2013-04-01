//
//  parser.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/30.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <ctype.h>
#include <stdio.h>

#include "parser.h"
#include "core.h"

void v6502_parseError(const char *fmt, ...) {
	
}

v6502_opcode v6502_opcodeForStringAndMode(const char *string, v6502_address_mode mode) {
	char *arg1 = strchr(string, ' ');
	
	if (!strncmp(string, "brk", 3)) {
		return v6502_opcode_brk;
	}
	if (!strncmp(string, "ora", 3)) {
		if (arg1[0] == 'X') {
			return v6502_opcode_ora_x;
		}
		if (arg1[0] == '#') {
			return v6502_opcode_ora_val;
		}
	}
	if (!strncmp(string, "nop", 3)) {
		return v6502_opcode_nop;
	}
	
	char exception[50];
	snprintf(exception, 50, "Unknown Opcode - %s", string);
	v6502_fault(exception);
	return v6502_opcode_brk;
}

void v6502_populateOperandsFromLine(const char *line, size_t len, uint8_t *operand1, uint8_t *operand2, uint8_t *operand3) {
	// Work backwards filling an array, swapping bytes as needed, remember this is little endian
}

void v6502_addressForString(uint8_t *highByte, uint8_t *lowByte, const char *string) {
	
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
	 √ OPC	....	implied
	 √ OPC A	....	Accumulator
	 √ OPC #BB	....	immediate
	 OPC HHLL	....	absolute
	 OPC HHLL,X	....	absolute, X-indexed
	 OPC HHLL,Y	....	absolute, Y-indexed
	 √ OPC *LL	....	zeropage
	 √ OPC *LL,X	....	zeropage, X-indexed
	 √ OPC *LL,Y	....	zeropage, Y-indexed
	 √ OPC (BB,X)	....	X-indexed, indirect
	 √ OPC (LL),Y	....	indirect, Y-indexed
	 √ OPC (HHLL)	....	indirect
	 OPC BB	....	relative
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
			// TODO: Use v6502_addressForString to test byte length
		} break;
	}
	
	v6502_parseError("Unknown address mode for line - %s", string);
	return -1;
}

void v6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len) {
	v6502_address_mode mode;
	v6502_opcode opcode;
	uint8_t operand1, operand2, operand3;
	char string[len];
	
	// Normalize text (all lowercase) and copy into a non-const string
	// Perhaps this should collapse whitespace as well?
	for(int i = 0; string[i]; i++){
		string[i] = tolower(line[i]);
	}
	
	// Determine address mode
	mode = v6502_addressModeForLine(string);
	
	// Determine opcode, based on entire line
	opcode = v6502_opcodeForStringAndMode(string, mode);
	
	// Determine operands
	v6502_populateOperandsFromLine(string, len, &operand1, &operand2, &operand3);
	
	// Execute whole instruction
	v6502_execute(cpu, opcode, operand1, operand2, operand3);
}
