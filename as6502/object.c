//
//  object.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "common.h"
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
	
	die("obj malloc in as6502_createObject");
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
			die("obj malloc in as6502_createObjectContext");
		}
	}

	die("ctx malloc in as6502_createObjectContext");
	return NULL;
}

void as6502_destroyObjectContext(as6502_object_context *ctx) {
	as6502_destroyObject(ctx->obj);
	free(ctx);
}

// Object Accessors
void as6502_writeObjectToFile(as6502_object *obj, FILE *file) {
	// TODO: Proper object file writeout
	fwrite(obj->blobs[0].data, 1, obj->blobs[0].len, file);
}

void as6502_addBlobToObject(as6502_object *obj, uint16_t start) {	
	obj->blobs = realloc(obj->blobs, sizeof(as6502_object_blob) * (obj->count + 1));
	if (!obj->blobs) {
		die("blobs realloc in as6502_addBlobToObject");
	}
	
	obj->blobs[obj->count].start = start;
	
	obj->count++;
}

void as6502_appendByteToBlob(as6502_object_blob *blob, uint8_t byte) {
	if (!blob) {
		die("Null blob in as6502_appendByteToBlob");
	}
	
	uint16_t newSize = blob->len + 1;
	blob->data = realloc(blob->data, newSize);
	if (!blob->data) {
		die("blobs realloc in as6502_appendByteToBlob");
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
	if (!strncasecmp(line + 1, "byte", 4)) {
		// convert byte and append to current blob
		uint8_t low;
		as6502_byteValuesForString(NULL, &low, NULL, line + 5);
		as6502_appendByteToBlob(as6502_currentBlobInContext(ctx), low);
	}
}

as6502_object_blob *as6502_currentBlobInContext(as6502_object_context *ctx) {
	return &(ctx->obj->blobs[ctx->currentBlob]);
}
