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

#include <stdlib.h>
#include "breakpoint.h"
#include "parser.h"

v6502_breakpoint_list *v6502_createBreakPointList(void) {
	v6502_breakpoint_list *result = malloc(sizeof(v6502_breakpoint_list));
	
	result->breakpoints = NULL;
	result->count = 0;
	
	return result;
}

void v6502_destroyBreakPointList(v6502_breakpoint_list *list) {
	free(list->breakpoints);
	free(list);
}

void v6502_addBreakpointToList(v6502_breakpoint_list *list, uint8_t address) {
	if (!v6502_breakpointIsInList(list, address)) {
		list->breakpoints = realloc(list->breakpoints, sizeof(uint8_t) * (list->count + 1));
		list->breakpoints[list->count] = address;
		list->count++;
	}
}

static uint8_t *locationOfBreakpointInList(v6502_breakpoint_list *list, uint8_t address) {
	for (size_t i = 0; i < list->count; i++) {
		if (list->breakpoints[i] == address) {
			return &list->breakpoints[i];
		}
	}
	return NULL;
}

int v6502_breakpointIsInList(v6502_breakpoint_list *list, uint8_t address) {
	if (locationOfBreakpointInList(list, address)) {
		return YES;
	}
	return NO;
}

void v6502_removieBreakpointFromList(v6502_breakpoint_list *list, uint8_t address) {
	size_t loc = (locationOfBreakpointInList(list, address) - list->breakpoints) / sizeof(uint8_t);
	
	if (loc) {
		//<#statements#>
	}
}