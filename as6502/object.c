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
	
	obj->count = 0;
	obj->blobs = NULL;
	
	return obj;
}

void as6502_destroyObject(as6502_object *obj) {
	for (int i = 0; i < obj->count; i++) {
		free(obj->blobs[i].data);
	}
	free(obj->blobs);
	free(obj);
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
