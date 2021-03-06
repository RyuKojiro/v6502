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

#include <v6502/cpu.h>
#include <as6502/parser.h>
#include <ld6502/object.h>

/** @defgroup rev Instruction Disassembly */
/**@{*/

/** @brief Return YES if a given v6502_opcode reflects a branching instruction */
int dis6502_isBranchOpcode(v6502_opcode opcode);

/** @brief Get the string representation of an opcode */
void dis6502_stringForOpcode(char *string, size_t len, v6502_opcode opcode);

/** @brief Get the string representation of the operands in a given address mode */
void dis6502_stringForOperand(char *string, size_t len, v6502_address_mode opcode, uint8_t byte2, uint8_t byte3);

/** @brief Get the complete string representation of an instruction line */
void dis6502_stringForInstruction(char *string, size_t len, v6502_opcode opcode, uint8_t byte2, uint8_t byte3);

/** @brief Create symbols in a as6502_symbol_table for all branch instructions in a given ld6502_object_blob that do not already have corresponding entries. */
void dis6502_deriveSymbolsForObjectBlob(as6502_symbol_table *table, ld6502_object_blob *blob);

/** @brief Create symbols for all branch instructions that do not already have corresponding entries for all ld6502_object_blobs in a given ld6502_object. */
void dis6502_deriveSymbolsForObject(ld6502_object *obj);

/** @brief Print, to file pointer, an address and byte annotated disassembled line of assembly at the given address on a given cpu. */
int dis6502_printAnnotatedInstruction(FILE *out, v6502_cpu *cpu, uint16_t address, as6502_symbol_table *table);

/**@}*/

#endif
