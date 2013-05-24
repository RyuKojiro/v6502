//
//  object.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef as6502_object_h
#define as6502_object_h

#include <stdio.h>
#include <stdint.h>

typedef struct {
	uint16_t start;
	uint16_t len;
	uint8_t *data;
} as6502_object_blob;

typedef struct {
	int count;
	as6502_object_blob *blobs;
} as6502_object;

typedef struct {
	as6502_object *obj;
	int currentBlob;
} as6502_object_context;

// Object Lifecycle
as6502_object *as6502_createObject();
void as6502_destroyObject(as6502_object *obj);
as6502_object_context *as6502_createObjectContext();
void as6502_destroyObjectContext(as6502_object_context *ctx);

// Object Accessors
void as6502_writeObjectToFile(as6502_object *obj, FILE *file);
void as6502_addBlobToObject(as6502_object *obj, uint16_t start, uint16_t len, uint8_t *data);

// Contextual Object Mutators
void as6502_processObjectDirectiveForLine(as6502_object_context *ctx, const char *line, size_t len);

#endif
