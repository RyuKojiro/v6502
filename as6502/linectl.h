/** @brief Portable per-line string manipulation */
/** @file linectl.h */

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

#ifndef v6502_linectl_h
#define v6502_linectl_h

#include <sys/types.h>

/** @brief This is to cope with <ctype.h> functions, which all expect int sized chars */
#define CTYPE_CAST	(int)

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

/** @brief Search string for first encounter of a space, safely. If no whitespace is found, a pointer to the end is returned */
char *strnspc(const char *str, size_t len);

/** @brief Search string for first encounter of a printable char (non-whitespace), safely. If no printable character is found, a pointer to the end is returned */
char *strnpc(const char *str, size_t len);

/** @brief Reverse search string for space, safely */
/** @param[in] start The location in the string to start searching backwards from */
/** @param[in] stop The earliest location in the string to search up to, usually the beginning of the string */
char *rev_strnspc(const char *stop, const char *start);

/** @brief Reverse search string for printable char (non-whitespace), safely */
/** @param[in] start The location in the string to start searching backwards from */
/** @param[in] stop The earliest location in the string to search up to, usually the beginning of the string */
char *rev_strnpc(const char *stop, const char *start);

/** @brief Reverse search string for character, safely */
/** @param[in] start The location in the string to start searching backwards from */
/** @param[in] stop The earliest location in the string to search up to, usually the beginning of the string */
/** @param[in] chr The character to look for */
const char *rev_strnchr(const char *stop, const char *start, const char chr);

/** @brief Safely search potentially unterminated string for character */
char *strnchr(const char *str, char chr, size_t len);
/**@}*/

#endif
