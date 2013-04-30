//
//  mem.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_mem_h
#define v6502_mem_h

#include <stdint.h>

// Memory Object
typedef struct {
	uint8_t *bytes;
	uint16_t size;
} v6502_memory;

// Memory Lifecycle
v6502_memory *v6502_createMemory(uint16_t size);
void v6502_destroyMemory(v6502_memory *memory);
void v6502_loadExpansionRomIntoMemory(v6502_memory *memory, uint8_t *rom, uint16_t size);

// Memory Access
uint8_t *v6502_map(v6502_memory *memory, uint16_t offset);
signed int v6502_signedValueOfByte(uint8_t byte);

#endif
