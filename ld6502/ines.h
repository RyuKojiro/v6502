/** @brief iNES File Format Generator */
/** @file ines.h */

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

#ifndef ld6502_ines_h
#define ld6502_ines_h

#include "object.h"

typedef struct {
	int x;
} ines_properties;

/** @brief Take fully linked single flat as6502_object_blob of program code, a as6502_object_blob of the CHR ROM, and a struct of iNES ROM-specific ines_properties, and create an iNES ROM using them. */
void writeToINES(FILE *outfile, as6502_object_blob *prg_rom, as6502_object_blob *chr_rom, ines_properties *props);

/** @brief Load an iNES ROM in, populate an ines_properties struct using the header data, and split the ROM into it's PRG and CHR ROMs. */
void readFromINES(FILE *infile, as6502_object_blob *prg_rom, as6502_object_blob *chr_rom, ines_properties *props);

#endif
