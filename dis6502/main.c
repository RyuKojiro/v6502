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

#include "object.h"
#include "error.h"
#include "reverse.h"
#include "symbols.h"

#include "flat.h"
#include "ines.h"

#define MAX_FILENAME_LEN	255
#define MAX_LINE_LEN		80

static void disassembleFile(const char *in, FILE *out, ld6502_file_type format) {
	char line[MAX_LINE_LEN];
	int insideOfString = 0;
	
	ld6502_object *obj = ld6502_createObject();
	ld6502_loadObjectFromFile(obj, in, format);
	
	as6502_symbol_table *table = as6502_createSymbolTable();
	
	for (int i = 0; i < obj->count; i++) {
		ld6502_object_blob *blob = &obj->blobs[i];

		// Build Symbol Table
		currentLineNum = 1;
		dis6502_deriveSymbolsForObjectBlob(table, blob);

		// Disassemble
		currentLineNum = 1;
		for (uint16_t offset = 0; offset < blob->len; ){
			uint8_t opcode = blob->data[offset];
			as6502_symbol *label = as6502_symbolForAddress(table, offset);
			if (label) {
				fprintf(out, "%s:\n", label->name);
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
				dis6502_stringForInstruction(line, MAX_LINE_LEN, opcode, blob->data[offset + 2], blob->data[offset + 1]);
				as6502_symbolicateLine(table, line, MAX_LINE_LEN, v6502_memoryStartProgram, offset);

				if(!strncmp("???", line, 3) && isascii(opcode) && isprint(opcode)) {
					// We've encountered something that isn't runnable code, or is misaligned, but it is ascii.
					// Let's see if it's a string
					fprintf(out, "; String at $%04x:\n.ascii \"", offset);
					insideOfString = 1;
					continue;
				}

				offset += v6502_instructionLengthForOpcode(opcode) ;
			}

			fprintf(out, "\t%s\n", line);
			currentLineNum++;
		}
	}
	
	ld6502_destroyObject(obj);
}

static void usage() {
	fprintf(stderr, "usage: dis6502 [-o out_file] [-F format] [file ...]\n");
}

int main(int argc, char * const argv[]) {
	FILE *out = stdout;
	ld6502_file_type format = ld6502_file_type_None;
	char outName[MAX_FILENAME_LEN] = "";
	
	int ch;
	while ((ch = getopt(argc, argv, "o:F:")) != -1) {
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
				strncpy(outName, optarg, MAX_FILENAME_LEN);
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
		currentFileName = argv[i];
		if (*outName) {
			out = fopen(outName, "w");
		}
		disassembleFile(argv[i], out, format);
		fclose(out);
	}
}

