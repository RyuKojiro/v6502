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
#include "linectl.h"
#include "error.h"
#include "mem.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <sys/time.h>

as6502_token *as6502_tokenCreate(const char *text, size_t loc, size_t len) {
	as6502_token *result = malloc(sizeof(as6502_token));
	result->loc = loc;
	result->len = len;
	result->next = NULL;
	result->type = as6502_token_type_other;

	result->text = malloc(len + 1);
	strncpy(result->text, text, len);
	result->text[len] = '\0';
	
	return result;
}

as6502_token *as6502_tokenCopy(as6502_token *original) {
	as6502_token *copy = as6502_tokenCreate(original->text, original->loc, original->len);
	copy->type = original->type;
	return copy;
}

void as6502_tokenDestroy(as6502_token *token) {
	free(token->text);
	free(token);
}


void as6502_tokenListDestroy(as6502_token *token) {
	as6502_token *next;
	while (token) {
		next = token->next;
		as6502_tokenDestroy(token);
		token = next;
	}
}

int as6502_tokenIsEqualToString(as6502_token *token, const char *string, size_t len) {
	if (token->len != len) {
		return NO;
	}

	return !strncmp(token->text, string, len);
}

void as6502_stringForTokenList(char *output, size_t len, as6502_token *head) {
	bzero(output, len);
	
	while (head) {
		strncat(output, head->text, len);
		len -= head->len;

		head = head->next;
	}
}

as6502_token *as6502_firstTokenOfTypeInList(as6502_token *head, as6502_token_type type) {
	while (head) {
		if (head->type == type) {
			return head;
		}
		head = head->next;
	}
	return NULL;
}

static int _valueLengthInChars(const char *string, size_t len) {
	int i;
	for (i = 0; string[i] && (isdigit(CTYPE_CAST string[i]) || (string[i] >= 'a' && string[i] <= 'f')); i++);

	return i;
}

static int _isPartOfToken(char c) {
	return isalnum(c) || c == '_';
}

size_t as6502_lengthOfToken(const char *start, size_t len) {
	size_t i = 0;

	while (start[i] && i < len) {
		if (!_isPartOfToken(start[i])) {
			break;
		}

		i++;
	}

	return i;
}

#define insert(t)	if(!head) { head = t; tail = t; } \
					else { tail->next = t; tail = t; }

#define consumed	(cur - line)
#define	remaining	(len - consumed)

as6502_token *as6502_lex(const char *line, size_t len) {
	as6502_token *head = NULL;
	as6502_token *tail = NULL;
	
	for (const char *cur = line; *cur && cur < line + len;) {
		switch (*cur) {
			case ';':
				return head;
			case '.': {
				size_t tlen = as6502_lengthOfToken(cur + 1, remaining - 1);
				as6502_token *t = as6502_tokenCreate(cur, consumed, tlen + 1);
				insert(t);
				cur += tlen + 1;
			} break;
			case ':':
			case ')':
			case '(':
			case ',':
			case '=':
			case '-':
			case '+': {
				as6502_token *t = as6502_tokenCreate(cur, cur - line, 1);
				insert(t);
				cur++;
			} break;
			default:
				if (isspace(*cur)) {
					// seek over whitespace
					cur = strnpc(cur, remaining);
				}
				else if (isalpha(*cur)) {
					// handle alpha text, probably an instruction or symbol
					size_t tlen = as6502_lengthOfToken(cur, remaining);
					as6502_token *t = as6502_tokenCreate(cur, consumed, tlen);
					insert(t);
					cur += tlen;
				}
				else if (isnumber(*cur) || *cur == '$' || *cur == '%' || *cur == '#' || *cur == '*') {
					// handle what is definitely a number
					const char *start = cur;
					size_t tlen = 0;

					while (!isnumber(*start) && tlen <= 2) {
						start++;
						tlen++;
					}

					tlen += _valueLengthInChars(start, remaining);

					as6502_token *t = as6502_tokenCreate(cur, consumed, tlen);
					t->type = as6502_token_type_value;
					insert(t);
					cur += tlen;
				}
				else {
					as6502_warn(consumed, 1, "Don't know how to handle this char!");
					cur++;
				}
				break;
		}
	}

	return head;
}

as6502_token *as6502_tokenListFindToken(as6502_token *token, const char *text, size_t len) {
	while (token) {
		if (token->len == len && !strncmp(token->text, text, len)) {
			return token;
		}
		token = token->next;
	}
	
	return NULL;
}

void as6502_printDotRankForList(FILE *stream, as6502_token *head) {
	fprintf(stream, "{ rank = same; %lu; ", currentLineNum);
	while (head) {
		fprintf(stream, "\t\"%p\" [label=\"%s\"];", head, head->text);

		if (head->next) {
			fprintf(stream, "\t\"%p\" -> \"%p\";", head, head->next);
		}

		head = head->next;
	}

	fprintf(stream, "}\n");
}

void as6502_printDotForList(FILE *stream, as6502_token *head) {
	fprintf(stream, "digraph \"%p\" { label=\"Lex Results for line %lu\";rankdir=LR;", head, currentLineNum);
	while (head) {
		fprintf(stream, "\t\"%p\" [label=\"%s\"];", head, head->text);

		if (head->next) {
			fprintf(stream, "\t\"%p\" -> \"%p\";", head, head->next);
		}

		head = head->next;
	}

	fprintf(stream, "}\n");
}

void as6502_showDotForLinkedList(as6502_token *head) {
	char command[100];

	snprintf(command, 100, "open /tmp/%lu_%lu.gv", time(NULL), currentLineNum);
	//snprintf(command, 100, "dot -o /tmp/gv.png -Tpng /tmp/%lu.gv", time(NULL));

	FILE *out = fopen(command + 5, "w");
	//FILE *out = fopen(command + 25, "w");
	as6502_printDotForList(out, head);
	fclose(out);

	system(command);
	//system("feh /tmp/gv.png");
}
