//
/** @brief Assembly language generation, optimization, and manipulation */
/** @file codegen.h */
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/21.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_codegen_h
#define v6502_codegen_h

#include <stdio.h>

#include "object.h"
#include "symbols.h"

/** @brief A line-based callback for code generation functions that emit multiple lines of code */
typedef uint16_t (* as6502_lineCallback)(as6502_object_blob *blob, const char *line, size_t len);

/** @defgroup codegen Code Generation Functions */
/**@{*/
/** @brief Resolves arithmetical operations down to their literal result */
void as6502_resolveArithmetic(char *line, size_t len);
/** @brief Replaces variable declarations with the instructions needed to initialize a variable without altering the current running state of the CPU */
int as6502_resolveVariableDeclaration(as6502_symbol_table *table, void *context, as6502_lineCallback cb, const char *line, size_t len);
/**@}*/

#endif
