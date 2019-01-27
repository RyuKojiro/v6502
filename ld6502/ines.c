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
#include <stdlib.h>
#include <assert.h>

#include <as6502/parser.h>

#include "ines.h"

#define ines_16kUnits					16384
#define ines_8kUnits					8192

#define ines_magic						"NES\x1A"
#define ines_magicLength				4

typedef struct {
	char magic[4];        // "NES\x1a"
	uint8_t prg_rom_size; // 16 kB units
	uint8_t chr_rom_size; //  8 kB units
	uint8_t mapper_low;   // Lower nibble of mapper + flags
	uint8_t mapper_high;  // Upper nibble of mapper + flags
	uint8_t prg_ram_size; //  8 kB units
	uint8_t tv_system;    // 0=NTSC 1=PAL
	uint8_t extensions;   // Bus conflict stuff
	uint8_t padding[5];
} inesHeader;

int fileIsINES(FILE *infile) {
	if (!infile) {
		return NO;
	}

	// Read Magic
	char magic[ines_magicLength];
	if(fread(magic, ines_magicLength, 1, infile)) {
		if (!strncmp(magic, ines_magic, ines_magicLength)) {
			return YES;
		}
	}

	return NO;
}

void writeToINES(FILE *outfile, ld6502_object_blob *prg_rom, ld6502_object_blob *chr_rom, ines_properties *props) {
	// Create Header
	inesHeader header;
	memcpy(&header.magic, ines_magic, ines_magicLength);
	if (prg_rom) {
		header.prg_rom_size = prg_rom->len / ines_16kUnits;
	}
	if (chr_rom) {
		header.chr_rom_size = chr_rom->len / ines_8kUnits;
	}

	// Write header
	fwrite(&header, sizeof(header), 1, outfile);

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
	assert(sizeof(inesHeader) == 16);

	// Read Header
	inesHeader header;
	fread(&header, sizeof(inesHeader), 1, infile);
	const size_t prgRomSize = header.prg_rom_size * ines_16kUnits;

	if (prg_rom) {
		// Read PRG ROM
		prg_rom->len = prgRomSize;
		prg_rom->data = realloc(prg_rom->data, prgRomSize);
		fread(prg_rom->data, prgRomSize, 1, infile);
	}

	if (chr_rom) {
		// Read CHR ROM
		fseek(infile, sizeof(inesHeader) + prgRomSize, SEEK_SET); // If we didn't load PRG, or over-ran
		chr_rom->len = header.chr_rom_size * ines_8kUnits;
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
