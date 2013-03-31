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
	
	// Execute whole instruction
	v6502_execute(cpu, opcode, operand1, operand2, operand3);
}
