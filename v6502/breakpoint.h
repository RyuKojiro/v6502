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

#ifndef v6502_breakpoint_h
#define v6502_breakpoint_h

typedef struct {
	uint16_t *breakpoints;
	size_t count;
} v6502_breakpoint_list;

v6502_breakpoint_list *v6502_createBreakpointList(void);
void v6502_destroyBreakpointList(v6502_breakpoint_list *list);
void v6502_addBreakpointToList(v6502_breakpoint_list *list, uint16_t address);
void v6502_removeBreakpointFromList(v6502_breakpoint_list *list, uint16_t address);
int v6502_breakpointIsInList(v6502_breakpoint_list *list, uint16_t address);
void v6502_printBreakpointList(v6502_breakpoint_list *list);

#endif
