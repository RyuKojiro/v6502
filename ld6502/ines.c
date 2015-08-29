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
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <as6502/parser.h>

#include "ines.h"

#define ines_16kUnits					16384
#define ines_8kUnits					8192

#define ines_magic						"NES\x1A"
#define ines_magicLength				4
#define ines_headerDataLength			16
#define ines_headerPrgRomSizeField16	4
#define ines_headerChrRomSizeField8		5
#define ines_headerPrgRomSizeField8		8
#define ines_headerZeroPaddingStart		11

int fileIsINES(FILE *infile) {
	if (!infile) {
		return NO;
	}
	
	// Read Magic
	char magic[ines_magicLength];
	fread(magic, ines_magicLength, 1, infile);
	
	if (!strncmp(magic, ines_magic, ines_magicLength)) {
		return YES;
	}
	
	return NO;
}

void writeToINES(FILE *outfile, ld6502_object_blob *prg_rom, ld6502_object_blob *chr_rom, ines_properties *props) {
	// Create Header
	char headerData[ines_headerDataLength];
	snprintf(headerData, ines_magicLength, ines_magic);
	headerData[ines_headerPrgRomSizeField16] = (prg_rom ? prg_rom->len : 0) / ines_16kUnits;
	headerData[ines_headerChrRomSizeField8] = (chr_rom ? chr_rom->len : 0) / ines_8kUnits;
	headerData[/* flags */ 6] = 0;
	headerData[/* flags */ 7] = 0;
	headerData[ines_headerPrgRomSizeField8] = (prg_rom ? prg_rom->len : 0) / ines_8kUnits;
	headerData[/* flags */ 9] = 0;
	headerData[/* flags */ 10] = 0;
	bzero(headerData + ines_headerZeroPaddingStart, ines_headerDataLength - ines_headerZeroPaddingStart);
	
	// Write header
	fwrite(headerData, ines_headerDataLength, 1, outfile);
	
	// Write PRG ROM
	if (prg_rom) {
		fwrite(prg_rom->data, prg_rom->len, 1, outfile);
	}

	// Write CHR ROM
	if (chr_rom) {
		fwrite(chr_rom->data, chr_rom->len, 1, outfile);
	}
}

void readFromINES(FILE *infile, ld6502_object_blob *prg_rom, ld6502_object_blob *chr_rom, ines_properties *props) {
	// Read Header
	char headerData[ines_headerDataLength];
	fread(headerData, ines_headerDataLength, 1, infile);
	uint16_t prgRomSize;
	
	// Try for 8k precision, if older format fallback to 16k
	if (headerData[ines_headerPrgRomSizeField8]) {
		prgRomSize = headerData[ines_headerPrgRomSizeField8] * ines_8kUnits;
	}
	else {
		prgRomSize = headerData[ines_headerPrgRomSizeField16] * ines_16kUnits;
	}
	
	if (prg_rom) {
		// Read PRG ROM
		prg_rom->len = prgRomSize;
		prg_rom->data = realloc(prg_rom->data, prgRomSize);
		fread(prg_rom->data, prgRomSize, 1, infile);
	}

	if (chr_rom) {
		// Read CHR ROM
		fseek(infile, prgRomSize, SEEK_SET); // If we didn't load PRG, or over-ran
		chr_rom->len = headerData[ines_headerChrRomSizeField8] * ines_8kUnits;
		chr_rom->data = realloc(chr_rom->data, chr_rom->len);
		fread(chr_rom->data, chr_rom->len, 1, infile);
	}
}

void as6502_writeObjectToINES(ld6502_object *obj, FILE *file) {
	ines_properties props;
	props.videoMode = ines_videoMode_NTSC;
	
	ld6502_object_blob prg_rom;
	
	// Flatten all segments into a single PRG ROM blob
	for (int i = 0; i < obj->count; i++) {
		memcpy(&prg_rom.data[obj->blobs[i].start], obj->blobs[i].data, obj->blobs[i].len);
	}
	
	writeToINES(file, &prg_rom, NULL, &props);
}

void as6502_readObjectFromINES(ld6502_object *obj, FILE *file) {

}
