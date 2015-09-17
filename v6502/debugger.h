//
//  debugger.h
//  v6502
//
//  Created by Daniel Loffgren on 9/17/15.
//  Copyright (c) 2015 Hello-Channel, LLC. All rights reserved.
//

#ifndef __v6502__debugger__
#define __v6502__debugger__

#include <v6502/cpu.h>

void v6502_loadFileAtAddress(v6502_memory *mem, const char *fname, uint16_t address);
int v6502_handleDebuggerCommand(v6502_cpu *cpu, char *command, size_t len, v6502_breakpoint_list *breakpoint_list);

#endif /* defined(__v6502__debugger__) */
