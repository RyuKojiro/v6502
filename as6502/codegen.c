//
//  codegen.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/21.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include "codegen.h"

void as6502_resolveArithmetic(char *line, size_t len) {
	
}

void as6502_resolveVariableDeclaration(char *line, size_t len) {
	// This will take 1 line in and output 4 lines
	// e.g.	 IN: var1 = $ff
	//		OUT:	pha
	//				lda #$ff
	//				sta var1
	//				pla
}
