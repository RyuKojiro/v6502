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
	if (obj->count > 1) {
		as6502_warn("Writing flat file with multiple segments will result in loss of object data");
	}
	fwrite(obj->blobs[0].data, 1, obj->blobs[0].len, file);
}

void as6502_readObjectFromFlatFile(as6502_object *obj, FILE *file) {
	as6502_addBlobToObject(obj, 0);
	as6502_object_blob *blob = &obj->blobs[0];
	
	uint8_t byte;
	while (!feof(file)) {
		fread(&byte, 1, 1, file);
		as6502_appendByteToBlob(blob, byte);
	}
}
