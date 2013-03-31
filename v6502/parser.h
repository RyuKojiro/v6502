//
//  parser.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/30.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_parser_h
#define v6502_parser_h

#include <string.h>

#include "cpu.h"

typedef enum {
	v6502_address_mode_implied = 0,		// Or none
	v6502_address_mode_accumulator,
	v6502_address_mode_immediate,
	v6502_address_mode_absolute,
	v6502_address_mode_absolute_x,
	v6502_address_mode_absolute_y,
	v6502_address_mode_indirect,
	v6502_address_mode_indirect_x,
	v6502_address_mode_indirect_y,
	v6502_address_mode_relative,
	v6502_address_mode_zeropage,
	v6502_address_mode_zeropage_x,
	v6502_address_mode_zeropage_y
} v6502_address_mode;

// Instruction Transliteration
const char *v6502_stringForInstruction(uint16_t instruction);
v6502_opcode v6502_opcodeForStringAndMode(const char *string, v6502_address_mode mode);
v6502_address_mode v6502_addressModeForLine(const char *string);

// Line Based Parsing
void v6502_populateOperandsFromLine(const char *line, size_t len,
									uint8_t *operand1, uint8_t *operand2, uint8_t *operand3);
void v6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len);

#endif
