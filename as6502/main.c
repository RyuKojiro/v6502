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
#include <stdlib.h> // free

#include "linectl.h"
#include "parser.h"
#include "symbols.h"
#include "object.h"
#include "codegen.h"
#include "error.h"
#include "color.h"
#include "token.h"

#include "flat.h"
#include "ines.h"

#define MAX_LINE_LEN		80
#define MAX_FILENAME_LEN	255

#define v6502_BadLiteralValueErrorText		"Invalid literal value or unresolved/undefined symbol '%s'"

// This macro is only for use inside of isValidLiteral
#define advanceOverCharIfPresent(ch)		if (start[0] == ch) { \
												start++; \
												len--; \
											}

// Figures out if the number is valid, or a stray symbol
// TODO: @todo make these actually throw the errors, so that the error is more specific, and more helpful, rather than being more generic
static int isValidLiteral(const char *start, size_t len) {
	if (!len || !start[0]) {
		return YES;
	}
	
	// First skip any parens, immediate, zeropage, or hex markers
	// NOTE: The order of these is significant in that immediate values can not be indirectly referenced (follow a parens), dollar signs cannot come before any of the other symbols, etc.
	// TODO: @todo parenthesis tracking to make sure they line up properly
	advanceOverCharIfPresent('#');
	advanceOverCharIfPresent('(');
	advanceOverCharIfPresent('*');
	advanceOverCharIfPresent('$');
	
	// Must be valid hex, oct, dec
	for (size_t i = 0; start[i] && (i < len); i++) {
		// NOTE: The order of these if statements is critically important, it determines the priority of each character during validation
		if (start[i] == ';') {
			// The rest are comments
			break;
		}

		if (start[i] == ')') {
			continue;
		}

		if (start[i] == ',') {
			// Now look for a register starting after the comma
			for (i++; start[i] && (i < len); i++) {
				if(!isspace(CTYPE_CAST start[i]) && !(start[i] == 'x' || start[i] == 'y' || start[i] == 'X' || start[i] == 'Y' || start[i] == ')')) {
					return NO;
				}
			}
			
			break;
		}

		if (!(start[i] >= '0' && start[i] <= '9') &&
			!(start[i] >= 'a' && start[i] <= 'f') &&
			!(start[i] >= 'A' && start[i] <= 'F') ) {
			return NO;
		}
	}
	
	
	return YES;
}

static void printSpaces(unsigned long num) {
	for (/* num */; num > 0; num--) {
		printf(" ");
	}
}

static uint16_t assembleLine(ld6502_object_blob *blob, as6502_token *head, as6502_symbol_table *table, int printProcess) {
	uint8_t opcode, low, high;
	int addrLen;
	v6502_address_mode mode;

//	if (line[3] && !isValidLiteral(line + 4, len - 4)) {
//		size_t sLen = as6502_lengthOfToken(line + 4, len - 4);
//		char *symbol = malloc(sLen + 1);
//		strncpy(symbol, line + 4, sLen);
//		symbol[sLen] = '\0';
//		as6502_error(4,  sLen, v6502_BadLiteralValueErrorText, symbol);
//		free(symbol);
//
//		/** TODO: @todo Should this short circuit the rest of the assembly for this line? */
//	}

	as6502_instructionForExpression(&opcode, &low, &high, &mode, head);
	addrLen = as6502_instructionLengthForAddressMode(mode);
	
	if (addrLen >= 1) {
		ld6502_appendByteToBlob(blob, opcode);
	}
	if (addrLen >= 2) {
		ld6502_appendByteToBlob(blob, low);
	}
	if (addrLen >= 3) {
		ld6502_appendByteToBlob(blob, high);
	}
		
	if (printProcess || (lastProblematicLine == currentLineNum)) {
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

		char line[80];
		as6502_stringForTokenList(line, 80, head);
		printf(" - %4lu:  \t%s\n", currentLineNum, line);
		
		if (lengthOfProblem) {
			printSpaces(19);
			for (unsigned long i = currentLineNum; i >= 10; i %= 10) {
				printf(" ");
			}
			printSpaces(4);
			printf("\t");
			
			printSpaces(startOfProblem);
			printf(ANSI_COLOR_BRIGHT_GREEN "^");
			
			if (lengthOfProblem) {
				for (lengthOfProblem--; lengthOfProblem; lengthOfProblem--) {
					printf("~");
				}
			}

			printf(ANSI_COLOR_RESET "\n");
		}
	}
	
	return addrLen;
}

static void assembleFile(FILE *in, FILE *out, int printProcess, int printTable, ld6502_file_type format) {
	char line[MAX_LINE_LEN];
	uint16_t address = 0;
	currentLineNum = 0;
	ld6502_object *obj = ld6502_createObject();
	obj->table = as6502_createSymbolTable();
	int currentBlob = 0;

	do {
		currentLineNum++;
		fgets(line, MAX_LINE_LEN, in);
		as6502_token *head = as6502_lex(line, MAX_LINE_LEN);

		if (!head) {
			continue;
		}

		// Dot Directives
		if (head->text[0] == '.') {
			// Handle them!
		}
		// Label
		else if (head->next && head->next->len == 1 && head->next->text[0] == ':') {
			as6502_addSymbolToTable(obj->table, currentLineNum, head->text, address, as6502_symbol_type_label);
		}
		// Variable
		else if(NO) {
			as6502_addSymbolToTable(obj->table, currentLineNum, head->text, address, as6502_symbol_type_variable);
		}
		// Instruction (needed to keep track of offset)
		else {
			v6502_address_mode mode = as6502_addressModeForExpression(head);
			address += as6502_instructionLengthForAddressMode(mode);
		}

		as6502_tokenListDestroy(head);
	} while (!feof(in));

	if (printTable) {
		as6502_printSymbolTable(obj->table);
	}

	// Reset for pass 2
	rewind(in);
	address = 0;
	currentLineNum = 0;

	do {
		fgets(line, MAX_LINE_LEN, in);
		currentLineNum++;

		as6502_token *head = as6502_lex(line, MAX_LINE_LEN);
		if (!head) {
			continue;
		}

		if (as6502_tokenListContainsTokenLiteral(head, ":")) {
			as6502_tokenListDestroy(head);
			continue;
		}

		assembleLine(&obj->blobs[currentBlob], head, obj->table, printProcess);
		
		as6502_tokenListDestroy(head);
	} while (!feof(in));

	currentLineNum = 0;

	// Write out the object to whatever file format we were told
	switch (format) {
		case ld6502_file_type_None:
		case ld6502_file_type_FlatFile: {
			as6502_writeObjectToFlatFile(obj, out);
		} break;
		case ld6502_file_type_iNES: {
			ines_properties props;
			props.videoMode = ines_videoMode_NTSC;

			writeToINES(out, &(obj->blobs[0]), NULL, &props);
		} break;
	}

	as6502_destroySymbolTable(obj->table);
	ld6502_destroyObject(obj);

	/*
	char *trimmedLine;
	v6502_address_mode mode;
	int newline;
	size_t lineLen, maxLen;
	int instructionLength;

	// First pass, build symbol table
	do {
		newline = NO;
		fgets(line, MAX_LINE_LEN, in);
		if (strchr(line, '\n')) {
			newline = YES;
		}
		
		// Truncate at comments
		trimgreedytailchard(line, ';');
		
		// Check for .org directives, which reset the current address offset (this is a special case)
		if (!strncasecmp(line, ".org", 3)) {
			// reset current offset address
			address = as6502_valueForString(NULL, line + 5);
		}

		// Check for symbols
		trimmedLine = line; //trimheadchar(line, '\n', MAX_LINE_LEN); // FIXME: @bug Does this do anything at all?
		lineLen = MAX_LINE_LEN; // - (trimmedLine - line);
		if (trimmedLine && (isalnum(CTYPE_CAST trimmedLine[0]))) {
			as6502_symbol_type type = as6502_addSymbolForLine(obj->table, line, currentLineNum, address);
			
			if (as6502_symbolTypeIsVariable(type)) {
				address++;
				
				if (newline) {
					currentLineNum++;
				}
				
				continue;
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
		else if(feof(in)) {
			// If the end of the file has a blank line, don't try to assemble it
			break;
		}
		
		// Truncate at comments
		trimgreedytailchard(line, ';');
		
		// Trim trailing whitespace
		trimtaild(line);
		lineLen = strlen(line);

		// Handle dot directives
		if (line[0] == '.') {
			if (dotDirectiveEq("asci")) {
				char *stop = line + lineLen;
				for (char *ch = trimheadchar(line, '"', lineLen) + 1; *ch && ch < stop && *ch != '"'; ch++) {
					ld6502_appendByteToBlob(&obj->blobs[currentBlob], *ch);
				}
				
				if (dotDirectiveEq("asciz")) {
					ld6502_appendByteToBlob(&obj->blobs[currentBlob], 0x00);
				}
			}
			else if (dotDirectiveEq("byte")) {
				int skip = 1 + 4; // "." + "byte"
				char *start = line + skip;
				start = trimhead(start, lineLen - skip);
				uint8_t low;
				int wide;
				as6502_byteValuesForString(NULL, &low, &wide, start);
				if (wide) {
					as6502_warn(start - line, 1, "Value larger than 8-bits specified in .byte directive, only the lower 8-bits will be used.");
				}
				ld6502_appendByteToBlob(&obj->blobs[currentBlob], low);
			}
			else {
				as6502_processObjectDirectiveForLine(obj, &currentBlob, line, lineLen);
			}			
			
			if (newline) {
				currentLineNum++;
			}
			
			continue;
		}
		
		// TODO: @todo Handle Variable Declarations
		if (as6502_resolveVariableDeclaration(&obj->blobs[currentBlob], obj->table, line, lineLen)) {
			continue;
		}
		
		// Remove label clause
		trimmedLine = trimheadtospc(line, lineLen);
		trimmedLine = trimhead(trimmedLine, lineLen - (trimmedLine - line));
		maxLen = MAX_LINE_LEN - (trimmedLine - line);
		
		// Convert symbols to hard addresses from symbol table
		trimmedLine = as6502_desymbolicateLine(obj->table, trimmedLine, maxLen, 0x0600, address, NO, &lineLen);

		// Assemble whatever is left, if anything
		if (lineLen) {
			// Check for Variable Declarations and Arithmetic
			as6502_resolveArithmetic(trimmedLine, lineLen, address);
			
			instructionLength = assembleLine(&obj->blobs[currentBlob], trimmedLine, lineLen, obj->table, printProcess);
			address += instructionLength;
		}
		free(trimmedLine);
				
		// Check if we are on the next line yet
		if (newline) {
			currentLineNum++;
		}
	} while (!feof(in));
	
	 */
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
	fprintf(stderr, "usage: as6502 [-STW] [-F format] [-o outfile] [file ...]\n");
}

int main(int argc, char * const argv[]) {
	FILE *in;
	FILE *out;
	char outName[MAX_FILENAME_LEN];
	int printProcess = NO;
	int printTable = NO;
	ld6502_file_type format = ld6502_file_type_FlatFile;
	
	outName[0] = '\0';
	
	// If no arguments
	int ch;
	
	while ((ch = getopt(argc, argv, "STWF:o:")) != -1) {
		switch (ch) {
			case 'F': {
				if (!strncmp(optarg, "flat", 4)) {
					format = ld6502_file_type_FlatFile;
				}
				if (!strncmp(optarg, "ines", 4)) {
					format = ld6502_file_type_iNES;
				}
			} break;
			case 'S': {
				printProcess = YES;
			} break;
			case 'T': {
				printTable = YES;
			} break;
			case 'o': {
				strncpy(outName, optarg, MAX_FILENAME_LEN);
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
		
		as6502_warn(0, 0, "Assembling from stdin does not support symbols");
		
		assembleFile(stdin, stdout, NO, NO, format);
	}
	else {
		for (/* i */; i < argc; i++) {
			in = fopen(argv[i], "r");
			if (!in) {
				perror("fopen");
				return 1;
			}
			
			currentFileName = argv[i];
			if (!outName[0]) {
				outNameFromInName(outName, MAX_FILENAME_LEN, argv[i]);
			}
			out = fopen(outName, "w");
			assembleFile(in, out, printProcess, printTable, format);
			fclose(in);
			fclose(out);
		}
	}
}

