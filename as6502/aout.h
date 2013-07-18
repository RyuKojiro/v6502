//
/** @brief a.out object file transformer */
/** @file aout.h */
//  v6502
//
//  Created by Daniel Loffgren on H.25/07/17.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef as6502_aout_h
#define as6502_aout_h

#include "object.h"

/** @brief Writes an as6502_object directly to an a.out file */
void as6502_writeObjectToAOFile(as6502_object *obj, FILE *file);

#endif
