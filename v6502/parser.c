//
//  parser.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/30.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include "parser.h"
#include "core.h"

void v6502_executeAsmLineOnCPU(const char *line, v6502_cpu *cpu) {
	v6502_execute(cpu, v6502_instructionForString(line));
}
