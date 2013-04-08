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

#include "parser.h"
#include "core.h"

#define YES		1
#define NO		0

#define kBadAddressModeErrorText		"Bad address mode for operation"
#define kUnknownAddressModeErrorText	"Unknown address mode for operation"

static v6502_opcode _opError(const char *op, const char *error) {
	char e[80];
	strncpy(e, error, 73);
	strncat(e, " - ", 3);
	strncat(e, op, 3);
	v6502_fault(e);
	return v6502_opcode_nop;
}

v6502_opcode v6502_opcodeForStringAndMode(const char *string, v6502_address_mode mode) {
	// Single-byte Instructions
	if (!strncmp(string, "brk", 3)) {
		return v6502_opcode_brk;
	}
	if (!strncmp(string, "nop", 3)) {
		return v6502_opcode_nop;
	}
	if (!strncmp(string, "clc", 3)) {
		return v6502_opcode_clc;
	}
	if (!strncmp(string, "cld", 3)) {
		return v6502_opcode_cld;
	}
	if (!strncmp(string, "cli", 3)) {
		return v6502_opcode_cli;
	}
	if (!strncmp(string, "clv", 3)) {
		return v6502_opcode_clv;
	}
	if (!strncmp(string, "sec", 3)) {
		return v6502_opcode_sec;
	}
	if (!strncmp(string, "sed", 3)) {
		return v6502_opcode_sed;
	}
	if (!strncmp(string, "sei", 3)) {
		return v6502_opcode_sei;
	}
	if (!strncmp(string, "dex", 3)) {
		return v6502_opcode_dex;
	}
	if (!strncmp(string, "dey", 3)) {
		return v6502_opcode_dey;
	}
	if (!strncmp(string, "tax", 3)) {
		return v6502_opcode_tax;
	}
	if (!strncmp(string, "tay", 3)) {
		return v6502_opcode_tay;
	}
	if (!strncmp(string, "tsx", 3)) {
		return v6502_opcode_tsx;
	}
	if (!strncmp(string, "txa", 3)) {
		return v6502_opcode_txa;
	}
	if (!strncmp(string, "txs", 3)) {
		return v6502_opcode_txs;
	}
	if (!strncmp(string, "tya", 3)) {
		return v6502_opcode_dey;
	}
	if (!strncmp(string, "inx", 3)) {
		return v6502_opcode_inx;
	}
	if (!strncmp(string, "iny", 3)) {
		return v6502_opcode_iny;
	}
	
	// Branching Instructions
	if (!strncmp(string, "bcc", 3)) {
		if (mode == v6502_address_mode_relative) {
			return v6502_opcode_bcc;
		}
		else {
			return _opError(string, kBadAddressModeErrorText);
		}
	}
	
	// All of the rest
	if (!strncmp(string, "adc", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "and", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "asl", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "dec", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "eor", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "ora", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "jmp", 3)) {
		switch (mode) {
			case v6502_address_mode_absolute:
				return v6502_opcode_jmp_abs;
			case v6502_address_mode_indirect:
				return v6502_opcode_jmp_ind;
			default:
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "lda", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "ldx", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "ldy", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "lsr", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "rol", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "ror", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "sbc", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "sta", 3)) {
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
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "stx", 3)) {
		switch (mode) {
			case v6502_address_mode_zeropage:
				return v6502_opcode_stx_zpg;
			case v6502_address_mode_zeropage_y:
				return v6502_opcode_stx_zpgy;
			case v6502_address_mode_absolute:
				return v6502_opcode_stx_abs;
			default:
				return _opError(string, kBadAddressModeErrorText);
		}
	}
	if (!strncmp(string, "sty", 3)) {
		switch (mode) {
			case v6502_address_mode_zeropage:
				return v6502_opcode_sty_zpg;
			case v6502_address_mode_zeropage_x:
				return v6502_opcode_sty_zpgx;
			case v6502_address_mode_absolute:
				return v6502_opcode_sty_abs;
			default:
				return _opError(string, kBadAddressModeErrorText);
		}
	}

	char exception[50];
	snprintf(exception, 50, "Unknown Opcode - %s", string);
	v6502_fault(exception);
	return v6502_opcode_nop;
}

static int _valueLengthInChars(const char *string) {
	int i;
	for (i = 0; string[i] && (isdigit(string[i]) || (string[i] >= 'a' && string[i] <= 'f')); i++);
	
	return i;
}

void v6502_valueForString(uint8_t *high, uint8_t *low, int *wide, const char *string) {
	char workString[6];
	uint16_t result;
	int base;
	
	// Remove all whitespace, #'s, *'s, high ascii, and parenthesis, also, truncate at comma
	int i = 0;
	for (const char *cur = string; *cur; cur++) {
		if (!isspace(*cur) && *cur != '#' && *cur !='*' && *cur !='(' && *cur !=')' && *cur < 0x7F) {
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
			base = 16;
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 2) ? YES : NO;
			}
		} break;
		case '%': { // Binary
			base = 2;
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 8) ? YES : NO;
			}
		} break;
		case '0': { // Octal
			base = 8;
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 3) ? YES : NO;
			}
		} break;
		default: { // Decimal
			base = 10;
			if (wide) {
				*wide = (_valueLengthInChars(workString + 1) > 3) ? YES : NO;
			}
		} break;
	}
	
	result = strtol(workString + 1, NULL, base);

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

static v6502_address_mode _incrementModeByFoundRegister(v6502_address_mode mode, const char *string) {
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

v6502_address_mode v6502_addressModeForLine(const char *string) {
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

	// Skip opcode and whitespace to find first argument
	for (cur = string + 3; isspace(*cur) || *cur > 0x7F; cur++) {
		if (*cur == '\0' || *cur == ';') {
			return v6502_address_mode_implied;
		}
	}
	
	// Check first character of argument, and byte length
	switch (*cur) {
		case 'a': // Accumulator (normalized)
			return v6502_address_mode_accumulator;
		case '#': // Immediate
			return v6502_address_mode_immediate;
		case '*': { // Zeropage
			return _incrementModeByFoundRegister(v6502_address_mode_zeropage, cur);
		} break;
		case '(': { // Indirect
			return _incrementModeByFoundRegister(v6502_address_mode_indirect, cur);
		} break;
		default: { // Relative or Absolute
			// TODO: Better byte length determination, this doesn't tell shit
			v6502_valueForString(NULL, NULL, &wide, cur);
			if (wide) {
				return _incrementModeByFoundRegister(v6502_address_mode_absolute, cur);
			}
			else {
				return v6502_address_mode_relative;
			}
		} break;
	}
	
	_opError(string, kUnknownAddressModeErrorText);
	return -1;
}

void v6502_executeAsmLineOnCPU(v6502_cpu *cpu, const char *line, size_t len) {
	v6502_address_mode mode;
	v6502_opcode opcode;
	uint8_t low, high;
	char string[len];
	
	// Normalize text (all lowercase) and copy into a non-const string
	// Perhaps this should collapse whitespace as well?
	int i;
	for(i = 0; line[i]; i++){
		string[i] = tolower(line[i]);
	}
	string[i] = '\0';
	
	// Determine address mode
	mode = v6502_addressModeForLine(string);
	
	/* TODO: Make none of this rely on the operation being the first 3 chars every time */
	// Determine opcode, based on entire line
	opcode = v6502_opcodeForStringAndMode(string, mode);
	
	// Determine operands
	v6502_valueForString(&high, &low, NULL, string + 3);
	
	// Execute whole instruction
	v6502_execute(cpu, opcode, low, high);
}
