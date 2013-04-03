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

#define	BOTH_BYTES	(high << 8 | low)

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
	v6502_execute(cpu, cpu->memory->bytes[cpu->pc], cpu->memory->bytes[cpu->pc + 1], cpu->memory->bytes[cpu->pc + 2]);
}

void v6502_execute(v6502_cpu *cpu, uint8_t opcode, uint8_t low, uint8_t high) {
	switch (opcode) {
		// Single Byte Instructions
		case v6502_opcode_brk: {
			cpu->sp -= 3;
			cpu->sr |= v6502_cpu_status_break;
			cpu->sr |= v6502_cpu_status_interrupt;
		} return;
		case v6502_opcode_nop: {
			cpu->pc++;
		} return;
		case v6502_opcode_cld: {
			cpu->sr &= ~v6502_cpu_status_decimal;
		} return;
		case v6502_opcode_sed: {
			cpu->sr |= v6502_cpu_status_decimal;
		} return;

		// JMP
		case v6502_opcode_jmp_abs: {
			cpu->pc = BOTH_BYTES;
		} return;
		case v6502_opcode_jmp_ind: {
			cpu->pc = cpu->memory->bytes[low];
		} return;
		
		// ORA
		case v6502_opcode_ora_absx: {
			cpu->ac |= BOTH_BYTES + cpu->x;
		} return;
		case v6502_opcode_ora_imm: {
			cpu->ac |= low;
		} return;
		case v6502_opcode_ora_zpg: {
			cpu->ac |= cpu->memory->bytes[low];
		} return;
			
		// LDA
		case v6502_opcode_lda_imm: {
			cpu->ac = low;
		}
		case v6502_opcode_lda_zpg: {
			cpu->ac = cpu->memory->bytes[low];
		} return;
		case v6502_opcode_lda_zpgx: {
			cpu->ac = cpu->memory->bytes[low + cpu->x];
		} return;
		case v6502_opcode_lda_abs: {
			cpu->ac = cpu->memory->bytes[BOTH_BYTES];
		} return;
		case v6502_opcode_lda_absx: {
			cpu->ac = cpu->memory->bytes[BOTH_BYTES + cpu->x];
		} return;
		case v6502_opcode_lda_absy: {
			cpu->ac = cpu->memory->bytes[BOTH_BYTES + cpu->y];
		} return;
		case v6502_opcode_lda_indx: { // ???: Not sure about these indirect operations
			cpu->ac = cpu->memory->bytes[cpu->memory->bytes[BOTH_BYTES] + cpu->x];
		} return;
		case v6502_opcode_lda_indy: {
			cpu->ac = cpu->memory->bytes[cpu->memory->bytes[BOTH_BYTES + cpu->y]];
		} return;
		
		// LDX
		case v6502_opcode_ldx_imm: {
			cpu->x = low;
		}
		case v6502_opcode_ldx_zpg: {
			cpu->x = cpu->memory->bytes[low];
		} return;
		case v6502_opcode_ldx_zpgy: {
			cpu->x = cpu->memory->bytes[low + cpu->y];
		} return;
		case v6502_opcode_ldx_abs: {
			cpu->x = cpu->memory->bytes[BOTH_BYTES];
		} return;
		case v6502_opcode_ldx_absy: {
			cpu->x = cpu->memory->bytes[BOTH_BYTES + cpu->y];
		} return;

		// LDY
		case v6502_opcode_ldy_imm: {
			cpu->y = low;
		}
		case v6502_opcode_ldy_zpg: {
			cpu->y = cpu->memory->bytes[low];
		} return;
		case v6502_opcode_ldy_zpgx: {
			cpu->y = cpu->memory->bytes[low + cpu->x];
		} return;
		case v6502_opcode_ldy_abs: {
			cpu->y = cpu->memory->bytes[BOTH_BYTES];
		} return;
		case v6502_opcode_ldy_absx: {
			cpu->y = cpu->memory->bytes[BOTH_BYTES + cpu->x];
		} return;

		
		// STA
		case v6502_opcode_sta_zpg: {
			cpu->memory->bytes[low] = cpu->ac;
		} return;
		case v6502_opcode_sta_zpgx: {
			cpu->memory->bytes[low + cpu->x] = cpu->ac;
		} return;
		case v6502_opcode_sta_abs: {
			cpu->memory->bytes[BOTH_BYTES] = cpu->ac;
		} return;
		case v6502_opcode_sta_absx: {
			cpu->memory->bytes[BOTH_BYTES + cpu->x] = cpu->ac;
		} return;
		case v6502_opcode_sta_absy: {
			cpu->memory->bytes[BOTH_BYTES + cpu->y] = cpu->ac;
		} return;
		case v6502_opcode_sta_indx: { // ???: Not sure about these indirect operations
			cpu->memory->bytes[cpu->memory->bytes[BOTH_BYTES] + cpu->x] = cpu->ac;
		} return;
		case v6502_opcode_sta_indy: {
			cpu->memory->bytes[cpu->memory->bytes[BOTH_BYTES + cpu->y]] = cpu->ac;
		} return;
			
		// STX
		case v6502_opcode_stx_zpg: {
			cpu->memory->bytes[low] = cpu->x;
		} return;
		case v6502_opcode_stx_zpgy: {
			cpu->memory->bytes[low + cpu->y] = cpu->x;
		} return;
		case v6502_opcode_stx_abs: {
			cpu->memory->bytes[BOTH_BYTES] = cpu->x;
		} return;
			
		// STY
		case v6502_opcode_sty_zpg: {
			cpu->memory->bytes[low] = cpu->y;
		} return;
		case v6502_opcode_sty_zpgx: {
			cpu->memory->bytes[low + cpu->x] = cpu->y;
		} return;
		case v6502_opcode_sty_abs: {
			cpu->memory->bytes[BOTH_BYTES] = cpu->y;
		} return;
			
		// Failure
		default: {
			v6502_fault("Unhandled CPU Instruction");
		} return;
	}
}