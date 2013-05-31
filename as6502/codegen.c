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
#include "symbols.h"

#include <string.h>
#include <stdlib.h>

static size_t _lengthOfValue(char *start) {
	size_t i;
	for (i = 0; start[i]; i++) {
		if (!as6502_isDigit(start[i])) {
			return i;
		}
	}
	return i;
}

void as6502_resolveArithmetic(char *line, size_t len) {
	char *cur, *start;
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
		// FIXME: This currently relies on the first space found in reverse
		// being separate from the arithmetic clause, and a delimeter for the
		// left hand value. Not sure if this is proper if whitespace is allowed
		// in between operators and values.
		start = rev_strnspc(line, cur) + 1;
		left = as6502_valueForString(NULL, start);
		
		// Solve
		result = left + right;
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

int as6502_resolveVariableDeclaration(char *line, size_t len) {
	// This will take 1 line in and output 4 lines
	// e.g.	 IN: var1 = $ff
	//		OUT:	pha
	//				lda #$ff
	//				sta var1
	//				pla
	
	return NO;
}
