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
#include <assert.h>

static size_t _lengthOfValue(const char *start) {
	size_t i;
	for (i = 0; start[i]; i++) {
		if (!as6502_isDigit(start[i])) {
			return i;
		}
	}
	return i;
}

void as6502_resolveArithmetic(char *line, size_t len, uint16_t offset) {
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
		/* This works by first reversing from the operator to the the first
		 * whitespace found in reverse, then reversing over that until the
		 */
		start = rev_strnpc(line, cur - 1);
		start = rev_strnspc(line, start) + 1;
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
		strncpy(clauseString, start, clause);
		clauseString[clause] = '\0';
		// If the instruction is a branch instruction, swap in a relative address
		if (as6502_isBranchInstruction(line)) {
			snprintf(resultString, 4, "$%02x", (uint8_t)result - (uint8_t)offset); // @todo FIXME: Is this doing relative properly?
		}
		else {
			if (result <= 0xff) {
				snprintf(resultString, 6, "*$%02x", result);
			}
			else {
				snprintf(resultString, 7, "$%04x", result);
			}
		}
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
	uint8_t low;
	uint8_t high;
	int wide;
	
	if (!strnchr(line, '=', len)) {
		// No assignments on the line
		return NO;
	}
	
	const char *cur = rev_strnspc(line, line + len - 1);
	if (!cur) {
		// Couldn't find a space
		return NO;
	}
	
	// TODO: A sanity check here to make sure our label lines up with where we are appending blob data
	//if(as6502_addressForLabel(table, <#const char *name#>) == blob->len) {
	
	as6502_byteValuesForString(&high, &low, &wide, cur + 1);
	
	ld6502_appendByteToBlob(blob, low);
	if (wide) {
		ld6502_appendByteToBlob(blob, high);
	}
	
	return YES;
}

void as6502_processObjectDirectiveForLine(ld6502_object *obj, int *currentBlob, const char *line, size_t len) {
	assert(obj);
	
	if (len <= 3) {
		return;
	}
	
	if (!strncasecmp(line + 1, "data", 3)) {
		// start new blob
		ld6502_addBlobToObject(obj, v6502_memoryStartProgram);
		*currentBlob = obj->count - 1;
	}
	else if (!strncasecmp(line + 1, "org", 3)) {
		// start new blob
		ld6502_addBlobToObject(obj, as6502_valueForString(NULL, line + 5));
		*currentBlob = obj->count - 1;
	}
	else if (!strncasecmp(line + 1, "end", 3)) {
		// revert to top blob
		*currentBlob = 0;
	}
	else {
		as6502_warn(0, strnspc(line, len) - line, "Unknown assembler directive");
	}
}
