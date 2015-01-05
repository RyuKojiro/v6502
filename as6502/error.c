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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "error.h"
#include "color.h"

unsigned long currentLineNum;
const char *currentFileName;
unsigned long lastProblematicLine;
unsigned long startOfProblem;
unsigned long lengthOfProblem;

__attribute((noreturn)) void as6502_fatal(const char *reason) {
	fprintf(stderr, "as6502: fatal: %s\n", reason);
	exit(EXIT_FAILURE);
}

static void _setProblemLocation(unsigned long loc, unsigned long len) {
	lastProblematicLine = currentLineNum;

	startOfProblem = loc;
	lengthOfProblem = len;
}

#pragma clang diagnostic ignored "-Wformat-nonliteral"
static void as6502_vlog(unsigned long line, unsigned long loc, unsigned long len, const char *color, const char *type, const char *reason, va_list ap) {
	_setProblemLocation(loc, len);
	
	// Only use color codes on real TTYs
	if (isatty(fileno(stdin))) {
		fprintf(stderr,
				ANSI_COLOR_BRIGHT_WHITE "%s:%lu:", currentFileName, line);
		
		if (len) {
			fprintf(stderr, "%lu:", loc);
		}
		
		fprintf(stderr,
				" %s%s: "
				ANSI_COLOR_BRIGHT_WHITE, color, type);
		vfprintf(stderr, reason, ap);
		
		fprintf(stderr, ANSI_COLOR_RESET);
	}
	else {
		fprintf(stderr, "%s:%lu: %s: ", currentFileName, line, type);
		vfprintf(stderr, reason, ap);
	}
	
	if (reason[strlen(reason)] != '\n') {
		fprintf(stderr, "\n");
	}
}
#pragma clang diagnostic warning "-Wformat-nonliteral"

void as6502_error(unsigned long loc, unsigned long len, const char *reason, ...) {
	va_list ap;
	va_start(ap, reason);
	as6502_vlog(currentLineNum, loc, len, ANSI_COLOR_BRIGHT_RED, "error", reason, ap);
	va_end(ap);
}

void as6502_warn(unsigned long loc, unsigned long len, const char *reason, ...) {
	va_list ap;
	va_start(ap, reason);
	as6502_vlog(currentLineNum, loc, len, ANSI_COLOR_BRIGHT_MAGENTA, "warning", reason, ap);
	va_end(ap);
}

void as6502_note(unsigned long lineNumber, const char *reason, ...) {
	va_list ap;
	va_start(ap, reason);
	as6502_vlog(lineNumber, 0, 0, ANSI_COLOR_BRIGHT_CYAN, "note", reason, ap);
	va_end(ap);
}
