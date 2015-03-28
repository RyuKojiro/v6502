/** @brief Assembly language parsing */
/** @file parser.h */

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

#ifndef v6502_parser_h
#define v6502_parser_h

#include <string.h>

#include "cpu.h"
#include "token.h"

/** @defgroup parser_translit Instruction Transliteration */
/**@{*/
/** @brief Returns the v6502_opcode for a given instruction string at a specified v6502_address_mode */
v6502_opcode as6502_opcodeForStringAndMode(const char *string, v6502_address_mode mode);
/** @brief Returns the v6502_address_mode for a given (already lexed) expression by analyzing the operands */
v6502_address_mode as6502_addressModeForExpression(as6502_token *head);
/** @brief Returns the instruction length for a given v6502_address_mode */
int as6502_instructionLengthForAddressMode(v6502_address_mode mode);
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
void as6502_instructionForExpression(uint8_t *opcode, uint8_t *low, uint8_t *high, v6502_address_mode *mode, as6502_token *head);
/** @brief Returns the byte-length of a given v6502_address_mode */
int as6502_instructionLengthForAddressMode(v6502_address_mode mode);
/** @brief Executes a symbol-free line of assembly on a specified v6502_cpu */
void as6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len);
/**@}*/

/** @defgroup parser_help Parsing Assistance */
/**@{*/
/** @brief Return YES if a given string reflects a branching instruction */
int as6502_isBranchInstruction(const char *string);
/** @brief Tests a single character for the possibility of being a hex/oct/dec digit */
int as6502_isDigit(char c);
/** @brief Determines whether or not a token is a number literal */
int as6502_isNumber(const char *c);
/**@}*/

#endif
