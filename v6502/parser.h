//
//  parser.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/30.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_parser_h
#define v6502_parser_h

#include "cpu.h"

void v6502_executeAsmLineOnCPU(const char *line, v6502_cpu *cpu);

#endif
