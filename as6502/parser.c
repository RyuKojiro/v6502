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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "linectl.h"
#include "parser.h"
#include "error.h"	// as6502_error
#include "symbols.h"

#define v6502_BadAddressModeErrorText			"Address mode '%s' invalid for operation '%s'"
#define v6502_InvalidOpcodeFormatText			"Invalid opcode '%s'"
#define v6502_UnknownSymbolErrorText			"Unknown symbol for operation '%s'"

// FIXME: This is now a shim for the old asmeq macro, so that we can support case insensitivity
#define asmeq(a, b) (!strncasecmp(a, b, 3))


static v6502_opcode _addrModeError(as6502_token *instruction, v6502_address_mode mode) {
	char m[12];
	
	as6502_stringForAddressMode(m, mode);
	as6502_error(instruction->loc, instruction->len, v6502_BadAddressModeErrorText, m, instruction->text);

	return v6502_opcode_nop;
}

void as6502_stringForAddressMode(char *out, v6502_address_mode mode) {
	switch (mode) {
		case v6502_address_mode_accumulator: {
			strncpy(out, "accumulator", 12);
		} return;
		case v6502_address_mode_implied: {
			strncpy(out, "implied", 8);
		} return;
		case v6502_address_mode_zeropage: {
			strncpy(out, "zeropage", 9);
		} return;
		case v6502_address_mode_indirect: {
			strncpy(out, "indirect", 9);
		} return;
		case v6502_address_mode_relative: {
			strncpy(out, "relative", 9);
		} return;
		case v6502_address_mode_immediate: {
			strncpy(out, "immediate", 10);
		} return;
		case v6502_address_mode_zeropage_x: {
			strncpy(out, "zeropage+x", 11);
		} return;
		case v6502_address_mode_zeropage_y: {
			strncpy(out, "zeropage+y", 11);
		} return;
		case v6502_address_mode_absolute: {
			strncpy(out, "absolute", 9);
		} return;
		case v6502_address_mode_absolute_x: {
			strncpy(out, "absolute+x", 11);
		} return;
		case v6502_address_mode_absolute_y: {
			strncpy(out, "absolute+y", 11);
		} return;
		case v6502_address_mode_indirect_x: {
			strncpy(out, "indirect+x", 11);
		} return;
		case v6502_address_mode_indirect_y: {
			strncpy(out, "indirect+y", 11);
		} return;
		case v6502_address_mode_symbol: {
			strncpy(out, "symbol", 7);
		} return;
		case v6502_address_mode_unknown:
		default:
			strncpy(out, "unknown", 8);
			break;
	}
}

static void _badOpcode(const char *string) {
	size_t len = as6502_lengthOfToken(string, strlen(string));
	char *opcode = malloc(len + 1);
	strncpy(opcode, string, len);
	opcode[len] = '\0';
	as6502_error(0, len, v6502_InvalidOpcodeFormatText, opcode);
	free(opcode);
}

v6502_opcode as6502_opcodeForInstructionAndMode(as6502_token *instruction, v6502_address_mode mode) {
	const char *string = instruction->text;
	if (strlen(string) < 3) {
		_badOpcode(string);
		return v6502_opcode_nop;
	}
	
	// Single-byte Instructions
	if (asmeq(string, "brk")) {
		return v6502_opcode_brk;
	}
	if (asmeq(string, "nop")) {
		return v6502_opcode_nop;
	}
	if (asmeq(string, "clc")) {
		return v6502_opcode_clc;
	}
	if (asmeq(string, "cld")) {
		return v6502_opcode_cld;
	}
	if (asmeq(string, "cli")) {
		return v6502_opcode_cli;
	}
	if (asmeq(string, "clv")) {
		return v6502_opcode_clv;
	}
	if (asmeq(string, "sec")) {
		return v6502_opcode_sec;
	}
	if (asmeq(string, "sed")) {
		return v6502_opcode_sed;
	}
	if (asmeq(string, "sei")) {
		return v6502_opcode_sei;
	}
	if (asmeq(string, "dex")) {
		return v6502_opcode_dex;
	}
	if (asmeq(string, "dey")) {
		return v6502_opcode_dey;
	}
	if (asmeq(string, "tax")) {
		return v6502_opcode_tax;
	}
	if (asmeq(string, "tay")) {
		return v6502_opcode_tay;
	}
	if (asmeq(string, "tsx")) {
		return v6502_opcode_tsx;
	}
	if (asmeq(string, "txa")) {
		return v6502_opcode_txa;
	}
	if (asmeq(string, "txs")) {
		return v6502_opcode_txs;
	}
	if (asmeq(string, "tya")) {
		return v6502_opcode_dey;
	}
	if (asmeq(string, "inx")) {
		return v6502_opcode_inx;
	}
	if (asmeq(string, "iny")) {
		return v6502_opcode_iny;
	}
	
	// Stack Instructions
	if (asmeq(string, "jsr")) {
		return v6502_opcode_jsr;
	}
	if (asmeq(string, "rti")) {
		return v6502_opcode_rti;
	}
	if (asmeq(string, "rts")) {
		return v6502_opcode_rts;
	}
	if (asmeq(string, "pha")) {
		return v6502_opcode_pha;
	}
	if (asmeq(string, "pla")) {
		return v6502_opcode_pla;
	}
	if (asmeq(string, "php")) {
		return v6502_opcode_php;
	}
	if (asmeq(string, "plp")) {
		return v6502_opcode_plp;
	}
	
	// Branching Instructions
	if (asmeq(string, "bcc")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bcc;
		}
		else {
			return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "bcs")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bcs;
		}
		else {
			return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "beq")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_beq;
		}
		else {
			return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "bne")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bne;
		}
		else {
			return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "bmi")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bmi;
		}
		else {
			return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "bpl")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bpl;
		}
		else {
			return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "bvc")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bvc;
		}
		else {
			return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "bvs")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bvs;
		}
		else {
			return _addrModeError(instruction, mode);
		}
	}
	
	// If it's an unresolved symbol, might as well not go any further
	if (mode == v6502_address_mode_symbol) {
		as6502_error(0, strlen(string), v6502_UnknownSymbolErrorText, string);
		return v6502_opcode_nop;
	}
	
	// All of the rest
	if (asmeq(string, "adc")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_adc_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_adc_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_adc_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_adc_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_adc_absx;
			case v6502_address_mode_absolute_y:
				return v6502_opcode_adc_absy;
			case v6502_address_mode_indirect_x:
				return v6502_opcode_adc_indx;
			case v6502_address_mode_indirect_y:
				return v6502_opcode_adc_indy;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "and")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_and_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_and_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_and_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_and_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_and_absx;
			case v6502_address_mode_absolute_y:
				return v6502_opcode_and_absy;
			case v6502_address_mode_indirect_x:
				return v6502_opcode_and_indx;
			case v6502_address_mode_indirect_y:
				return v6502_opcode_and_indy;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "asl")) {
		switch (mode) {
			case v6502_address_mode_accumulator:
				return v6502_opcode_asl_acc;
			case v6502_address_mode_zeropage:
				return v6502_opcode_asl_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_asl_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_asl_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_asl_absx;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "cmp")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_cmp_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_cmp_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_cmp_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_cmp_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_cmp_absx;
			case v6502_address_mode_absolute_y:
				return v6502_opcode_cmp_absy;
			case v6502_address_mode_indirect_x:
				return v6502_opcode_cmp_indx;
			case v6502_address_mode_indirect_y:
				return v6502_opcode_cmp_indy;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "bit")) {
		switch (mode) {
			case v6502_address_mode_zeropage:
				return v6502_opcode_bit_zpg;
			case v6502_address_mode_absolute:
				return v6502_opcode_bit_abs;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "cpx")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_cpx_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_cpx_zpg;
			case v6502_address_mode_absolute:
				return v6502_opcode_cpx_abs;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "cpy")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_cpy_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_cpy_zpg;
			case v6502_address_mode_absolute:
				return v6502_opcode_cpy_abs;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "dec")) {
		switch (mode) {
			case v6502_address_mode_zeropage:
				return v6502_opcode_dec_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_dec_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_dec_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_dec_absx;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "eor")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_eor_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_eor_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_eor_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_eor_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_eor_absx;
			case v6502_address_mode_absolute_y:
				return v6502_opcode_eor_absy;
			case v6502_address_mode_indirect_x:
				return v6502_opcode_eor_indx;
			case v6502_address_mode_indirect_y:
				return v6502_opcode_eor_indy;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "ora")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_ora_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_ora_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_ora_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_ora_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_ora_absx;
			case v6502_address_mode_absolute_y:
				return v6502_opcode_ora_absy;
			case v6502_address_mode_indirect_x:
				return v6502_opcode_ora_indx;
			case v6502_address_mode_indirect_y:
				return v6502_opcode_ora_indy;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "inc")) {
		switch (mode) {
			case v6502_address_mode_zeropage:
				return v6502_opcode_inc_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_inc_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_inc_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_inc_absx;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "jmp")) {
		switch (mode) {
			case v6502_address_mode_absolute:
				return v6502_opcode_jmp_abs;
			case v6502_address_mode_indirect:
				return v6502_opcode_jmp_ind;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "lda")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_lda_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_lda_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_lda_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_lda_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_lda_absx;
			case v6502_address_mode_absolute_y:
				return v6502_opcode_lda_absy;
			case v6502_address_mode_indirect_x:
				return v6502_opcode_lda_indx;
			case v6502_address_mode_indirect_y:
				return v6502_opcode_lda_indy;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "ldx")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_ldx_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_ldx_zpg;
			case v6502_address_mode_zeropage_y:
				return v6502_opcode_ldx_zpgy;
			case v6502_address_mode_absolute:
				return v6502_opcode_ldx_abs;
			case v6502_address_mode_absolute_y:
				return v6502_opcode_ldx_absy;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "ldy")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_ldy_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_ldy_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_ldy_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_ldy_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_ldy_absx;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "lsr")) {
		switch (mode) {
			case v6502_address_mode_accumulator:
				return v6502_opcode_lsr_acc;
			case v6502_address_mode_zeropage:
				return v6502_opcode_lsr_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_lsr_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_lsr_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_lsr_absx;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "rol")) {
		switch (mode) {
			case v6502_address_mode_accumulator:
				return v6502_opcode_rol_acc;
			case v6502_address_mode_zeropage:
				return v6502_opcode_rol_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_rol_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_rol_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_rol_absx;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "ror")) {
		switch (mode) {
			case v6502_address_mode_accumulator:
				return v6502_opcode_ror_acc;
			case v6502_address_mode_zeropage:
				return v6502_opcode_ror_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_ror_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_ror_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_ror_absx;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "sbc")) {
		switch (mode) {
			case v6502_address_mode_immediate:
				return v6502_opcode_sbc_imm;
			case v6502_address_mode_zeropage:
				return v6502_opcode_sbc_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_sbc_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_sbc_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_sbc_absx;
			case v6502_address_mode_absolute_y:
				return v6502_opcode_sbc_absy;
			case v6502_address_mode_indirect_x:
				return v6502_opcode_sbc_indx;
			case v6502_address_mode_indirect_y:
				return v6502_opcode_sbc_indy;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "sta")) {
		switch (mode) {
			case v6502_address_mode_zeropage:
				return v6502_opcode_sta_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_sta_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_sta_abs;
			case v6502_address_mode_absolute_x:
				return v6502_opcode_sta_absx;
			case v6502_address_mode_absolute_y:
				return v6502_opcode_sta_absy;
			case v6502_address_mode_indirect_x:
				return v6502_opcode_sta_indx;
			case v6502_address_mode_indirect_y:
				return v6502_opcode_sta_indy;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "stx")) {
		switch (mode) {
			case v6502_address_mode_zeropage:
				return v6502_opcode_stx_zpg;
			case v6502_address_mode_zeropage_y:
				return v6502_opcode_stx_zpgy;
			case v6502_address_mode_absolute:
				return v6502_opcode_stx_abs;
			default:
				return _addrModeError(instruction, mode);
		}
	}
	if (asmeq(string, "sty")) {
		switch (mode) {
			case v6502_address_mode_zeropage:
				return v6502_opcode_sty_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_sty_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_sty_abs;
			default:
				return _addrModeError(instruction, mode);
		}
	}

	_badOpcode(string);
	return v6502_opcode_nop;
}

static int _valueLengthInChars(const char *string) {
	int i;
	for (i = 0; string[i] && (isdigit(CTYPE_CAST string[i]) || (string[i] >= 'a' && string[i] <= 'f')); i++);
	
	return i;
}

uint16_t as6502_valueForString(int *wide, const char *string) {
	char workString[80];
	uint16_t result;
	
	if (!string) {
		return 0;
	}
	
	// Remove all whitespace, #'s, *'s, high ascii, and parenthesis, also, truncate at comma
	const char *cur;
	for (cur = string; *cur; cur++) {
		if (!isspace(CTYPE_CAST *cur) && *cur != '#' && *cur !='*' && *cur !='(' && *cur !=')' && *cur < 0x7F && *cur != ';') {
			break;
		}
	}
	
	// Truncate to end of token
	size_t starter = 0;
	if (*cur == '$' || *cur == '%') {
		starter++;
	}

	size_t len = as6502_lengthOfToken(cur + starter, (80 - (cur - string)) - starter);
	strncpy(workString, cur, len + starter);
	workString[len + starter] = '\0';
	
	// Check first char to determine base
	switch (workString[0]) {
		case '$': { // Hex
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 2) ? YES : NO;
			}
			result = strtol(workString + 1, NULL, 16);
		} break;
		case '%': { // Binary
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 8) ? YES : NO;
			}
			result = strtol(workString + 1, NULL, 2);
		} break;
		case '0': {
			// Traditional Hex
			if (workString[1] == 'x') {
				if (wide) {
					*wide = (_valueLengthInChars(workString + 1) > 2) ? YES : NO;
				}
				result = strtol(workString + 2, NULL, 16);
			}
			// Octal
			else {
				if (wide) {
					*wide = (_valueLengthInChars(workString + 1) > 3) ? YES : NO;
				}
				result = strtol(workString, NULL, 8);
			}
		} break;
		default: { // Decimal
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 3) ? YES : NO;
			}
			result = strtol(workString, NULL, 10);
		} break;
	}
	
	if (result > BYTE_MAX && wide) {
		// Octal and decimal split digits
		*wide = YES;
	}
	
	return result;
}

void as6502_byteValuesForString(uint8_t *high, uint8_t *low, int *wide, const char *string) {
	uint16_t result = as6502_valueForString(wide, string);

	if (low) {
		*low = result & BYTE_MAX;
	}

	if (high) {
		*high = result >> 8;
	}
}

int as6502_isDigit(char c) {
	if ((c >= 'a' && c <= 'f') ||
		(c >= 'A' && c <= 'F') ||
		(c >= '0' && c <= '9')) {
		return YES;
	}
	return NO;
}

int as6502_isNumber(const char *c) {
	if (*c == '*') {
		c++;
	}

	// Hex
	if (c[0] == '$' && as6502_isDigit(c[2])) {
		return YES;
	}
	// Oct/Dec
	if ((c[0] >= '0' && c[0] <= '9')) {
		return YES;
	}
	return NO;
}

int as6502_isBranchInstruction(const char *string) {
	if (string[0] == 'b' || string[0] == 'B') {
		if (!asmeq(string, "bit")) {
			return YES;
		}
	}
	
	return NO;
}

v6502_address_mode as6502_addressModeForExpression(as6502_token *head) {
	/*
	 √ OPC			....	implied
	 √ OPC A		....	Accumulator
	 √ OPC #BB		....	immediate
	 √ OPC HHLL		....	absolute
	 √ OPC HHLL,X	....	absolute, X-indexed
	 √ OPC HHLL,Y	....	absolute, Y-indexed
	 √ OPC *LL		....	zeropage
	 √ OPC *LL,X	....	zeropage, X-indexed
	 √ OPC *LL,Y	....	zeropage, Y-indexed
	 √ OPC (BB,X)	....	X-indexed, indirect
	 √ OPC (LL),Y	....	indirect, Y-indexed
	 √ OPC (HHLL)	....	indirect
	 √ OPC BB		....	relative
	 */

	if (!head) {
		return v6502_address_mode_unknown;
	}

	if (!head->next) {
		return v6502_address_mode_implied;
	}

	if (as6502_tokenIsEqualToStringLiteral(head->next, "A") || as6502_tokenIsEqualToStringLiteral(head->next, "a")) {
		return v6502_address_mode_accumulator;
	}

	int wide;
	as6502_byteValuesForString(NULL, NULL, &wide, head->next->text);
	if (!wide) { // FIXME: Starting to doubt this logic
		wide = as6502_symbolShouldBeReplacedDoubleWidth(head);
	}

	int zpg = NO;
	as6502_token *value = as6502_firstTokenOfTypeInList(head, as6502_token_type_value);
	if (value && value->text[0] == '*') {
		zpg = YES;
	}

	if (as6502_tokenListFindTokenLiteral(head, ")")) {
		if (as6502_tokenListFindTokenLiteral(head, ",")) {
			if (as6502_tokenListFindTokenLiteral(head, "X") || as6502_tokenListFindTokenLiteral(head, "x")) {
				return v6502_address_mode_indirect_x;
			}
			else if (as6502_tokenListFindTokenLiteral(head, "Y") || as6502_tokenListFindTokenLiteral(head, "y")) {
				return v6502_address_mode_indirect_y;
			}
		}
		else {
			return v6502_address_mode_indirect;
		}
	}
	else {
		if (as6502_tokenListFindTokenLiteral(head, ",")) {
			if (as6502_tokenListFindTokenLiteral(head, "X") || as6502_tokenListFindTokenLiteral(head, "x")) {
				if (!zpg) {
					return v6502_address_mode_absolute_x;
				}
				else {
					return v6502_address_mode_zeropage_x;
				}
			}
			else if (as6502_tokenListFindTokenLiteral(head, "Y") || as6502_tokenListFindTokenLiteral(head, "y")) {
				if (!zpg) {
					return v6502_address_mode_absolute_y;
				}
				else {
					return v6502_address_mode_zeropage_y;
				}
			}
		}
		else {
			if (head->next->text[0] == '#') {
				return v6502_address_mode_immediate;
			}
			else if (head->next->text[0] == '*') {
				return v6502_address_mode_zeropage;
			}
			else if (head->next->text[0] == '#') {
				return v6502_address_mode_immediate;
			}
			else if (wide) {
				return v6502_address_mode_absolute;
			}
			else {
				return v6502_address_mode_relative;
			}
		}
	}

	return v6502_address_mode_unknown;
}

int as6502_instructionLengthForAddressMode(v6502_address_mode mode) {
	switch (mode) {
		case v6502_address_mode_accumulator:
		case v6502_address_mode_implied:
			return 1;
		case v6502_address_mode_immediate:
		case v6502_address_mode_zeropage:
		case v6502_address_mode_zeropage_x:
		case v6502_address_mode_zeropage_y:
		case v6502_address_mode_relative:
		case v6502_address_mode_indirect_x:
		case v6502_address_mode_indirect_y:
			return 2;
		case v6502_address_mode_absolute:
		case v6502_address_mode_absolute_x:
		case v6502_address_mode_absolute_y:
		case v6502_address_mode_indirect:
			return 3;
		case v6502_address_mode_symbol:
		case v6502_address_mode_unknown:
		default:
			return 0;
	}
}

void as6502_instructionForExpression(uint8_t *opcode, uint8_t *low, uint8_t *high, v6502_address_mode *mode, as6502_token *head) {
	// Use stack if required storage is not passed in
	if (!mode) {
		v6502_address_mode _mode;
		mode = &_mode;
	}

	if (!opcode) {
		uint8_t _opcode;
		opcode = &_opcode;
	}

	// Determine address mode
	*mode = as6502_addressModeForExpression(head);
	
	/* TODO: Make none of this rely on the operation being the first 3 chars every time */
	// Determine opcode, based on entire line
	*opcode = as6502_opcodeForInstructionAndMode(head, *mode);
	
	// Determine operands
	if (as6502_instructionLengthForAddressMode(*mode) > 1) {
		// We already know the address mode at this point, so we just want the actual value
		as6502_token *value = as6502_firstTokenOfTypeInList(head, as6502_token_type_value);
		if (value) {
			as6502_byteValuesForString(high, low, NULL, value->text);
		}
	}
}

void as6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len) {
	uint8_t opcode, low, high;

	as6502_token *head = as6502_lex(line, len);
	as6502_instructionForExpression(&opcode, &low, &high, NULL, head);
	as6502_tokenListDestroy(head);

	v6502_execute(cpu, opcode, low, high);
}
