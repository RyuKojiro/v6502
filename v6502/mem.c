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
#include <assert.h>

#include "mem.h"

#pragma mark -
#pragma mark Memory Lifecycle

v6502_mappedRange *v6502_mappedRangeForOffset(v6502_memory *memory, uint16_t offset) {
	for (size_t i = 0; i < memory->rangeCount; i++) {
		v6502_mappedRange *currentRange = &memory->mappedRanges[i];
		if (offset >= currentRange->start && offset < (currentRange->start + currentRange->size)) {
			return currentRange;
		}
	}
	return NULL;
}

int v6502_memoryRangesIntersect(uint16_t start1, uint16_t size1, uint16_t start2, uint16_t size2) {
	// Curly braces are 1
	// Square braces are 2
	uint16_t end1 = start1 + size1;
	uint16_t end2 = start2 + size2;
	
	// [ { ]
	if (start1 >= start2 && start1 < end2) {
		return YES;
	}
	// [ } ]
	if (end1 >= start2 && end1 < end2) {
		return YES;
	}
	// { [ ] }
	if (start2 >= start1 && end2 <= end1) {
		return YES;
	}
	return NO;
}

int v6502_map(v6502_memory *memory, uint16_t start, uint16_t size, v6502_readFunction *read, v6502_writeFunction *write, void *context) {
	// Make sure it's not already mapped
	for (size_t i = 0; i < memory->rangeCount; i++) {
		v6502_mappedRange *currentRange = &memory->mappedRanges[i];
		if (v6502_memoryRangesIntersect(start, size, currentRange->start, currentRange->size)) {
			return YES;
		}
	}

	// Create a struct and add it to the list
	memory->mappedRanges = realloc(memory->mappedRanges, sizeof(v6502_mappedRange) * (memory->rangeCount + 1));
	
	v6502_mappedRange *this = &memory->mappedRanges[memory->rangeCount];
	
	this->start = start;
	this->size = size;
	this->read = read;
	this->write = write;
	this->context = context;
	
	memory->rangeCount++;

	return NO;
}

void v6502_write(v6502_memory *memory, uint16_t offset, uint8_t value) {
	assert(memory);

	// Check map
	v6502_mappedRange *range = v6502_mappedRangeForOffset(memory, offset);
	assert((offset < memory->size) || (range && range->write));

	if (range && range->write) {
		range->write(memory, offset, value, range->context);
	}
	else {
		assert(memory->bytes);
		memory->bytes[offset] = value;
	}
}

uint8_t v6502_read(v6502_memory *memory, uint16_t offset, int trap) {
	assert(memory);
	
	// Search mapped memory regions to see if we should defer to the map
	v6502_mappedRange *range = v6502_mappedRangeForOffset(memory, offset);
	assert((offset < memory->size) || (range && range->read));

	if (range && range->read) {
		return range->read(memory, offset, trap, range->context);
	}
	else {
		assert(memory->bytes);
		return memory->bytes[offset];
	}
}

void v6502_loadExpansionRomIntoMemory(v6502_memory *memory, uint8_t *rom, uint16_t size) {
	assert(memory);

	for (uint16_t i = 0; i < size; i++) {
		v6502_write(memory, v6502_memoryStartExpansionRom + i, rom[i]);
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
	
	// Zero out the Map
	memory->rangeCount = 0;
	memory->mappedRanges = NULL;
	
	return memory;
}

void v6502_destroyMemory(v6502_memory *memory) {
	if (!memory) {
		return;
	}
	
	free(memory->mappedRanges);
	free(memory->bytes);
	free(memory);
}

#pragma mark -
#pragma mark Signedness Management

int8_t v6502_signedValueOfByte(uint8_t byte) {
	return (int8_t)byte;
}

uint8_t v6502_byteValueOfSigned(int8_t i) {
	return (uint8_t)i;
}
