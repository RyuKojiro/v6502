//
//  ines.h
//  v6502
//
//  Created by Daniel Loffgren on 11/8/13.
//  Copyright (c) 2013 Hello-Channel, LLC. All rights reserved.
//

#ifndef ld6502_ines_h
#define ld6502_ines_h

struct ines_properties {
	//<#instance variables#>
};

void writeToINES(FILE *outfile, as6502_object_blob *prg_rom, as6502_object_blob *chr_rom);

#endif
