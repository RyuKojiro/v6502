//
//  main.c
//  as6502
//
//  Created by Daniel Loffgren on H.25/04/10.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>

#include "parser.h"
#include "labels.h"

#define MAX_LINE_LEN		80
#define MAX_FILENAME_LEN	255

static unsigned long currentLineNum;
static const char *currentFileName;

void v6502_fault(const char *error) {
	fprintf(stderr, "%s:%lu: error: ", currentFileName, currentLineNum);
	fprintf(stderr, "%s", error);
	if (error[strlen(error)] != '\n') {
		fprintf(stderr, "\n");
	}
}

void as6502_warn(const char *warning) {
	fprintf(stderr, "%s:%lu: warning: %s", currentFileName, currentLineNum, warning);
	if (warning[strlen(warning)] != '\n') {
		fprintf(stderr, "\n");
	}
}

static void assembleFile(FILE *in, FILE *out) {
	char line[MAX_LINE_LEN];
	uint8_t opcode, low, high;
	v6502_address_mode mode;
	uint16_t address = 0;
	int addrLen;
	currentLineNum = 0;

	do {
		currentLineNum++;
		fgets(line, MAX_LINE_LEN, in);

		if (line[0] != ';' && line[0] != '\n') {
			v6502_instructionForLine(&opcode, &low, &high, &mode, line, strlen(line));
			addrLen = v6502_instructionLengthForAddressMode(mode);
			
			switch (addrLen) {
				case 1:
					fwrite(&opcode , 1, 1, out);
				case 2:
					fwrite(&low , 1, 1, out);
				case 3:
					fwrite(&high , 1, 1, out);
			}
			address += addrLen;
		}
	} while (!feof(in));
}

static void outNameFromInName(char *out, int len, const char *in) {
	int c;
	for (c = 0; c < len && in[c] && in[c] != '.'; c++) {
		out[c] = in[c];
	}
	out[c] = '.';
	out[++c] = 'o';
	out[++c] = 'u';
	out[++c] = 't';
	out[++c] = '\0';
}

int main(int argc, const char * argv[]) {
	FILE *in;
	FILE *out;
	char outName[MAX_FILENAME_LEN];
	
	if (argc < 2) {
		currentFileName = "stdin";
		currentLineNum = 0;
		
		as6502_warn("Assembling from stdin does not support labels");
		
		assembleFile(stdin, stdout);
		return 0;
	}
	
	for (int i = 1; i < argc; i++) {
		in = fopen(argv[i], "r");
		currentFileName = argv[i];
		outNameFromInName(outName, MAX_FILENAME_LEN, argv[i]);
		out = fopen(outName, "w");
		assembleFile(in, out);
		fclose(in);
		fclose(out);
	}
}

