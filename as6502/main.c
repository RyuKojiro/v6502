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
#include <ctype.h>
#include <unistd.h> // getopt

#include "linectl.h"
#include "parser.h"
#include "symbols.h"
#include "object.h"
#include "codegen.h"
#include "error.h"

#include "flat.h"

#define MAX_LINE_LEN		80
#define MAX_FILENAME_LEN	255

#define v6502_BadLiteralValueErrorText		"Invalid literal value or unresolved/undefined symbol '%s'"

typedef enum {
	as6502_outputFormat_FlatFile
}as6502_outputFormat;

// Figures out if the number is valid, or a stray symbol
int isValidLiteral(const char *start, size_t len) {
	if (!len || !start[0]) {
		return YES;
	}
	
	// First skip any parens, immediate, or hex markers
	if (start[0] == '(' || start[0] == '#' || start[0] == '$') {
		start++;
		len--;
	}
	if (start[0] == '$') {
		start++;
		len--;
	}
	
	// Must be valid hex, oct, dec
	for (size_t i = 0; start[i] && (i < len); i++) {
		if (!(start[i] >= '0' && start[i] <= '9') &&
			!(start[i] >= 'a' && start[i] <= 'f') &&
			!(start[i] >= 'A' && start[i] <= 'F') ) {
			return NO;
		}
	}
	
	
	return YES;
}

static uint16_t assembleLine(as6502_object_blob *blob, const char *line, size_t len, as6502_symbol_table *table, int printProcess) {
	uint8_t opcode, low, high;
	int addrLen;
	v6502_address_mode mode;

	if (line[3] && !isValidLiteral(line + 4, len - 4)) {
		as6502_error(v6502_BadLiteralValueErrorText, line + 4);
		/** TODO: @todo Should this short circuit the rest of the assembly for this line? */
	}
	
	as6502_instructionForLine(&opcode, &low, &high, &mode, line, len);
	addrLen = as6502_instructionLengthForAddressMode(mode);
	
	if (addrLen >= 1) {
		as6502_appendByteToBlob(blob, opcode);
	}
	if (addrLen >= 2) {
		as6502_appendByteToBlob(blob, low);
	}
	if (addrLen >= 3) {
		as6502_appendByteToBlob(blob, high);
	}
		
	if (printProcess) {
		as6502_symbol *label = as6502_symbolForAddress(table, blob->len - addrLen);
		if (label) {
			printf("0x%04x:          - %4lu: %s:\n", blob->len - addrLen, label->line, label->name);
		}
		
		printf("0x%04x: ", blob->len - addrLen);
		
		switch (addrLen) {
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
		
		printf(" - %4lu:  \t%s\n", currentLineNum, line);
	}
	
	return addrLen;
}

static void assembleFile(FILE *in, FILE *out, int printProcess, int printTable, as6502_outputFormat format) {
	char line[MAX_LINE_LEN];
	char *trimmedLine;
	v6502_address_mode mode;
	uint16_t address = 0;
	currentLineNum = 1;
	int newline;
	size_t lineLen, maxLen;
	int instructionLength;
	uint16_t currentVarAddress = 0x200;	// Variable storage will grow upwards, opposite the stack
	as6502_object *obj = as6502_createObject();
	obj->table = as6502_createSymbolTable();
	int currentBlob = 0;

	// First pass, build symbol table
	do {
		newline = NO;
		fgets(line, MAX_LINE_LEN, in);
		if (strchr(line, '\n')) {
			newline = YES;
		}
		
		// Truncate at comments
		trimgreedytailchard(line, ';');
		
		// Check for symbols
		trimmedLine = trimheadchar(line, '\n', MAX_LINE_LEN); /** FIXME: @bug Does this do anything at all? */
		lineLen = MAX_LINE_LEN - (trimmedLine - line);
		if (trimmedLine && (isalnum(CTYPE_CAST trimmedLine[0]) || trimmedLine[0] == '.')) {
			as6502_symbol_type type = as6502_addSymbolForLine(obj->table, line, currentLineNum, address, currentVarAddress);
			
			if (type == as6502_symbol_type_variable) {
				if (currentVarAddress >= 0x7FF) {
					as6502_error("Maximum number of addressable variables exceeded.");
				}
				else {
					currentVarAddress++;
					
					if (newline) {
						currentLineNum++;
					}
					
					continue;
				}
			}

			// Trim any possible labels we encountered
			trimmedLine = trimheadtospc(line, lineLen);
			lineLen = MAX_LINE_LEN - (trimmedLine - line);
		}
		
		// Increment offset if there is an actual instruction line here
		trimmedLine = trimhead(trimmedLine, lineLen);
		if (trimmedLine[0] && trimmedLine[0] != ';') {
			mode = as6502_addressModeForLine(trimmedLine, lineLen);
			address += as6502_instructionLengthForAddressMode(mode);
		}
		
		// Check if we are on the next line yet
		if (newline) {
			currentLineNum++;
		}
	} while (!feof(in));
	
	if (printTable) {
		as6502_printSymbolTable(obj->table);
	}
	
	// Reset for pass 2
	rewind(in);
	address = 0;
	currentLineNum = 1;
		
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
		lineLen = strlen(line);

		// Handle dot directives
		if (line[0] == '.') {
			as6502_processObjectDirectiveForLine(obj, &currentBlob, line, lineLen);
			
			if (newline) {
				currentLineNum++;
			}
			
			continue;
		}
		
		// TODO: @todo Handle Variable Declarations
		// as6502_resolveVariableDeclaration(table, as6502_currentBlobInContext(ctx), assembleLine, trimmedLine, maxLen);
		
		// Remove label clause
		trimmedLine = trimheadtospc(line, lineLen);
		trimmedLine = trimhead(trimmedLine, lineLen - (trimmedLine - line));
		maxLen = MAX_LINE_LEN - (trimmedLine - line);

		// Check for Variable Declarations and Arithmetic
		as6502_resolveArithmetic(line, maxLen);
		
		// Convert symbols to hard addresses from symbol table
		as6502_desymbolicateLine(obj->table, trimmedLine, maxLen, 0x0600, address, NO);
		lineLen = strlen(trimmedLine);

		// Assemble whatever is left, if anything
		if (lineLen) {
			instructionLength = assembleLine(&obj->blobs[currentBlob], trimmedLine, lineLen, obj->table, printProcess);
			address += instructionLength;
		}
				
		// Check if we are on the next line yet
		if (newline) {
			currentLineNum++;
		}
	} while (!feof(in));
	
	currentLineNum = 0;
	
	// Write out the object to whatever file format we were told
	switch (format) {
		case as6502_outputFormat_FlatFile: {
			as6502_writeObjectToFlatFile(obj, out);
		} break;
	}
	
	as6502_destroySymbolTable(obj->table);
	as6502_destroyObject(obj);
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
	fprintf(stderr, "usage: as6502 [-STWw] [-F format] [file ...]\n");
}

int main(int argc, char * const argv[]) {
	FILE *in;
	FILE *out;
	char outName[MAX_FILENAME_LEN];
	int printProcess = NO;
	int printTable = NO;
	as6502_outputFormat format = as6502_outputFormat_FlatFile;
	
	// If no arguments
	int ch;
	
	while ((ch = getopt(argc, argv, "STWF:")) != -1) {
		switch (ch) {
			case 'F': {
				if (!strncmp(optarg, "flat", 4)) {
					format = as6502_outputFormat_FlatFile;
				}
			} break;
			case 'S': {
				printProcess = YES;
			} break;
			case 'T': {
				printTable = YES;
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
		
		assembleFile(stdin, stdout, NO, NO, format);
	}
	else {
		for (/* i */; i < argc; i++) {
			in = fopen(argv[i], "r");
			currentFileName = argv[i];
			outNameFromInName(outName, MAX_FILENAME_LEN, argv[i]);
			out = fopen(outName, "w");
			assembleFile(in, out, printProcess, printTable, format);
			fclose(in);
			fclose(out);
		}
	}
}

