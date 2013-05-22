//
//  codegen.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/21.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_codegen_h
#define v6502_codegen_h

#include <stdio.h>

void as6502_resolveArithmetic(char *line, size_t len);
void as6502_resolveVariableDeclaration(char *line, size_t len);

#endif
