//
//  mem.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdlib.h>

#include "mem.h"
#include "core.h"

#pragma mark -
#pragma mark Memory Lifecycle

v6502_memory *v6502_createMemory(uint16_t size) {
	// Allocate Memory Struct
	v6502_memory *memory = malloc(sizeof(v6502_memory));
	if (!memory) {
		v6502_fault("VM Allocation - Internal Structure");
		return NULL;
	}
	
	// Allocate Virtual Memory
	memory->bytes = malloc(size);
	if (!memory->bytes) {
		free(memory);
		v6502_fault("VM Allocation - Blob");
		return NULL;
	}
	memory->size = size;
	
	return memory;
}

void v6502_destroyMemory(v6502_memory *memory) {
	free(memory->bytes);
	free(memory);
}

#pragma mark -
#pragma mark Memory Access Functionality

signed int v6502_signedValueOfByte(uint8_t byte) {
	if (byte & 0x80) {
		return (0 - (byte & 0x7F));
	}
	return byte;
}
