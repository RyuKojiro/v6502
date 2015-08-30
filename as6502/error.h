/** @brief Assembler error handling */
/** @file error.h */

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

#ifndef v6502_error_h
#define v6502_error_h

/** @brief Current line number */
extern const char *currentLineText;
/** @brief Current line number */
extern unsigned long currentLineNum;
/** @brief Current file name */
extern const char *currentFileName;
/** @brief Last line number that encountered an error or warning */
extern unsigned long lastProblematicLine;

/** @defgroup as_error Assembler Error Handling */
/**@{*/
/** @brief Called for fatal errors during assembly, such as internal memory failures */
void as6502_fatal(const char *reason) __attribute((noreturn));
/** @brief Called for code errors detected during assembly, which prevent producing a binary */
void as6502_error(unsigned long loc, unsigned long len, const char *reason, ...);
/** @brief Called for code warnings detected during assembly, which don't prevent producing a binary */
void as6502_warn(unsigned long loc, unsigned long len, const char *reason, ...);
/** @brief Called for code notation which pertain to preceding errors or warnings, this can reference other code */
void as6502_note(unsigned long lineNumber, const char *reason, ...);
/** @brief This function outputs an underline annotation for a given character range */
void as6502_underline(unsigned long loc, unsigned long len);
/**@}*/

#endif
