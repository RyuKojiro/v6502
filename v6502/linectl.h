//
/** @file linectl.h */
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_linectl_h
#define v6502_linectl_h

#include <sys/types.h>

/** @defgroup linectl String Manipulation Functions */
/**@{*/
/** @brief Destructively trim trailing whitespace with NUL */
void trimtaild(char *str);

/** @brief Destructively tail trim at first encounter of token char from the tail end */
void trimtailchard(char *str, char token);

/** @brief Destructively tail trim at first encounter of token char from the head end */
void trimgreedytaild(char *str);

/** @brief Destructively tail trim at first encounter of token char from the head end */
void trimgreedytailchard(char *str, char token);

/** @brief Safely trim leading whitespace by pushing pointer */
char *trimhead(char *str);

/** @brief Safely trim head til first encounter of token char from the head end */
char *trimheadchar(char *str, char token);

/** @brief Reverse search string for space, safely */
char *rev_strnspc(char *str, char *start);

/** @brief Reverse search string for character, safely */
char *rev_strnchr(char *str, char *start, char chr);

/** @brief Safely search potentially unterminated string for character */
char *strnchr(char *str, char chr, size_t len);
/**@}*/

#endif
