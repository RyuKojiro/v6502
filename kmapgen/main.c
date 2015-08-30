//
//  main.c
//  kmapgen
//
//  Created by Daniel Loffgren on 8/30/15.
//  Copyright (c) 2015 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <v6502/cpu.h>

void generateTable(FILE *out) {
	fprintf(out, "<table border=\"1\" cellspacing=\"0\" cellpadding=\"5\" class=\"opctable\">");
	for (uint8_t high = 0; high < 0x10; high++) {
		fprintf(out, "<tr valign=\"top\"><td nowrap=\"\">%2x</td>", high << 4);
		for (uint8_t low = 0; low < 0x10; low++) {
//			<#statements#>
		}
		fprintf(out, "</tr>");
	}
	fprintf(out, "</table>");
}


int main(int argc, const char * argv[]) {
	generateTable(stdout);
    return 0;
}
