//
//  cpu.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdlib.h>

#include "cpu.h"
#include "core.h"

v6502_cpu *v6502_createCPU(void) {
	// Allocate CPU Struct
	v6502_cpu *cpu = malloc(sizeof(v6502_cpu));
	if (!cpu) {
		v6502_fault("CPU Allocation - Internal Structure");
		return NULL;
	}
	return cpu;
}

void v6502_destroyCPU(v6502_cpu *cpu) {
	free(cpu);
}

void v6502_step(v6502_cpu *cpu) {
	v6502_execute(cpu, *(uint16_t *)cpu->memory + cpu->pc);
	cpu->pc++;
}

void v6502_execute(v6502_cpu *cpu, uint16_t instruction) {
	v6502_opcode opcode = instruction >> 8;
	//uint8_t operand = instruction & 0xFF;
	
	switch (opcode) {
		case v6502_opcode_brk: {
			cpu->pc+=2;
			cpu->sr++;
		} return;
		case v6502_opcode_ora_x: {
			cpu->ac |= *(uint8_t *)(cpu->memory + cpu->x);
		} return;
		case v6502_opcode_nop:
			return;
		default: {
			v6502_fault("Unhandled CPU Instruction");
		} return;
	}
}