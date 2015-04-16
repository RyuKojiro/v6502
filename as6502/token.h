/** @brief Tokenization functionality */
/** @file token.h */

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

#ifndef __v6502__token__
#define __v6502__token__

#include <sys/types.h>

typedef enum {
	as6502_token_type_other,
	as6502_token_type_value
} as6502_token_type;

/** @struct */
/** @brief An individual token which will often be a part of a linked list of tokens */
typedef struct _as6502_token {
	/** @brief The raw text of a token */
	char *text;
	/** @brief The token's column location in the line */
	size_t loc;
	/** @brief The token's character length */
	size_t len;
	/** @brief The next token in the linked list */
	struct _as6502_token *next;
	/** @brief Significant type information to help identify this token during parsing */
	as6502_token_type type;
} as6502_token;

/** @defgroup tok_lifecycle Token Lifecycle Methods */
/**@{*/
/** @brief Creates a single token object. Text is copied up to length indicated by len. */
as6502_token *as6502_tokenCreate(const char *text, size_t loc, size_t len);
/** @brief Creates a single token object that is identical to the passed in object, but in a different location. The next pointer is not copied. */
as6502_token *as6502_tokenCopy(as6502_token *original);
/** @brief Destroys a single token object */
void as6502_tokenDestroy(as6502_token *token);
/** @brief Destroys a token and all tokens attached to it by traversing the linked list of tokens */
void as6502_tokenListDestroy(as6502_token *token);
/** @brief Does a quick token comparison to a string */
int as6502_tokenIsEqualToString(as6502_token *token, const char *string, size_t len);
/** @brief Does a quick token comparison to a string literal */
#define as6502_tokenIsEqualToStringLiteral(token, string) as6502_tokenIsEqualToString(token, string, sizeof(string) - 1)
/** @brief Outputs a flat string for a given token list */
void as6502_stringForTokenList(char *output, size_t len, as6502_token *head);
/** @brief Searches through a token list and returns the first token whose type matches the specified as6502_token_type */
as6502_token *as6502_firstTokenOfTypeInList(as6502_token *head, as6502_token_type type);
/**@}*/

/** @defgroup tok_lex Lexing */
/**@{*/
/** @brief Finds the first non-token character encountered, and returns it's location relative to the start pointer */
size_t as6502_lengthOfToken(const char *start, size_t len);
/** @brief Lexes a line of text into a linked list of tokens for later parsing */
as6502_token *as6502_lex(const char *line, size_t len);
/** @brief Searches through a linked list of tokens to find a token matching a given string of a given length */
as6502_token *as6502_tokenListFindToken(as6502_token *token, const char *text, size_t len);
/** @brief Searches through a linked list of tokens to find a token matching a given string literal */
#define as6502_tokenListFindTokenLiteral(token, literal) as6502_tokenListFindToken(token, literal, sizeof(literal) - 1)
/**@}*/

void as6502_showDotForLinkedList (as6502_token *head);

#endif /* defined(__v6502__token__) */
