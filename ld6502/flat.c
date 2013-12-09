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
