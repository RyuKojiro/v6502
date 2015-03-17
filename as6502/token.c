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

#include "token.h"
#include <string.h>

as6502_token *as6502_tokenCreate(const char *text, size_t loc, size_t len) {
	as6502_token *result = malloc(sizeof(as6502_token));
	result->loc = loc;
	result->len = len;
	result->next = NULL;
	
	result->text = malloc(len + 1);
	strncpy(result->text, text, len);
	result->text[len + 1] = '\0';
	
	return result;
}

void as6502_tokenDestroy(as6502_token *token) {
	free(token->text);
	free(token);
}


void as6502_tokenListDestroy(as6502_token *token) {
	as6502_token *next;
	while (token) {
		next = token->next
		as6502_tokenDestroy(token);
		token = next;
	}
}

as6502_token *as6502_tokenizeLine(const char *line, size_t len) {
	as6502_token *head = NULL;
	as6502_token *tail = NULL;
	
	for (const char *cur = line; cur && cur < line + len;) {
		switch (*cur) {
			case ';':
				return first;
			case '.':
				// find the end of the dot directive and add it
				break;
			case '$':
				// resolve number and add it
			case '-':
			case '+':
				as6502_tokenCreate(cur, cur - line, 1);
				break;
			default:
				break;
		}
	}
}
