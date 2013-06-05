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

#define	BOTH_BYTES									(high << 8 | low)
#define FLAG_CARRY_WITH_HIGH_BIT(a)					cpu->sr &= (~v6502_cpu_status_carry | (a >> 7)); \
													cpu->sr |= a >> 7;
#define FLAG_CARRY_WITH_LOW_BIT(a)					cpu->sr &= ~v6502_cpu_status_carry | a; \
													cpu->sr |= v6502_cpu_status_carry & a;
#define FLAG_ZERO_WITH_RESULT(a)					cpu->sr &= (a ? ~v6502_cpu_status_zero : ~0); \
													cpu->sr |= (a ? 0 : v6502_cpu_status_zero);
#define FLAG_NEGATIVE_WITH_RESULT(a)				cpu->sr |= ((a & 0x80) ? v6502_cpu_status_negative : 0);
// FIXME: (a > 0xFF) is always true with uint8_t
#define FLAG_CARRY_WITH_EXPRESSION(a)				cpu->sr |= ((a > 0xFF) ? v6502_cpu_status_carry : 0);
#define FLAG_OVERFLOW_PREPARE(a)					uint8_t _overflowCheck = a;
#define FLAG_OVERFLOW_CHECK(a)						_overflowCheck ^= a; \
													_overflowCheck >>= 1; /* Shift to overflow flag position */ \
													cpu->sr &= (_overflowCheck | ~v6502_cpu_status_overflow); \
													cpu->sr |= (_overflowCheck & v6502_cpu_status_overflow); \

#pragma mark -
#pragma mark CPU Internal Instruction Execution

static void _executeInPlaceASL(v6502_cpu *cpu, uint8_t *operand) {
	FLAG_CARRY_WITH_HIGH_BIT(*operand);
	*operand <<= 1;
	FLAG_NEGATIVE_WITH_RESULT(*operand);
	FLAG_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceLSR(v6502_cpu *cpu, uint8_t *operand) {
	FLAG_CARRY_WITH_LOW_BIT(*operand);
	*operand >>= 1;
	FLAG_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceROL(v6502_cpu *cpu, uint8_t *operand) {
	FLAG_CARRY_WITH_HIGH_BIT(*operand);
	*operand = (*operand << 1) | (*operand >> 7);
	FLAG_NEGATIVE_WITH_RESULT(*operand);
	FLAG_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceROR(v6502_cpu *cpu, uint8_t *operand) {
	FLAG_CARRY_WITH_HIGH_BIT(*operand);
	*operand = (*operand >> 1) | (*operand << 7);
	FLAG_NEGATIVE_WITH_RESULT(*operand);
	FLAG_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceORA(v6502_cpu *cpu, uint8_t operand) {
	cpu->ac |= operand;
	FLAG_NEGATIVE_WITH_RESULT(cpu->ac);
	FLAG_ZERO_WITH_RESULT(cpu->ac);
}

static void _executeInPlaceAND(v6502_cpu *cpu, uint8_t operand) {
	cpu->ac &= operand;
	FLAG_NEGATIVE_WITH_RESULT(cpu->ac);
	FLAG_ZERO_WITH_RESULT(cpu->ac);
}

static void _executeInPlaceADC(v6502_cpu *cpu, uint8_t operand) {
	FLAG_OVERFLOW_PREPARE(cpu->ac);
	cpu->ac += operand;
	FLAG_CARRY_WITH_EXPRESSION(cpu->ac);
	FLAG_OVERFLOW_CHECK(cpu->ac);
	FLAG_NEGATIVE_WITH_RESULT(cpu->ac);
	FLAG_ZERO_WITH_RESULT(cpu->ac);
}

static void _executeInPlaceSBC(v6502_cpu *cpu, uint8_t operand) {
	FLAG_OVERFLOW_PREPARE(cpu->ac);
	cpu->ac -= operand;
	FLAG_CARRY_WITH_EXPRESSION(cpu->ac);
	FLAG_OVERFLOW_CHECK(cpu->ac);
	FLAG_NEGATIVE_WITH_RESULT(cpu->ac);
	FLAG_ZERO_WITH_RESULT(cpu->ac);
}

static void _executeInPlaceDEC(v6502_cpu *cpu, uint8_t *operand) {
	(*operand)--;
	FLAG_NEGATIVE_WITH_RESULT(*operand);
	FLAG_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceINC(v6502_cpu *cpu, uint8_t *operand) {
	(*operand)++;
	FLAG_NEGATIVE_WITH_RESULT(*operand);
	FLAG_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceCMP(v6502_cpu *cpu, uint8_t operand) {
	uint8_t result = cpu->ac - operand;
	FLAG_NEGATIVE_WITH_RESULT(result);
	FLAG_ZERO_WITH_RESULT(result);
}

static void _executeInPlaceCPY(v6502_cpu *cpu, uint8_t operand) {
	uint8_t result = cpu->y - operand;
	FLAG_NEGATIVE_WITH_RESULT(result);
	FLAG_ZERO_WITH_RESULT(result);
}

static void _executeInPlaceCPX(v6502_cpu *cpu, uint8_t operand) {
	uint8_t result = cpu->x - operand;
	FLAG_NEGATIVE_WITH_RESULT(result);
	FLAG_ZERO_WITH_RESULT(result);
}

static void _executeInPlaceBIT(v6502_cpu *cpu, uint8_t operand) {
	uint8_t result = cpu->ac & operand;
	cpu->sr &= ~v6502_cpu_status_overflow;
	cpu->sr |= (result & v6502_cpu_status_overflow);
	FLAG_NEGATIVE_WITH_RESULT(result);
	FLAG_ZERO_WITH_RESULT(result);
}

#pragma mark -
#pragma mark CPU Lifecycle

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

#pragma mark -
#pragma mark CPU Runtime

int v6502_instructionLengthForOpcode(v6502_opcode opcode) {
	if (((opcode & 0x0F) > 0x00) && ((opcode & 0x0F) < 0x07)) {
		return 2;
	}
	
	if ((opcode & 0x0F) > 0x0B) {
		return 3;
	}
	
	if (((opcode & 0x0F) == 0x09) && (opcode & 0x10)) {
		return 3;
	}
	
	if (opcode == v6502_opcode_jsr) {
		return 3;
	}
	
	if (((opcode & 0x0F) == 0x00) && ((opcode & 0x10) || ((opcode & 0xF0) > 0x80))) {
		return 2;
	}
	
	return 1;
}

v6502_address_mode v6502_addressModeForOpcode(v6502_opcode opcode) {
	switch (opcode) {
		case v6502_opcode_brk:
		case v6502_opcode_nop:
		case v6502_opcode_clc:
		case v6502_opcode_cld:
		case v6502_opcode_cli:
		case v6502_opcode_clv:
		case v6502_opcode_sec:
		case v6502_opcode_sed:
		case v6502_opcode_sei:
		case v6502_opcode_dex:
		case v6502_opcode_dey:
		case v6502_opcode_tax:
		case v6502_opcode_tay:
		case v6502_opcode_tsx:
		case v6502_opcode_txa:
		case v6502_opcode_txs:
		case v6502_opcode_tya:
		case v6502_opcode_inx:
		case v6502_opcode_iny:
			return v6502_address_mode_implied;
		case v6502_opcode_bcc:
		case v6502_opcode_bcs:
		case v6502_opcode_beq:
		case v6502_opcode_bne:
		case v6502_opcode_bmi:
		case v6502_opcode_bpl:
		case v6502_opcode_bvc:
		case v6502_opcode_bvs:
		case v6502_opcode_jsr:
		case v6502_opcode_rti:
		case v6502_opcode_rts:
		case v6502_opcode_pha:
		case v6502_opcode_pla:
		case v6502_opcode_php:
		case v6502_opcode_plp:
			return v6502_address_mode_relative;
		case v6502_opcode_asl_acc:
		case v6502_opcode_lsr_acc:
		case v6502_opcode_rol_acc:
		case v6502_opcode_ror_acc:
			return v6502_address_mode_accumulator;
		case v6502_opcode_adc_imm:
		case v6502_opcode_and_imm:
		case v6502_opcode_cmp_imm:
		case v6502_opcode_cpx_imm:
		case v6502_opcode_cpy_imm:
		case v6502_opcode_eor_imm:
		case v6502_opcode_ora_imm:
		case v6502_opcode_lda_imm:
		case v6502_opcode_ldx_imm:
		case v6502_opcode_ldy_imm:
		case v6502_opcode_sbc_imm:
			return v6502_address_mode_immediate;
		case v6502_opcode_adc_abs:
		case v6502_opcode_and_abs:
		case v6502_opcode_asl_abs:
		case v6502_opcode_bit_abs:
		case v6502_opcode_cmp_abs:
		case v6502_opcode_cpx_abs:
		case v6502_opcode_cpy_abs:
		case v6502_opcode_dec_abs:
		case v6502_opcode_eor_abs:
		case v6502_opcode_inc_abs:
		case v6502_opcode_jmp_abs:
		case v6502_opcode_ora_abs:
		case v6502_opcode_lda_abs:
		case v6502_opcode_ldx_abs:
		case v6502_opcode_ldy_abs:
		case v6502_opcode_lsr_abs:
		case v6502_opcode_rol_abs:
		case v6502_opcode_ror_abs:
		case v6502_opcode_sbc_abs:
		case v6502_opcode_sta_abs:
		case v6502_opcode_stx_abs:
		case v6502_opcode_sty_abs:
			return v6502_address_mode_absolute;
		case v6502_opcode_adc_absx:
		case v6502_opcode_and_absx:
		case v6502_opcode_asl_absx:
		case v6502_opcode_cmp_absx:
		case v6502_opcode_dec_absx:
		case v6502_opcode_eor_absx:
		case v6502_opcode_inc_absx:
		case v6502_opcode_ora_absx:
		case v6502_opcode_lda_absx:
		case v6502_opcode_ldy_absx:
		case v6502_opcode_lsr_absx:
		case v6502_opcode_rol_absx:
		case v6502_opcode_ror_absx:
		case v6502_opcode_sbc_absx:
		case v6502_opcode_sta_absx:
			return v6502_address_mode_absolute_x;
		case v6502_opcode_adc_absy:
		case v6502_opcode_and_absy:
		case v6502_opcode_cmp_absy:
		case v6502_opcode_eor_absy:
		case v6502_opcode_ora_absy:
		case v6502_opcode_lda_absy:
		case v6502_opcode_ldx_absy:
		case v6502_opcode_sbc_absy:
		case v6502_opcode_sta_absy:
			return v6502_address_mode_absolute_y;
		case v6502_opcode_jmp_ind:
			return v6502_address_mode_indirect;
		case v6502_opcode_adc_indx:
		case v6502_opcode_and_indx:
		case v6502_opcode_cmp_indx:
		case v6502_opcode_eor_indx:
		case v6502_opcode_ora_indx:
		case v6502_opcode_lda_indx:
		case v6502_opcode_sbc_indx:
		case v6502_opcode_sta_indx:
			return v6502_address_mode_indirect_x;
		case v6502_opcode_adc_indy:
		case v6502_opcode_and_indy:
		case v6502_opcode_cmp_indy:
		case v6502_opcode_eor_indy:
		case v6502_opcode_ora_indy:
		case v6502_opcode_lda_indy:
		case v6502_opcode_sbc_indy:
		case v6502_opcode_sta_indy:
			return v6502_address_mode_indirect_y;
		case v6502_opcode_adc_zpg:
		case v6502_opcode_and_zpg:
		case v6502_opcode_asl_zpg:
		case v6502_opcode_bit_zpg:
		case v6502_opcode_cmp_zpg:
		case v6502_opcode_cpx_zpg:
		case v6502_opcode_cpy_zpg:
		case v6502_opcode_dec_zpg:
		case v6502_opcode_eor_zpg:
		case v6502_opcode_inc_zpg:
		case v6502_opcode_ora_zpg:
		case v6502_opcode_lda_zpg:
		case v6502_opcode_ldx_zpg:
		case v6502_opcode_ldy_zpg:
		case v6502_opcode_lsr_zpg:
		case v6502_opcode_rol_zpg:
		case v6502_opcode_ror_zpg:
		case v6502_opcode_sbc_zpg:
		case v6502_opcode_sta_zpg:
		case v6502_opcode_stx_zpg:
		case v6502_opcode_sty_zpg:
			return v6502_address_mode_zeropage;
		case v6502_opcode_adc_zpgx:
		case v6502_opcode_and_zpgx:
		case v6502_opcode_asl_zpgx:
		case v6502_opcode_cmp_zpgx:
		case v6502_opcode_dec_zpgx:
		case v6502_opcode_eor_zpgx:
		case v6502_opcode_inc_zpgx:
		case v6502_opcode_ora_zpgx:
		case v6502_opcode_lda_zpgx:
		case v6502_opcode_ldy_zpgx:
		case v6502_opcode_lsr_zpgx:
		case v6502_opcode_rol_zpgx:
		case v6502_opcode_ror_zpgx:
		case v6502_opcode_sbc_zpgx:
		case v6502_opcode_sta_zpgx:
		case v6502_opcode_sty_zpgx:
			return v6502_address_mode_zeropage_x;
		case v6502_opcode_ldx_zpgy:
		case v6502_opcode_stx_zpgy:
			return v6502_address_mode_zeropage_y;
		default:
			return v6502_address_mode_unknown;
	}
}

void v6502_reset(v6502_cpu *cpu) {
	cpu->pc = 0x0600;
	cpu->ac = 0x00;
	cpu->x  = 0x00;
	cpu->y  = 0x00;
	cpu->sr = 0x20;
	cpu->sp = 0xFF;
}

void v6502_step(v6502_cpu *cpu) {
	v6502_execute(cpu, *v6502_map(cpu->memory, cpu->pc), *v6502_map(cpu->memory, cpu->pc + 1), *v6502_map(cpu->memory, cpu->pc + 2));
	cpu->pc += v6502_instructionLengthForOpcode(*v6502_map(cpu->memory, cpu->pc));
}

void v6502_execute(v6502_cpu *cpu, uint8_t opcode, uint8_t low, uint8_t high) {
	uint8_t *value;
	
	switch (v6502_addressModeForOpcode(opcode)) {
		case v6502_address_mode_accumulator: {
			value = &(cpu->ac);
		} break;
		case v6502_address_mode_implied: {
			value = &(cpu->ac);
		} break;
		case v6502_address_mode_immediate: {
			value = &low;
		} break;
		case v6502_address_mode_zeropage: {
			value = v6502_map(cpu->memory, low);
		} break;
		case v6502_address_mode_zeropage_x: {
			value = v6502_map(cpu->memory, low + cpu->x);
		} break;
		case v6502_address_mode_zeropage_y:
		case v6502_address_mode_relative:
		case v6502_address_mode_absolute: {
			value = v6502_map(cpu->memory, BOTH_BYTES);
		} break;
		case v6502_address_mode_absolute_x: {
			value = v6502_map(cpu->memory, BOTH_BYTES + cpu->x);
		} break;
		case v6502_address_mode_absolute_y: {
			value = v6502_map(cpu->memory, BOTH_BYTES + cpu->y);
		} break;
		case v6502_address_mode_indirect:
		case v6502_address_mode_indirect_x: {
			value = v6502_map(cpu->memory, *v6502_map(cpu->memory, BOTH_BYTES) + cpu->x);
		} break;
		case v6502_address_mode_indirect_y: {
			v6502_map(cpu->memory, BOTH_BYTES + cpu->y);
		} break;
		case v6502_address_mode_symbol:
		case v6502_address_mode_unknown:
		default:
			break;
	}
	
	switch ((v6502_opcode)opcode) {
		// Single Byte Instructions
		case v6502_opcode_brk: {
			cpu->sp -= 3;
			cpu->sr |= v6502_cpu_status_break;
			cpu->sr |= v6502_cpu_status_interrupt;
		} return;
		case v6502_opcode_nop:
			return;
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

		// Branch Instructions
		case v6502_opcode_bcc: {
			if (!(cpu->sr & v6502_cpu_status_carry)) {
				cpu->pc += v6502_signedValueOfByte(low);
			}
		} return;
		case v6502_opcode_bcs: {
			if (cpu->sr & v6502_cpu_status_carry) {
				cpu->pc += v6502_signedValueOfByte(low);
			}
		} return;
		case v6502_opcode_beq: {
			if (cpu->sr & v6502_cpu_status_zero) {
				cpu->pc += v6502_signedValueOfByte(low);
			}
		} return;
		case v6502_opcode_bne: {
			if (!(cpu->sr & v6502_cpu_status_zero)) {
				cpu->pc += v6502_signedValueOfByte(low);
			}
		} return;
		case v6502_opcode_bmi: {
			if (cpu->sr & v6502_cpu_status_negative) {
				cpu->pc += v6502_signedValueOfByte(low);
			}
		} return;
		case v6502_opcode_bpl: {
			if (!(cpu->sr & v6502_cpu_status_negative)) {
				cpu->pc += v6502_signedValueOfByte(low);
			}
		} return;
		case v6502_opcode_bvc: {
			if (!(cpu->sr & v6502_cpu_status_overflow)) {
				cpu->pc += v6502_signedValueOfByte(low);
			}
		} return;
		case v6502_opcode_bvs: {
			if (cpu->sr & v6502_cpu_status_overflow) {
				cpu->pc += v6502_signedValueOfByte(low);
			}
		} return;
		
		// Stack Instructions
		case v6502_opcode_jsr: {
			*v6502_map(cpu->memory, cpu->sp--) = cpu->pc;
			cpu->pc = BOTH_BYTES;
		} return;
		case v6502_opcode_rti: {
			// TODO: Interrupts
		} return;
		case v6502_opcode_rts: {
			cpu->pc = *v6502_map(cpu->memory, ++cpu->sp);
		} return;
		case v6502_opcode_pha: {
			*v6502_map(cpu->memory, cpu->sp--) = cpu->ac;
		} return;
		case v6502_opcode_pla: {
			cpu->ac = *v6502_map(cpu->memory, ++cpu->sp);
		} return;
		case v6502_opcode_php: {
			*v6502_map(cpu->memory, cpu->sp--) = cpu->sr;
		} return;
		case v6502_opcode_plp: {
			cpu->sr = *v6502_map(cpu->memory, ++cpu->sp);
		} return;

		// ADC
		case v6502_opcode_adc_imm:
		case v6502_opcode_adc_zpg:
		case v6502_opcode_adc_zpgx:
		case v6502_opcode_adc_abs:
		case v6502_opcode_adc_absx:
		case v6502_opcode_adc_absy:
		case v6502_opcode_adc_indx:
		case v6502_opcode_adc_indy:
			_executeInPlaceADC(cpu, *value);
			return;

		// AND
		case v6502_opcode_and_imm: {
			_executeInPlaceAND(cpu, low);
		} return;
		case v6502_opcode_and_zpg: {
			_executeInPlaceAND(cpu, *v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_and_zpgx: {
			_executeInPlaceAND(cpu, *v6502_map(cpu->memory, low + cpu->x));
		} return;
		case v6502_opcode_and_abs: {
			_executeInPlaceAND(cpu, *v6502_map(cpu->memory, BOTH_BYTES));
		} return;
		case v6502_opcode_and_absx: {
			_executeInPlaceAND(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->x));
		} return;
		case v6502_opcode_and_absy: {
			_executeInPlaceAND(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->y));
		} return;
		case v6502_opcode_and_indx: {
			_executeInPlaceAND(cpu, *v6502_map(cpu->memory, *v6502_map(cpu->memory, BOTH_BYTES) + cpu->x));
		} return;
		case v6502_opcode_and_indy: {
			_executeInPlaceAND(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->y));
		} return;
		
		// ASL
		case v6502_opcode_asl_acc: {
			_executeInPlaceASL(cpu, &cpu->ac);
		} return;
		case v6502_opcode_asl_zpg: {
			_executeInPlaceASL(cpu, &*v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_asl_zpgx: {
			_executeInPlaceASL(cpu, &*v6502_map(cpu->memory, low + cpu->x));
		} return;
		case v6502_opcode_asl_abs: {
			_executeInPlaceASL(cpu, &*v6502_map(cpu->memory, BOTH_BYTES));
		} return;
		case v6502_opcode_asl_absx: {
			_executeInPlaceASL(cpu, &*v6502_map(cpu->memory, BOTH_BYTES + cpu->x));
		} return;
			
		// BIT
		case v6502_opcode_bit_zpg: {
			_executeInPlaceBIT(cpu, *v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_bit_abs: {
			_executeInPlaceBIT(cpu, *v6502_map(cpu->memory, BOTH_BYTES));
		} return;

		// CMP
		case v6502_opcode_cmp_imm: {
			_executeInPlaceCMP(cpu, low);
		} return;
		case v6502_opcode_cmp_zpg: {
			_executeInPlaceCMP(cpu, *v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_cmp_zpgx: {
			_executeInPlaceCMP(cpu, *v6502_map(cpu->memory, low + cpu->x));
		} return;
		case v6502_opcode_cmp_abs: {
			_executeInPlaceCMP(cpu, *v6502_map(cpu->memory, BOTH_BYTES));
		} return;
		case v6502_opcode_cmp_absx: {
			_executeInPlaceCMP(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->x));
		} return;
		case v6502_opcode_cmp_absy: {
			_executeInPlaceCMP(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->y));
		} return;
		case v6502_opcode_cmp_indx: {
			_executeInPlaceCMP(cpu, *v6502_map(cpu->memory, *v6502_map(cpu->memory, BOTH_BYTES) + cpu->x));
		} return;
		case v6502_opcode_cmp_indy: {
			_executeInPlaceCMP(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->y));
		} return;
		
		// CPX
		case v6502_opcode_cpx_imm: {
			_executeInPlaceCPX(cpu, low);
		} return;
		case v6502_opcode_cpx_zpg: {
			_executeInPlaceCPX(cpu, *v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_cpx_abs: {
			_executeInPlaceCPX(cpu, *v6502_map(cpu->memory, BOTH_BYTES));
		} return;

		// CPY
		case v6502_opcode_cpy_imm: {
			_executeInPlaceCPY(cpu, low);
		} return;
		case v6502_opcode_cpy_zpg: {
			_executeInPlaceCPY(cpu, *v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_cpy_abs: {
			_executeInPlaceCPY(cpu, *v6502_map(cpu->memory, BOTH_BYTES));
		} return;

		// DEC
		case v6502_opcode_dec_zpg: {
			_executeInPlaceDEC(cpu, &*v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_dec_zpgx: {
			_executeInPlaceDEC(cpu, &*v6502_map(cpu->memory, low + cpu->x));
		} return;
		case v6502_opcode_dec_abs: {
			_executeInPlaceDEC(cpu, &*v6502_map(cpu->memory, BOTH_BYTES));
		} return;
		case v6502_opcode_dec_absx: {
			_executeInPlaceDEC(cpu, &*v6502_map(cpu->memory, BOTH_BYTES + cpu->x));
		} return;

		// EOR
		case v6502_opcode_eor_imm: {
			cpu->ac ^= low;
		} return;
		case v6502_opcode_eor_zpg: {
			cpu->ac ^= *v6502_map(cpu->memory, low);
		} return;
		case v6502_opcode_eor_zpgx: {
			cpu->ac ^= *v6502_map(cpu->memory, low + cpu->x);
		} return;
		case v6502_opcode_eor_abs: {
			cpu->ac ^= *v6502_map(cpu->memory, BOTH_BYTES);
		} return;
		case v6502_opcode_eor_absx: {
			cpu->ac ^= *v6502_map(cpu->memory, BOTH_BYTES + cpu->x);
		} return;
		case v6502_opcode_eor_absy: {
			cpu->ac ^= *v6502_map(cpu->memory, BOTH_BYTES + cpu->y);
		} return;
		case v6502_opcode_eor_indx: {
			cpu->ac ^= *v6502_map(cpu->memory, *v6502_map(cpu->memory, BOTH_BYTES) + cpu->x);
		} return;
		case v6502_opcode_eor_indy: {
			cpu->ac ^= *v6502_map(cpu->memory, BOTH_BYTES + cpu->y);
		} return;
		
		// INC
		case v6502_opcode_inc_zpg: {
			_executeInPlaceINC(cpu, &*v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_inc_zpgx: {
			_executeInPlaceINC(cpu, &*v6502_map(cpu->memory, low + cpu->x));
		} return;
		case v6502_opcode_inc_abs: {
			_executeInPlaceINC(cpu, &*v6502_map(cpu->memory, BOTH_BYTES));
		} return;
		case v6502_opcode_inc_absx: {
			_executeInPlaceINC(cpu, &*v6502_map(cpu->memory, BOTH_BYTES + cpu->x));
		} return;

		// JMP
		case v6502_opcode_jmp_abs: {
			cpu->pc = BOTH_BYTES;
		} return;
		case v6502_opcode_jmp_ind: {
			cpu->pc = *v6502_map(cpu->memory, low);
		} return;
		
		// ORA
		case v6502_opcode_ora_imm: {
			_executeInPlaceORA(cpu, low);
		} return;
		case v6502_opcode_ora_zpg: {
			_executeInPlaceORA(cpu, *v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_ora_zpgx: {
			_executeInPlaceORA(cpu, *v6502_map(cpu->memory, low + cpu->x));
		} return;
		case v6502_opcode_ora_abs: {
			_executeInPlaceORA(cpu, *v6502_map(cpu->memory, BOTH_BYTES));
		} return;
		case v6502_opcode_ora_absx: {
			_executeInPlaceORA(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->x));
		} return;
		case v6502_opcode_ora_absy: {
			_executeInPlaceORA(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->y));
		} return;
		case v6502_opcode_ora_indx: {
			_executeInPlaceORA(cpu, *v6502_map(cpu->memory, *v6502_map(cpu->memory, BOTH_BYTES) + cpu->x));
		} return;
		case v6502_opcode_ora_indy: {
			_executeInPlaceORA(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->y));
		} return;
			
		// LDA
		case v6502_opcode_lda_imm: {
			cpu->ac = low;
		}
		case v6502_opcode_lda_zpg: {
			cpu->ac = *v6502_map(cpu->memory, low);
		} return;
		case v6502_opcode_lda_zpgx: {
			cpu->ac = *v6502_map(cpu->memory, low + cpu->x);
		} return;
		case v6502_opcode_lda_abs: {
			cpu->ac = *v6502_map(cpu->memory, BOTH_BYTES);
		} return;
		case v6502_opcode_lda_absx: {
			cpu->ac = *v6502_map(cpu->memory, BOTH_BYTES + cpu->x);
		} return;
		case v6502_opcode_lda_absy: {
			cpu->ac = *v6502_map(cpu->memory, BOTH_BYTES + cpu->y);
		} return;
		case v6502_opcode_lda_indx: { // ???: Not sure about these indirect operations
			cpu->ac = *v6502_map(cpu->memory, *v6502_map(cpu->memory, BOTH_BYTES) + cpu->x);
		} return;
		case v6502_opcode_lda_indy: {
			cpu->ac = *v6502_map(cpu->memory, *v6502_map(cpu->memory, BOTH_BYTES + cpu->y));
		} return;
		
		// LDX
		case v6502_opcode_ldx_imm: {
			cpu->x = low;
		}
		case v6502_opcode_ldx_zpg: {
			cpu->x = *v6502_map(cpu->memory, low);
		} return;
		case v6502_opcode_ldx_zpgy: {
			cpu->x = *v6502_map(cpu->memory, low + cpu->y);
		} return;
		case v6502_opcode_ldx_abs: {
			cpu->x = *v6502_map(cpu->memory, BOTH_BYTES);
		} return;
		case v6502_opcode_ldx_absy: {
			cpu->x = *v6502_map(cpu->memory, BOTH_BYTES + cpu->y);
		} return;

		// LDY
		case v6502_opcode_ldy_imm: {
			cpu->y = low;
		}
		case v6502_opcode_ldy_zpg: {
			cpu->y = *v6502_map(cpu->memory, low);
		} return;
		case v6502_opcode_ldy_zpgx: {
			cpu->y = *v6502_map(cpu->memory, low + cpu->x);
		} return;
		case v6502_opcode_ldy_abs: {
			cpu->y = *v6502_map(cpu->memory, BOTH_BYTES);
		} return;
		case v6502_opcode_ldy_absx: {
			cpu->y = *v6502_map(cpu->memory, BOTH_BYTES + cpu->x);
		} return;
		
		// LSR
		case v6502_opcode_lsr_acc: {
			_executeInPlaceLSR(cpu, &cpu->ac);
		} return;
		case v6502_opcode_lsr_zpg: {
			_executeInPlaceLSR(cpu, &*v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_lsr_zpgx: {
			_executeInPlaceLSR(cpu, &*v6502_map(cpu->memory, low + cpu->x));
		} return;
		case v6502_opcode_lsr_abs: {
			_executeInPlaceLSR(cpu, &*v6502_map(cpu->memory, BOTH_BYTES));
		} return;
		case v6502_opcode_lsr_absx: {
			_executeInPlaceLSR(cpu, &*v6502_map(cpu->memory, BOTH_BYTES + cpu->x));
		} return;

		// ROL
		case v6502_opcode_rol_acc: {
			_executeInPlaceROL(cpu, &cpu->ac);
		} return;
		case v6502_opcode_rol_zpg: {
			_executeInPlaceROL(cpu, &*v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_rol_zpgx: {
			_executeInPlaceROL(cpu, &*v6502_map(cpu->memory, low + cpu->x));
		} return;
		case v6502_opcode_rol_abs: {
			_executeInPlaceROL(cpu, &*v6502_map(cpu->memory, BOTH_BYTES));
		} return;
		case v6502_opcode_rol_absx: {
			_executeInPlaceROL(cpu, &*v6502_map(cpu->memory, BOTH_BYTES + cpu->x));
		} return;

		// ROR
		case v6502_opcode_ror_acc: {
			_executeInPlaceROR(cpu, &cpu->ac);
		} return;
		case v6502_opcode_ror_zpg: {
			_executeInPlaceROR(cpu, &*v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_ror_zpgx: {
			_executeInPlaceROR(cpu, &*v6502_map(cpu->memory, low + cpu->x));
		} return;
		case v6502_opcode_ror_abs: {
			_executeInPlaceROR(cpu, &*v6502_map(cpu->memory, BOTH_BYTES));
		} return;
		case v6502_opcode_ror_absx: {
			_executeInPlaceROR(cpu, &*v6502_map(cpu->memory, BOTH_BYTES + cpu->x));
		} return;

		// SBC
		case v6502_opcode_sbc_imm: {
			_executeInPlaceSBC(cpu, low);
		} return;
		case v6502_opcode_sbc_zpg: {
			_executeInPlaceSBC(cpu, *v6502_map(cpu->memory, low));
		} return;
		case v6502_opcode_sbc_zpgx: {
			_executeInPlaceSBC(cpu, *v6502_map(cpu->memory, low + cpu->x));
		} return;
		case v6502_opcode_sbc_abs: {
			_executeInPlaceSBC(cpu, *v6502_map(cpu->memory, BOTH_BYTES));
		} return;
		case v6502_opcode_sbc_absx: {
			_executeInPlaceSBC(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->x));
		} return;
		case v6502_opcode_sbc_absy: {
			_executeInPlaceSBC(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->y));
		} return;
		case v6502_opcode_sbc_indx: {
			_executeInPlaceSBC(cpu, *v6502_map(cpu->memory, *v6502_map(cpu->memory, BOTH_BYTES) + cpu->x));
		} return;
		case v6502_opcode_sbc_indy: {
			_executeInPlaceSBC(cpu, *v6502_map(cpu->memory, BOTH_BYTES + cpu->y));
		} return;

		// STA
		case v6502_opcode_sta_zpg: {
			*v6502_map(cpu->memory, low) = cpu->ac;
		} return;
		case v6502_opcode_sta_zpgx: {
			*v6502_map(cpu->memory, low + cpu->x) = cpu->ac;
		} return;
		case v6502_opcode_sta_abs: {
			*v6502_map(cpu->memory, BOTH_BYTES) = cpu->ac;
		} return;
		case v6502_opcode_sta_absx: {
			*v6502_map(cpu->memory, BOTH_BYTES + cpu->x) = cpu->ac;
		} return;
		case v6502_opcode_sta_absy: {
			*v6502_map(cpu->memory, BOTH_BYTES + cpu->y) = cpu->ac;
		} return;
		case v6502_opcode_sta_indx: { // ???: Not sure about these indirect operations
			*v6502_map(cpu->memory, *v6502_map(cpu->memory, BOTH_BYTES) + cpu->x) = cpu->ac;
		} return;
		case v6502_opcode_sta_indy: {
			*v6502_map(cpu->memory, *v6502_map(cpu->memory, BOTH_BYTES + cpu->y)) = cpu->ac;
		} return;
			
		// STX
		case v6502_opcode_stx_zpg: {
			*v6502_map(cpu->memory, low) = cpu->x;
		} return;
		case v6502_opcode_stx_zpgy: {
			*v6502_map(cpu->memory, low + cpu->y) = cpu->x;
		} return;
		case v6502_opcode_stx_abs: {
			*v6502_map(cpu->memory, BOTH_BYTES) = cpu->x;
		} return;
			
		// STY
		case v6502_opcode_sty_zpg: {
			*v6502_map(cpu->memory, low) = cpu->y;
		} return;
		case v6502_opcode_sty_zpgx: {
			*v6502_map(cpu->memory, low + cpu->x) = cpu->y;
		} return;
		case v6502_opcode_sty_abs: {
			*v6502_map(cpu->memory, BOTH_BYTES) = cpu->y;
		} return;
			
		// Failure
		default: {
			v6502_fault("Unhandled CPU Instruction");
		} return;
	}
}
