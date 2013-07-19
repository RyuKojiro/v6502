//
/** @brief Portable per-line string manipulation */
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

/** @brief Destructively tail trim at first encounter of whitespace from the head end */
void trimgreedytaild(char *str);

/** @brief Destructively tail trim at first encounter of token char from the head end */
void trimgreedytailchard(char *str, char token);

/** @brief Safely trim leading whitespace by pushing pointer */
char *trimhead(const char *str, size_t len);

/** @brief Safely trim head til first encounter of token char from the head end */
char *trimheadchar(char *str, char token, size_t len);

/** @brief Safely trim head til first encounter of whitespace from the head end */
char *trimheadtospc(const char *str, size_t len);

/** @brief Reverse search string for space, safely */
/** @param[in] start The location in the string to start searching backwards from */
/** @param[in] stop The earliest location in the string to search up to, usually the beginning of the string */
char *rev_strnspc(const char *stop, const char *start);

/** @brief Reverse search string for character, safely */
/** @param[in] start The location in the string to start searching backwards from */
/** @param[in] stop The earliest location in the string to search up to, usually the beginning of the string */
/** @param[in] chr The character to look for */
const char *rev_strnchr(const char *stop, const char *start, const char chr);

/** @brief Safely search potentially unterminated string for character */
char *strnchr(const char *str, char chr, size_t len);
/**@}*/

#endif
