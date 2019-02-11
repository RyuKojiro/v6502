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

#include "perf.h"
#include "brute.h"
#include "time.h"

#include <stdio.h>

#define HTML_BGCOLOR		"bgcolor="

const char *perfColors[] = {
	HTML_BGCOLOR "#00FFFF", // Cyan
	HTML_BGCOLOR "#FF00FF", // Magenta
};

const char *perfLabels[] = {
	"Fastest",
	"Slowest",
};

static struct timespec max, min;
static struct timespec bruteModePerfTable[256];
static struct timespec bruteLengthPerfTable[256];
static struct timespec optModePerfTable[256];
static struct timespec optLengthPerfTable[256];

#define PERF_RUN_COUNT 100000

typedef int (*opcodeInFunction)(v6502_opcode opcode);

static void buildTable(opcodeInFunction function, struct timespec *table) {
	for (v6502_opcode opcode = 0; opcode <= 0xFF; opcode++) {
		struct timespec start, end, diff;
		clock_gettime(CLOCK_MONOTONIC, &start);
		for(int i = 0; i < PERF_RUN_COUNT; i++) {
			(void)function(opcode);
		}
		clock_gettime(CLOCK_MONOTONIC, &end);

		diff = timespec_subtract(&end, &start);
		table[opcode] = diff;

		if (timespec_compare(&diff, &max)) {
			max = diff;
		}

		if (timespec_compare(&min, &diff)) {
			min = diff;
		}
	}
}

void buildPerfTables(void) {
	buildTable(bruteForce_addressModeForOpcode, bruteModePerfTable);
	buildTable(v6502_addressModeForOpcode, optModePerfTable);
	buildTable(bruteForce_instructionLengthForOpcode, bruteLengthPerfTable);
	buildTable(v6502_instructionLengthForOpcode, optLengthPerfTable);
}

static char temporaryColorBuffer[] = HTML_BGCOLOR "#0000FF";

static const char *colorForOpcodeInTable(v6502_opcode opcode, struct timespec *table) {
	const struct timespec diff = table[opcode];
	const short hex = (diff.tv_nsec * 255) / max.tv_nsec; // Assume all runs are sub-second

	snprintf(temporaryColorBuffer + sizeof(HTML_BGCOLOR), 7, "%02X%02X%02X", hex, 0xFF-hex, 0xFF);
	return temporaryColorBuffer;
}

const char *bruteModePerfCallback(v6502_opcode opcode) {
	return colorForOpcodeInTable(opcode, bruteModePerfTable);
}

const char *optModePerfCallback(v6502_opcode opcode) {
	return colorForOpcodeInTable(opcode, optModePerfTable);
}

const char *bruteLengthPerfCallback(v6502_opcode opcode) {
	return colorForOpcodeInTable(opcode, bruteLengthPerfTable);
}

const char *optLengthPerfCallback(v6502_opcode opcode) {
	return colorForOpcodeInTable(opcode, optLengthPerfTable);
}
