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
#include <sysexits.h>
#include <string.h> // strdup
#include <strings.h> // strncasecmp

#include <ld6502/object.h>
#include <ld6502/flat.h>
#include <ld6502/ines.h>

#include "linectl.h"
#include "parser.h"
#include "symbols.h"
#include "codegen.h"
#include "error.h"
#include "color.h"
#include "token.h"

#define MAX_LINE_LEN		80

#define EXTENSION_OBJECT	"o"
#define EXTENSION_SCRIPT	"dbg"

static uint16_t assembleLine(ld6502_object_blob *blob, as6502_token *head, as6502_symbol_table *table, int printProcess, int printDot, uint16_t offset) {
	uint8_t opcode, low, high;
	int addrLen;
	v6502_address_mode mode;

	as6502_token *desymedHead = as6502_desymbolicateExpression(table, head, offset, YES);
	desymedHead = as6502_resolveArithmeticInExpression(desymedHead);
	if (printDot) as6502_printDotRankForList(stdout, desymedHead);
	as6502_instructionForExpression(&opcode, &low, &high, &mode, desymedHead);
	if (printDot) as6502_printDotRankForList(stdout, desymedHead);
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

	if (opcode == v6502_opcode_jmp_ind && low == 0xFF) {
		as6502_warn(head->loc, head->len, "Indirect jumps that span page boundaries are often not implemented correctly in hardware.");
	}

	if (printProcess) {
		FILE *lineout = printProcess ? stdout : stderr;
		
		uint16_t address = blob->len - addrLen;
		as6502_symbol *label = as6502_symbolForAddress(table, address + blob->start);
		if (label) {
			fprintf(lineout, "%#04x:          - %4lu: %s:\n", address, label->line, label->name);
		}
		
		fprintf(lineout, "%#04x: ", blob->len - addrLen);
		
		switch (addrLen) {
			case 1: {
				fprintf(lineout, "%02x      ", opcode);
			} break;
			case 2: {
				fprintf(lineout, "%02x %02x   ", opcode, low);
			} break;
			case 3: {
				fprintf(lineout, "%02x %02x %02x", opcode, low, high);
			} break;
			default: {
				fprintf(lineout, "        ");
			} break;
		}

		char line[MAX_LINE_LEN];
		as6502_stringForTokenList(line, MAX_LINE_LEN, head->next);
		fprintf(lineout, " - %4lu:  \t%s %s\n", currentLineNum, head->text, line);
	}
	
	return addrLen;
}

static void assembleFile(FILE *in, FILE *out, FILE *sym, int printProcess, int printTable, int printDot, ld6502_file_type format) {
	char *line = NULL;
	ssize_t len;
	size_t linecap = 0;
	uint16_t address = 0;
	currentLineNum = 0;
	ld6502_object *obj = ld6502_createObject();
	obj->table = as6502_createSymbolTable();
	int currentBlob = 0;

	// Render rank for final pass
	if (printDot) {
		printf("digraph G {\n");
		printf("{ node [shape = plaintext]; ");
	}

	while ((len = getline(&line, &linecap, in)) > 0) {
		currentLineNum++;
		currentLineText = line;

		as6502_token *head = as6502_lex(line, len);
		as6502_token *_head = head; // For memory management purposes if we bump the head forward for a label
		if (!head) {
			continue;
		}
		
		if (printDot) {
			printf("%lu -> ", currentLineNum);
		}

		// Label First
		if (head->next && head->next->len == 1 && head->next->text[0] == ':') {
			as6502_addSymbolToTable(obj->table, currentLineNum, head->text, address + obj->blobs[currentBlob].start, as6502_symbol_type_label);
			head = head->next->next;
		}
		
		// Dot Directives
		if (head && head->text[0] == '.') {
			if (!strncasecmp(head->text, ".org", 5) && head->next) {
				uint16_t start = as6502_valueForString(NULL, head->next->text);

				// Close old blob (FIXME: This doesn't need to happen currently since we allocate a byte at a time)
//				obj->blobs[currentBlob].data = malloc(sizeof(uint8_t) * address);
//				obj->blobs[currentBlob].len = address;

				// New blob
				ld6502_addBlobToObject(obj, start);
				currentBlob++;
				address = 0;
			}
		}
		// Variable
		else if(as6502_tokenListFindTokenLiteral(head, "=")) {
			as6502_token *eq = head->next;
			if (!eq) {
				as6502_error(head->loc, head->len, "Variable declarations require a name before the equal operator.");
				continue;
			}


			if (!eq->next) {
				as6502_error(eq->loc, eq->len, "Variable declaration requires a value after the equal operator.");
				continue;
			}
			// These should always be: <varname>, '=', <target_address>
			uint16_t target = as6502_valueForString(NULL, head->next->next->text);
			
			as6502_addSymbolToTable(obj->table, currentLineNum, head->text, target, as6502_symbol_type_variable);
		}
		// Instruction (needed to keep track of offset)
		else {
			as6502_token *desymedHead = as6502_desymbolicateExpression(obj->table, head, address, YES);
			desymedHead = as6502_resolveArithmeticInExpression(desymedHead);
			v6502_address_mode mode = as6502_addressModeForExpression(desymedHead);
			as6502_tokenListDestroy(desymedHead);
			address += as6502_instructionLengthForAddressMode(mode);
		}

		as6502_tokenListDestroy(_head);
	}

	// TODO: Allocate last open blob

	if (printTable) {
		as6502_printSymbolTable(obj->table);
	}
	if (sym) {
		as6502_printSymbolScript(obj->table, sym);
	}
	
	if (printDot) {
		printf("EOF; }");
	}

	// Reset for pass 2
	rewind(in);
	address = 0;
	currentLineNum = 0;

	while ((len = getline(&line, &linecap, in)) > 0) {
		currentLineNum++;

		as6502_token *head = as6502_lex(line, len);

		if (printDot && head) {
			as6502_printDotRankForList(stdout, head);
		}

		// Trim off labels
		as6502_token *colon = as6502_tokenListFindTokenLiteral(head, ":");
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

		// Dot Directives
		if (head->text[0] == '.') {
			/* FIXME: This should probably be taken care of in the
			 * as6502_processObjectDirectiveInExpression that follows, but since
			 * this does preallocation in the first pass, and works quickly, we
			 * are going to keep it around for now.
			 */
			if (!strncasecmp(head->text, ".org", 5) && head->next) {
				// Find the preallocated blob and change to it
				uint16_t start = as6502_valueForString(NULL, head->next->text);

				for (int blob = 0; blob < obj->count; blob++) {
					if (obj->blobs[blob].start == start) {
						currentBlob = blob;
						break;
					}
				}
			}
			else {
				as6502_processObjectDirectiveInExpression(obj, &currentBlob, head);
			}
		}
		// Variable assignments
		else if (as6502_tokenListFindTokenLiteral(head, "=")) {
			// This is now merely here to prevent the else from handling them, but continuing would leak the token list
		}
		// Normal instructions
		else {
			address += assembleLine(&obj->blobs[currentBlob], head, obj->table, printProcess, printDot, address);
		}

		as6502_tokenListDestroy(head);
	}

	if (printDot) {
		printf("}\n");
	}
	currentLineNum = 0;

	// Write out the object to whatever file format we were told
	switch (format) {
		case ld6502_file_type_None:
		case ld6502_file_type_FlatFile: {
			as6502_writeObjectToFlatFile(obj, out);
		} break;
		case ld6502_file_type_iNES: {
			as6502_writeObjectToINES(obj, out);
		} break;
	}

	as6502_destroySymbolTable(obj->table);
	ld6502_destroyObject(obj);
}

static char *outNameFromInName(const char *in, const char *ext) {
	char *outName = strdup(in);
	size_t inLen = strlen(in);
	outName = realloc(outName, inLen + sizeof('.') + strlen(ext));
	char *extStart = strchr(outName, '.');
	if(!extStart) {
		extStart = outName + inLen;
		extStart[0] = '.';
	}
	extStart++;

	strcpy(extStart, ext);
	return outName;
}

static void usage() {
	fprintf(stderr, "usage: as6502 [-dStT] [-F format] [-o outfile] [file ...]\n");
}

int main(int argc, char * const argv[]) {
	FILE *in;
	FILE *out;
	FILE *sym = NULL;
	char *outName = NULL;
	int printProcess = NO;
	int printTable = NO;
	int printDot = NO;
	int makeSymScript = NO;
	ld6502_file_type format = ld6502_file_type_FlatFile;
	currentErrorCount = 0;

	int ch;
	while ((ch = getopt(argc, argv, "dSTF:o:t")) != -1) {
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
			case 't': {
				makeSymScript = YES;
			} break;
			case 'd': {
				printDot = YES;
			} break;
			case 'o': {
				outName = strdup(optarg);
			} break;
			case '?':
			default:
				usage();
				return EX_USAGE;
		}
	}

	int i = optind;
	
	if (argc - i == 0) {
		currentFileName = "stdin";
		currentLineNum = 0;
		
		as6502_warn(0, 0, "Assembling from stdin does not support symbols");
		
		if (makeSymScript) {
			char *fname = outNameFromInName(argv[i], EXTENSION_SCRIPT);
			sym = fopen(fname, "w");
			free(fname);
		}
		assembleFile(stdin, stdout, sym, NO, NO, NO, format);
	}
	else {
		for (/* i */; i < argc; i++) {
			in = fopen(argv[i], "r");
			if (!in) {
				perror("fopen");
				return 1;
			}
			
			currentFileName = argv[i];
			if (!outName) {
				outName = outNameFromInName(argv[i], EXTENSION_OBJECT);
			}
			out = fopen(outName, "w");
			free(outName);
			outName = NULL;

			if (makeSymScript) {
				char *fname = outNameFromInName(argv[i], EXTENSION_SCRIPT);
				sym = fopen(fname, "w");
				free(fname);
			}
			assembleFile(in, out, sym, printProcess, printTable, printDot, format);
			fclose(in);
			fclose(out);
		}
	}
	if(sym) {
		fclose(sym);
	}

	// FIXME: Should this return EX_DATAERR on non-zero error count instead of returning the count?
	return currentErrorCount;
}

