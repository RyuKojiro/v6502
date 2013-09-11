//
/** @brief Assembly language parsing */
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

#ifndef YES
/** @brief Boolean true */
#define YES		1
#endif

#ifndef NO
/** @brief Boolean false */
#define NO		0
#endif

/** @defgroup parser_translit Instruction Transliteration */
/**@{*/
/** @brief Returns the v6502_opcode for a given instruction string at a specified v6502_address_mode */
v6502_opcode as6502_opcodeForStringAndMode(const char *string, v6502_address_mode mode);
/** @brief Returns the v6502_address_mode for a given instruction string by analyzing the operands */
v6502_address_mode as6502_addressModeForLine(const char *string, size_t len);
/** @brief Returns the string representation of an v6502_address_mode */
void as6502_stringForAddressMode(char *out, v6502_address_mode mode);
/**@}*/

/** @defgroup parser_line Line Based Assembly Parsing */
/**@{*/
/** @brief Returns the numeric value of a literal regardless of base */
uint16_t as6502_valueForString(int *wide, const char *string);
/** @brief Determines the numeric value of a literal separated into its high and low bytes */
void as6502_byteValuesForString(uint8_t *high, uint8_t *low, int *wide, const char *string);
/** @brief Completely parses a line of text to extract the instruction and v6502_address_mode */
void as6502_instructionForLine(uint8_t *opcode, uint8_t *low, uint8_t *high, v6502_address_mode *mode, const char *line, size_t len);
/** @brief Returns the byte-length of a given v6502_address_mode */
int as6502_instructionLengthForAddressMode(v6502_address_mode mode);
/** @brief Executes a symbol-free line of assembly on a specified v6502_cpu */
void as6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len);
/**@}*/

/** @defgroup parser_help Parsing assistance */
/**@{*/
/** @brief Tests a single character for the possibility of being a hex/oct/dec digit */
int as6502_isDigit(char c);
/** @brief Determines whether or not a token is a number literal */
int as6502_isNumber(const char *c);
/**@}*/

#endif
