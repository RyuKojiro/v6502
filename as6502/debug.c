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

#include "debug.h"

void as6502_printAnnotatedLabel(FILE *out, uint16_t address, const char *text, unsigned long line) {
	const int target = 20;
	const int fixed = sizeof("0xff: ff ff ff - ") - 1;
	const int printed = fprintf(out, "%#06x:          - %s:", address, text);
	if (line) {
		const int spaces = target - (printed - fixed);
		fprintf(out, "%*s ; line %lu", spaces > 0 ? spaces : 0, "", line);
	}
	fprintf(out, "\n");
}

void as6502_printAnnotatedInstruction(FILE *out, uint16_t address, v6502_opcode opcode, uint8_t low, uint8_t high, const char *text) {
	fprintf(out, "%#06x: ", address);

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

	fprintf(out, " -    %s\n", text);
}
