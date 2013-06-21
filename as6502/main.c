//
//  main.c
//  as6502
//
//  Created by Daniel Loffgren on H.25/04/10.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <ctype.h>
#include <unistd.h> // getopt

#include "linectl.h"
#include "parser.h"
#include "symbols.h"
#include "object.h"
#include "codegen.h"
#include "error.h"

#define MAX_LINE_LEN		80
#define MAX_FILENAME_LEN	255

static uint16_t assembleLine(as6502_object_blob *blob, const char *line, size_t len) {
	uint8_t opcode, low, high;
	int addrLen;
	v6502_address_mode mode;

	as6502_instructionForLine(&opcode, &low, &high, &mode, line, len);
	addrLen = as6502_instructionLengthForAddressMode(mode);
	
	// TODO: Write machine code to object, not directly to file
	if (addrLen >= 1) {
		as6502_appendByteToBlob(blob, opcode);
	}
	if (addrLen >= 2) {
		as6502_appendByteToBlob(blob, low);
	}
	if (addrLen >= 3) {
		as6502_appendByteToBlob(blob, high);
	}
	
	return addrLen;
}

static void assembleFile(FILE *in, FILE *out, int printProcess) {
	char line[MAX_LINE_LEN];
	char *trimmedLine;
	v6502_address_mode mode;
	uint16_t address = 0;
	currentLineNum = 1;
	int newline;
	as6502_symbol_table *table = as6502_createSymbolTable();
	size_t lineLen;
	int instructionLength;
	
	// First pass, build symbol table
	do {
		newline = NO;
		fgets(line, MAX_LINE_LEN, in);
		if (strchr(line, '\n')) {
			newline = YES;
		}
		
		// Check for symbols
		trimmedLine = trimheadchar(line, '\n'); // FIXME: Does this do anything at all?
		if (isalnum(trimmedLine[0])) {
			as6502_addSymbolForLine(table, line, currentLineNum, address);

			// Trim any possible labels we encountered
			trimmedLine = trimheadtospc(line);
		}
		
		// Increment offset if there is an actual instruction line here
		if (trimmedLine[0] && trimmedLine[0] != ';') {
			mode = as6502_addressModeForLine(trimmedLine);
			address += as6502_instructionLengthForAddressMode(mode);
		}
		
		// Check if we are on the next line yet
		if (newline) {
			currentLineNum++;
		}
	} while (!feof(in));
	
	// Reset for pass 2
	rewind(in);
	address = 0;
	currentLineNum = 1;
	
	// Prepare object structure
	as6502_object_context *ctx = as6502_createObjectContext();
		
	// Final pass, convert source to object code
	do {
		newline = NO;
		fgets(line, MAX_LINE_LEN, in);
		if (strchr(line, '\n')) {
			newline = YES;
		}
		
		// Truncate at comments
		trimgreedytailchard(line, ';');
		
		// Trim trailing whitespace
		trimtaild(line);
		
		// Convert symbols to hard addresses from symbol table
		as6502_desymbolicateLine(table, line, MAX_LINE_LEN, address, NO);
		
		// Check for Variable Declarations and Arithmetic
		as6502_resolveArithmetic(line, MAX_LINE_LEN);
		
		// Handle dot directives
		if (line[0] == '.') {
			as6502_processObjectDirectiveForLine(ctx, line, MAX_LINE_LEN);
			continue;
		}
		
		// Trim leading whitespace
		trimmedLine = trimhead(line);
		lineLen = strlen(trimmedLine);

		if (as6502_resolveVariableDeclaration(table, as6502_currentBlobInContext(ctx), assembleLine, trimmedLine, lineLen)) {
			continue;
		}

		// Assemble whatever is left
		if (lineLen) {
			instructionLength = assembleLine(as6502_currentBlobInContext(ctx), trimmedLine, lineLen);
			address += instructionLength;
			
			if (printProcess) {
				uint8_t opcode, low, high;
				as6502_instructionForLine(&opcode, &low, &high, NULL, trimmedLine, lineLen);
				
				printf("0x%04x: ", address - instructionLength);
				
				switch (instructionLength) {
					case 1: {
						printf("%02x      ", opcode);
					} break;
					case 2: {
						printf("%02x %02x   ", opcode, low);
					} break;
					case 3: {
						printf("%02x %02x %02x", opcode, low, high);
					} break;
					default: {
						printf("        ");
					} break;
				}
				
				printf(" - %4lu: %s\n", currentLineNum, trimmedLine);
			}
		}
		
		// Check if we are on the next line yet
		if (newline) {
			currentLineNum++;
		}
	} while (!feof(in));
	
	as6502_writeObjectToFile(ctx->obj, out);
	
	as6502_destroyObjectContext(ctx);
	as6502_destroySymbolTable(table);
}

static void outNameFromInName(char *out, int len, const char *in) {
	int c;
	for (c = 0; c < len && in[c] && in[c] != '.'; c++) {
		out[c] = in[c];
	}
	out[c] = '.';
	out[++c] = 'o';
	out[++c] = '\0';
}

static void usage() {
	fprintf(stderr, "usage: as6502 [-SW] [file ...]\n");
}

int main(int argc, char * const argv[]) {
	FILE *in;
	FILE *out;
	char outName[MAX_FILENAME_LEN];
	int printProcess = NO;
	
	// If no arguments
	int ch;
	
	while ((ch = getopt(argc, argv, "SW")) != -1) {
		switch (ch) {
			case 'S': {
				printProcess = YES;
			} break;
			case '?':
			default:
				usage();
				return 0;
		}
	}

	int i = optind;
	
	if (argc - i == 0) {
		currentFileName = "stdin";
		currentLineNum = 0;
		
		as6502_warn("Assembling from stdin does not support symbols");
		
		assembleFile(stdin, stdout, NO);
	}
	else {
		for (/* i */; i < argc; i++) {
			in = fopen(argv[i], "r");
			currentFileName = argv[i];
			outNameFromInName(outName, MAX_FILENAME_LEN, argv[i]);
			out = fopen(outName, "w");
			assembleFile(in, out, printProcess);
			fclose(in);
			fclose(out);
		}
	}
}

