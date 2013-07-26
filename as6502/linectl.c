//
//  linectl.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include "linectl.h"

#include <string.h>
#include <ctype.h>

#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

void trimtaild(char *str) {
	char *cur = str + strlen(str) - 1;
	while(cur > str && isspace(*cur)) {
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

void trimgreedytaild(char *str) {
	while (*str) {
		if (isspace(*str)) {
			*str = '\0';
			return;
		}
		
		str++;
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
	char *newstr = str;
	while (*newstr && isspace(*newstr) && (newstr < str + len)) {
		newstr++;
	}
	
	return newstr;
}

char *trimheadchar(char *str, char token, size_t len) {
	char *newstr = str;
	
	while ((*newstr == token) && (newstr < str + len)) {
		newstr++;
	}
	
	return newstr;
}

char *trimheadtospc(const char *str, size_t len) {
	char *newstr = str;

	while (*newstr && !isspace(*newstr) && (newstr < str + len)) {
		newstr++;
	}
	
	return newstr;
}

char *rev_strnspc(const char *stop, const char *start) {
	for (/* start */; start > stop; start--) {
		if (isspace(*start)) {
			return start;
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
			return str + i;
		}
	}
	return NULL;
}

#pragma GCC diagnostic warning "-Wcast-qual"
#pragma clang diagnostic warning "-Wincompatible-pointer-types"
