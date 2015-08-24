/** @brief Virtual Machine Logging */
/** @file log.h */

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

#ifndef v6502_core_h
#define v6502_core_h

#include "cpu.h"
#include "breakpoint.h"

/** @defgroup log VM State Logging */
/**@{*/
/** @brief Prints the current states and values of the CPU's registers. */
void v6502_printCpuState(v6502_cpu *cpu);
/** @brief Neatly prints a 52 column wide hex dump of a specified memory range. */
void v6502_printMemoryRange(v6502_memory *memory, uint16_t start, uint16_t len);
/** @brief Neatly prints the contents of a v6502_breakpoint_list. */
void v6502_printBreakpointList(v6502_breakpoint_list *list);
/**@}*/

#endif
