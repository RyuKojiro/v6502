//
//  ines.c
//  v6502
//
//  Created by Daniel Loffgren on 11/8/13.
//  Copyright (c) 2013 Hello-Channel, LLC. All rights reserved.
//

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
