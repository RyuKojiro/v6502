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

#include <string.h>

#include <as6502/error.h>
#include <as6502/debug.h>

#include "reverse.h"

/** @brief The maximum allowed buffer size for symbol names */
#define MAX_SYMBOL_LEN			256
#define MAX_INSTRUCTION_LEN		32

int dis6502_printAnnotatedInstruction(FILE *out, v6502_cpu *cpu, uint16_t address, as6502_symbol_table *table) {
	// Bytes
	v6502_opcode opcode = v6502_read(cpu->memory, address, NO);
	uint8_t low = v6502_read(cpu->memory, address + 1, NO);
	uint8_t high = v6502_read(cpu->memory, address + 2, NO);

	// Text
	char instruction[MAX_INSTRUCTION_LEN];
	dis6502_stringForInstruction(instruction, MAX_INSTRUCTION_LEN, opcode, high, low);

	// Symbolicate
	if (table) {
		as6502_symbolicateLine(table, instruction, MAX_INSTRUCTION_LEN, 0, address);

		// If we've got a symbol table, and this also happens to be a label site, print that too
		as6502_symbol *symbol = as6502_symbolForAddress(table, address);
		if (symbol) {
			as6502_printAnnotatedLabel(out, symbol->address, symbol->name, symbol->line);
		}
	}

	// Print
	as6502_printAnnotatedInstruction(out, address, opcode, low, high, instruction);

	return v6502_instructionLengthForOpcode(opcode);
}

int dis6502_isBranchOpcode(v6502_opcode opcode) {
	switch (opcode) {
		case v6502_opcode_bcc:
		case v6502_opcode_bcs:
		case v6502_opcode_beq:
		case v6502_opcode_bmi:
		case v6502_opcode_bne:
		case v6502_opcode_bpl:
		case v6502_opcode_bvc:
		case v6502_opcode_bvs:
		case v6502_opcode_jmp_abs:
		case v6502_opcode_jmp_ind:
		case v6502_opcode_jsr:
			return YES;
		default:
			return NO;
	}
}

void dis6502_stringForOpcode(char *string, size_t len, v6502_opcode opcode) {
	switch (opcode) {
		case v6502_opcode_brk:
			strncpy(string, "brk", len);
			return;
		case v6502_opcode_nop:
			strncpy(string, "nop", len);
			return;
		case v6502_opcode_clc:
			strncpy(string, "clc", len);
			return;
		case v6502_opcode_cld:
			strncpy(string, "cld", len);
			return;
		case v6502_opcode_cli:
			strncpy(string, "cli", len);
			return;
		case v6502_opcode_clv:
			strncpy(string, "clv", len);
			return;
		case v6502_opcode_sec:
			strncpy(string, "sec", len);
			return;
		case v6502_opcode_sed:
			strncpy(string, "sed", len);
			return;
		case v6502_opcode_sei:
			strncpy(string, "sei", len);
			return;
		case v6502_opcode_dex:
			strncpy(string, "dex", len);
			return;
		case v6502_opcode_dey:
			strncpy(string, "dey", len);
			return;
		case v6502_opcode_tax:
			strncpy(string, "tax", len);
			return;
		case v6502_opcode_tay:
			strncpy(string, "tay", len);
			return;
		case v6502_opcode_tsx:
			strncpy(string, "tsx", len);
			return;
		case v6502_opcode_txa:
			strncpy(string, "txa", len);
			return;
		case v6502_opcode_txs:
			strncpy(string, "txs", len);
			return;
		case v6502_opcode_tya:
			strncpy(string, "tya", len);
			return;
		case v6502_opcode_inx:
			strncpy(string, "inx", len);
			return;
		case v6502_opcode_iny:
			strncpy(string, "iny", len);
			return;
		case v6502_opcode_bcc:
			strncpy(string, "bcc", len);
			return;
		case v6502_opcode_bcs:
			strncpy(string, "bcs", len);
			return;
		case v6502_opcode_beq:
			strncpy(string, "beq", len);
			return;
		case v6502_opcode_bne:
			strncpy(string, "bne", len);
			return;
		case v6502_opcode_bmi:
			strncpy(string, "bmi", len);
			return;
		case v6502_opcode_bpl:
			strncpy(string, "bpl", len);
			return;
		case v6502_opcode_bvc:
			strncpy(string, "bvc", len);
			return;
		case v6502_opcode_bvs:
			strncpy(string, "bvs", len);
			return;
		case v6502_opcode_jsr:
			strncpy(string, "jsr", len);
			return;
		case v6502_opcode_rti:
			strncpy(string, "rti", len);
			return;
		case v6502_opcode_rts:
			strncpy(string, "rts", len);
			return;
		case v6502_opcode_pha:
			strncpy(string, "pha", len);
			return;
		case v6502_opcode_pla:
			strncpy(string, "pla", len);
			return;
		case v6502_opcode_php:
			strncpy(string, "php", len);
			return;
		case v6502_opcode_plp:
			strncpy(string, "plp", len);
			return;
		case v6502_opcode_adc_imm:
		case v6502_opcode_adc_zpg:
		case v6502_opcode_adc_zpgx:
		case v6502_opcode_adc_abs:
		case v6502_opcode_adc_absx:
		case v6502_opcode_adc_absy:
		case v6502_opcode_adc_indx:
		case v6502_opcode_adc_indy:
			strncpy(string, "adc", len);
			return;
		case v6502_opcode_and_imm:
		case v6502_opcode_and_zpg:
		case v6502_opcode_and_zpgx:
		case v6502_opcode_and_abs:
		case v6502_opcode_and_absx:
		case v6502_opcode_and_absy:
		case v6502_opcode_and_indx:
		case v6502_opcode_and_indy:
			strncpy(string, "and", len);
			return;
		case v6502_opcode_asl_acc:
		case v6502_opcode_asl_zpg:
		case v6502_opcode_asl_zpgx:
		case v6502_opcode_asl_abs:
		case v6502_opcode_asl_absx:
			strncpy(string, "asl", len);
			return;
		case v6502_opcode_bit_zpg:
		case v6502_opcode_bit_abs:
			strncpy(string, "bit", len);
			return;
		case v6502_opcode_cmp_imm:
		case v6502_opcode_cmp_zpg:
		case v6502_opcode_cmp_zpgx:
		case v6502_opcode_cmp_abs:
		case v6502_opcode_cmp_absx:
		case v6502_opcode_cmp_absy:
		case v6502_opcode_cmp_indx:
		case v6502_opcode_cmp_indy:
			strncpy(string, "cmp", len);
			return;
		case v6502_opcode_cpx_imm:
		case v6502_opcode_cpx_zpg:
		case v6502_opcode_cpx_abs:
			strncpy(string, "cpx", len);
			return;
		case v6502_opcode_cpy_imm:
		case v6502_opcode_cpy_zpg:
		case v6502_opcode_cpy_abs:
			strncpy(string, "cpy", len);
			return;
		case v6502_opcode_dec_zpg:
		case v6502_opcode_dec_zpgx:
		case v6502_opcode_dec_abs:
		case v6502_opcode_dec_absx:
			strncpy(string, "dec", len);
			return;
		case v6502_opcode_eor_imm:
		case v6502_opcode_eor_zpg:
		case v6502_opcode_eor_zpgx:
		case v6502_opcode_eor_abs:
		case v6502_opcode_eor_absx:
		case v6502_opcode_eor_absy:
		case v6502_opcode_eor_indx:
		case v6502_opcode_eor_indy:
			strncpy(string, "eor", len);
			return;
		case v6502_opcode_inc_zpg:
		case v6502_opcode_inc_zpgx:
		case v6502_opcode_inc_abs:
		case v6502_opcode_inc_absx:
			strncpy(string, "inc", len);
			return;
		case v6502_opcode_jmp_abs:
		case v6502_opcode_jmp_ind:
			strncpy(string, "jmp", len);
			return;
		case v6502_opcode_ora_imm:
		case v6502_opcode_ora_zpg:
		case v6502_opcode_ora_zpgx:
		case v6502_opcode_ora_abs:
		case v6502_opcode_ora_absx:
		case v6502_opcode_ora_absy:
		case v6502_opcode_ora_indx:
		case v6502_opcode_ora_indy:
			strncpy(string, "ora", len);
			return;
		case v6502_opcode_lda_imm:
		case v6502_opcode_lda_zpg:
		case v6502_opcode_lda_zpgx:
		case v6502_opcode_lda_abs:
		case v6502_opcode_lda_absx:
		case v6502_opcode_lda_absy:
		case v6502_opcode_lda_indx:
		case v6502_opcode_lda_indy:
			strncpy(string, "lda", len);
			return;
		case v6502_opcode_ldx_imm:
		case v6502_opcode_ldx_zpg:
		case v6502_opcode_ldx_zpgy:
		case v6502_opcode_ldx_abs:
		case v6502_opcode_ldx_absy:
			strncpy(string, "ldx", len);
			return;
		case v6502_opcode_ldy_imm:
		case v6502_opcode_ldy_zpg:
		case v6502_opcode_ldy_zpgx:
		case v6502_opcode_ldy_abs:
		case v6502_opcode_ldy_absx:
			strncpy(string, "ldy", len);
			return;
		case v6502_opcode_lsr_acc:
		case v6502_opcode_lsr_zpg:
		case v6502_opcode_lsr_zpgx:
		case v6502_opcode_lsr_abs:
		case v6502_opcode_lsr_absx:
			strncpy(string, "lsr", len);
			return;
		case v6502_opcode_rol_acc:
		case v6502_opcode_rol_zpg:
		case v6502_opcode_rol_zpgx:
		case v6502_opcode_rol_abs:
		case v6502_opcode_rol_absx:
			strncpy(string, "rol", len);
			return;
		case v6502_opcode_ror_acc:
		case v6502_opcode_ror_zpg:
		case v6502_opcode_ror_zpgx:
		case v6502_opcode_ror_abs:
		case v6502_opcode_ror_absx:
			strncpy(string, "ror", len);
			return;
		case v6502_opcode_sbc_imm:
		case v6502_opcode_sbc_zpg:
		case v6502_opcode_sbc_zpgx:
		case v6502_opcode_sbc_abs:
		case v6502_opcode_sbc_absx:
		case v6502_opcode_sbc_absy:
		case v6502_opcode_sbc_indx:
		case v6502_opcode_sbc_indy:
			strncpy(string, "sbc", len);
			return;
		case v6502_opcode_sta_zpg:
		case v6502_opcode_sta_zpgx:
		case v6502_opcode_sta_abs:
		case v6502_opcode_sta_absx:
		case v6502_opcode_sta_absy:
		case v6502_opcode_sta_indx:
		case v6502_opcode_sta_indy:
			strncpy(string, "sta", len);
			return;
		case v6502_opcode_stx_zpg:
		case v6502_opcode_stx_zpgy:
		case v6502_opcode_stx_abs:
			strncpy(string, "stx", len);
			return;
		case v6502_opcode_sty_zpg:
		case v6502_opcode_sty_zpgx:
		case v6502_opcode_sty_abs:
			strncpy(string, "sty", len);
			return;
		case v6502_opcode_wai:
			strncpy(string, "wai", len);
			return;
	}

	strncpy(string, "???", len);
}

void dis6502_stringForOperand(char *string, size_t len, v6502_address_mode mode, uint8_t high, uint8_t low) {
	switch (mode) {
		case v6502_address_mode_accumulator: {
			strncpy(string, "A", len);
		} return;
		case v6502_address_mode_implied: {
			if (len) {
				string[0] = '\0';
			}
		} return;
		case v6502_address_mode_immediate: {
			snprintf(string, len, "#$%02x", low);
		} return;
		case v6502_address_mode_zeropage: {
			snprintf(string, len, "*$%02x", low);
		} return;
		case v6502_address_mode_zeropage_x: {
			snprintf(string, len, "*$%02x,X", low);
		} return;
		case v6502_address_mode_zeropage_y: {
			snprintf(string, len, "*$%02x,Y", low);
		} return;
		case v6502_address_mode_relative: {
			snprintf(string, len, "$%02x", low);
		} return;
		case v6502_address_mode_absolute: {
			snprintf(string, len, "$%02x%02x", high, low);
		} return;
		case v6502_address_mode_absolute_x: {
			snprintf(string, len, "$%02x%02x,X", high, low);
		} return;
		case v6502_address_mode_absolute_y: {
			snprintf(string, len, "$%02x%02x,Y", high, low);
		} return;
		case v6502_address_mode_indirect: {
			snprintf(string, len, "($%02x%02x)", high, low);
		} return;
		case v6502_address_mode_indirect_x: {
			snprintf(string, len, "($%02x,X)", low);
		} return;
		case v6502_address_mode_indirect_y: {
			snprintf(string, len, "($%02x),Y", low);
		} return;
		case v6502_address_mode_symbol:
		case v6502_address_mode_unknown:
		default:
			return;
	}
}

void dis6502_stringForInstruction(char *string, size_t len, v6502_opcode opcode, uint8_t high, uint8_t low) {
	dis6502_stringForOpcode(string, len, opcode);
	string[3] = ' ';
	string += 4;
	len -= 4;
	dis6502_stringForOperand(string, len, v6502_addressModeForOpcode(opcode), high, low);
}

void dis6502_deriveSymbolsForObjectBlob(as6502_symbol_table *table, ld6502_object_blob *blob) {
	char symbolName[MAX_SYMBOL_LEN];
	int currentLabel = 1;
	uint16_t address;

	for (uint16_t offset = 0; offset < blob->len; offset += v6502_instructionLengthForOpcode(blob->data[offset])) {
		v6502_opcode opcode = blob->data[offset];
		if (dis6502_isBranchOpcode(opcode)) {
			if (opcode == v6502_opcode_jmp_abs || opcode == v6502_opcode_jmp_ind || opcode == v6502_opcode_jsr) {
				address = ((blob->data[offset + 2] << 8 | blob->data[offset + 1]) - blob->start);
			}
			else {
				address = offset + 2 + v6502_signedValueOfByte(blob->data[offset + 1]);
			}

			if (!as6502_symbolForAddress(table, address)) {
				snprintf(symbolName, MAX_SYMBOL_LEN, "Label%d", currentLabel++);
				as6502_addSymbolToTable(table, currentLineNum, symbolName, address + blob->start, as6502_symbol_type_label);
			}
		}
		currentLineNum++;
	}
}

void dis6502_deriveSymbolsForObject(ld6502_object *object) {
	if(!object->table) {
		object->table = as6502_createSymbolTable();
	}

	for (int i = 0; i < object->count; i++) {
		dis6502_deriveSymbolsForObjectBlob(object->table, &object->blobs[i]);
	}
}
