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

#include "object.h"

#define ines_magic				"NES\x1A"
#define ines_magicLength		4
#define ines_prgRomUnits		16384
#define ines_chrRomUnits		8192
#define ines_headerDataLength	12

void writeToINES(FILE *outfile, as6502_object_blob *prg_rom, as6502_object_blob *chr_rom) {
	// Write Header
	char headerData[ines_headerDataLength];
	
	headerData[0] = prg_rom->len / ines_prgRomUnits;
	headerData[1] = chr_rom->len / ines_chrRomUnits;
	
	
	fwrite(ines_magic, ines_magicLength, 1, outfile);
	fwrite(headerData, ines_headerDataLength, 1, outfile);
}
