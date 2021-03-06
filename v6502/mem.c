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

/**
 * This function is the slow brute force way (the original implementation) of
 * looking up v6502_mappedRange's for a given address. This function should
 * never be hit if map caching is enabled.
 *
 * A notable side effect of the joined mapping system is that a given range can
 * only ever be mapped once. This means, for example, that you can't map 0xA000
 * through 0xB000 only for reading, and then follow it up with a map of 0xA700
 * through 0xB700 only for writing. It is still possible to achieve this
 * behavior, but three map calls would be required.
 */

v6502_mappedRange *v6502_mappedRangeForOffset(v6502_memory *memory, uint16_t offset) {
	assert(!memory->mapCacheEnabled);

	for (size_t i = 0; i < memory->rangeCount; i++) {
		v6502_mappedRange *currentRange = &memory->mappedRanges[i];
		if (offset >= currentRange->start && offset < (currentRange->start + currentRange->size)) {
			return currentRange;
		}
	}
	return NULL;
}

static int v6502_memoryRangesIntersect(uint16_t start1, uint16_t size1, uint16_t start2, uint16_t size2) {
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

int v6502_map(v6502_memory *memory, uint16_t start, size_t size, v6502_readFunction *read, v6502_writeFunction *write, void *context) {
	assert(memory);

	// Mapping beyond the end of the address space is prohibited
	if ((uint32_t)start + size > 0x10000) {
		return NO;
	}

	// Make sure it's not already mapped
	for (size_t i = 0; i < memory->rangeCount; i++) {
		v6502_mappedRange *currentRange = &memory->mappedRanges[i];
		if (v6502_memoryRangesIntersect(start, size, currentRange->start, currentRange->size)) {
			return NO;
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

	// Finally, if caching is enabled, update the cache
	if (memory->mapCacheEnabled) {
		// Make sure allocations are safe, first
		if (read && !memory->readCache) {
			memory->readCache = calloc(sizeof(void *), memory->size);
		}
		if (write && !memory->writeCache) {
			memory->writeCache = calloc(sizeof(void *), memory->size);
		}
		if (context && !memory->contextCache) {
			memory->contextCache = calloc(sizeof(void *), memory->size);
		}

		// Blit the map
		for (uint16_t i = start; i < start + size; i++) {
			if (read) {
				memory->readCache[i] = read;
			}
			if (write) {
				memory->writeCache[i] = write;
			}
			if (context) {
				memory->contextCache[i] = context;
			}
		}
	}

	return YES;
}

void v6502_write(v6502_memory *memory, uint16_t offset, uint8_t value) {
	assert(memory);

	if (memory->mapCacheEnabled) {
		// Check cache
		if (memory->writeCache && memory->writeCache[offset]) {
			void *context = memory->contextCache ? memory->contextCache[offset] : NULL;
			memory->writeCache[offset](memory, offset, value, context);
			return;
		}
	}
	else {
		// Check hard map
		v6502_mappedRange *range = v6502_mappedRangeForOffset(memory, offset);
		assert((offset < memory->size) || (range && range->write));

		if (range && range->write) {
			range->write(memory, offset, value, range->context);
			return;
		}
	}

	// Not memory mapped
	assert(memory->bytes);
	memory->bytes[offset] = value;
}

uint8_t v6502_read(v6502_memory *memory, uint16_t offset, int trap) {
	assert(memory);

	if (memory->mapCacheEnabled) {
		// Check cache
		if (memory->readCache && memory->readCache[offset]) {
			void *context = memory->contextCache ? memory->contextCache[offset] : NULL;
			return memory->readCache[offset](memory, offset, trap, context);
		}
	}
	else {
		// Search mapped memory regions to see if we should defer to the map
		v6502_mappedRange *range = v6502_mappedRangeForOffset(memory, offset);
		assert((offset < memory->size) || (range && range->read));

		if (range && range->read) {
			return range->read(memory, offset, trap, range->context);
		}
	}

	// Not memory mapped
	assert(memory->bytes);
	return memory->bytes[offset];
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
v6502_memory *v6502_createMemory(size_t size) {
	// Allocate Memory Struct
	v6502_memory *memory = calloc(1, sizeof(v6502_memory));
	if (!memory) {
		return NULL;
	}

	// Allocate the backing storage
	// Is doing it this way overly pedantic? The count and size are effectively reversed.
	memory->bytes = calloc(size, sizeof(uint8_t));
	if (!memory->bytes) {
		free(memory);
		return NULL;
	}

	memory->size = size;

	return memory;
}

void v6502_destroyMemory(v6502_memory *memory) {
	if (!memory) {
		return;
	}

	free(memory->readCache);
	free(memory->writeCache);
	free(memory->contextCache);

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
