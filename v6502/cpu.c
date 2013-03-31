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

void v6502_reset(v6502_cpu *cpu) {
	cpu->pc = 0x0000;
	cpu->ac = 0x00;
	cpu->x  = 0x00;
	cpu->y  = 0x00;
	cpu->sr = 0x20;
	cpu->sp = 0xFF;
}

void v6502_step(v6502_cpu *cpu) {
	v6502_execute(cpu, cpu->memory->bytes[cpu->pc], cpu->memory->bytes[cpu->pc + 1], cpu->memory->bytes[cpu->pc + 2], cpu->memory->bytes[cpu->pc + 3]);
}

void v6502_execute(v6502_cpu *cpu, uint8_t opcode, uint8_t operand1, uint8_t operand2, uint8_t operand3) {	
	switch (opcode) {
		case v6502_opcode_brk: {
			cpu->sp -= 3;
			cpu->sr |= v6502_cpu_status_break;
			cpu->sr |= v6502_cpu_status_interrupt;
		} return;
		case v6502_opcode_jmp_abs: {
			cpu->pc = operand1;
		} return;
		case v6502_opcode_jmp_ind: {
			cpu->pc = cpu->memory->bytes[operand1];
		} return;			
		case v6502_opcode_ora_x: {
			cpu->ac |= cpu->x;
		} return;
		case v6502_opcode_ora_val: {
			cpu->ac |= operand1;
		} return;
		case v6502_opcode_ora_zpg: {
			cpu->ac |= cpu->memory->bytes[operand1];
		} return;
		case v6502_opcode_nop: {
			cpu->pc++;
		} return;
		default: {
			v6502_fault("Unhandled CPU Instruction");
		} return;
	}
}