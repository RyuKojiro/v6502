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

#define YES		1
#define NO		0

typedef enum {
	v6502_address_mode_unknown = -1,
	v6502_address_mode_implied = 0,		// Or none
	v6502_address_mode_accumulator = 1,
	v6502_address_mode_immediate = 2,
	v6502_address_mode_absolute = 3,
	v6502_address_mode_absolute_x = 4,
	v6502_address_mode_absolute_y = 5,
	v6502_address_mode_indirect = 6,
	v6502_address_mode_indirect_x = 7,
	v6502_address_mode_indirect_y = 8,
	v6502_address_mode_relative = 9,
	v6502_address_mode_zeropage = 10,
	v6502_address_mode_zeropage_x = 11,
	v6502_address_mode_zeropage_y = 12
} v6502_address_mode;

// Instruction Transliteration
const char *v6502_stringForInstruction(uint16_t instruction);
v6502_opcode v6502_opcodeForStringAndMode(const char *string, v6502_address_mode mode);
v6502_address_mode v6502_addressModeForLine(const char *string);
void v6502_stringForAddressMode(char *out, v6502_address_mode mode);

// Line Based Parsing
void v6502_valueForString(uint8_t *high, uint8_t *low, int *wide, const char *string);
void v6502_instructionForLine(uint8_t *opcode, uint8_t *low, uint8_t *high, v6502_address_mode *mode, const char *line, size_t len);
int v6502_instructionLengthForAddressMode(v6502_address_mode mode);
void v6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len);

#endif
