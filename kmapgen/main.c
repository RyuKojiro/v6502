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

#define HTML_BGCOLOR		"bgcolor="
#define HTML_RED			"#FF91A4"	// Salmon
#define HTML_BLUE			"#A4DDED"	// Non-photo Blue
#define HTML_GREEN			"#7FFF00"	// Chartreuse
#define HTML_YELLOW			"#FFC000"	// Sodium-vapor Lamp
#define HTML_PURPLE			"#DF73FF"	// Heliotrope
#define HTML_BLACK			"#000000"

typedef const char *(kmapCallback)(v6502_opcode opcode);

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
	fprintf(out, "<table border=\"1\" cellspacing=\"0\" cellpadding=\"3\">\n");
	
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

void generateAllMaps(FILE *out) {
	fprintf(out, "<html><head><style>table{ border-style: solid; border-width: 1px; border-color: " HTML_BLACK "; font-family: courier,fixed,sans-serif; font-size: 10px; }</style></head><body>\n");
	
	// Instruction Length
	generateMap(out, instructionLengthCallback, "Instruction Length");

	// Opcode
	
	// Address Mode by Register
	generateMap(out, addressModeByRegisterCallback, "Address Mode by Register");
	
	// Address Mode by Operand
	generateMap(out, addressModeByOperandCallback, "Address Mode by Operand");
	
	fprintf(out, "</html></body>\n");
}

int main(int argc, const char * argv[]) {
	FILE *out = fopen("test.html", "w");
	generateAllMaps(out);
	fclose(out);
    return 0;
}
