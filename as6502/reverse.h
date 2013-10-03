/** @brief Instruction Disassembly */
/** @file reverse.h */

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

#ifndef v6502_reverse_h
#define v6502_reverse_h

#include <stdio.h>

#include "cpu.h"
#include "parser.h"

/** @defgroup rev Instruction Disassembly */
/**@{*/
/** @brief Get the string representation of an opcode */
void as6502_stringForOpcode(char *string, size_t len, v6502_opcode opcode);
/** @brief Get the string representation of the operands in a given address mode */
void as6502_stringForOperand(char *string, size_t len, v6502_address_mode opcode, uint8_t high, uint8_t low);
/** @brief Get the complete string representation of an instruction line */
void as6502_stringForInstruction(char *string, size_t len, v6502_opcode opcode, uint8_t high, uint8_t low);
/**@}*/

#endif
