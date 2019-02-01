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
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#include <ld6502/object.h>
#include <as6502/error.h>
#include <dis6502/reverse.h>
#include <as6502/symbols.h>
#include <ld6502/flat.h>
#include <ld6502/ines.h>
#include <as6502/debug.h>

#define MAX_LINE_LEN		80

static void disassembleFile(const char *in, FILE *out, ld6502_file_type format, uint16_t pstart, int printTable, int verbose, FILE *sym) {
	char line[MAX_LINE_LEN];
	int insideOfString = 0;

	ld6502_object *obj = ld6502_createObject();
	ld6502_loadObjectFromFile(obj, in, format);

	as6502_symbol_table *table = as6502_createSymbolTable();

	for (int i = 0; i < obj->count; i++) {
		ld6502_object_blob *blob = &obj->blobs[i];

		// Emit org directives for slid blobs
		blob->start += pstart;
		if (blob->start) {
			fprintf(out, ".org $%04x\n", pstart);
		}

		// Build Symbol Table
		currentLineNum = 1;
		dis6502_deriveSymbolsForObjectBlob(table, blob);
		if (printTable) {
			as6502_printSymbolTable(table);
		}
		if (sym) {
			as6502_printSymbolScript(table, sym);
		}

		/*
		 * Trim Symbol Table, so that we don't symbolicate addresses outside of
		 * our address space. FIXME: This currently assumes only one blob!
		 * TODO: Would it perhaps be better to symbolicate them and emit
		 * assembler directives stating their locations?
		 */
		as6502_truncateTableToAddressSpace(table, blob->start, blob->len);

		// Disassemble
		uint8_t low = 0; // lint
		uint8_t high = 0; // lint
		currentLineNum = 1;
		for (uint16_t offset = 0; offset < blob->len; ){
			uint8_t opcode = blob->data[offset];
			as6502_symbol *label = as6502_symbolForAddress(table, blob->start + offset);
			if (label) {
				if (verbose) {
					as6502_printAnnotatedLabel(out, label->address, label->name, label->line);
				}
				else {
					fprintf(out, "%s:\n", label->name);
				}
				currentLineNum++;
			}

			if(insideOfString) {
				if(!opcode) {
					insideOfString = 0;
					fprintf(out, "\" ; The next brk serves as the terminator\n");
				}
				else {
					if(isascii(opcode) && isprint(opcode)) {
						fprintf(out, "%c", opcode);
					}
					else {
						insideOfString = 0;
						fprintf(out, "\" ; This string is unterminated\n");
						continue;
					}
				}
				offset++;
				continue;
			}
			else {
				if (offset + 2 < blob->len) {
					low = blob->data[offset + 1];
				}
				if (offset + 1 < blob->len) {
					high = blob->data[offset + 2];
				}

				dis6502_stringForInstruction(line, MAX_LINE_LEN, opcode, high, low);
				as6502_symbolicateLine(table, line, MAX_LINE_LEN, blob->start + offset);

				if(!strncmp("???", line, 3) && isascii(opcode) && isprint(opcode)) {
					// We've encountered something that isn't runnable code, or is misaligned, but it is ascii.
					// Let's see if it's a string
					fprintf(out, "; String at $%04x:\n.ascii \"", offset);
					insideOfString = 1;
					continue;
				}

				if (verbose) {
					as6502_printAnnotatedInstruction(out, blob->start + offset, opcode, low, high, line);
				}
				else {
					fprintf(out, "\t%s\n", line);
				}

				offset += v6502_instructionLengthForOpcode(opcode) ;
			}

			currentLineNum++;
		}
	}

	as6502_destroySymbolTable(table);
	ld6502_destroyObject(obj);
}

static void usage() {
	fprintf(stderr, "usage: dis6502 [-tTv] [-o out_file] [-F format] [-s load_address] [file ...]\n");
}

int main(int argc, char * const argv[]) {
	FILE *out = stdout;
	ld6502_file_type format = ld6502_file_type_None;
	uint16_t programStart = 0;
	int printTable = NO;
	FILE *sym = NULL;
	int verbose = NO;

	int ch;
	while ((ch = getopt(argc, argv, "o:F:s:Tt:v")) != -1) {
		switch (ch) {
			case 'F': {
				if (!strncmp(optarg, "ines", 4)) {
					format = ld6502_file_type_iNES;
				}
				if (!strncmp(optarg, "flat", 4)) {
					format = ld6502_file_type_FlatFile;
				}
			} break;
			case 'o': {
				out = fopen(optarg, "w");
			} break;
			case 's': {
				programStart = strtol(optarg, NULL, 16);
			} break;
			case 'T': {
				printTable = YES;
			} break;
			case 't': {
				sym = fopen(optarg, "w");
			} break;
			case 'v': {
				verbose = YES;
			} break;
			case '?':
			default:
				usage();
				return 0;
		}
	}

	argc -= optind;
	argv += optind;

	for (int i = 0; i < argc; i++) {
		disassembleFile(argv[i], out, format, programStart, printTable, verbose, sym);
	}

	if (sym) {
		fclose(sym);
	}
	fclose(out);
}

