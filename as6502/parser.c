//
//  parser.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/30.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "linectl.h"
#include "parser.h"
#include "core.h"

#define MIN(a, b)	((a < b) ? a : b)

/* v6502_opcodeForStringAndMode is a huge function with very repetetive behavior.
 * In order to alleviate a lot of linear calls to strncmp(), asmeq() was created.
 * Much faster than strncmp, slower than a jump table?
 */
#define asmeq(a, b) ((a[0] == b[0] && a[1] == b[1] && a[2] == b[2]) ? YES : NO)

#define MAX_ERROR_LEN					255

#define kBadAddressModeErrorText		"Address mode '"
#define kForOperationErrorText			"' invalid for operation '"
#define kInvalidOpcodeErrorText			"Invalid opcode '"
#define kUnknownSymbolErrorText			"Unknown symbol for operation '"

static v6502_opcode _addrModeError(const char *op, as6502_address_mode mode) {
	char e[MAX_ERROR_LEN];
	char m[12];
	int depth = 0;
	
	if (mode == as6502_address_mode_symbol) {
		strncpy(e, kUnknownSymbolErrorText, MIN(sizeof(kUnknownSymbolErrorText), MAX_ERROR_LEN - depth));
		depth += sizeof(kUnknownSymbolErrorText);
	}
	else {
		as6502_stringForAddressMode(m, mode);
		strncpy(e, kBadAddressModeErrorText, MIN(sizeof(kBadAddressModeErrorText), MAX_ERROR_LEN - depth));
		depth += sizeof(kBadAddressModeErrorText);
		strncat(e, m, MIN(12, MAX_ERROR_LEN - depth));
		depth += 12;
		strncat(e, kForOperationErrorText, MIN(sizeof(kForOperationErrorText), MAX_ERROR_LEN - depth));
		depth += sizeof(kForOperationErrorText);
	}
	strncat(e, op, MIN(strlen(op) + 1, MAX_ERROR_LEN - depth));
	trimtaild(e);
	strncat(e, "'", 2);
	v6502_fault(e);
	return v6502_opcode_nop;
}

static v6502_opcode _opError(const char *op, const char *error) {
	char e[MAX_ERROR_LEN];	
	int depth = 0;
	
	strncpy(e, error, MIN(strlen(error) + 1, MAX_ERROR_LEN - depth));
	depth += sizeof(kForOperationErrorText);
	strncat(e, op, MIN(strlen(op), MAX_ERROR_LEN - depth));
	trimtaild(e);
	strncat(e, "'", 2);
	v6502_fault(e);
	return v6502_opcode_nop;
}

void as6502_stringForAddressMode(char *out, as6502_address_mode mode) {
	switch (mode) {
		case as6502_address_mode_accumulator: {
			strncpy(out, "accumulator", 12);
		} return;
		case as6502_address_mode_implied: {
			strncpy(out, "implied", 8);
		} return;
		case as6502_address_mode_zeropage: {
			strncpy(out, "zeropage", 9);
		} return;
		case as6502_address_mode_indirect: {
			strncpy(out, "indirect", 9);
		} return;
		case as6502_address_mode_relative: {
			strncpy(out, "relative", 9);
		} return;
		case as6502_address_mode_immediate: {
			strncpy(out, "immediate", 10);
		} return;
		case as6502_address_mode_zeropage_x: {
			strncpy(out, "zeropage+x", 11);
		} return;
		case as6502_address_mode_zeropage_y: {
			strncpy(out, "zeropage+y", 11);
		} return;
		case as6502_address_mode_absolute: {
			strncpy(out, "absolute", 9);
		} return;
		case as6502_address_mode_absolute_x: {
			strncpy(out, "absolute+y", 11);
		} return;
		case as6502_address_mode_absolute_y: {
			strncpy(out, "absolute+y", 11);
		} return;
		case as6502_address_mode_indirect_x: {
			strncpy(out, "indirect+y", 11);
		} return;
		case as6502_address_mode_indirect_y: {
			strncpy(out, "indirect+y", 11);
		} return;
		case as6502_address_mode_unknown:
		default:
			strncpy(out, "unknown", 8);
			break;
	}
}

v6502_opcode as6502_opcodeForStringAndMode(const char *string, as6502_address_mode mode) {
	if (strlen(string) < 3) {
		return _opError(string, kInvalidOpcodeErrorText);
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
		if (mode == as6502_address_mode_relative) {
			return v6502_opcode_bcc;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bcs")) {
		if (mode == as6502_address_mode_relative) {
			return v6502_opcode_bcs;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "beq")) {
		if (mode == as6502_address_mode_relative) {
			return v6502_opcode_beq;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bne")) {
		if (mode == as6502_address_mode_relative) {
			return v6502_opcode_bne;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bmi")) {
		if (mode == as6502_address_mode_relative) {
			return v6502_opcode_bmi;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bpl")) {
		if (mode == as6502_address_mode_relative) {
			return v6502_opcode_bpl;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bvc")) {
		if (mode == as6502_address_mode_relative) {
			return v6502_opcode_bvc;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bvs")) {
		if (mode == as6502_address_mode_relative) {
			return v6502_opcode_bvs;
		}
		else {
			return _addrModeError(string, mode);
		}
	}
	
	// All of the rest
	if (asmeq(string, "adc")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_adc_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_adc_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_adc_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_adc_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_adc_absx;
			case as6502_address_mode_absolute_y:
				return v6502_opcode_adc_absy;
			case as6502_address_mode_indirect_x:
				return v6502_opcode_adc_indx;
			case as6502_address_mode_indirect_y:
				return v6502_opcode_adc_indy;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "and")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_and_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_and_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_and_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_and_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_and_absx;
			case as6502_address_mode_absolute_y:
				return v6502_opcode_and_absy;
			case as6502_address_mode_indirect_x:
				return v6502_opcode_and_indx;
			case as6502_address_mode_indirect_y:
				return v6502_opcode_and_indy;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "asl")) {
		switch (mode) {
			case as6502_address_mode_accumulator:
				return v6502_opcode_asl_acc;
			case as6502_address_mode_zeropage:
				return v6502_opcode_asl_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_asl_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_asl_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_asl_absx;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "cmp")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_cmp_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_cmp_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_cmp_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_cmp_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_cmp_absx;
			case as6502_address_mode_absolute_y:
				return v6502_opcode_cmp_absy;
			case as6502_address_mode_indirect_x:
				return v6502_opcode_cmp_indx;
			case as6502_address_mode_indirect_y:
				return v6502_opcode_cmp_indy;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "bit")) {
		switch (mode) {
			case as6502_address_mode_zeropage:
				return v6502_opcode_bit_zpg;
			case as6502_address_mode_absolute:
				return v6502_opcode_bit_abs;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "cpx")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_cpx_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_cpx_zpg;
			case as6502_address_mode_absolute:
				return v6502_opcode_cpx_abs;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "cpy")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_cpy_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_cpy_zpg;
			case as6502_address_mode_absolute:
				return v6502_opcode_cpy_abs;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "dec")) {
		switch (mode) {
			case as6502_address_mode_zeropage:
				return v6502_opcode_dec_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_dec_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_dec_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_dec_absx;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "eor")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_eor_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_eor_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_eor_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_eor_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_eor_absx;
			case as6502_address_mode_absolute_y:
				return v6502_opcode_eor_absy;
			case as6502_address_mode_indirect_x:
				return v6502_opcode_eor_indx;
			case as6502_address_mode_indirect_y:
				return v6502_opcode_eor_indy;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "ora")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_ora_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_ora_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_ora_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_ora_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_ora_absx;
			case as6502_address_mode_absolute_y:
				return v6502_opcode_ora_absy;
			case as6502_address_mode_indirect_x:
				return v6502_opcode_ora_indx;
			case as6502_address_mode_indirect_y:
				return v6502_opcode_ora_indy;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "inc")) {
		switch (mode) {
			case as6502_address_mode_zeropage:
				return v6502_opcode_inc_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_inc_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_inc_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_inc_absx;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "jmp")) {
		switch (mode) {
			case as6502_address_mode_absolute:
				return v6502_opcode_jmp_abs;
			case as6502_address_mode_indirect:
				return v6502_opcode_jmp_ind;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "lda")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_lda_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_lda_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_lda_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_lda_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_lda_absx;
			case as6502_address_mode_absolute_y:
				return v6502_opcode_lda_absy;
			case as6502_address_mode_indirect_x:
				return v6502_opcode_lda_indx;
			case as6502_address_mode_indirect_y:
				return v6502_opcode_lda_indy;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "ldx")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_ldx_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_ldx_zpg;
			case as6502_address_mode_zeropage_y:
				return v6502_opcode_ldx_zpgy;
			case as6502_address_mode_absolute:
				return v6502_opcode_ldx_abs;
			case as6502_address_mode_absolute_y:
				return v6502_opcode_ldx_absy;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "ldy")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_ldy_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_ldy_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_ldy_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_ldy_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_ldy_absx;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "lsr")) {
		switch (mode) {
			case as6502_address_mode_accumulator:
				return v6502_opcode_lsr_acc;
			case as6502_address_mode_zeropage:
				return v6502_opcode_lsr_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_lsr_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_lsr_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_lsr_absx;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "rol")) {
		switch (mode) {
			case as6502_address_mode_accumulator:
				return v6502_opcode_rol_acc;
			case as6502_address_mode_zeropage:
				return v6502_opcode_rol_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_rol_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_rol_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_rol_absx;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "ror")) {
		switch (mode) {
			case as6502_address_mode_accumulator:
				return v6502_opcode_ror_acc;
			case as6502_address_mode_zeropage:
				return v6502_opcode_ror_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_ror_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_ror_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_ror_absx;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "sbc")) {
		switch (mode) {
			case as6502_address_mode_immediate:
				return v6502_opcode_sbc_imm;
			case as6502_address_mode_zeropage:
				return v6502_opcode_sbc_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_sbc_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_sbc_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_sbc_absx;
			case as6502_address_mode_absolute_y:
				return v6502_opcode_sbc_absy;
			case as6502_address_mode_indirect_x:
				return v6502_opcode_sbc_indx;
			case as6502_address_mode_indirect_y:
				return v6502_opcode_sbc_indy;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "sta")) {
		switch (mode) {
			case as6502_address_mode_zeropage:
				return v6502_opcode_sta_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_sta_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_sta_abs;
			case as6502_address_mode_absolute_x:
				return v6502_opcode_sta_absx;
			case as6502_address_mode_absolute_y:
				return v6502_opcode_sta_absy;
			case as6502_address_mode_indirect_x:
				return v6502_opcode_sta_indx;
			case as6502_address_mode_indirect_y:
				return v6502_opcode_sta_indy;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "stx")) {
		switch (mode) {
			case as6502_address_mode_zeropage:
				return v6502_opcode_stx_zpg;
			case as6502_address_mode_zeropage_y:
				return v6502_opcode_stx_zpgy;
			case as6502_address_mode_absolute:
				return v6502_opcode_stx_abs;
			default:
				return _addrModeError(string, mode);
		}
	}
	if (asmeq(string, "sty")) {
		switch (mode) {
			case as6502_address_mode_zeropage:
				return v6502_opcode_sty_zpg;
			case as6502_address_mode_zeropage_x:
				return v6502_opcode_sty_zpgx;
			case as6502_address_mode_absolute:
				return v6502_opcode_sty_abs;
			default:
				return _addrModeError(string, mode);
		}
	}

	return _opError(string, kInvalidOpcodeErrorText);
}

static int _valueLengthInChars(const char *string) {
	int i;
	for (i = 0; string[i] && (isdigit(string[i]) || (string[i] >= 'a' && string[i] <= 'f')); i++);
	
	return i;
}

void as6502_valueForString(uint8_t *high, uint8_t *low, int *wide, const char *string) {
	char workString[80];
	uint16_t result;
	
	// Remove all whitespace, #'s, *'s, high ascii, and parenthesis, also, truncate at comma
	int i = 0;
	for (const char *cur = string; *cur; cur++) {
		if (!isspace(*cur) && *cur != '#' && *cur !='*' && *cur !='(' && *cur !=')' && *cur < 0x7F && *cur != ';') {
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
	
	if (result > 0xFF && wide) {
		// Octal and decimal split digits
		*wide = YES;
	}
	if (low) {
		*low = result & 0xFF;
	}

	if (high) {
		*high = result >> 8;
	}
}

static as6502_address_mode _incrementModeByFoundRegister(as6502_address_mode mode, const char *string) {
	/* This relies on the fact that the enum is always ordered in normal, x, y. */
	const char *cur;
	
	cur = strchr(string, 'x');
	if (cur) {
		return mode + 1;
	}
	
	cur = strchr(string, 'y');
	if (cur) {
		return mode + 2;
	}
	
	return mode;
}

static int _isEndOfString(const char *c) {
	for (/* c */; *c; c++) {
		if (!isspace(*c)) {
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

as6502_address_mode as6502_addressModeForLine(const char *string) {
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
		v6502_fault("Cannot determine address mode for null string");
		return as6502_address_mode_unknown;
	}
	
	// Skip opcode and whitespace to find first argument
	for (cur = string + 3; isspace(*cur); cur++) {
		if (*cur == '\0' || *cur == ';') {
			return as6502_address_mode_implied;
		}
	}
	
	// Check first character of argument, and byte length
	switch (*cur) {
		case 'a': { // Accumulator (normalized)
			if (isalnum(*(cur + 1))) {
				return as6502_address_mode_symbol;
			}
			else {
				return as6502_address_mode_accumulator;
			}
		}
		case '#': // Immediate
			return as6502_address_mode_immediate;
		case '*': // Zeropage
			return _incrementModeByFoundRegister(as6502_address_mode_zeropage, cur);
		case '(': // Indirect
			return _incrementModeByFoundRegister(as6502_address_mode_indirect, cur);
		default: { // Relative, Absolute, or Implied
			// TODO: Better byte length determination, this doesn't tell shit
			as6502_valueForString(NULL, NULL, &wide, cur);
			if (wide) {
				return _incrementModeByFoundRegister(as6502_address_mode_absolute, cur);
			}
			else {
				if ( (as6502_isDigit(cur[0]) && as6502_isDigit(cur[1]) && as6502_isDigit(cur[2])) || // Octal
					(cur[0] == '$' && as6502_isDigit(cur[1]) && as6502_isDigit(cur[2])) ) { // Hex
					return as6502_address_mode_relative;
				}
				else {
					if (_isEndOfString(cur)) {
						return as6502_address_mode_implied;
					}
					else {
						if (!as6502_isDigit(*cur)) {
							return as6502_address_mode_symbol;
						}
						else {
							return as6502_address_mode_unknown;							
						}
					}
				}
			}
		}
	}
}

int as6502_instructionLengthForAddressMode(as6502_address_mode mode) {
	switch (mode) {
		case as6502_address_mode_accumulator:
		case as6502_address_mode_implied:
			return 1;
		case as6502_address_mode_immediate:
		case as6502_address_mode_zeropage:
		case as6502_address_mode_zeropage_x:
		case as6502_address_mode_zeropage_y:
		case as6502_address_mode_relative:
			return 2;
		case as6502_address_mode_absolute:
		case as6502_address_mode_absolute_x:
		case as6502_address_mode_absolute_y:
		case as6502_address_mode_indirect:
		case as6502_address_mode_indirect_x:
		case as6502_address_mode_indirect_y:
			return 3;
		case as6502_address_mode_symbol:
		case as6502_address_mode_unknown:
			return 0;
	}
}

void as6502_instructionForLine(uint8_t *opcode, uint8_t *low, uint8_t *high, as6502_address_mode *mode, const char *line, size_t len) {
	char *string = malloc(len);

	// Use stack if required storage is not passed in
	if (!mode) {
		as6502_address_mode _mode;
		mode = &_mode;
	}

	if (!opcode) {
		uint8_t _opcode;
		opcode = &_opcode;
	}
	
	// Normalize text (all lowercase,) trim leading whitespace, and copy into a non-const string, all in one shot (mangling text?)
	size_t i = 0;
	int o = 0;
	int charEncountered = NO;
	for(; line[i] && i < len; i++){
		if (!isspace(line[i]) || charEncountered) {
			charEncountered = YES;
			string[o++] = tolower(line[i]);
		}
	}
	string[o] = '\0';
	
	// Determine address mode
	*mode = as6502_addressModeForLine(string);
	
	/* TODO: Make none of this rely on the operation being the first 3 chars every time */
	// Determine opcode, based on entire line
	*opcode = as6502_opcodeForStringAndMode(string, *mode);
	
	// Determine operands
	if (as6502_instructionLengthForAddressMode(*mode) > 1) {
		as6502_valueForString(high, low, NULL, string + 3);
	}
	
	free(string);
}

void as6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len) {
	uint8_t opcode, low, high;
	as6502_instructionForLine(&opcode, &low, &high, NULL, line, len);
	v6502_execute(cpu, opcode, low, high);
}
