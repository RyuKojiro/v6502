//
//  main.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "cpu.h"
#include "mem.h"
#include "core.h"

int main(int argc, const char * argv[])
{
	printf("Creating 1 virtual CPU…\n");
	v6502_cpu *cpu = v6502_createCPU();
	
	printf("Allocating virtual memory of size 2k…\n");
	cpu->memory = v6502_createMemory(2048);
	
	char command[10];
	for (;;) {
		printf("] ");
		scanf("%s", command);
		
		if (!strncmp(command, "!", 2)) {
			v6502_printCpuState(cpu);
		}
	}
	
    return 0;
}

