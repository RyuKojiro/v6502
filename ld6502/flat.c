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

#include <as6502/error.h>

#include "flat.h"

int _nonZeroBlobCount(ld6502_object *obj) {
	int total = 0;
	for (int i = 0; i < obj->count; i++) {
		if (obj->blobs[i].len) {
			total++;
		}
	}
	return total;
}

void as6502_writeObjectToFlatFile(ld6502_object *obj, FILE *file) {
	if (_nonZeroBlobCount(obj) > 1) {
		as6502_warn(0, 0, "Writing flat file with multiple segments will result in loss of object data");
	}
	fwrite(obj->blobs[obj->count - 1].data, 1, obj->blobs[obj->count - 1].len, file);
}

void as6502_readObjectFromFlatFile(ld6502_object *obj, FILE *file) {
	ld6502_addBlobToObject(obj, 0);
	ld6502_object_blob *blob = &obj->blobs[0];
	
	uint8_t byte;
	
	// FIXME: if the first byte is the last, this is probably going to break
	fread(&byte, 1, 1, file);
	do {
		ld6502_appendByteToBlob(blob, byte);
		fread(&byte, 1, 1, file);
	} while (!feof(file));
}
