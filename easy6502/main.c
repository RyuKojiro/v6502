//
//  main.c
//  easy6502-xcode
//
//  Created by Daniel Loffgren on H.25/06/02.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include "cpu.h"
#include "mem.h"

#include "video.h"
#include "state.h"

static int faulted;

void v6502_fault(const char *e) {
	faulted++;
}


static FILE *logFile;
static unsigned long cycle;

static void logCpuStatus(v6502_cpu *cpu) {	
	fprintf(logFile, "Cycle #%lu\n", cycle);
	
	fprintf(logFile, "Status Register: %c%c%c%c%c%c%c%c\n",
			cpu->sr & v6502_cpu_status_negative ? 'N' : '-',
			cpu->sr & v6502_cpu_status_overflow ? 'V' : '-',
			cpu->sr & v6502_cpu_status_ignored ? 'X' : '-',
			cpu->sr & v6502_cpu_status_break ? 'B' : '-',
			cpu->sr & v6502_cpu_status_decimal ? 'D' : '-',
			cpu->sr & v6502_cpu_status_interrupt ? 'I' : '-',
			cpu->sr & v6502_cpu_status_zero ? 'Z' : '-',
			cpu->sr & v6502_cpu_status_carry ? 'C' : '-');
	fprintf(logFile, "CPU %p: pc = %#04x, ac = %#02x, x = %#02x, y = %#02x, sr = %#02x, sp = %#02x\nMEM %p: memsize = %hu (%#04x)\n\n", cpu, cpu->pc, cpu->ac, cpu->x, cpu->y, cpu->sr, cpu->sp, cpu->memory, cpu->memory->size, cpu->memory->size);
}

void broken() {
	mvaddnstr(2, 2, "BREAK", 5);
	while (getch() != KEY_ENTER) {}
}

void loadProgram(v6502_memory *mem, const char *fname) {
	FILE *f = fopen(fname, "r");
	uint8_t byte;
	uint8_t offset = 0;
	
	while (fread(&byte, 1, 1, f)) {
		mem->bytes[v6502_memoryStartProgram + (offset++)] = byte;
	}
	
	fprintf(logFile, "Loaded ROM of %d bytes\n\n", offset);
	
	fclose(f);
}

int main(int argc, const char * argv[])
{
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(2048);
	WINDOW *scr = initVideo();
	initState();
	
	if (!logFile) {
		logFile = fopen("easy6502.log", "w");
	}

	// Load program code into memory
	loadProgram(cpu->memory, "/Users/kojiro/Code/v6502/easy6502/easy_test.o");
	
	v6502_reset(cpu);
	while (!faulted) {
		// Update keypress byte, hold for clock cycle, and refresh random byte
		stateCycle(cpu->memory);
		
		// Processor time
		v6502_step(cpu);
				
		// Debug Logging
		logCpuStatus(cpu);
		cycle++;
		
		// Exit on break
		if (cpu->sr & v6502_cpu_status_break) {
			broken();
			break;
		}

		// Refresh Video
		updateVideo(cpu->memory, scr);
	}

	fclose(logFile);
	
	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
}

