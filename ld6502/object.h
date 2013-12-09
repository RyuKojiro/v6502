/** @brief Binary object management */
/** @file object.h */

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

#ifndef as6502_object_h
#define as6502_object_h

#include <stdio.h>
#include <stdint.h>

#include "symbols.h"

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
	/** @brief The as6502_symbol_table that contains all symbols that correspond to each of the attached blobs */
	as6502_symbol_table *table;
	/** @brief An array of as6502_object_blob's */
	as6502_object_blob *blobs;
	/** @brief The number of as6502_object_blob's */
	int count;
} as6502_object;

/** @defgroup obj_lifecycle Object Lifecycle Functions */
/**@{*/
/** @brief Creates a new as6502_object */
as6502_object *as6502_createObject();
/** @brief Destroys an as6502_object */
void as6502_destroyObject(as6502_object *obj);
/**@}*/

/** @defgroup obj_access Object Accessors */
/**@{*/
/** @brief Creates an empty as6502_object_blob and adds it to an as6502_object */
void as6502_addBlobToObject(as6502_object *obj, uint16_t start);
/** @brief Appends a single byte to an as6502_object_blob */
void as6502_appendByteToBlob(as6502_object_blob *blob, uint8_t byte);
/**@}*/

/** @defgroup obj_mutate Contextual Object Mutators */
/**@{*/
/** @brief Automatically processes any dot directive in a given line and updates an as6502_object_context */
void as6502_processObjectDirectiveForLine(as6502_object *obj, int *currentBlob, const char *line, size_t len);
/**@}*/

#endif
