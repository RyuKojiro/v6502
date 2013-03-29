//
//  core.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_core_h
#define v6502_core_h

#include "cpu.h"

// Debugging
void v6502_fault(const char *error);
void v6502_printCpuState(v6502_cpu *cpu);
void v6502_printMemoryRange(v6502_memory *memory, uint16_t start, uint16_t len);

// Instruction Transliteration
const char *v6502_stringForInstruction(uint16_t instruction);
uint16_t v6502_instructionForString(const char *string);
v6502_opcode v6502_opcodeForString(const char *string);

#endif
