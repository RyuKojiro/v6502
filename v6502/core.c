//
//  core.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "core.h"

void v6502_fault(const char *error) {
	fprintf(stderr, "fault: ");
	fprintf(stderr, "%s", error);
	if (error[strlen(error)] != '\n') {
		fprintf(stderr, "\n");
	}
}

void v6502_printCpuState(v6502_cpu *cpu) {
	fprintf(stderr, "CPU %p: pc = %x, ac = %x, x = %x, y = %x, sr = %x, sp = %x, mem = %p, memsize = %lu\n", cpu, cpu->pc, cpu->ac, cpu->x, cpu->y, cpu->sr, cpu->sp, cpu->memory, cpu->memory->size);
}

void v6502_printMemoryRange(v6502_memory *memory, uint8_t start, uint8_t len) {
	// Round to the nearest 0x10
	int alignment = start % 16;
	start -= alignment;
	
	// Make sure we go to at least the same range specified, but then also round up to the nearest 0x0F
	len += alignment;
	uint8_t end = start + len;
	alignment = end % 16;
	end -= alignment;
	end += 0x0F;
	
	printf("      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	for (uint8_t y = start; y < end - 0x10; y += 0x10) {
		printf("%04x ", y);
		for (uint8_t x = y; x <= y + 0x0F; x++) {
			printf("%02x ", *(uint8_t *)(memory->bytes + x));
		}
		printf("\n");
	}
}