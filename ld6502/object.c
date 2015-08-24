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

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include <as6502/error.h>
#include <as6502/parser.h>
#include <dis6502/reverse.h>

#include "object.h"
#include "flat.h"
#include "ines.h"

// Object Lifecycle
ld6502_object *ld6502_createObject() {
	ld6502_object *obj = malloc(sizeof(ld6502_object));
	if (obj) {
		obj->count = 0;
		obj->blobs = malloc(sizeof(ld6502_object_blob));
        obj->table = NULL;
		bzero(obj->blobs, sizeof(ld6502_object_blob));
		
		return obj;
	}
	
	as6502_fatal("obj malloc in ld6502_createObject");
	return NULL;
}

void ld6502_destroyObject(ld6502_object *obj) {
	if (!obj) {
		return;
	}
	
	for (int i = 0; i < obj->count; i++) {
		free(obj->blobs[i].data);
	}
	free(obj->blobs);
	free(obj);
}

void ld6502_loadObjectFromFile(ld6502_object *object, const char *fileName, ld6502_file_type type) {
	assert(object);
	
	FILE *in = fopen(fileName, "r");
	
	// Try to detect the file format if none is specified
	if (type == ld6502_file_type_None && fileIsINES(in)) {
		type = ld6502_file_type_iNES;
	}
	rewind(in);
	
	// Give up and take it in flat
	if (type == ld6502_file_type_None) {
		type = ld6502_file_type_FlatFile;
	}
	
	// Import object data
	switch (type) {
		case ld6502_file_type_FlatFile: {
			as6502_readObjectFromFlatFile(object, in);
			dis6502_deriveSymbolsForObject(object);
		} break;
		case ld6502_file_type_iNES: {
			ld6502_addBlobToObject(object, 0);
			readFromINES(in, &object->blobs[0], NULL, NULL);
		} break;
		case ld6502_file_type_None:
			// Something has gone horribly wrong.
			return;
	}
}

// Object Accessors
void ld6502_addBlobToObject(ld6502_object *obj, uint16_t start) {	
	assert(obj);
	
	obj->blobs = realloc(obj->blobs, sizeof(ld6502_object_blob) * (obj->count + 1));
	if (!obj->blobs) {
		as6502_fatal("blobs realloc in ld6502_addBlobToObject");
	}
	
	obj->blobs[obj->count].start = start;
	obj->blobs[obj->count].len = 0;
	obj->blobs[obj->count].data = NULL;

	obj->count++;
}

void ld6502_appendByteToBlob(ld6502_object_blob *blob, uint8_t byte) {
	assert(blob);
	
	if (!blob) {
		as6502_fatal("Null blob in ld6502_appendByteToBlob");
	}
	
	// This only applies while we still do single byte-at-a-time appending to blobs
	if (blob->len == UINT16_MAX) {
		as6502_fatal("Tried to overfill a blob in ld6502_appendByteToBlob");
	}
	
	uint16_t newSize = blob->len + 1;
	blob->data = realloc(blob->data, newSize);
	if (!blob->data) {
		as6502_fatal("blobs realloc in ld6502_appendByteToBlob");
	}
	blob->data[newSize - 1] = byte;
	blob->len = newSize;
}
