//
//  core.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "core.h"

void v6502_fault(const char *error) {
	fprintf(stderr, "fault: ");
	fprintf(stderr, "%s", error);
	if (error[strlen(error)] != '\n') {
		fprintf(stderr, "\n");
	}
}

void v6502_printCpuState(v6502_cpu *cpu) {
	fprintf(stderr, "CPU %p: pc = %x, ac = %x, x = %x, y = %x, sr = %x, sp = %x\nMEM %p: memsize = %hu\n", cpu, cpu->pc, cpu->ac, cpu->x, cpu->y, cpu->sr, cpu->sp, cpu->memory, cpu->memory->size);
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
			printf("%02x ", memory->bytes[x]);
		}
		printf("\n");
	}
}
