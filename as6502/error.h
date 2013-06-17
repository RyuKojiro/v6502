//
/** @brief Assembler error handling */
/** @file error.h */
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_error_h
#define v6502_error_h

extern unsigned long currentLineNum;
extern const char *currentFileName;

/** @defgroup as_error Assembler Error Handling */
/**@{*/
/** @brief Called for fatal errors during assembly, such as internal memory failures */
__attribute((noreturn)) void as6502_fatal(const char *reason);
/** @brief Called for code errors detected during assembly, which prevent producing a binary */
void as6502_error(const char *error);
/** @brief Called for code warnings detected during assembly, which don't prevent producing a binary */
void as6502_warn(const char *warning);
/**@}*/

#endif
