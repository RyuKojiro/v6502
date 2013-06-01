//
/** @brief Binary object management */
/** @file object.h */
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef as6502_object_h
#define as6502_object_h

#include <stdio.h>
#include <stdint.h>

/** @struct */
/** @brief A single blob of typeless object data */
typedef struct {
	/** @brief The start address of the data */
	uint16_t start;
	/** @brief The length of the data */
	uint16_t len;
	/** @brief The data */
	uint8_t *data;
} as6502_object_blob;

/** @struct */
/** @brief The assembler's representation of an object file in memory */
typedef struct {
	/** @brief The number of as6502_object_blob's */
	int count;
	/** @brief An array of as6502_object_blob's */
	as6502_object_blob *blobs;
} as6502_object;

/** @struct */
/** @brief A running context that can be kept to build an object file procedurally */
typedef struct {
	/** @brief The as6502_object that a context refers to */
	as6502_object *obj;
	/** @brief The current blob in obj->blobs */
	int currentBlob;
} as6502_object_context;

/** @defgroup obj_lifecycle Object Lifecycle Functions */
/**@{*/
/** @brief Creates a new as6502_object */
as6502_object *as6502_createObject();
/** @brief Destroys an as6502_object */
void as6502_destroyObject(as6502_object *obj);
/** @brief Creates a new as6502_object_context */
as6502_object_context *as6502_createObjectContext();
/** @brief Destroys an as6502_object_context */
void as6502_destroyObjectContext(as6502_object_context *ctx);
/**@}*/

/** @defgroup obj_access Object Accessors */
/**@{*/
/** @brief Writes an as6502_object directly to a file handle */
void as6502_writeObjectToFile(as6502_object *obj, FILE *file);
/** @brief Creates an empty as6502_object_blob and adds it to an as6502_object */
void as6502_addBlobToObject(as6502_object *obj, uint16_t start);
/** @brief Appends a single byte to an as6502_object_blob */
void as6502_appendByteToBlob(as6502_object_blob *blob, uint8_t byte);
/**@}*/

/** @defgroup obj_mutate Contextual Object Mutators */
/**@{*/
/** @brief Automatically processes any dot directive in a given line and updates an as6502_object_context */
void as6502_processObjectDirectiveForLine(as6502_object_context *ctx, const char *line, size_t len);
/** @brief Get the current blob for a given as6502_object_context */
as6502_object_blob *as6502_currentBlobInContext(as6502_object_context *ctx);
/**@}*/

#endif
