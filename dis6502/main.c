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

#include "object.h"
#include "error.h"
#include "reverse.h"
#include "symbols.h"

#include "flat.h"
#include "ines.h"

#define MAX_FILENAME_LEN	255
#define MAX_LINE_LEN		80

typedef enum {
	dis6502_inputFormat_None = 0,
	dis6502_inputFormat_iNES,
	dis6502_inputFormat_FlatFile
} dis6502_inputFormat;

static void disassembleFile(FILE *in, FILE *out, dis6502_inputFormat format) {
	char line[MAX_LINE_LEN];
	
	as6502_object *obj = as6502_createObject();
	
	// Try to detect the file format if none is specified
	if (format == dis6502_inputFormat_None && fileIsINES(in)) {
		format = dis6502_inputFormat_iNES;
	}
	rewind(in);

	// Give up and take it in flat
	if (format == dis6502_inputFormat_None) {
		format = dis6502_inputFormat_FlatFile;
	}
	
	// Import object data
	switch (format) {
		case dis6502_inputFormat_FlatFile: {
			as6502_readObjectFromFlatFile(obj, in);
			dis6502_deriveSymbolsForObject(obj);
		} break;
		case dis6502_inputFormat_iNES: {
			as6502_addBlobToObject(obj, 0);
			readFromINES(in, &obj->blobs[0], NULL, NULL);
		} break;
		case dis6502_inputFormat_None:
			// Something has gone horribly wrong.
			return;
	}
	
	as6502_symbol_table *table = as6502_createSymbolTable();
	
	for (int i = 0; i < obj->count; i++) {
		as6502_object_blob *blob = &obj->blobs[i];

		// Build Symbol Table
		currentLineNum = 0;
		dis6502_deriveSymbolsForObjectBlob(table, blob);

		// Disassemble
		currentLineNum = 0;
		for (uint16_t offset = 0; offset < blob->len; offset += v6502_instructionLengthForOpcode(blob->data[offset])) {
			as6502_symbol *label = as6502_symbolForAddress(table, offset);
			if (label) {
				fprintf(out, "%s:\n", label->name);
			}
			
			dis6502_stringForInstruction(line, MAX_LINE_LEN, blob->data[offset], blob->data[offset + 2], blob->data[offset + 1]);
			as6502_symbolicateLine(table, line, MAX_LINE_LEN, v6502_memoryStartProgram, offset);
			
			fprintf(out, "\t%s\n", line);
			currentLineNum++;
		}
	}
	
	as6502_destroyObject(obj);
}

static void usage() {
	fprintf(stderr, "usage: dis6502 [-o out_file] [-F format] [file ...]\n");
}

int main(int argc, char * const argv[]) {
	FILE *in;
	FILE *out = stdout;
	dis6502_inputFormat format = dis6502_inputFormat_None;
	char outName[MAX_FILENAME_LEN] = "";
	
	int ch;
	while ((ch = getopt(argc, argv, "o:F:")) != -1) {
		switch (ch) {
			case 'F': {
				if (!strncmp(optarg, "ines", 4)) {
					format = dis6502_inputFormat_iNES;
				}
				if (!strncmp(optarg, "flat", 4)) {
					format = dis6502_inputFormat_FlatFile;
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
		in = fopen(argv[i], "r");
		currentFileName = argv[i];
		if (*outName) {
			out = fopen(outName, "w");
		}
		disassembleFile(in, out, format);
		fclose(in);
		fclose(out);
	}
}

