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

#include "codegen.h"
#include "parser.h"
#include "linectl.h"
#include "error.h"

#include <string.h>
#include <stdlib.h>

static size_t _lengthOfValue(const char *start) {
	size_t i;
	for (i = 0; start[i]; i++) {
		if (!as6502_isDigit(start[i])) {
			return i;
		}
	}
	return i;
}

void as6502_resolveArithmetic(char *line, size_t len) {
	const char *cur;
	char *start;
	size_t clause = 0;
	uint16_t left, right, result;
	char resultString[7];
	char *clauseString;
	
	// Check for addition
	cur = strnchr(line, '+', len);
	if (cur) {
		// Get right hand side
		cur++;
		right = as6502_valueForString(NULL, cur);
		
		// Get left hand side
		/** FIXME: @bug This currently relies on the first space found in reverse
		 * being separate from the arithmetic clause, and a delimeter for the
		 * left hand value. Not sure if this is proper if whitespace is allowed
		 * in between operators and values. 
		 */
		start = rev_strnspc(line, cur) + 1;
		left = as6502_valueForString(NULL, start);
		
		// Solve
		result = left + right;
		clause = (cur + _lengthOfValue(cur)) - start;
	}
	
	// Check for subtraction
	cur = strnchr(line, '-', len);
	if (cur) {
		// Get right hand side
		cur++;
		right = as6502_valueForString(NULL, cur);
		
		// Get left hand side
		start = rev_strnspc(line, cur) + 1;
		left = as6502_valueForString(NULL, start);
		
		// Solve
		result = left - right;
		clause = (cur + _lengthOfValue(cur)) - start;
	}
	
	// Put resolved value in
	if (clause) {
		clauseString = malloc(clause + 1);
		snprintf(resultString, 7, "$%04x", result);
		as6502_replaceSymbolInLineAtLocationWithText(line, len, start, clauseString, resultString);
		free(clauseString);
	}
}

// NOTE: We only support single byte declarations
int as6502_resolveVariableDeclaration(ld6502_object_blob *blob, as6502_symbol_table *table, const char *line, size_t len) {
	/* This will take 1 line in and output 4 lines
	 * e.g.	 IN: var1 = $ff
	 *		OUT:	pha;
	 *				lda #$ff;
	 *				sta var1;
	 *				pla;
	 */
	uint8_t initialValue;

	if (!strnchr(line, '=', len)) {
		// No assignments on the line
		return NO;
	}
	
	const char *cur = rev_strnspc(line, line + len - 1);
	if (!cur) {
		// Couldn't find a space
		return NO;
	}
	
	as6502_byteValuesForString(NULL, &initialValue, NULL, cur + 1);
	
	as6502_addSymbolToTable(table, currentLineNum, trimhead(line, len), blob->len, as6502_symbol_type_variable);
	ld6502_appendByteToBlob(blob, initialValue);
	
	return YES;
}
