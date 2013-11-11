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
#include <string.h>

#include "object.h"
#include "error.h"
#include "flat.h"
#include "symbols.h"
#include "ines.h"

#define MAX_FILENAME_LEN	255

typedef enum {
	ld6502_outputFormat_iNES
} ld6502_outputFormat;

static void linkObjects(FILE *outFile, FILE *chrFile, int numFiles, char * const files[]) {
	as6502_object *linkResult = as6502_createObject();
	
	// Read in a flat file as the only object
	FILE *flatFile = fopen(files[numFiles - 1], "r");
	as6502_readObjectFromFlatFile(linkResult, flatFile);
	fclose(flatFile);
	
	as6502_object *chrRom = NULL;
	if (chrFile) {
		as6502_readObjectFromFlatFile(chrRom, chrFile);
	}
	
	// Create the property struct
	ines_properties props;
	
	writeToINES(outFile, &linkResult->blobs[0], &chrRom->blobs[0], &props);
	as6502_destroyObject(linkResult);
}

static void usage() {
	fprintf(stderr, "usage: ld6502 [-o out_file] [-F format] [-C chr_rom] [file ...]\n");
}

int main(int argc, char * const argv[]) {
	ld6502_outputFormat format = ld6502_outputFormat_iNES;
	char outName[MAX_FILENAME_LEN] = "out.nes";
	char chrName[MAX_FILENAME_LEN] = "";
	
	int ch;
	while ((ch = getopt(argc, argv, "o:C:F:")) != -1) {
		switch (ch) {
			case 'F': {
				if (!strncmp(optarg, "ines", 4)) {
					format = ld6502_outputFormat_iNES;
				}
			} break;
			case 'o': {
				strncpy(outName, optarg, MAX_FILENAME_LEN);
			} break;
			case 'C': {
				strncpy(chrName, optarg, MAX_FILENAME_LEN);
			} break;
			case '?':
			default:
				usage();
				return 0;
		}
	}
	
	argc -= optind;
	argv += optind;
	
	FILE *out;
	FILE *chrFile = NULL;
	out = fopen(outName, "w");
	currentFileName = outName;

	if (*chrName) {
		chrFile = fopen(chrName, "r");
	}
	
	linkObjects(out, chrFile, argc, argv);
	fclose(chrFile);
	fclose(out);
}

