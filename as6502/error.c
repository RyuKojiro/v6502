//
//  common.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "error.h"

unsigned long currentLineNum;
const char *currentFileName;

__attribute((noreturn)) void as6502_fatal(const char *reason) {
	fprintf(stderr, "as6502: fatal: %s\n", reason);
	exit(EXIT_FAILURE);
}

void as6502_error(const char *reason, ...) {
	va_list ap;
	va_start(ap, reason);
	
	fprintf(stderr, "%s:%lu: error: ", currentFileName, currentLineNum);
	vfprintf(stderr, reason, ap);
	va_end(ap);

	if (reason[strlen(reason)] != '\n') {
		fprintf(stderr, "\n");
	}
}

void as6502_warn(const char *reason) {
	fprintf(stderr, "%s:%lu: warning: %s", currentFileName, currentLineNum, reason);
	if (reason[strlen(reason)] != '\n') {
		fprintf(stderr, "\n");
	}
}

void as6502_note(unsigned long lineNumber, const char *reason, ...) {
	va_list ap;
	va_start(ap, reason);
	
	fprintf(stderr, "%s:%lu: note: ", currentFileName, lineNumber);
	vfprintf(stderr, reason, ap);
	va_end(ap);
	
	if (reason[strlen(reason)] != '\n') {
		fprintf(stderr, "\n");
	}
}
