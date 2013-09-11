//
/** @brief Flat object file transformer */
/** @file flat.h */
//  v6502
//
//  Created by Daniel Loffgren on H.25/07/17.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef as6502_flat_h
#define as6502_flat_h

#include "object.h"

/** @brief Writes an as6502_object directly to a flat file */
void as6502_writeObjectToFlatFile(as6502_object *obj, FILE *file);
/** @brief Read an as6502_object from a flat file */
void as6502_readObjectFromFlatFile(as6502_object *obj, FILE *file);

#endif
