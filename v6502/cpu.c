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

#define	BOTH_BYTES								(high << 8 | low)
#define FLAG_CARRY_WITH_HIGH_BIT(a)				{ cpu->sr &= ~v6502_cpu_status_carry; \
												  cpu->sr |= a >> 7; }
#define FLAG_CARRY_WITH_LOW_BIT(a)				{ cpu->sr &= ~v6502_cpu_status_carry; \
												  cpu->sr |= (a & v6502_cpu_status_carry); }
#define FLAG_ZERO_WITH_RESULT(a)				{ cpu->sr &= (a ? ~v6502_cpu_status_zero : ~0); \
												  cpu->sr |= (a ? 0 : v6502_cpu_status_zero); }
#define FLAG_NEGATIVE_WITH_RESULT(a)			{ cpu->sr &= ~ v6502_cpu_status_negative; \
												  cpu->sr |= (a & v6502_cpu_status_negative); }
/** After normalization to addition, arguments are:
 a = accumulator pre-operation,
 b = operand
 c = accumulator result */
#define FLAG_OVERFLOW_WITH_COMPARISON(a, b, c)	{ cpu->sr &= ~v6502_cpu_status_overflow; \
												  cpu->sr |= (( ~(a ^ b) & (a ^ c) & 0x80) ? 0 : v6502_cpu_status_overflow); }
/** After normalization to addition, arguments are:
  a = result
  b = operand,
 for subtraction:
  a = operand
  b = register */
#define FLAG_CARRY_WITH_COMPARISON(a, b)		{ cpu->sr &= ~v6502_cpu_status_carry; \
												  cpu->sr |= ((a <= b) ? v6502_cpu_status_carry : 0); }
#define FLAG_NEG_AND_ZERO_WITH_RESULT(a)		{ FLAG_NEGATIVE_WITH_RESULT(a); \
												  FLAG_ZERO_WITH_RESULT(a); }

#define v6502_unhandledInstructionErrorText		"Unhandled CPU Instruction"

#pragma mark -
#pragma mark CPU Internal Instruction Execution

static uint8_t _executeInPlaceASL(v6502_cpu *cpu, uint8_t operand) {
	//! [asl]
	FLAG_CARRY_WITH_HIGH_BIT(operand);
	operand <<= 1;
	FLAG_NEG_AND_ZERO_WITH_RESULT(operand);
	return operand;
	//! [asl]
}

static uint8_t _executeInPlaceLSR(v6502_cpu *cpu, uint8_t operand) {
	//! [lsr]
	FLAG_CARRY_WITH_LOW_BIT(operand);
	operand >>= 1;
	FLAG_ZERO_WITH_RESULT(operand);
	return operand;
	//! [lsr]
}

static uint8_t _executeInPlaceROL(v6502_cpu *cpu, uint8_t operand) {
	//! [rol]
	uint8_t carry = (cpu->sr & v6502_cpu_status_carry);
	FLAG_CARRY_WITH_HIGH_BIT(operand);
	operand = (operand << 1) | (carry);
	FLAG_NEG_AND_ZERO_WITH_RESULT(operand);
	return operand;
	//! [rol]
}

static uint8_t _executeInPlaceROR(v6502_cpu *cpu, uint8_t operand) {
	//! [ror]
	uint8_t carry = (cpu->sr & v6502_cpu_status_carry);
	FLAG_CARRY_WITH_LOW_BIT(operand);
	operand = (operand >> 1) | (carry << 7);
	FLAG_NEG_AND_ZERO_WITH_RESULT(operand);
	return operand;
	//! [ror]
}

static void _executeInPlaceORA(v6502_cpu *cpu, uint8_t operand) {
	//! [ora]
	cpu->ac |= operand;
	FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
	//! [ora]
}

static void _executeInPlaceAND(v6502_cpu *cpu, uint8_t operand) {
	//! [and]
	cpu->ac &= operand;
	FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
	//! [and]
}

static void _executeInPlaceADC(v6502_cpu *cpu, uint8_t operand) {
	//! [adc]
	uint8_t a = cpu->ac;
	cpu->ac += operand + ((cpu->sr & v6502_cpu_status_carry) ? 1 : 0);
	FLAG_OVERFLOW_WITH_COMPARISON(a, operand, cpu->ac);
	FLAG_CARRY_WITH_COMPARISON(cpu->ac, operand);
	FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
	//! [adc]
}

static uint8_t _executeInPlaceDecrement(v6502_cpu *cpu, uint8_t operand) {
	//! [Decrement]
	operand--;
	FLAG_NEG_AND_ZERO_WITH_RESULT(operand);
	//! [Decrement]
	return operand;
}

static uint8_t _executeInPlaceIncrement(v6502_cpu *cpu, uint8_t operand) {
	//! [Increment]
	operand++;
	FLAG_NEG_AND_ZERO_WITH_RESULT(operand);
	//! [Increment]
	return operand;
}

static void _executeInPlaceCompare(v6502_cpu *cpu, uint8_t reg, uint8_t operand) {
	//! [Compare]
	uint8_t result = reg - operand;
	FLAG_CARRY_WITH_COMPARISON(operand, reg);
	FLAG_NEG_AND_ZERO_WITH_RESULT(result);
	//! [Compare]
}

static void _executeInPlaceBIT(v6502_cpu *cpu, uint8_t operand) {
	//! [bit]
	uint8_t result = cpu->ac & operand;
	cpu->sr &= ~(v6502_cpu_status_overflow | v6502_cpu_status_negative);
	cpu->sr |= (operand & (v6502_cpu_status_overflow | v6502_cpu_status_negative));
	FLAG_ZERO_WITH_RESULT(result);
	//! [bit]
}

#pragma mark -
#pragma mark CPU Lifecycle

v6502_cpu *v6502_createCPU(void) {
	return calloc(1, sizeof(v6502_cpu));
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
	if ((opcode & 0xf) == 0x8) { // lower nibble is 0x8
		return v6502_address_mode_implied;
	}

	// Zeropage (0x?4-0x?6)
	if ((opcode & 0xf) >= 0x4 && (opcode & 0xf) <= 0x6) {
		if ((opcode & 0x10)) { // odd high nibble
			if (opcode == v6502_opcode_stx_zpgy ||
				opcode == v6502_opcode_ldx_zpgy) { // special case
				return v6502_address_mode_zeropage_y;
			}
			else {
				return v6502_address_mode_zeropage_x;
			}
		}
		else { // even high nibble
			return v6502_address_mode_zeropage;
		}
	}

	// Absolute (0x?c+)
	if ((opcode & 0xf) >= 0xc) {
		if ((opcode & 0x10)) { // odd high nibble
			if (opcode == v6502_opcode_ldx_absy) { // special case
				return v6502_address_mode_absolute_y;
			}
			else {
				return v6502_address_mode_absolute_x;
			}
		}
		else { // even high nibble
			if (opcode == v6502_opcode_jmp_ind) { // special case
				return v6502_address_mode_indirect;
			}
			else {
				return v6502_address_mode_absolute;
			}
		}
	}

	// Accumulator/Implied (0x?a-0x?b)
	if ((opcode & 0xf) >= 0xa && (opcode & 0xf) <= 0xb) {
		if ((opcode & 0xf0) >= 0x80) { // high nibble is 8 or higher
			return v6502_address_mode_implied;
		}
		else { // high nibble is 7 or lower
			return v6502_address_mode_accumulator;
		}
	}

	// Immediate/Absolute Y (0x?9)
	if ((opcode & 0xf) == 0x9) {
		return opcode & 0x10 ? v6502_address_mode_absolute_y : v6502_address_mode_immediate;
	}

	// Indirect (0x?1)
	if ((opcode & 0xf) == 0x1) {
		return opcode & 0x10 ? v6502_address_mode_indirect_y : v6502_address_mode_indirect_x;
	}

	// Branches and other garbage (0x?0)
	if ((opcode & 0xf) == 0x0) {
		if (opcode & 0x10) { // odd high nibble
			return v6502_address_mode_relative;
		}
		else { // even high nibble
			if ((opcode & 0xf0) >= 0x80) { // high nibble is 8 or higher
				return v6502_address_mode_immediate;
			}
			else {
				if (opcode == v6502_opcode_jsr) {
					return v6502_address_mode_absolute;
				}
				else {
					return v6502_address_mode_implied;
				}
			}
		}
	}

	// This one is very alone (0x?2)
	if (opcode == v6502_opcode_ldx_imm) {
		return v6502_address_mode_immediate;
	}

	return v6502_address_mode_unknown;
}

void v6502_nmi(v6502_cpu *cpu) {
	cpu->pc = (v6502_read(cpu->memory, v6502_memoryVectorNMIHigh, NO) << 8);
	cpu->pc |= v6502_read(cpu->memory, v6502_memoryVectorNMILow, NO);
}

void v6502_reset(v6502_cpu *cpu) {
	cpu->pc = (v6502_read(cpu->memory, v6502_memoryVectorResetHigh, NO) << 8);
	cpu->pc |= v6502_read(cpu->memory, v6502_memoryVectorResetLow, NO);
	cpu->ac = 0;
	cpu->x  = 0;
	cpu->y  = 0;
	cpu->sr = v6502_cpu_status_ignored;
	cpu->sp = BYTE_MAX;
}

void v6502_step(v6502_cpu *cpu) {
	// This could potentially be faster without the lint zeroing
	uint8_t low = 0;
	uint8_t high = 0;
	v6502_opcode opcode = v6502_read(cpu->memory, cpu->pc, YES);
	int instructionLength = v6502_instructionLengthForOpcode(opcode);
	if (instructionLength > 1) { low = v6502_read(cpu->memory, cpu->pc + 1, YES); }
	if (instructionLength > 2) { high = v6502_read(cpu->memory, cpu->pc + 2, YES); }
	v6502_execute(cpu, opcode, low, high);
	cpu->pc += instructionLength;
}

/* 	1) Determine address mode, and form an operand pointer based on that
	2) Execute operation, some in-place functions replace the value of *operand with the new resulting value
 */
void v6502_execute(v6502_cpu *cpu, uint8_t opcode, uint8_t low, uint8_t high) {
	// These don't need to be initialized, but do so to silence false positive clang lint warnings
	uint8_t operand = 0;
	uint16_t ref = 0;
	
	switch (v6502_addressModeForOpcode(opcode)) {
		case v6502_address_mode_implied:
		case v6502_address_mode_accumulator: {
			operand = cpu->ac;
		} break;
		case v6502_address_mode_immediate: {
			operand = low;
		} break;
		case v6502_address_mode_indirect: {
			ref = v6502_read(cpu->memory, BOTH_BYTES, YES); // Low byte first
			ref |= v6502_read(cpu->memory, BOTH_BYTES + 1, YES) << 8; // High byte second
			operand = v6502_read(cpu->memory, ref, YES);
		} break;
		case v6502_address_mode_indirect_x: {
			low += cpu->x;
			ref = v6502_read(cpu->memory, low, YES); // Low byte first
			ref |= v6502_read(cpu->memory, low + 1, YES) << 8; // High byte second
			operand = v6502_read(cpu->memory, ref, YES);
		} break;
		case v6502_address_mode_indirect_y: {
			ref = v6502_read(cpu->memory, low, YES); // Low byte first
			ref |= v6502_read(cpu->memory, low + 1, YES) << 8; // High byte second
			ref += cpu->y;
			operand = v6502_read(cpu->memory, ref, YES);
		} break;
		case v6502_address_mode_zeropage: {
			ref = low;
			operand = v6502_read(cpu->memory, ref, YES);
		} break;
		case v6502_address_mode_zeropage_x: {
			ref = low + cpu->x;
			operand = v6502_read(cpu->memory, ref, YES);
		} break;
		case v6502_address_mode_zeropage_y: {
			ref = low + cpu->y;
			operand = v6502_read(cpu->memory, ref, YES);
		} break;
		case v6502_address_mode_absolute: {
			ref = BOTH_BYTES;
			operand = v6502_read(cpu->memory, ref, YES);
		} break;
		case v6502_address_mode_absolute_x: {
			ref = BOTH_BYTES + cpu->x;
			operand = v6502_read(cpu->memory, ref, YES);
		} break;
		case v6502_address_mode_absolute_y: {
			ref = BOTH_BYTES + cpu->y;
			operand = v6502_read(cpu->memory, ref, YES);
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
		case v6502_opcode_nop: {
			/* Do nothing */
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
			cpu->x = _executeInPlaceDecrement(cpu, cpu->x);
		} return;
		case v6502_opcode_dey: {
			cpu->y = _executeInPlaceDecrement(cpu, cpu->y);
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
			cpu->x = _executeInPlaceIncrement(cpu, cpu->x);
		} return;
		case v6502_opcode_iny: {
			cpu->y = _executeInPlaceIncrement(cpu, cpu->y);
		} return;
		case v6502_opcode_wai: {
			// On a POSIX system, we'd sleep here, but that's not portable enough
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
			_executeInPlaceADC(cpu, operand);
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
			_executeInPlaceAND(cpu, operand);
			return;
		
		// ASL
		case v6502_opcode_asl_acc:
			cpu->ac = _executeInPlaceASL(cpu, operand);
			return;
		case v6502_opcode_asl_zpg:
		case v6502_opcode_asl_zpgx:
		case v6502_opcode_asl_abs:
		case v6502_opcode_asl_absx:
			v6502_write(cpu->memory, ref, _executeInPlaceASL(cpu, operand));
			return;
			
		// BIT
		case v6502_opcode_bit_zpg:
		case v6502_opcode_bit_abs:
			_executeInPlaceBIT(cpu, operand);
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
			_executeInPlaceCompare(cpu, cpu->ac, operand);
			return;
		
		// CPX
		case v6502_opcode_cpx_imm:
		case v6502_opcode_cpx_zpg:
		case v6502_opcode_cpx_abs:
			_executeInPlaceCompare(cpu, cpu->x, operand);
			return;

		// CPY
		case v6502_opcode_cpy_imm:
		case v6502_opcode_cpy_zpg:
		case v6502_opcode_cpy_abs:
			_executeInPlaceCompare(cpu, cpu->y, operand);
			return;

		// DEC
		case v6502_opcode_dec_zpg:
		case v6502_opcode_dec_zpgx:
		case v6502_opcode_dec_abs:
		case v6502_opcode_dec_absx:
			v6502_write(cpu->memory, ref, _executeInPlaceDecrement(cpu, operand));
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
			//! [eor]
			cpu->ac ^= operand;
			FLAG_NEG_AND_ZERO_WITH_RESULT(cpu->ac);
			//! [eor]
			return;
		
		// INC
		case v6502_opcode_inc_zpg:
		case v6502_opcode_inc_zpgx:
		case v6502_opcode_inc_abs:
		case v6502_opcode_inc_absx:
			v6502_write(cpu->memory, ref, _executeInPlaceIncrement(cpu, operand));
			return;

		// JMP
		//! [jmp]
		case v6502_opcode_jmp_abs: {
			if (BOTH_BYTES == cpu->pc) {
				v6502_execute(cpu, v6502_opcode_wai, 0, 0);
				cpu->pc -= 3; // PC shift
				return;
			}
			
			cpu->pc = BOTH_BYTES;
			cpu->pc -= 3; // PC shift
		} return;
		case v6502_opcode_jmp_ind: {
			uint16_t address = BOTH_BYTES;
			low = v6502_read(cpu->memory, address, NO);
			high = v6502_read(cpu->memory, address + 1, NO);
			
			// Trap was already triggered by indirect memory classification in prior switch
			cpu->pc = BOTH_BYTES;
			cpu->pc -= 3; // PC shift
		} return;
		//! [jmp]

		// ORA
		case v6502_opcode_ora_imm:
		case v6502_opcode_ora_zpg:
		case v6502_opcode_ora_zpgx:
		case v6502_opcode_ora_abs:
		case v6502_opcode_ora_absx:
		case v6502_opcode_ora_absy:
		case v6502_opcode_ora_indx:
		case v6502_opcode_ora_indy:
			_executeInPlaceORA(cpu, operand);
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
			//! [lda]
			cpu->ac = operand;
			FLAG_NEG_AND_ZERO_WITH_RESULT(operand);
			//! [lda]
			return;
		
		// LDX
		case v6502_opcode_ldx_imm:
		case v6502_opcode_ldx_zpg:
		case v6502_opcode_ldx_zpgy:
		case v6502_opcode_ldx_abs:
		case v6502_opcode_ldx_absy:
			//! [ldx]
			cpu->x = operand;
			FLAG_NEG_AND_ZERO_WITH_RESULT(operand);
			//! [ldx]
			return;

		// LDY
		case v6502_opcode_ldy_imm:
		case v6502_opcode_ldy_zpg:
		case v6502_opcode_ldy_zpgx:
		case v6502_opcode_ldy_abs:
		case v6502_opcode_ldy_absx:
			//! [ldy]
			cpu->y = operand;
			FLAG_NEG_AND_ZERO_WITH_RESULT(operand);
			//! [ldy]
			return;
		
		// LSR
		case v6502_opcode_lsr_acc:
			cpu->ac = _executeInPlaceLSR(cpu, operand);
			return;
		case v6502_opcode_lsr_zpg:
		case v6502_opcode_lsr_zpgx:
		case v6502_opcode_lsr_abs:
		case v6502_opcode_lsr_absx:
			v6502_write(cpu->memory, ref, _executeInPlaceLSR(cpu, operand));
			return;

		// ROL
		case v6502_opcode_rol_acc:
			cpu->ac = _executeInPlaceROL(cpu, operand);
			return;
		case v6502_opcode_rol_zpg:
		case v6502_opcode_rol_zpgx:
		case v6502_opcode_rol_abs:
		case v6502_opcode_rol_absx:
			v6502_write(cpu->memory, ref, _executeInPlaceROL(cpu, operand));
			return;

		// ROR
		case v6502_opcode_ror_acc:
			cpu->ac = _executeInPlaceROR(cpu, operand);
			return;
		case v6502_opcode_ror_zpg:
		case v6502_opcode_ror_zpgx:
		case v6502_opcode_ror_abs:
		case v6502_opcode_ror_absx:
			v6502_write(cpu->memory, ref, _executeInPlaceROR(cpu, operand));
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
			//! [sbc]
			_executeInPlaceADC(cpu, operand ^ BYTE_MAX);
			//! [sbc]
			return;

		// STA
		case v6502_opcode_sta_zpg:
		case v6502_opcode_sta_zpgx:
		case v6502_opcode_sta_abs:
		case v6502_opcode_sta_absx:
		case v6502_opcode_sta_absy:
		case v6502_opcode_sta_indx:
		case v6502_opcode_sta_indy:
			//! [sta]
			v6502_write(cpu->memory, ref, cpu->ac);
			//! [sta]
			return;
			
		// STX
		case v6502_opcode_stx_zpg:
		case v6502_opcode_stx_zpgy:
		case v6502_opcode_stx_abs:
			//! [stx]
			v6502_write(cpu->memory, ref, cpu->x);
			//! [stx]
			return;
			
		// STY
		case v6502_opcode_sty_zpg:
		case v6502_opcode_sty_zpgx:
		case v6502_opcode_sty_abs:
			//! [sty]
			v6502_write(cpu->memory, ref, cpu->y);
			//! [sty]
			return;
			
		// Failure
		default: {
			if (cpu->fault_callback) {
				cpu->fault_callback(cpu->fault_context, v6502_unhandledInstructionErrorText);
			}
		} return;
	}
}
