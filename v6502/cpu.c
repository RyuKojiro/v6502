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
#define FLAG_CARRY_WITH_HIGH_BIT(a)					cpu->sr &= (0xFE | (a >> 7)); \
													cpu->sr |= a >> 7;
#define FLAG_CARRY_WITH_LOW_BIT(a)					cpu->sr &= 0xFE | a; \
													cpu->sr |= a & 0x01;
// ???: Should this clear the zero flag when arithmetic results in non-zero?
#define FLAG_ZERO_WITH_RESULT(a)					cpu->sr |= (a ? 0 : v6502_cpu_status_zero);
#define FLAG_NEGATIVE_WITH_RESULT(a)				cpu->sr |= ((a & 0x80) ? v6502_cpu_status_negative : 0);

static void _executeInPlaceASL(v6502_cpu *cpu, uint8_t *operand) {
	FLAG_CARRY_WITH_HIGH_BIT(*operand);
	*operand <<= 1;
	FLAG_NEGATIVE_WITH_RESULT(*operand);
	FLAG_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceLSR(v6502_cpu *cpu, uint8_t *operand) {
	FLAG_CARRY_WITH_LOW_BIT(*operand);
	*operand <<= 1;
	FLAG_ZERO_WITH_RESULT(*operand);
}

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
	switch ((v6502_opcode)opcode) {
		// Single Byte Instructions
		case v6502_opcode_brk: {
			cpu->sp -= 3;
			cpu->sr |= v6502_cpu_status_break;
			cpu->sr |= v6502_cpu_status_interrupt;
		} return;
		case v6502_opcode_nop: {
			cpu->pc++;
		} return;
		case v6502_opcode_clc: {
			cpu->sr &= ~v6502_cpu_status_carry;
		} return;
		case v6502_opcode_cld: {
			cpu->sr &= ~v6502_cpu_status_decimal;
		} return;
		case v6502_opcode_cli: {
			cpu->sr &= ~v6502_cpu_status_interrupt;
		} return;
		case v6502_opcode_clv: {
			cpu->sr &= ~v6502_cpu_status_overflow;
		} return;
		case v6502_opcode_sec: {
			cpu->sr |= v6502_cpu_status_carry;
		} return;
		case v6502_opcode_sed: {
			cpu->sr |= v6502_cpu_status_decimal;
		} return;
		case v6502_opcode_sei: {
			cpu->sr |= v6502_cpu_status_interrupt;
		} return;
		case v6502_opcode_dex: {
			cpu->x--;
		} return;
		case v6502_opcode_dey: {
			cpu->y--;
		} return;
		case v6502_opcode_tax: {
			cpu->x = cpu->ac;
		} return;
		case v6502_opcode_tay: {
			cpu->y = cpu->ac;
		} return;
		case v6502_opcode_tsx: {
			cpu->x = cpu->sp;
		} return;
		case v6502_opcode_txa: {
			cpu->ac = cpu->x;
		} return;
		case v6502_opcode_txs: {
			cpu->sp = cpu->x;
		} return;
		case v6502_opcode_tya: {
			cpu->ac = cpu->y;
		} return;
		case v6502_opcode_inx: {
			cpu->x++;
		} return;
		case v6502_opcode_iny: {
			cpu->y++;
		} return;

		// AND
		case v6502_opcode_and_imm: {
			
			cpu->ac &= low;
		} return;
		case v6502_opcode_and_zpg: {
			cpu->ac &= cpu->memory->bytes[low];
		} return;
		case v6502_opcode_and_zpgx: {
			cpu->ac &= cpu->memory->bytes[low + cpu->x];
		} return;
		case v6502_opcode_and_abs: {
			cpu->ac &= cpu->memory->bytes[low];
		} return;
		case v6502_opcode_and_absx: {
			cpu->ac &= cpu->memory->bytes[BOTH_BYTES + cpu->x];
		} return;
		case v6502_opcode_and_absy: {
			cpu->ac &= cpu->memory->bytes[BOTH_BYTES + cpu->y];
		} return;
		case v6502_opcode_and_indx: {
			cpu->ac &= cpu->memory->bytes[cpu->memory->bytes[BOTH_BYTES] + cpu->x];
		} return;
		case v6502_opcode_and_indy: {
			cpu->ac &= cpu->memory->bytes[BOTH_BYTES + cpu->y];
		} return;
		
		// ASL
		case v6502_opcode_asl_acc: {
			_executeInPlaceASL(cpu, &cpu->ac);
		} return;
		case v6502_opcode_asl_zpg: {
			_executeInPlaceASL(cpu, &cpu->memory->bytes[low]);
		} return;
		case v6502_opcode_asl_zpgx: {
			_executeInPlaceASL(cpu, &cpu->memory->bytes[low + cpu->x]);
		} return;
		case v6502_opcode_asl_abs: {
			_executeInPlaceASL(cpu, &cpu->memory->bytes[BOTH_BYTES]);
		} return;
		case v6502_opcode_asl_absx: {
			_executeInPlaceASL(cpu, &cpu->memory->bytes[BOTH_BYTES + cpu->x]);
		} return;

		// EOR
		case v6502_opcode_eor_imm: {
			cpu->ac ^= low;
		} return;
		case v6502_opcode_eor_zpg: {
			cpu->ac ^= cpu->memory->bytes[low];
		} return;
		case v6502_opcode_eor_zpgx: {
			cpu->ac ^= cpu->memory->bytes[low + cpu->x];
		} return;
		case v6502_opcode_eor_abs: {
			cpu->ac ^= cpu->memory->bytes[low];
		} return;
		case v6502_opcode_eor_absx: {
			cpu->ac ^= cpu->memory->bytes[BOTH_BYTES + cpu->x];
		} return;
		case v6502_opcode_eor_absy: {
			cpu->ac ^= cpu->memory->bytes[BOTH_BYTES + cpu->y];
		} return;
		case v6502_opcode_eor_indx: {
			cpu->ac ^= cpu->memory->bytes[cpu->memory->bytes[BOTH_BYTES] + cpu->x];
		} return;
		case v6502_opcode_eor_indy: {
			cpu->ac ^= cpu->memory->bytes[BOTH_BYTES + cpu->y];
		} return;
			
		// JMP
		case v6502_opcode_jmp_abs: {
			cpu->pc = BOTH_BYTES;
		} return;
		case v6502_opcode_jmp_ind: {
			cpu->pc = cpu->memory->bytes[low];
		} return;
		
		// ORA
		case v6502_opcode_ora_imm: {
			cpu->ac |= low;
		} return;
		case v6502_opcode_ora_zpg: {
			cpu->ac |= cpu->memory->bytes[low];
		} return;
		case v6502_opcode_ora_zpgx: {
			cpu->ac |= cpu->memory->bytes[low + cpu->x];
		} return;
		case v6502_opcode_ora_abs: {
			cpu->ac |= cpu->memory->bytes[low];
		} return;
		case v6502_opcode_ora_absx: {
			cpu->ac |= cpu->memory->bytes[BOTH_BYTES + cpu->x];
		} return;
		case v6502_opcode_ora_absy: {
			cpu->ac |= cpu->memory->bytes[BOTH_BYTES + cpu->y];
		} return;
		case v6502_opcode_ora_indx: {
			cpu->ac |= cpu->memory->bytes[cpu->memory->bytes[BOTH_BYTES] + cpu->x];
		} return;
		case v6502_opcode_ora_indy: {
			cpu->ac |= cpu->memory->bytes[BOTH_BYTES + cpu->y];
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
		
		// LSR
		case v6502_opcode_lsr_acc: {
			FLAG_CARRY_WITH_LOW_BIT(cpu->ac);
			cpu->ac >>= 1;
		} return;
		case v6502_opcode_lsr_zpg: {
			FLAG_CARRY_WITH_LOW_BIT(cpu->memory->bytes[low]);
			cpu->memory->bytes[low] >>= 1;
		} return;
		case v6502_opcode_lsr_zpgx: {
			FLAG_CARRY_WITH_LOW_BIT(cpu->memory->bytes[low + cpu->x]);
			cpu->memory->bytes[low + cpu->x] >>= 1;
		} return;
		case v6502_opcode_lsr_abs: {
			FLAG_CARRY_WITH_LOW_BIT(cpu->memory->bytes[BOTH_BYTES]);
			cpu->memory->bytes[BOTH_BYTES] >>= 1;
		} return;
		case v6502_opcode_lsr_absx: {
			FLAG_CARRY_WITH_LOW_BIT(cpu->memory->bytes[BOTH_BYTES + cpu->x]);
			cpu->memory->bytes[BOTH_BYTES + cpu->x] >>= 1;
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