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

#include "object.h"
#include "error.h"
#include "parser.h"

// Object Lifecycle
as6502_object *as6502_createObject() {
	as6502_object *obj = malloc(sizeof(as6502_object));
	if (obj) {
		obj->count = 0;
		obj->blobs = malloc(sizeof(as6502_object_blob));
		bzero(obj->blobs, sizeof(as6502_object_blob));
		
		return obj;
	}
	
	as6502_fatal("obj malloc in as6502_createObject");
	return NULL;
}

void as6502_destroyObject(as6502_object *obj) {
	for (int i = 0; i < obj->count; i++) {
		free(obj->blobs[i].data);
	}
	free(obj->blobs);
	free(obj);
}

as6502_object_context *as6502_createObjectContext() {
	as6502_object_context *ctx = malloc(sizeof(as6502_object_context));
	
	if (ctx) {
		ctx->obj = as6502_createObject();
		ctx->currentBlob = 0;

		if (ctx->obj) {
			return ctx;
		}
		else {
			as6502_fatal("obj malloc in as6502_createObjectContext");
		}
	}

	as6502_fatal("ctx malloc in as6502_createObjectContext");
	return NULL;
}

void as6502_destroyObjectContext(as6502_object_context *ctx) {
	as6502_destroyObject(ctx->obj);
	free(ctx);
}

// Object Accessors
void as6502_addBlobToObject(as6502_object *obj, uint16_t start) {	
	obj->blobs = realloc(obj->blobs, sizeof(as6502_object_blob) * (obj->count + 1));
	if (!obj->blobs) {
		as6502_fatal("blobs realloc in as6502_addBlobToObject");
	}
	
	obj->blobs[obj->count].start = start;
	
	obj->count++;
}

void as6502_appendByteToBlob(as6502_object_blob *blob, uint8_t byte) {
	if (!blob) {
		as6502_fatal("Null blob in as6502_appendByteToBlob");
	}
	
	uint16_t newSize = blob->len + 1;
	blob->data = realloc(blob->data, newSize);
	if (!blob->data) {
		as6502_fatal("blobs realloc in as6502_appendByteToBlob");
	}
	blob->data[newSize - 1] = byte;
	blob->len = newSize;
}

// Contextual Object Mutators
void as6502_processObjectDirectiveForLine(as6502_object_context *ctx, const char *line, size_t len) {
	if (len <= 3) {
		return;
	}

	if (!strncasecmp(line + 1, "org", 3)) {
		// start new blob
		as6502_addBlobToObject(ctx->obj, as6502_valueForString(NULL, line + 5));
		ctx->currentBlob = ctx->obj->count - 1;
	}
	if (!strncasecmp(line + 1, "end", 3)) {
		// revert to top blob
		ctx->currentBlob = 0;
	}
}

as6502_object_blob *as6502_currentBlobInContext(as6502_object_context *ctx) {
	return &(ctx->obj->blobs[ctx->currentBlob]);
}
