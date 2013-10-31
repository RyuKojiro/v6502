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

#include "linectl.h"
#include "parser.h"
#include "error.h"	// as6502_error

#define kBadAddressModeErrorText		"Address mode '%s' invalid for operation '%s'"
#define kInvalidOpcodeFormatText		"Invalid opcode '%s'"
#define kUnknownSymbolErrorText			"Unknown symbol for operation '%s'"
#define kAddressModeNullStringErrorText	"Cannot determine address mode for null string"
#define kLineMallocErrorText			"Could not allocate work buffer for line"

static v6502_opcode _addrModeError(const char *op, v6502_address_mode mode) {
	char m[12];
	
	as6502_stringForAddressMode(m, mode);
	as6502_error(kBadAddressModeErrorText, m, op);

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

v6502_opcode as6502_opcodeForStringAndMode(const char *string, v6502_address_mode mode) {
	if (strlen(string) < 3) {
		as6502_error(kInvalidOpcodeFormatText, string);
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
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bcs")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bcs;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "beq")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_beq;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bne")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bne;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bmi")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bmi;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bpl")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bpl;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bvc")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bvc;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bvs")) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bvs;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	
	// If it's an unresolved symbol, might as well not go any further
	if (mode == v6502_address_mode_symbol) {
		as6502_error(kUnknownSymbolErrorText, string);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bit")) {
		switch (mode) {
			case v6502_address_mode_zeropage:
				return v6502_opcode_bit_zpg;
			case v6502_address_mode_absolute:
				return v6502_opcode_bit_abs;
			default:
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "jmp")) {
		switch (mode) {
			case v6502_address_mode_absolute:
				return v6502_opcode_jmp_abs;
			case v6502_address_mode_indirect:
				return v6502_opcode_jmp_ind;
			default:
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
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
				return _addrModeError(string, mode);
		}
	}

	as6502_error(kInvalidOpcodeFormatText, string);
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
	int i = 0;
	for (const char *cur = string; *cur; cur++) {
		if (!isspace(CTYPE_CAST *cur) && *cur != '#' && *cur !='*' && *cur !='(' && *cur !=')' && *cur < 0x7F && *cur != ';') {
			workString[i++] = *cur;
		}
		if (*cur == ',') {
			break;
		}
	}
	workString[i] = '\0';
	
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
		case '0': { // Octal
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 3) ? YES : NO;
			}
			result = strtol(workString, NULL, 8);
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

static v6502_address_mode _incrementModeByFoundRegister(v6502_address_mode mode, const char *string) {
	/* This relies on the fact that the enum is always ordered in normal, x, y. */
	const char *cur;
	
	cur = strchr(string, ',');
	
	if (!cur) {
		return mode;
	}
	
	// Get the letter after the comma
	cur++;
	
	if (*cur == 'x' || *cur == 'X') {
		return mode + 1;
	}
	
	if (*cur == 'y' || *cur == 'Y') {
		return mode + 2;
	}
	
	return mode;
}

static int _isEndOfString(const char *c) {
	for (/* c */; *c; c++) {
		if (!isspace(CTYPE_CAST *c)) {
			return NO;
		}
	}
	return YES;
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

int _isBranchInstruction(const char *string) {
	if (string[0] == 'b' || string[0] == 'B') {
		if (!asmeq(string, "bit")) {
			return YES;
		}
	}
	
	return NO;
}

v6502_address_mode as6502_addressModeForLine(const char *string, size_t len) {
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
	
	const char *cur;
	int wide;

	if (!string) {
		as6502_error(kAddressModeNullStringErrorText);
		return v6502_address_mode_unknown;
	}
	
	// In case it wasn't trimmed beforehand
	string = trimhead(string, len);
	
	if (!string[0]) {
		return v6502_address_mode_unknown;
	}

	// Skip opcode and whitespace to find first argument
	for (cur = string + 3; isspace(CTYPE_CAST *cur); cur++) {
		if (*cur == '\0' || *cur == ';') {
			return v6502_address_mode_implied;
		}
	}
	
	// Check first character of argument, and byte length
	switch (*cur) {
		case 'A':
		case 'a': { // Accumulator (normalized)
			if (isalnum(CTYPE_CAST *(cur + 1))) {
				// For symbols, check to see if it is a branch instruction, if so, relative, if not, absolute
				if (_isBranchInstruction(string)) {
					return v6502_address_mode_relative;
				}
				else {
					return _incrementModeByFoundRegister(v6502_address_mode_absolute, cur);
				}
			}
			else {
				return v6502_address_mode_accumulator;
			}
		}
		case '#': // Immediate
			return v6502_address_mode_immediate;
		case '*': // Zeropage
			return _incrementModeByFoundRegister(v6502_address_mode_zeropage, cur);
		case '(': // Indirect
			return _incrementModeByFoundRegister(v6502_address_mode_indirect, cur);
		default: { // Relative, Absolute, or Implied
			/** TODO: @todo Better byte length determination, this doesn't tell shit */
			as6502_byteValuesForString(NULL, NULL, &wide, cur);
			if (wide) {
				return _incrementModeByFoundRegister(v6502_address_mode_absolute, cur);
			}
			else {
				if (as6502_isNumber(cur)) {
					return v6502_address_mode_relative;
				}
				else {
					if (_isEndOfString(cur)) {
						return v6502_address_mode_implied;
					}
					else {
						if (!as6502_isNumber(cur)) {
							if (_isBranchInstruction(string)) {
								return v6502_address_mode_relative;
							}
							else {
								return _incrementModeByFoundRegister(v6502_address_mode_absolute, cur);
							}
						}
						else {
							return v6502_address_mode_unknown;							
						}
					}
				}
			}
		}
	}
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

void as6502_instructionForLine(uint8_t *opcode, uint8_t *low, uint8_t *high, v6502_address_mode *mode, const char *line, size_t len) {
	char *string = malloc(len + 1); // Malloc an extra char in case the passed in len does not include a null
	if (!string) {
		as6502_error(kLineMallocErrorText);
		return;
	}
	
	// Use stack if required storage is not passed in
	if (!mode) {
		v6502_address_mode _mode;
		mode = &_mode;
	}

	if (!opcode) {
		uint8_t _opcode;
		opcode = &_opcode;
	}
	
	// Normalize text (all lowercase,) trim leading whitespace, and copy into a non-const string, all in one shot
	int o = 0;
	int charEncountered = NO;
	for(size_t i = 0; line[i] && i <= len; i++){
		if (!isspace(CTYPE_CAST line[i]) || charEncountered) {
			charEncountered = YES;
			string[o++] = tolower(line[i]);
		}
	}
	string[o] = '\0';
	
	// Determine address mode
	*mode = as6502_addressModeForLine(string, len);
	
	/* TODO: Make none of this rely on the operation being the first 3 chars every time */
	// Determine opcode, based on entire line
	*opcode = as6502_opcodeForStringAndMode(string, *mode);
	
	// Determine operands
	if (as6502_instructionLengthForAddressMode(*mode) > 1) {
		as6502_byteValuesForString(high, low, NULL, string + 4);
	}
	
	free(string);
}

void as6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len) {
	uint8_t opcode, low, high;
	as6502_instructionForLine(&opcode, &low, &high, NULL, line, len);
	v6502_execute(cpu, opcode, low, high);
}
