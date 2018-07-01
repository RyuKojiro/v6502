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

#include <stdio.h>
#include <v6502/cpu.h>
#include <dis6502/reverse.h>

#define OPCODE_STRING_LEN	4
#define MODE_STRING_LEN		6

#define HTML_BGCOLOR		"bgcolor="
#define HTML_RED			"#FF91A4"	// Salmon
#define HTML_BLUE			"#A4DDED"	// Non-photo Blue
#define HTML_GREEN			"#7FFF00"	// Chartreuse
#define HTML_YELLOW			"#FFC000"	// Sodium-vapor Lamp
#define HTML_PURPLE			"#DF73FF"	// Heliotrope
#define HTML_BLACK			"#000000"

typedef const char *(kmapCallback)(v6502_opcode opcode);

#define _instructionIsUnknown(opcode)	(v6502_addressModeForOpcode(opcode) == v6502_address_mode_unknown)

void _shortStringForAddressMode(char *out, size_t len, v6502_address_mode mode) {
	switch (mode) {
		case v6502_address_mode_accumulator: {
			strncpy(out, "A", len);
		} return;
		case v6502_address_mode_implied: {
			strncpy(out, "imp", len);
		} return;
		case v6502_address_mode_zeropage: {
			strncpy(out, "zpg", len);
		} return;
		case v6502_address_mode_indirect: {
			strncpy(out, "ind", len);
		} return;
		case v6502_address_mode_relative: {
			strncpy(out, "rel", len);
		} return;
		case v6502_address_mode_immediate: {
			strncpy(out, "#", len);
		} return;
		case v6502_address_mode_zeropage_x: {
			strncpy(out, "zpg,X", len);
		} return;
		case v6502_address_mode_zeropage_y: {
			strncpy(out, "zpg,Y", len);
		} return;
		case v6502_address_mode_absolute: {
			strncpy(out, "abs", len);
		} return;
		case v6502_address_mode_absolute_x: {
			strncpy(out, "abs,X", len);
		} return;
		case v6502_address_mode_absolute_y: {
			strncpy(out, "abs,Y", len);
		} return;
		case v6502_address_mode_indirect_x: {
			strncpy(out, "X,ind", len);
		} return;
		case v6502_address_mode_indirect_y: {
			strncpy(out, "ind,Y", len);
		} return;
		case v6502_address_mode_symbol: {
			strncpy(out, "sym", len);
		} return;
		case v6502_address_mode_unknown:
		default:
			strncpy(out, "---", len);
			break;
	}
}

void generateMap(FILE *out, kmapCallback colorizer, const char *title) {
	char opcodeString[OPCODE_STRING_LEN];
	char modeString[MODE_STRING_LEN];

	if (title) {
		fprintf(out, "<h1>%s</h1>\n", title);
	}
	fprintf(out, "<table border=\"1\" cellspacing=\"0\" cellpadding=\"3\" class=\"optable\">\n");

	// Print low nibble header
	fprintf(out, "<tr valign=\"top\"><td nowrap=\"\">&nbsp;</td>");
	for (uint8_t low = 0; low < 0x10; low++) {
		fprintf(out, "<td nowrap>%02x</td>", low);
	}
	fprintf(out, "</tr>\n");

	// Print the entire table
	for (uint8_t high = 0; high < 0x10; high++) {
		fprintf(out, "<tr valign=\"top\"><td nowrap=\"\">%02x</td>", high << 4);
		for (uint8_t low = 0; low < 0x10; low++) {
			uint8_t opcode = ((high << 4) | low);
			v6502_address_mode mode = v6502_addressModeForOpcode(opcode);

			dis6502_stringForOpcode(opcodeString, OPCODE_STRING_LEN, opcode);
			_shortStringForAddressMode(modeString, MODE_STRING_LEN, mode);

			fprintf(out, "<td nowrap %s>%s %s</td>", colorizer ? colorizer(opcode) : "", opcodeString, modeString);
		}
		fprintf(out, "</tr>\n");
	}
	fprintf(out, "</table>");
}

#define addressModeByRegisterCount 4

static const char *addressModeByRegisterColors[] = {
	HTML_BGCOLOR HTML_GREEN,
	HTML_BGCOLOR HTML_YELLOW,
	HTML_BGCOLOR HTML_RED,
	HTML_BGCOLOR HTML_BLUE
};

static const char *addressModeByRegisterLabels[] = {
	"Accumulator",
	"Non-Register",
	"X Register",
	"Y Register"
};

const char *addressModeByRegisterCallback(v6502_opcode opcode) {
	switch (v6502_addressModeForOpcode(opcode)) {
		case v6502_address_mode_accumulator:
			return HTML_BGCOLOR HTML_GREEN;
		case v6502_address_mode_implied:
		case v6502_address_mode_zeropage:
		case v6502_address_mode_indirect:
		case v6502_address_mode_relative:
		case v6502_address_mode_immediate:
		case v6502_address_mode_absolute:
			return HTML_BGCOLOR HTML_YELLOW;
		case v6502_address_mode_zeropage_x:
		case v6502_address_mode_absolute_x:
		case v6502_address_mode_indirect_x:
			return HTML_BGCOLOR HTML_RED;
		case v6502_address_mode_zeropage_y:
		case v6502_address_mode_absolute_y:
		case v6502_address_mode_indirect_y:
			return HTML_BGCOLOR HTML_BLUE;
		case v6502_address_mode_symbol:
		case v6502_address_mode_unknown:
		default:
			return HTML_BGCOLOR HTML_BLACK;
	}
}

#define addressModeByOperandCount 5

static const char *addressModeByOperandColors[] = {
	HTML_BGCOLOR HTML_GREEN,
	HTML_BGCOLOR HTML_YELLOW,
	HTML_BGCOLOR HTML_PURPLE,
	HTML_BGCOLOR HTML_RED,
	HTML_BGCOLOR HTML_BLUE
};

static const char *addressModeByOperandLabels[] = {
	"Accumulator/Implied",
	"Zeropage",
	"Relative/Immediate",
	"Absolute",
	"Indirect"
};

const char *addressModeByOperandCallback(v6502_opcode opcode) {
	switch (v6502_addressModeForOpcode(opcode)) {
		case v6502_address_mode_implied:
		case v6502_address_mode_accumulator:
			return HTML_BGCOLOR HTML_GREEN;
		case v6502_address_mode_zeropage:
		case v6502_address_mode_zeropage_y:
		case v6502_address_mode_zeropage_x:
			return HTML_BGCOLOR HTML_YELLOW;
		case v6502_address_mode_relative:
		case v6502_address_mode_immediate:
			return HTML_BGCOLOR HTML_PURPLE;
		case v6502_address_mode_absolute:
		case v6502_address_mode_absolute_x:
		case v6502_address_mode_absolute_y:
			return HTML_BGCOLOR HTML_RED;
		case v6502_address_mode_indirect:
		case v6502_address_mode_indirect_y:
		case v6502_address_mode_indirect_x:
			return HTML_BGCOLOR HTML_BLUE;
		case v6502_address_mode_symbol:
		case v6502_address_mode_unknown:
		default:
			return HTML_BGCOLOR HTML_BLACK;
	}
}

#define instructionLengthCount 3

static const char *instructionLengthColors[] = {
	HTML_BGCOLOR HTML_GREEN,
	HTML_BGCOLOR HTML_YELLOW,
	HTML_BGCOLOR HTML_RED
};

static const char *instructionLengthLabels[] = {
	"1 byte",
	"2 bytes",
	"3 bytes"
};

const char *instructionLengthCallback(v6502_opcode opcode) {
	switch (v6502_instructionLengthForOpcode(opcode)) {
		case 1:
			return HTML_BGCOLOR HTML_GREEN;
		case 2:
			return HTML_BGCOLOR HTML_YELLOW;
		case 3:
			return HTML_BGCOLOR HTML_RED;
		default:
			return HTML_BGCOLOR HTML_BLACK;
	}
}

#define instructionTypeCount 6

static const char *instructionTypeColors[] = {
	HTML_BGCOLOR HTML_GREEN,
	HTML_BGCOLOR HTML_BLUE,
	HTML_BGCOLOR HTML_RED,
	HTML_BGCOLOR HTML_PURPLE,
	HTML_BGCOLOR HTML_YELLOW,
	""
};

static const char *instructionTypeLabels[] = {
	"Branch Instruction",
	"Load Instruction",
	"Store Instruction",
	"Transfer Instruction",
	"Special Instruction",
	"Arithmetic Instruction"
};


const char *instructionTypeCallback(v6502_opcode opcode) {
	if (_instructionIsUnknown(opcode)) {
		return HTML_BGCOLOR HTML_BLACK;
	}

	if (dis6502_isBranchOpcode(opcode)) {
		return HTML_BGCOLOR HTML_GREEN;
	}

	switch (opcode) {
		case v6502_opcode_lda_imm:
		case v6502_opcode_lda_zpg:
		case v6502_opcode_lda_zpgx:
		case v6502_opcode_lda_abs:
		case v6502_opcode_lda_absx:
		case v6502_opcode_lda_absy:
		case v6502_opcode_lda_indx:
		case v6502_opcode_lda_indy:
		case v6502_opcode_ldx_imm:
		case v6502_opcode_ldx_zpg:
		case v6502_opcode_ldx_zpgy:
		case v6502_opcode_ldx_abs:
		case v6502_opcode_ldx_absy:
		case v6502_opcode_ldy_imm:
		case v6502_opcode_ldy_zpg:
		case v6502_opcode_ldy_zpgx:
		case v6502_opcode_ldy_abs:
		case v6502_opcode_ldy_absx:
			return HTML_BGCOLOR HTML_BLUE;
		case v6502_opcode_sta_zpg:
		case v6502_opcode_sta_zpgx:
		case v6502_opcode_sta_abs:
		case v6502_opcode_sta_absx:
		case v6502_opcode_sta_absy:
		case v6502_opcode_sta_indx:
		case v6502_opcode_sta_indy:
		case v6502_opcode_stx_zpg:
		case v6502_opcode_stx_zpgy:
		case v6502_opcode_stx_abs:
		case v6502_opcode_sty_zpg:
		case v6502_opcode_sty_zpgx:
		case v6502_opcode_sty_abs:
			return HTML_BGCOLOR HTML_RED;
		case v6502_opcode_tax:
		case v6502_opcode_tay:
		case v6502_opcode_tsx:
		case v6502_opcode_txa:
		case v6502_opcode_txs:
		case v6502_opcode_tya:
			return HTML_BGCOLOR HTML_PURPLE;
		case v6502_opcode_brk:
		case v6502_opcode_nop:
		case v6502_opcode_wai:
			return HTML_BGCOLOR HTML_YELLOW;
		default:
			return "";
	}
}

void generateLegend(FILE *out, const char *colors[], const char *labels[], int count) {
	fprintf(out, "<p><table border=\"1\" cellspacing=\"0\" cellpadding=\"3\">\n");

	fprintf(out, "<tr valign=\"top\">");
	for (int i = 0; i < count; i++) {
		fprintf(out, "<td nowrap %s>%s</td>", colors[i], labels[i]);
	}
	fprintf(out, "</tr>\n");
	fprintf(out, "</table></p>");
}

void generateAllMaps(FILE *out) {
	fprintf(out, "<html><head><style>table.optable{ border-style: solid; border-width: 1px; border-color: " HTML_BLACK "; font-family: courier,fixed,sans-serif; font-size: 10px; }</style></head><body>\n");

	// Standard Opcode Table
	generateMap(out, NULL, "Standard Opcode Table");

	// Instruction Length
	generateMap(out, instructionLengthCallback, "Instruction Length");
	generateLegend(out, instructionLengthColors, instructionLengthLabels, instructionLengthCount);

	// Instruction Type
	generateMap(out, instructionTypeCallback, "Instruction Type");
	generateLegend(out, instructionTypeColors, instructionTypeLabels, instructionTypeCount);

	// Opcode

	// Address Mode by Register
	generateMap(out, addressModeByRegisterCallback, "Address Mode by Register");
	generateLegend(out, addressModeByRegisterColors, addressModeByRegisterLabels, addressModeByRegisterCount);

	// Address Mode by Operand
	generateMap(out, addressModeByOperandCallback, "Address Mode by Operand");
	generateLegend(out, addressModeByOperandColors, addressModeByOperandLabels, addressModeByOperandCount);

	fprintf(out, "</html></body>\n");
}

int main(int argc, const char * argv[]) {
	FILE *out = stdout;
	if(argc > 1) { 
		out = fopen(argv[argc-1], "w");
	}
	generateAllMaps(out);
	fclose(out);
    return 0;
}
