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

static size_t _lengthOfValue(char *start) {
	
}

void as6502_resolveArithmetic(char *line, size_t len) {
	char *cur, start;
	size_t clause = 0;
	uint16_t left, right;
	uint8_t high, low;
	
	// Check for addition
	cur = memchr(line, '+', len);
	if (cur) {
		v6502_valueForString(&high, &low, NULL, cur + 1);
		right = (high << 8) | low;
		
		// FIXME: This currently relies on the first space found in reverse
		// being separate from the arithmetic clause, and a delimeter for the
		// left hand value. Not sure if this is proper if whitespace is allowed
		// in between operators and values.
		cur = rev_strnchr(line, cur, ' ');
		v6502_valueForString(&high, &low, NULL, cur + 1);
		left = (high << 8) | low;
		
		// Solve
		left += right;
	}
	
	
	// Put resolved value in
	if (clause) {
		//as6502
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
