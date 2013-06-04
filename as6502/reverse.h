//
//  reverse.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/06/04.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_reverse_h
#define v6502_reverse_h

#include <stdio.h>

#include "cpu.h"

void as6502_stringForInstruction(char *string, size_t len, v6502_opcode opcode, uint8_t high, uint8_t low);

#endif
