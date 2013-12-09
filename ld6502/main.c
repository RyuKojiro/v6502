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
#include <stdlib.h>

#include "object.h"
#include "error.h"
#include "flat.h"
#include "symbols.h"
#include "ines.h"

#define MAX_FILENAME_LEN	255

typedef enum {
	ld6502_outputFormat_iNES
} ld6502_outputFormat;

static void loadObjectFromFile(ld6502_object *object, const char *fileName) {
	
}

static void linkObjects(FILE *outFile, FILE *chrFile, int numFiles, char * const files[]) {
	ld6502_object *linkResult = ld6502_createObject();
	ld6502_object **objects = malloc(numFiles * sizeof(ld6502_object *));
	
	// Read in a flat file as the only object
	FILE *flatFile = fopen(files[numFiles - 1], "r");
	as6502_readObjectFromFlatFile(linkResult, flatFile);
	fclose(flatFile);
	
	///////////// LOAD /////////////
	for (int o = 0; o < numFiles; o++) {
		loadObjectFromFile(objects[o], files[o]);
	}
	
	ld6502_object *chrRom = NULL;
	if (chrFile) {
		as6502_readObjectFromFlatFile(chrRom, chrFile);
	}
	
	///////////// LINK /////////////
	// Iterate through symbol table, copy all objects into a new single flat object until all unlinked symbols are resolved, if any symbols cannot be found, error.
	// Also make sure to consolidate all symbol tables into a singular master table that has all symbols.
	
	// Iterate thorugh unlinked objects
	for (int o = 0; o < numFiles; o++) {
		ld6502_object *currentObject = objects[o];
		for (as6502_symbol *currentSymbol = currentObject->table->first_symbol; currentSymbol; currentSymbol = currentSymbol->next) {
			// Does symbol already exist in linkResult, if not, we know it doesn't exist on any objects before objects[o + 1]
			
			// If not, copy required symbol, otherwise change symbol address
		}
		
	}
	
	/////////// CLEAN UP ///////////
	for (int o = 0; o < numFiles; o++) {
		ld6502_destroyObject(objects[o]);
	}
	
	// Create the property struct
	ines_properties props;
	
	writeToINES(outFile, &linkResult->blobs[0], &chrRom->blobs[0], &props);
	ld6502_destroyObject(linkResult);
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

