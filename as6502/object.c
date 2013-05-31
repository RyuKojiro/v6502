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

// Object Lifecycle
as6502_object *as6502_createObject() {
	as6502_object *obj = malloc(sizeof(as6502_object));
	if (obj) {
		obj->count = 0;
		obj->blobs = NULL;
		
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
	// TODO: object file writeout
}

void as6502_addBlobToObject(as6502_object *obj, uint16_t start, uint16_t len, uint8_t *data) {
	size_t size = sizeof(uint8_t) * len;
	uint8_t *newData = malloc(size);
	if (!newData) {
		die("data malloc in as6502_addBlobToObject");
	}
	
	memcpy(newData, data, size);
	
	obj->blobs = realloc(obj->blobs, sizeof(as6502_object_blob) * (obj->count + 1));
	if (!obj->blobs) {
		die("blobs realloc in as6502_addBlobToObject");
	}
	
	obj->blobs[obj->count].data = newData;
	obj->blobs[obj->count].start = start;
	obj->blobs[obj->count].len = len;
	
	obj->count++;
}


// Contextual Object Mutators
void as6502_processObjectDirectiveForLine(as6502_object_context *ctx, const char *line, size_t len) {
	if (len >= 3) {
		return;
	}

	if (!strncmp(line + 1, "org", 3)) {
		// start new blob
	}
	if (!strncmp(line + 1, "byte", 4)) {
		// convert byte and append to current blob
	}
}

as6502_object_blob *as6502_currentBlobInContext(as6502_object_context *ctx) {
	return ctx->obj->blobs[ctx->currentBlob];
}
