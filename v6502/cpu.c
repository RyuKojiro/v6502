/*
 * Copyright (c) 2013 Daniel Loffgren
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>

#include "cpu.h"

#define	BOTH_BYTES									(high << 8 | low)
#define FLAG_CARRY_WITH_HIGH_BIT(a)					cpu->sr &= ~v6502_cpu_status_carry; \
													cpu->sr |= a >> 7;
#define FLAG_CARRY_WITH_LOW_BIT(a)					cpu->sr &= ~v6502_cpu_status_carry; \
													cpu->sr |= (a & v6502_cpu_status_carry);
#define FLAG_ZERO_WITH_RESULT(a)					cpu->sr &= (a ? ~v6502_cpu_status_zero : ~0); \
													cpu->sr |= (a ? 0 : v6502_cpu_status_zero);
#define FLAG_NEGATIVE_WITH_RESULT(a)				cpu->sr &= ~ v6502_cpu_status_negative; \
													cpu->sr |= (a & v6502_cpu_status_negative)
/** The first if statement checks if both operands' signs agree (since we are
 normalized to addition at this point.) If they do, then the output sign must
 also agree after calculation, or we have overflown; this is checked in the
 following ternary. */
#define FLAG_OVERFLOW_WITH_COMPARISON(a, b, c)		cpu->sr &= ~v6502_cpu_status_overflow; \
													if ((~(a ^ b)) & 0x80) { cpu->sr |= (( (~(a ^ c)) & 0x80) ? 0 : v6502_cpu_status_overflow); }

/** If addition, b = operand, a = result, subtraction reverses it */
#define FLAG_CARRY_WITH_COMPARISON(a, b)			cpu->sr &= ~v6502_cpu_status_carry; \
													cpu->sr |= ((a < b) ? v6502_cpu_status_carry : 0);
#define FLAG_NEG_AND_ZERO_WITH_RESULT(a)			FLAG_NEGATIVE_WITH_RESULT(a); \
													FLAG_ZERO_WITH_RESULT(a);

#define kUnhandledInstructionErrorText				"Unhandled CPU Instruction"

#pragma mark -
#pragma mark CPU Internal Instruction Execution

static void _executeInPlaceASL(v6502_cpu *cpu, uint8_t *operand) {
	FLAG_CARRY_WITH_HIGH_BIT(*operand);
	*operand <<= 1;
	FLAG_NEG_AND_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceLSR(v6502_cpu *cpu, uint8_t *operand) {
	FLAG_CARRY_WITH_LOW_BIT(*operand);
	*operand >>= 1;
	FLAG_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceROL(v6502_cpu *cpu, uint8_t *operand) {
	FLAG_CARRY_WITH_HIGH_BIT(*operand);
	*operand = (*operand << 1) | (*operand >> 7);
	FLAG_NEG_AND_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceROR(v6502_cpu *cpu, uint8_t *operand) {
	FLAG_CARRY_WITH_HIGH_BIT(*operand);
	*operand = (*operand >> 1) | (*operand << 7);
	FLAG_NEG_AND_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceORA(v6502_cpu *cpu, uint8_t operand) {
	cpu->ac |= operand;
	FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
}

static void _executeInPlaceAND(v6502_cpu *cpu, uint8_t operand) {
	cpu->ac &= operand;
	FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
}

static void _executeInPlaceADC(v6502_cpu *cpu, uint8_t operand) {
	uint8_t a = cpu->ac;
	cpu->ac += operand + ((cpu->sr & v6502_cpu_status_carry) ? 1 : 0);
	FLAG_OVERFLOW_WITH_COMPARISON(a, operand, cpu->ac);
	FLAG_CARRY_WITH_COMPARISON(cpu->ac, operand);
	FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
}

static void _executeInPlaceDecrement(v6502_cpu *cpu, uint8_t *operand) {
	(*operand)--;
	FLAG_NEG_AND_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceIncrement(v6502_cpu *cpu, uint8_t *operand) {
	(*operand)++;
	FLAG_NEG_AND_ZERO_WITH_RESULT(*operand);
}

static void _executeInPlaceCompare(v6502_cpu *cpu, uint8_t *reg, uint8_t operand) {
	uint8_t result = (*reg) - operand;
	FLAG_CARRY_WITH_COMPARISON(operand, result);
	FLAG_NEG_AND_ZERO_WITH_RESULT(result);
}

static void _executeInPlaceBIT(v6502_cpu *cpu, uint8_t operand) {
	uint8_t result = cpu->ac & operand;
	cpu->sr &= ~(v6502_cpu_status_overflow | v6502_cpu_status_negative);
	cpu->sr |= (operand & (v6502_cpu_status_overflow | v6502_cpu_status_negative));
	FLAG_ZERO_WITH_RESULT(result);
}

#pragma mark -
#pragma mark CPU Lifecycle

v6502_cpu *v6502_createCPU(void) {
	// Allocate CPU Struct
	v6502_cpu *cpu = malloc(sizeof(v6502_cpu));
	
	if (cpu) {
		cpu->fault_callback = NULL;
		cpu->fault_context = NULL;
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
	
	if ((opcode & 0x0F) == 0x09) { // low nibble is 9
		if (opcode & 0x10) { // high nibble is odd
			return 3;
		}
		else { // high nibble is even
			return 2;
		}
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
		case v6502_opcode_rti:
		case v6502_opcode_rts:
			return v6502_address_mode_implied;
		case v6502_opcode_bcc:
		case v6502_opcode_bcs:
		case v6502_opcode_beq:
		case v6502_opcode_bne:
		case v6502_opcode_bmi:
		case v6502_opcode_bpl:
		case v6502_opcode_bvc:
		case v6502_opcode_bvs:
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
		case v6502_opcode_jsr:
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
	cpu->pc = v6502_memoryStartProgram;
	cpu->ac = 0;
	cpu->x  = 0;
	cpu->y  = 0;
	cpu->sr = v6502_cpu_status_ignored;
	cpu->sp = BYTE_MAX;
}

void v6502_step(v6502_cpu *cpu) {
	v6502_opcode opcode = *v6502_map(cpu->memory, cpu->pc);
	uint8_t low = *v6502_map(cpu->memory, cpu->pc + 1);
	uint8_t high = *v6502_map(cpu->memory, cpu->pc + 2);
	v6502_execute(cpu, opcode, low, high);
	cpu->pc += v6502_instructionLengthForOpcode(opcode);
}

void v6502_execute(v6502_cpu *cpu, uint8_t opcode, uint8_t low, uint8_t high) {
	uint8_t *operand;
	uint16_t ref;
	
	switch (v6502_addressModeForOpcode(opcode)) {
		case v6502_address_mode_implied:
		case v6502_address_mode_accumulator: {
			operand = &(cpu->ac);
		} break;
		case v6502_address_mode_immediate: {
			operand = &low;
		} break;
		case v6502_address_mode_indirect: {
			ref = *v6502_map(cpu->memory, BOTH_BYTES); // Low byte first
			ref |= *v6502_map(cpu->memory, BOTH_BYTES + 1) << 8; // High byte second
			operand = v6502_map(cpu->memory, ref);
		} break;
		case v6502_address_mode_indirect_x: {
			low += cpu->x;
			ref = *v6502_map(cpu->memory, low); // Low byte first
			ref |= *v6502_map(cpu->memory, low + 1) << 8; // High byte second
			operand = v6502_map(cpu->memory, ref);
		} break;
		case v6502_address_mode_indirect_y: {
			ref = *v6502_map(cpu->memory, low); // Low byte first
			ref |= *v6502_map(cpu->memory, low + 1) << 8; // High byte second
			ref += cpu->y;
			operand = v6502_map(cpu->memory, ref);
		} break;
		case v6502_address_mode_zeropage: {
			operand = v6502_map(cpu->memory, low);
		} break;
		case v6502_address_mode_zeropage_x: {
			operand = v6502_map(cpu->memory, low + cpu->x);
		} break;
		case v6502_address_mode_zeropage_y: {
			operand = v6502_map(cpu->memory, low + cpu->y);
		} break;
		case v6502_address_mode_absolute: {
			operand = v6502_map(cpu->memory, BOTH_BYTES);
		} break;
		case v6502_address_mode_absolute_x: {
			operand = v6502_map(cpu->memory, BOTH_BYTES + cpu->x);
		} break;
		case v6502_address_mode_absolute_y: {
			operand = v6502_map(cpu->memory, BOTH_BYTES + cpu->y);
		} break;
		case v6502_address_mode_relative:
			break;
		case v6502_address_mode_symbol:
		case v6502_address_mode_unknown:
		default:
			return;
	}
	
	switch ((v6502_opcode)opcode) {
		// Single Byte Instructions
		case v6502_opcode_brk: {
			/** @todo TODO: Should this prevent the automatic pc shift? */
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
			_executeInPlaceDecrement(cpu, &cpu->x);
		} return;
		case v6502_opcode_dey: {
			_executeInPlaceDecrement(cpu, &cpu->y);
		} return;
		case v6502_opcode_tax: {
			cpu->x = cpu->ac;
			FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
		} return;
		case v6502_opcode_tay: {
			cpu->y = cpu->ac;
			FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
		} return;
		case v6502_opcode_tsx: {
			cpu->x = cpu->sp;
			FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->sp);
		} return;
		case v6502_opcode_txa: {
			cpu->ac = cpu->x;
			FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
		} return;
		case v6502_opcode_txs: {
			cpu->sp = cpu->x;
			FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->sp);
		} return;
		case v6502_opcode_tya: {
			cpu->ac = cpu->y;
			FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
		} return;
		case v6502_opcode_inx: {
			_executeInPlaceIncrement(cpu, &cpu->x);
		} return;
		case v6502_opcode_iny: {
			_executeInPlaceIncrement(cpu, &cpu->y);
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
			cpu->memory->bytes[v6502_memoryStartStack + cpu->sp--] = cpu->pc;			// Low byte first
			cpu->memory->bytes[v6502_memoryStartStack + cpu->sp--] = (cpu->pc >> 8);	// High byte second
			cpu->pc = BOTH_BYTES;
			cpu->pc -= 3; // To compensate for post execution shift
		} return;
		case v6502_opcode_rti: {
			/** TODO: @todo Interrupts (RTI/RTS) */
		} return;
		case v6502_opcode_rts: {
			cpu->pc = (cpu->memory->bytes[v6502_memoryStartStack + ++cpu->sp] << 8);
			cpu->pc |= cpu->memory->bytes[v6502_memoryStartStack + ++cpu->sp];
			cpu->pc += 2; // To compensate for post execution shift ( - 1 rts, + 3 jsr )
		} return;
		case v6502_opcode_pha: {
			cpu->memory->bytes[v6502_memoryStartStack + cpu->sp--] = cpu->ac;
		} return;
		case v6502_opcode_pla: {
			cpu->ac = cpu->memory->bytes[v6502_memoryStartStack + ++cpu->sp];
			FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
		} return;
		case v6502_opcode_php: {
			cpu->memory->bytes[v6502_memoryStartStack + cpu->sp--] = cpu->sr;
		} return;
		case v6502_opcode_plp: {
			cpu->sr = cpu->memory->bytes[v6502_memoryStartStack + ++cpu->sp];
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
			_executeInPlaceADC(cpu, *operand);
			return;

		// AND
		case v6502_opcode_and_imm:
		case v6502_opcode_and_zpg:
		case v6502_opcode_and_zpgx:
		case v6502_opcode_and_abs:
		case v6502_opcode_and_absx:
		case v6502_opcode_and_absy:
		case v6502_opcode_and_indx:
		case v6502_opcode_and_indy:
			_executeInPlaceAND(cpu, *operand);
			return;
		
		// ASL
		case v6502_opcode_asl_acc:
		case v6502_opcode_asl_zpg:
		case v6502_opcode_asl_zpgx:
		case v6502_opcode_asl_abs:
		case v6502_opcode_asl_absx:
			_executeInPlaceASL(cpu, operand);
			return;
			
		// BIT
		case v6502_opcode_bit_zpg:
		case v6502_opcode_bit_abs:
			_executeInPlaceBIT(cpu, *operand);
			return;

		// CMP
		case v6502_opcode_cmp_imm:
		case v6502_opcode_cmp_zpg:
		case v6502_opcode_cmp_zpgx:
		case v6502_opcode_cmp_abs:
		case v6502_opcode_cmp_absx:
		case v6502_opcode_cmp_absy:
		case v6502_opcode_cmp_indx:
		case v6502_opcode_cmp_indy:
			_executeInPlaceCompare(cpu, &(cpu->ac), *operand);
			return;
		
		// CPX
		case v6502_opcode_cpx_imm:
		case v6502_opcode_cpx_zpg:
		case v6502_opcode_cpx_abs:
			_executeInPlaceCompare(cpu, &(cpu->x), *operand);
			return;

		// CPY
		case v6502_opcode_cpy_imm:
		case v6502_opcode_cpy_zpg:
		case v6502_opcode_cpy_abs:
			_executeInPlaceCompare(cpu, &(cpu->y), *operand);
			return;

		// DEC
		case v6502_opcode_dec_zpg:
		case v6502_opcode_dec_zpgx:
		case v6502_opcode_dec_abs:
		case v6502_opcode_dec_absx:
			_executeInPlaceDecrement(cpu, operand);
			return;

		// EOR
		case v6502_opcode_eor_imm:
		case v6502_opcode_eor_zpg:
		case v6502_opcode_eor_zpgx:
		case v6502_opcode_eor_abs:
		case v6502_opcode_eor_absx:
		case v6502_opcode_eor_absy:
		case v6502_opcode_eor_indx:
		case v6502_opcode_eor_indy:
			cpu->ac ^= *operand;
			FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
			return;
		
		// INC
		case v6502_opcode_inc_zpg:
		case v6502_opcode_inc_zpgx:
		case v6502_opcode_inc_abs:
		case v6502_opcode_inc_absx:
			_executeInPlaceIncrement(cpu, operand);
			return;

		// JMP
		case v6502_opcode_jmp_abs: {
			cpu->pc = BOTH_BYTES;
			cpu->pc -= 3; // PC shift
		} return;
		case v6502_opcode_jmp_ind: {
			cpu->pc = *v6502_map(cpu->memory, low);
			cpu->pc -= 2; // PC shift
		} return;
		
		// ORA
		case v6502_opcode_ora_imm:
		case v6502_opcode_ora_zpg:
		case v6502_opcode_ora_zpgx:
		case v6502_opcode_ora_abs:
		case v6502_opcode_ora_absx:
		case v6502_opcode_ora_absy:
		case v6502_opcode_ora_indx:
		case v6502_opcode_ora_indy:
			_executeInPlaceORA(cpu, *operand);
			return;
			
		// LDA
		case v6502_opcode_lda_imm:
		case v6502_opcode_lda_zpg:
		case v6502_opcode_lda_zpgx:
		case v6502_opcode_lda_abs:
		case v6502_opcode_lda_absx:
		case v6502_opcode_lda_absy:
		case v6502_opcode_lda_indx:
		case v6502_opcode_lda_indy:
			cpu->ac = *operand;
			FLAG_NEG_AND_ZERO_WITH_RESULT(*operand);
			return;
		
		// LDX
		case v6502_opcode_ldx_imm:
		case v6502_opcode_ldx_zpg:
		case v6502_opcode_ldx_zpgy:
		case v6502_opcode_ldx_abs:
		case v6502_opcode_ldx_absy:
			cpu->x = *operand;
			FLAG_NEG_AND_ZERO_WITH_RESULT(*operand);
			return;

		// LDY
		case v6502_opcode_ldy_imm:
		case v6502_opcode_ldy_zpg:
		case v6502_opcode_ldy_zpgx:
		case v6502_opcode_ldy_abs:
		case v6502_opcode_ldy_absx:
			cpu->y = *operand;
			FLAG_NEG_AND_ZERO_WITH_RESULT(*operand);
			return;
		
		// LSR
		case v6502_opcode_lsr_acc:
		case v6502_opcode_lsr_zpg:
		case v6502_opcode_lsr_zpgx:
		case v6502_opcode_lsr_abs:
		case v6502_opcode_lsr_absx:
			_executeInPlaceLSR(cpu, operand);
			return;

		// ROL
		case v6502_opcode_rol_acc:
		case v6502_opcode_rol_zpg:
		case v6502_opcode_rol_zpgx:
		case v6502_opcode_rol_abs:
		case v6502_opcode_rol_absx:
			_executeInPlaceROL(cpu, operand);
			return;

		// ROR
		case v6502_opcode_ror_acc:
		case v6502_opcode_ror_zpg:
		case v6502_opcode_ror_zpgx:
		case v6502_opcode_ror_abs:
		case v6502_opcode_ror_absx:
			_executeInPlaceROR(cpu, operand);
			return;

		// SBC
		case v6502_opcode_sbc_imm:
		case v6502_opcode_sbc_zpg:
		case v6502_opcode_sbc_zpgx:
		case v6502_opcode_sbc_abs:
		case v6502_opcode_sbc_absx:
		case v6502_opcode_sbc_absy:
		case v6502_opcode_sbc_indx:
		case v6502_opcode_sbc_indy:
			_executeInPlaceADC(cpu, *operand ^ BYTE_MAX);
			return;

		// STA
		case v6502_opcode_sta_zpg:
		case v6502_opcode_sta_zpgx:
		case v6502_opcode_sta_abs:
		case v6502_opcode_sta_absx:
		case v6502_opcode_sta_absy:
		case v6502_opcode_sta_indx:
		case v6502_opcode_sta_indy:
			*operand = cpu->ac;
			return;
			
		// STX
		case v6502_opcode_stx_zpg:
		case v6502_opcode_stx_zpgy:
		case v6502_opcode_stx_abs:
			*operand = cpu->x;
			return;
			
		// STY
		case v6502_opcode_sty_zpg:
		case v6502_opcode_sty_zpgx:
		case v6502_opcode_sty_abs:
			*operand = cpu->y;
			return;
			
		// Failure
		default: {
			v6502_fault(kUnhandledInstructionErrorText);
		} return;
	}
}
