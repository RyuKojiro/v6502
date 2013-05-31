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

char *trimhead(char *str) {
	while (isspace(*str)) {
		str++;
	}
	
	return str;
}

char *trimheadchar(char *str, char token) {
	while (*str == token) {
		str++;
	}
	
	return str;
}

const char *rev_strnspc(const char *stop, const char *start) {
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

const char *strnchr(const char *str, char chr, size_t len) {
	for (size_t i = 0; str[i] && i < len; i++) {
		if (str[i] == chr) {
			return str + i;
		}
	}
	return NULL;
}
