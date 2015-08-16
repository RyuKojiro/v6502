/** @brief Assembly language generation, optimization, and manipulation */
/** @file codegen.h */

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

#ifndef v6502_codegen_h
#define v6502_codegen_h

#include <stdio.h>

#include "object.h"
#include "symbols.h"

/** @brief A line-based callback for code generation functions that emit multiple lines of code */
typedef uint16_t (* as6502_lineCallback)(ld6502_object_blob *blob, const char *line, size_t len);

/** @defgroup codegen Code Generation Functions */
/**@{*/
/** @brief Resolves arithmetical operations down to their literal result */
as6502_token *as6502_resolveArithmeticInExpression(as6502_token *head);
/** @brief Replaces variable declarations with the instructions needed to initialize a variable without altering the current running state of the CPU */
int as6502_resolveVariableDeclaration(ld6502_object_blob *blob, as6502_symbol_table *table, const char *line, size_t len);
/** @brief Automatically processes any dot directive in a given line and updates an ld6502_object_context */
void as6502_processObjectDirectiveForLine(ld6502_object *obj, int *currentBlob, const char *line, size_t len);
/**@}*/

#endif
