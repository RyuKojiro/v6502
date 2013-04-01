//
//  main.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cpu.h"
#include "mem.h"
#include "core.h"
#include "parser.h"

#define MAX_COMMAND_LEN		80

static void popArg(char *str, size_t len) {
	char *space = strchr(str, ' ');
	if (!space) {
		return;
	}
	space++;

	size_t diff = space - str;
	strncpy(str, space, MAX_COMMAND_LEN - diff);
}

int main(int argc, const char * argv[])
{
	printf("Creating 1 virtual CPU…\n");
	v6502_cpu *cpu = v6502_createCPU();
	
	printf("Allocating virtual memory of size 2k…\n");
	cpu->memory = v6502_createMemory(2048);
	
	printf("Resetting CPU and dropping to interpreter…\n");
	v6502_reset(cpu);
	
	char command[MAX_COMMAND_LEN];
	for (;;) {
		printf("(0x%x) ", cpu->pc);
		fgets(command, sizeof(command), stdin);

		if (command[0] == '!') {
			if (!strncmp(command + 1, "cpu", 3)) {
				v6502_printCpuState(cpu);
				continue;
			}
			if (!strncmp(command + 1, "step", 4)) {
				v6502_step(cpu);
				continue;
			}
			if (!strncmp(command + 1, "peek", 4)) {
				popArg(command, MAX_COMMAND_LEN);
				
				// Make sure we don't go out of bounds either direction
				uint16_t start = strtol(command, NULL, 16);
				if (start <= 0x10) {
					start = 0x00;
				}
				else if (start >= cpu->memory->size - 0x30) {
					start = cpu->memory->size - 0x30;
				}
				else {
					start -= 0x10;
				}
				
				v6502_printMemoryRange(cpu->memory, start, 0x30);
				continue;
			}
			if (!strncmp(command + 1, "quit", 4)) {
				break;
			}
			printf("Unknown Command - %s\n", command);
		}
		else if (command[0] != ';') {
			v6502_executeAsmLineOnCPU(cpu, command, strlen(command));
		}
	}
	
	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	
    return 0;
}

