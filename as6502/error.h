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

/** @brief Current line number */
extern unsigned long currentLineNum;
/** @brief Current file name */
extern const char *currentFileName;

/** @defgroup as_error Assembler Error Handling */
/**@{*/
/** @brief Called for fatal errors during assembly, such as internal memory failures */
void as6502_fatal(const char *reason) __attribute((noreturn));
/** @brief Called for code errors detected during assembly, which prevent producing a binary */
void as6502_error(const char *reason, ...);
/** @brief Called for code warnings detected during assembly, which don't prevent producing a binary */
void as6502_warn(const char *reason);
/** @brief Called for code notation which pertain to preceding errors or warnings, this can reference other code */
void as6502_note(unsigned long lineNumber, const char *reason, ...);
/**@}*/

#endif
