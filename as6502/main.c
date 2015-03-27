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

static void printSpaces(unsigned long num) {
	for (/* num */; num > 0; num--) {
		printf(" ");
	}
}

static uint16_t assembleLine(ld6502_object_blob *blob, as6502_token *head, as6502_symbol_table *table, int printProcess, uint16_t offset) {
	uint8_t opcode, low, high;
	int addrLen;
	v6502_address_mode mode;

	as6502_token *desymedHead = as6502_desymbolicateExpression(table, head, 0x600, offset, YES);
	as6502_instructionForExpression(&opcode, &low, &high, &mode, desymedHead);
	as6502_tokenListDestroy(desymedHead);

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
		as6502_stringForTokenList(line, 80, head->next);
		printf(" - %4lu:  \t%s %s\n", currentLineNum, head->text, line);
		
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

	while (fgets(line, MAX_LINE_LEN, in)) {
		currentLineNum++;

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
		else if(as6502_tokenListContainsToken(head, "=", 1)) {
			as6502_addSymbolToTable(obj->table, currentLineNum, head->text, address, as6502_symbol_type_variable);
		}
		// Instruction (needed to keep track of offset)
		else {
			v6502_address_mode mode = as6502_addressModeForExpression(head);
			address += as6502_instructionLengthForAddressMode(mode);
		}

		as6502_tokenListDestroy(head);
	}

	if (printTable) {
		as6502_printSymbolTable(obj->table);
	}

	// Reset for pass 2
	rewind(in);
	address = 0;
	currentLineNum = 0;

	while (fgets(line, MAX_LINE_LEN, in)) {
		currentLineNum++;

		as6502_token *head = as6502_lex(line, MAX_LINE_LEN);

		// Trim off labels
		as6502_token *colon = as6502_tokenListContainsTokenLiteral(head, ":");
		if (colon) {
			as6502_token *tail = colon->next;
			colon->next = NULL;
			as6502_tokenListDestroy(head);
			head = tail;
		}

		// Make sure there's something left to assemble
		if (!head) {
			continue;
		}

		// Handle variable assignments
		if (as6502_tokenListContainsToken(head, "=", 1)) {
			as6502_token *value = as6502_firstTokenOfTypeInList(head, as6502_token_type_value);

			uint8_t low;
			as6502_byteValuesForString(NULL, &low, NULL, value->text);
			ld6502_appendByteToBlob(&obj->blobs[currentBlob], low);

			as6502_tokenListDestroy(head);
			continue;
		}

		address += assembleLine(&obj->blobs[currentBlob], head, obj->table, printProcess, address);
		
		as6502_tokenListDestroy(head);
	}

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

