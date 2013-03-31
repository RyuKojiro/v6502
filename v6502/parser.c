//
//  parser.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/30.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <ctype.h>

#include "parser.h"
#include "core.h"

v6502_opcode v6502_opcodeForString(const char *string) {
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

void v6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len) {
	v6502_opcode opcode;
	uint8_t operand1, operand2, operand3;
	char string[len];
	
	// Normalize text (all lowercase) and copy into a non-const string
	for(int i = 0; string[i]; i++){
		string[i] = tolower(line[i]);
	}
	
	// Determine opcode, based on entire line
	opcode = v6502_opcodeForString(string);
	
	// Determine operands
	v6502_populateOperandsFromLine(string, len, &operand1, &operand2, &operand3);
	
	// Execute whole instruction
	v6502_execute(cpu, opcode, operand1, operand2, operand3);
}
