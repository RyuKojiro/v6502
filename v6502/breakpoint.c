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
#include <assert.h>

#include <as6502/parser.h>

#include "breakpoint.h"

v6502_breakpoint_list *v6502_createBreakpointList(void) {
	return calloc(1, sizeof(v6502_breakpoint_list));
}

void v6502_destroyBreakpointList(v6502_breakpoint_list *list) {
	if (!list) {
		return;
	}
	
	free(list->breakpoints);
	free(list);
}

void v6502_addBreakpointToList(v6502_breakpoint_list *list, uint16_t address) {
	assert(list);

	if (!v6502_breakpointIsInList(list, address)) {
		list->breakpoints = realloc(list->breakpoints, sizeof(address) * (list->count + 1));
		list->breakpoints[list->count] = address;
		list->count++;
	}
}

static uint16_t *locationOfBreakpointInList(v6502_breakpoint_list *list, uint16_t address) {
	assert(list);

	for (size_t i = 0; i < list->count; i++) {
		if (list->breakpoints[i] == address) {
			return &list->breakpoints[i];
		}
	}
	return NULL;
}

int v6502_breakpointIsInList(v6502_breakpoint_list *list, uint16_t address) {
	return locationOfBreakpointInList(list, address) != NULL;
}

void v6502_removeBreakpointFromList(v6502_breakpoint_list *list, uint16_t address) {
	assert(list);

	uint16_t *loc = locationOfBreakpointInList(list, address);
	
	if (loc) {
		// Shift the entire list down one
		for (size_t i = ((loc - list->breakpoints) / sizeof(address)) + 1; i < list->count; i++) {
			list->breakpoints[i - 1] = list->breakpoints[i];
		}
		
		list->count--;
		list->breakpoints = realloc(list->breakpoints, sizeof(address) * list->count);
	}
}

