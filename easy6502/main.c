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

/*
 Notes:
 
 Memory location $fe contains a new random byte on every instruction.
 Memory location $ff contains the ascii code of the last key pressed.
 
 Memory locations $200 to $5ff map to the screen pixels. Different values will
 draw different colour pixels. The colours are:
 */

static int faulted;

void v6502_fault(const char *e) {
	faulted++;
}

int main(int argc, const char * argv[])
{
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(2048);
	
	v6502_reset(cpu);
	
	while (!faulted) {
		v6502_step(cpu);
		updateVideo(cpu->memory);
	}

	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
}

