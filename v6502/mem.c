/*
 * Copyright (c) 2013 Daniel Loffgren
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>

#include "mem.h"

// Error Text
#define v6502_unableToMapMemoryErrorTextv6502	"Unable to map memory address"
#define v6502_memoryStructErrorText				"Internal memory structure inconsitency"
#define v6502_memoryBoundsErrorText				"Memory access out of bounds"

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
		if (memory && memory->fault_callback) {
			memory->fault_callback(memory->fault_context, v6502_memoryStructErrorText);
		}
		
		return NULL;
	}
	if (offset > memory->size) {
		if (memory && memory->fault_callback) {
			memory->fault_callback(memory->fault_context, v6502_memoryBoundsErrorText);
		}
		
		return NULL;
	}
	
	// Work memory, 0x07FF + 3 mirrors = 0x1FFF
	if (offset < v6502_memoryStartPPURegisters) {
		offset %= v6502_memorySizeWorkMemory;
		
		return &memory->bytes[offset];
	}
	
	// PPU Registers
	if (offset >= v6502_memoryStartPPURegisters && offset < v6502_memoryStartAPURegisters) {
		offset %= v6502_memorySizePPURegisters;
		offset += v6502_memoryStartPPURegisters;
		
		return &memory->bytes[offset];
	}

	/** FIXME: @bug Everything past here, is it mapped or just in memory? */
	// APU registers
	if (offset >= v6502_memoryStartAPURegisters && offset < v6502_memoryStartExpansionRom) {
		return &memory->bytes[offset];
	}

	// Expansion ROM (Cartridge)
	if (offset >= v6502_memoryStartExpansionRom && offset < v6502_memoryStartSRAM) {
		return &memory->bytes[offset];
	}
	
	// SRAM
	if (offset >= v6502_memoryStartSRAM && offset < v6502_memoryStartPRGROM) {
		return &memory->bytes[offset];
	}

	// PRG ROM
	if (offset >= v6502_memoryStartPRGROM && offset < v6502_memoryStartCeiling) {
		return &memory->bytes[offset];
	}
	
	if (memory && memory->fault_callback) {
		memory->fault_callback(memory->fault_context, v6502_unableToMapMemoryErrorTextv6502);
	}

	return NULL;
}

void v6502_loadExpansionRomIntoMemory(v6502_memory *memory, uint8_t *rom, uint16_t size) {
	for (uint16_t i = 0; i < size; i++) {
		*v6502_map(memory, v6502_memoryStartExpansionRom + i) = rom[i];
	}
}

/**
 *	If there are allocation problems, v6502_createMemory will return NULL.
 */
v6502_memory *v6502_createMemory(uint16_t size) {
	// Allocate Memory Struct
	v6502_memory *memory = malloc(sizeof(v6502_memory));
	if (!memory) {
		return NULL;
	}
	
	// Allocate Virtual Memory
	memory->bytes = malloc(size);
	if (!memory->bytes) {
		free(memory);
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
#pragma mark Signedness Management

int8_t v6502_signedValueOfByte(uint8_t byte) {
//	if (byte & 0x80) {
//		return (0 - (BYTE_MAX - byte));
//	}
	return byte;
}

uint8_t v6502_byteValueOfSigned(int8_t i) {
/** TODO: @todo Reliable portable implementation */
//	if (i < 0) {
//		return (uint8_t)(0xff - i);
//	}
	return (uint8_t)i;
}
