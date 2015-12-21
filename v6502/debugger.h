/** @brief Interactive Debugger */
/** @file debugger.h */

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

#ifndef v6502_debugger_h
#define v6502_debugger_h

#include <v6502/cpu.h>
#include <v6502/breakpoint.h>

/** @defgroup debugger Interactive Debugger */
/**@{*/
/** @brief This callback is used when v6502_handleDebuggerCommand recieves a run command */
typedef void(v6502_debuggerRunCallback)(v6502_cpu *cpu);

/** @brief Loads the binary data from file at fname into memory mem at given starting address */
void v6502_loadFileAtAddress(v6502_memory *mem, const char *fname, uint16_t address);
/** @brief Runs all debugger commands contained in a FILE pointer */
void v6502_runDebuggerScript(v6502_cpu *cpu, FILE *file, v6502_breakpoint_list *breakpoint_list, as6502_symbol_table *table, v6502_debuggerRunCallback runCallback, int *verbose);
/** @brief This is the exact function used by v6502_handleDebuggerCommand to do fuzzy string comparisons. It is exposed for extending the debugger to support other commands outside the v6502_handleDebuggerCommand function. */
int v6502_compareDebuggerCommand(const char * command, size_t len, const char * literal);
/** @brief Handle a command given by an external debugger line editor on a given v6502_cpu */
int v6502_handleDebuggerCommand(v6502_cpu *cpu, char *command, size_t len,
								v6502_breakpoint_list *breakpoint_list,
								as6502_symbol_table *table,
								v6502_debuggerRunCallback runCallback,
								int *verbose);
/**@}*/

#endif /* defined(v6502_debugger_h) */
