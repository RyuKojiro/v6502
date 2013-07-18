//
//  flat.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/07/17.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>

#include "flat.h"
#include "error.h"

void as6502_writeObjectToFlatFile(as6502_object *obj, FILE *file) {
	as6502_warn("Flat file object format will lose all symbol table data");
	fwrite(obj->blobs[0].data, 1, obj->blobs[0].len, file);
}
