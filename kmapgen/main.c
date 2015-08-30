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

void generateTable(FILE *out) {
	char opcodeString[OPCODE_STRING_LEN];
	
	fprintf(out, "<html><body>\n");
	fprintf(out, "<table border=\"1\" cellspacing=\"0\" cellpadding=\"5\" class=\"opctable\">\n");
	for (uint8_t high = 0; high < 0x10; high++) {
		fprintf(out, "<tr valign=\"top\"><td nowrap=\"\">%2x</td>", high << 4);
		for (uint8_t low = 0; low < 0x10; low++) {
			uint8_t opcode = ((high << 4) | low);
			dis6502_stringForOpcode(opcodeString, OPCODE_STRING_LEN, opcode);
			fprintf(out, "<td nowrap>%s</td>", opcodeString);
		}
		fprintf(out, "</tr>\n");
	}
	fprintf(out, "</table>");
	fprintf(out, "</html></body>\n");
}


int main(int argc, const char * argv[]) {
	generateTable(stdout);
    return 0;
}
