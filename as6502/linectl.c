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

#include <string.h>
#include <ctype.h>

#include "linectl.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

void trimtaild(char *str) {
	char *cur = str + strlen(str) - 1;
	while(cur > str && isspace(CTYPE_CAST *cur)) {
		cur--;
	}
	
	cur[1] = '\0';
}

void trimtailchard(char *str, char token) {
	char *cur = str + strlen(str) - 1;
	while (cur > str) {
		if (*cur == token) {
			*cur = '\0';
			return;
		}
		
		cur--;
	}
}

void trimgreedytaild(char *str, size_t len) {
	for (size_t i = 0; str[i]; i++) {
		if (isspace(CTYPE_CAST str[i])) {
			str[i] = '\0';
			return;
		}
	}
}

void trimgreedytailchard(char *str, char token) {
	while (*str) {
		if (*str == token) {
			*str = '\0';
			return;
		}
		
		str++;
	}
}

char *trimhead(const char *str, size_t len) {
	char *newstr = (char *)str;
	while (*newstr && isspace(CTYPE_CAST *newstr) && (newstr < str + len)) {
		newstr++;
	}
	
	return newstr;
}

char *trimheadchar(char *str, char token, size_t len) {
	char *newstr = str;
	
	while ((*newstr != token) && (newstr < str + len)) {
		newstr++;
	}
	
	return newstr;
}

char *trimheadtospc(const char *str, size_t len) {
	char *newstr = (char *)str;

	while (*newstr && !isspace(CTYPE_CAST *newstr) && (newstr < str + len)) {
		newstr++;
	}
	
	return newstr;
}

char *strnspc(const char *str, size_t len) {
	while (len && !isspace((int)*str)) {
		str++;
		len--;
	}
	
	return (char *)str;
}

char *strnpc(const char *str, size_t len) {
	while (len && isspace((int)*str)) {
		str++;
		len--;
	}

	return (char *)str;
}

char *rev_strnspc(const char *stop, const char *start) {
	for (/* start */; start > stop; start--) {
		if (isspace(CTYPE_CAST *start)) {
			return (char *)start;
		}
	}
	return NULL;
}

char *rev_strnpc(const char *stop, const char *start) {
	for (/* start */; start > stop; start--) {
		if (!isspace(CTYPE_CAST *start)) {
			return (char *)start;
		}
	}
	return NULL;
}

const char *rev_strnchr(const char *stop, const char *start, const char chr) {
	for (/* start */; start > stop; start--) {
		if (*start == chr) {
			return start;
		}
	}
	return NULL;
}

char *strnchr(const char *str, char chr, size_t len) {
	size_t i;
	for (i = 0; str[i] && i < len; i++) {
		if (str[i] == chr) {
			return (char *)(str + i);
		}
	}
	return NULL;
}

#pragma clang diagnostic warning "-Wincompatible-pointer-types"
#pragma GCC diagnostic pop

