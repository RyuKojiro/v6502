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

#include "brute.h"

v6502_address_mode bruteForce_addressModeForOpcode(v6502_opcode opcode) {
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
		case v6502_opcode_wai:
		case v6502_opcode_pha:
		case v6502_opcode_pla:
		case v6502_opcode_php:
		case v6502_opcode_plp:
			return v6502_address_mode_implied;
		case v6502_opcode_bcc:
		case v6502_opcode_bcs:
		case v6502_opcode_beq:
		case v6502_opcode_bne:
		case v6502_opcode_bmi:
		case v6502_opcode_bpl:
		case v6502_opcode_bvc:
		case v6502_opcode_bvs:
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

int bruteForce_instructionLengthForOpcode(v6502_opcode opcode) {
	switch (bruteForce_addressModeForOpcode(opcode)) {
		case v6502_address_mode_implied:
		case v6502_address_mode_accumulator:
			return 1;
		case v6502_address_mode_immediate:
		case v6502_address_mode_relative:
		case v6502_address_mode_zeropage:
		case v6502_address_mode_zeropage_x:
		case v6502_address_mode_zeropage_y:
		case v6502_address_mode_indirect_x:
		case v6502_address_mode_indirect_y:
			return 2;
		case v6502_address_mode_absolute:
		case v6502_address_mode_absolute_x:
		case v6502_address_mode_absolute_y:
		case v6502_address_mode_indirect:
			return 3;
		default:
			return 0;
	}
}
