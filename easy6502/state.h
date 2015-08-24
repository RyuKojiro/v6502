//
//  state.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/06/03.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_state_h
#define v6502_state_h

#include "mem.h"

void initState();
void stateCycle(v6502_memory *mem);

#endif
