//
/** @file parser.h */
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
	v6502_address_mode_symbol = -2,
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

/** @defgroup parser_translit Instruction Transliteration */
/**@{*/
/** @brief Returns the string representation of an instruction */
const char *v6502_stringForInstruction(uint16_t instruction);
/** @brief Returns the v6502_opcode for a given instruction string at a specified v6502_address_mode */
v6502_opcode v6502_opcodeForStringAndMode(const char *string, v6502_address_mode mode);
/** @brief Returns the v6502_address_mode for a given instruction string by analyzing the operands */
v6502_address_mode v6502_addressModeForLine(const char *string);
/** @brief Returns the string representation of an v6502_address_mode */
void v6502_stringForAddressMode(char *out, v6502_address_mode mode);
/**@}*/

/** @defgroup parser_line Line Based Parsing */
/**@{*/
/** @brief Determines the numeric value of a literal regardless of base */
void v6502_valueForString(uint8_t *high, uint8_t *low, int *wide, const char *string);
/** @brief Completely parses a line of text to extract the instruction and v6502_address_mode */
void v6502_instructionForLine(uint8_t *opcode, uint8_t *low, uint8_t *high, v6502_address_mode *mode, const char *line, size_t len);
/** @brief Returns the byte-length of a given v6502_address_mode */
int v6502_instructionLengthForAddressMode(v6502_address_mode mode);
/** @brief Executes a symbol-free line of assembly on a specified v6502_cpu */
void v6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len);
/**@}*/

/** @defgroup parser_help Parsing assistance */
/**@{*/
int v6502_isDigit(char c);
/**@}*/

#endif
