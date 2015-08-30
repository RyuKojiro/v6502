//
//  main.c
//  kmapgen
//
//  Created by Daniel Loffgren on 8/30/15.
//  Copyright (c) 2015 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <v6502/cpu.h>
#include <dis6502/reverse.h>

#define OPCODE_STRING_LEN	4
#define MODE_STRING_LEN		6

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
			out[0] = '\0';
			break;
	}
}

void generateTable(FILE *out) {
	char opcodeString[OPCODE_STRING_LEN];
	char modeString[MODE_STRING_LEN];
	
	fprintf(out, "<table border=\"1\" cellspacing=\"0\" cellpadding=\"5\" class=\"opctable\">\n");
	
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
			
			fprintf(out, "<td nowrap>%s %s</td>", opcodeString, modeString);
		}
		fprintf(out, "</tr>\n");
	}
	fprintf(out, "</table>");
}

void generateAllTables(FILE *out) {
	fprintf(out, "<html><body>\n");
	
	// Address Mode
	generateTable(out);
	
	// Instruction Length
	
	// Opcode
	
	fprintf(out, "</html></body>\n");
}

int main(int argc, const char * argv[]) {
	FILE *out = fopen("test.html", "w");
	generateAllTables(out);
	fclose(out);
    return 0;
}
