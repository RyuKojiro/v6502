/** @brief Assembler introspection */
/** @file debug.h */

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

#ifndef as6502_debug_h
#define as6502_debug_h

#include <stdio.h>
#include <stdint.h>
#include <v6502/cpu.h>

/** @defgroup asm_debug Assembler Debug Printing Functions */
/**@{*/

/**
 * @brief Prints a label annotated with address (and optionally source line
 * number). This function is designed to be used in conjunction with
 * as6502_printAnnotatedInstruction for neatly printed assembly.
 */
void as6502_printAnnotatedLabel(FILE *out, uint16_t address, const char *text, unsigned long line);

/** @brief Prints a line of assembly annotated with address. */
void as6502_printAnnotatedInstruction(FILE *out, uint16_t address, v6502_opcode opcode, uint8_t low, uint8_t high, const char *text);

/**@}*/

#endif /* defined(as6502_debug_h) */
