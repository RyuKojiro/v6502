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

void v6502_faultExternal(const char *error) {
	fprintf(stderr, "External fault: ");
	fprintf(stderr, "%s", error);
	if (error[strlen(error)] != '\n') {
		fprintf(stderr, "\n");
	}
}

void v6502_printCpuState(v6502_cpu *cpu) {
	fprintf(stderr, "CPU %p: pc = %x, ac = %x, x = %x, y = %x, sr = %x, sp = %x, mem = %p, memsize = %d\n", cpu, cpu->pc, cpu->ac, cpu->x, cpu->y, cpu->sr, cpu->sp, cpu->memory, cpu->memory->size);
}