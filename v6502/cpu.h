//
//  cpu.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_cpu_h
#define v6502_cpu_h

#include <stdint.h>

#include "mem.h"

typedef struct {
	uint16_t pc;	// program counter
	uint8_t ac;		// accumulator
	uint8_t x;		// X register
	uint8_t y;		// Y register
	uint8_t sr;		// Status register
	uint8_t sp;		// Stack Pointer
	
	v6502_memory *memory;
} v6502_cpu;

// CPU object lifecycle
v6502_cpu *v6502_createCPU(void);
void v6502_destroyCPU(v6502_cpu *cpu);

// Execution
void v6502_execute(v6502_cpu *cpu, uint16_t instruction);
void v6502_step(v6502_cpu *cpu);

// Instruction Transliteration
const char *v6502_stringForInstruction(uint16_t instruction);
uint16_t v6502_instructionForString(const char *string);

// Instruction Set
typedef enum {
	v6502_opcode_brk = 0x0000,
	//<#enumerator2#> = <#value2#>
} v6502_opcode;

#endif
