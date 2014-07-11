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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"

void v6502_printCpuState(v6502_cpu *cpu) {
	fprintf(stderr, "Status Register: %c%c%c%c%c%c%c%c\n",
			cpu->sr & v6502_cpu_status_negative ? 'N' : '-',
			cpu->sr & v6502_cpu_status_overflow ? 'V' : '-',
			cpu->sr & v6502_cpu_status_ignored ? 'X' : '-',
			cpu->sr & v6502_cpu_status_break ? 'B' : '-',
			cpu->sr & v6502_cpu_status_decimal ? 'D' : '-',
			cpu->sr & v6502_cpu_status_interrupt ? 'I' : '-',
			cpu->sr & v6502_cpu_status_zero ? 'Z' : '-',
			cpu->sr & v6502_cpu_status_carry ? 'C' : '-');
	fprintf(stderr, "CPU %p: pc = 0x%04x, ac = 0x%02x, x = 0x%02x, y = 0x%02x, sr = 0x%02x, sp = 0x%02x\nMEM %p: memsize = %hu (0x%04x)\n", cpu, cpu->pc, cpu->ac, cpu->x, cpu->y, cpu->sr, cpu->sp, cpu->memory, cpu->memory->size, cpu->memory->size);
}

void v6502_printMemoryRange(v6502_memory *memory, uint16_t start, uint16_t len) {
	uint16_t end = start + len;
	
	// Make sure we go to at least the same range specified, but then also round up to the nearest 0x0F
	start &= ~15;
	end |= 15;
	
	printf("      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	for (uint16_t y = start; y < end - 0x10; y += 0x10) {
		printf("%04x ", y);
		for (uint16_t x = y; x <= y + 0x0F; x++) {
			printf("%02x ", *v6502_map(memory, x));
		}
		printf("\n");
	}
}

void v6502_printBreakpointList(v6502_breakpoint_list *list) {
	if (list->count == 0) {
		printf("No breakpoints set.\n");
		return;
	}
	
	printf("Breakpoints set:\n");
	for (size_t i = 0; i < list->count; i++) {
		printf("breakpoint #%zu: 0x%04x\n", i, list->breakpoints[i]);
	}
}

