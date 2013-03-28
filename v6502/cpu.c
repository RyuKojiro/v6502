//
//  cpu.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdlib.h>

#include "cpu.h"
#include "core.h"

v6502_cpu *v6502_createCPU(void) {
	// Allocate CPU Struct
	v6502_cpu *cpu = malloc(sizeof(v6502_cpu));
	if (!cpu) {
		v6502_faultExternal("CPU Allocation - Internal Structure");
		return NULL;
	}
	return cpu;
}

void v6502_destroyCPU(v6502_cpu *cpu) {
	free(cpu);
}