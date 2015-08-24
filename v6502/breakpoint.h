/** @brief Breakpoint Manangement */
/** @file breakpoint.h */

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

#include <stdint.h>

/** @defgroup breakpoint Breakpoint Management */
/**@{*/
/** @brief Breakpoint List Object */
typedef struct {
	/** @brief Array of breakpoint addresses */
	uint16_t *breakpoints;
	/** @brief Number of breakpoints in the array */
	size_t count;
} v6502_breakpoint_list;

/** @brief Create a v6502_breakpoint_list */
v6502_breakpoint_list *v6502_createBreakpointList(void);
/** @brief Destroy a v6502_breakpoint_list */
void v6502_destroyBreakpointList(v6502_breakpoint_list *list);
/** @brief Insert a breakpoint at the end of the specified v6502_breakpoint_list */
void v6502_addBreakpointToList(v6502_breakpoint_list *list, uint16_t address);
/** @brief Locate and remove the first matching address in a v6502_breakpoint_list */
void v6502_removeBreakpointFromList(v6502_breakpoint_list *list, uint16_t address);
/** @brief Check if an address is present in a v6502_breakpoint_list */
int v6502_breakpointIsInList(v6502_breakpoint_list *list, uint16_t address);
/**@}*/

#endif
