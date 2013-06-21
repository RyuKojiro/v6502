//
//  codegen.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/21.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include "codegen.h"
#include "parser.h"
#include "linectl.h"

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

int as6502_resolveVariableDeclaration(as6502_symbol_table *table, void *context, as6502_lineCallback cb, const char *line, size_t len) {
	/* This will take 1 line in and output 4 lines
	 * e.g.	 IN: var1 = $ff
	 *		OUT:	pha;
	 *				lda #$ff;
	 *				sta var1;
	 *				pla;
	 */
	uint8_t initialValue;
	char buf[9];
	uint8_t address;
	
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
	
	address = as6502_addressForVar(table, line);
	
	cb(context, "pha", 4);
	
	snprintf(buf, 9, "lda #$%02x", initialValue);
	cb(context, buf, 9);
	
	snprintf(buf, 9, "sta $%04x", address);
	cb(context, buf, 9);

	cb(context, "pla", 4);

	return YES;
}
