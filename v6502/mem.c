//
//  mem.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdlib.h>

#include "mem.h"

// Error Text
#define kMemoryStructAllocErrorText		"Internal memory structure allocation"
#define kMemoryBlobAllocErrorText		"Memory blob allocation"

#define kUnableToMapMemoryErrorText		"Unable to map memory address"
#define kMemoryStructErrorText			"Internal memory structure inconsitency"
#define kMemoryBoundsErrorText			"Memory access out of bounds"

// Memory Boundaries
#define kMemoryStartWorkMemory			0x0000
#define kMemoryStartPPURegisters		0x2000
#define kMemoryStartAPURegisters		0x4000
#define kMemoryStartExpansionRom		0x4020
#define kMemoryStartSRAM				0x6000
#define kMemoryStartPRGROM				0x8000
#define kMemoryStartCeiling				0xFFFF

// Memory Blob Sizes
#define kMemorySizeWorkMemory			0x0800
#define kMemorySizePPURegisters			0x0008

#pragma mark -
#pragma mark Memory Lifecycle

/**
 * This is actually much more compilcated than dereferencing an offset, in fact,
 * it is actually creating a reference to it. However, it is creating a mapped
 * pointer to whatever it is supposed to be pointing to. This is NOT safe for
 * larger than single byte access.
 */
uint8_t *v6502_map(v6502_memory *memory, uint16_t offset) {
	// Safety
	if (!memory || !memory->bytes) {
		v6502_mfault(kMemoryStructErrorText);
		return NULL;
	}
	if (offset > memory->size) {
		v6502_mfault(kMemoryBoundsErrorText);
		return NULL;
	}
	
	// Work memory, 0x07FF + 3 mirrors = 0x1FFF
	if (offset < kMemoryStartPPURegisters) {
		offset %= kMemorySizeWorkMemory;
		
		return &memory->bytes[offset];
	}
	
	// PPU Registers
	if (offset >= kMemoryStartPPURegisters && offset < kMemoryStartAPURegisters) {
		offset %= kMemorySizePPURegisters;
		offset += kMemoryStartPPURegisters;
		
		return &memory->bytes[offset];
	}

	// FIXME: Everything past here, is it mapped or just in memory?
	// APU registers
	if (offset >= kMemoryStartAPURegisters && offset < kMemoryStartExpansionRom) {
		return &memory->bytes[offset];
	}

	// Expansion ROM (Cartridge)
	if (offset >= kMemoryStartExpansionRom && offset < kMemoryStartSRAM) {
		return &memory->bytes[offset];
	}
	
	// SRAM
	if (offset >= kMemoryStartSRAM && offset < kMemoryStartPRGROM) {
		return &memory->bytes[offset];
	}

	// PRG ROM
	if (offset >= kMemoryStartPRGROM && offset < kMemoryStartCeiling) {
		return &memory->bytes[offset];
	}
	
	v6502_mfault(kUnableToMapMemoryErrorText);
	return NULL;
}

void v6502_loadExpansionRomIntoMemory(v6502_memory *memory, uint8_t *rom, uint16_t size) {
	for (uint16_t i = 0; i < size; i++) {
		*v6502_map(memory, kMemoryStartExpansionRom + i) = rom[i];
	}
}

v6502_memory *v6502_createMemory(uint16_t size) {
	// Allocate Memory Struct
	v6502_memory *memory = malloc(sizeof(v6502_memory));
	if (!memory) {
		v6502_mfault(kMemoryStructAllocErrorText);
		return NULL;
	}
	
	// Allocate Virtual Memory
	memory->bytes = malloc(size);
	if (!memory->bytes) {
		free(memory);
		v6502_mfault(kMemoryBlobAllocErrorText);
		return NULL;
	}
	memory->size = size;
	
	// Zero memory
	for (uint16_t i = 0; i < size; i++) {
		memory->bytes[i] = 0x00;
	}
	
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
		return (0 - (0xFF - byte));
	}
	return byte;
}
