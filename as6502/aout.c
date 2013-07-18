//
//  aout.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/07/17.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>

#include "aout.h"

void as6502_writeObjectToAOFile(as6502_object *obj, FILE *file) {
	fwrite(obj->blobs[0].data, 1, obj->blobs[0].len, file);
}
