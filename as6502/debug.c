//
//  debug.c
//  as6502
//
//  Created by Daniel Loffgren on 1/30/19.
//  Copyright © 2019 Daniel Loffgren. All rights reserved.
//

#include <stdio.h>

#include "debug.h"

void as6502_printAnnotatedInstruction(FILE *out, uint16_t address, v6502_opcode opcode, uint8_t low, uint8_t high, char *text) {
	fprintf(out, "%#04x: ", address);

	switch (v6502_instructionLengthForOpcode(opcode)) {
		case 1: {
			fprintf(out, "%02x      ", opcode);
		} break;
		case 2: {
			fprintf(out, "%02x %02x   ", opcode, low);
		} break;
		case 3: {
			fprintf(out, "%02x %02x %02x", opcode, low, high);
		} break;
		default: {
			fprintf(out, "        ");
		} break;
	}

	fprintf(out, " - %s\n", text);
}
