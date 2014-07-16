/** @brief Virtual Memory */
/** @file mem.h */

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

#ifndef v6502_mem_h
#define v6502_mem_h

#include <stdint.h>

/** @defgroup mem_boundaries Memory Map Definitions */
/**@{*/

// Memory Starts
/** @brief Start of work memory available for general program use */
#define v6502_memoryStartWorkMemory			0x0000
/** @brief Start of memory reserved for CPU stack */
#define v6502_memoryStartStack				0x0100
/** @brief Start of program code memory region */
#define v6502_memoryStartProgram			0x0600
/** @brief Start of PPU registers for the RP2C02 chipset */
#define v6502_memoryStartPPURegisters		0x2000
/** @brief Start of APU registers for the RP2A03 chipset */
#define v6502_memoryStartAPURegisters		0x4000
/** @brief Start of memory mapped expansion ROM */
#define v6502_memoryStartExpansionRom		0x4020
/** @brief Start of SRAM backed memory region */
#define v6502_memoryStartSRAM				0x6000
/** @brief Start of memory mapped PRGROM */
#define v6502_memoryStartPRGROM				0x8000
/** @brief Start of memory mapped interrupt vectors */
#define v6502_memoryStartInterruptVectors	0xFFFA
/** @brief Maximum memory boundary */
#define v6502_memoryStartCeiling			0xFFFF

// Memory Blob Sizes
/** @brief Size of work memory available for general program use */
#define v6502_memorySizeWorkMemory			0x0800
/** @brief Size of PPU registers for the RP2C02 chipset */
#define v6502_memorySizePPURegisters		0x0008
/** @brief Size of the six interrupt vector bytes (Hint: It's six.) */
#define v6502_memorySizeInterruptVectors	v6502_memoryStartCeiling - v6502_memoryStartInterruptVectors

// Vector Locations
/** @brief The low byte location of the NMI vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorNMILow			0xFFFA
/** @brief The high byte location of the NMI vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorNMIHigh			0xFFFB
/** @brief The low byte location of the reset vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorResetLow			0xFFFC
/** @brief The high byte location of the reset vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorResetHigh			0xFFFD
/** @brief The low byte location of the normal interrupt vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorInterruptLow		0xFFFE
/** @brief The high byte location of the normal interrupt vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorInterruptHigh		0xFFFF
/**@}*/

/** @brief Maximum possible value of an 8-bit byte */
#define BYTE_MAX	0xFF

/** @struct */
/** @brief Virtual Memory Object */
typedef struct {
	/** @brief Memory accessible as a byte-array */
	uint8_t *bytes;
	/** @brief Byte-length of memory object */
	uint16_t size;
	/** @brief Fault Callback Function */
	void(*fault_callback)(void *context, const char *reason);
	/** @brief Fault Callback Context */
	void *fault_context;
	/** @brief The three 16-bit (two 8-byte) interrupt vectors starting at address v6502_memoryVectorNMILow */
	uint8_t interrupt_vectors[v6502_memorySizeInterruptVectors];
} v6502_memory;

/** @defgroup mem_lifecycle Memory Lifecycle Functions */
/**@{*/
/** @brief Create v6502_memory */
v6502_memory *v6502_createMemory(uint16_t size);
/** @brief Destroy v6502_memory */
void v6502_destroyMemory(v6502_memory *memory);
/** @brief Load a binary blob of expansion ROM into a given v6502_memory */
void v6502_loadExpansionRomIntoMemory(v6502_memory *memory, uint8_t *rom, uint16_t size);
/**@}*/

/** @defgroup mem_access Memory Access */
/**@{*/
/** @brief The function prototype for memory mapped accessors to be used by external virtual hardware. */
typedef uint16_t *(v6502_memoryAccessor)(v6502_memory *memory, uint16_t offset);
/** @brief Map an address in v6502_memory */
/** This works by registering an v6502_memoryAccessor as the handler for that range of v6502_memory. Anytime an access is made to that range of memory, the v6502_memoryAccessor is called instead, and is expected to return a byte ready for access. When this function is called, it is also assumed that an access is actually going to happen, which means it is safe to use calls to your callback as trap signals. */
int v6502_map(v6502_memory *memory, uint16_t start, uint16_t size, v6502_memoryAccessor *callback);
/** @brief Access an address in v6502_memory. */
/** All accesses made by the v6502_cpu should travel through this function, so that they respect any hardware memory mapping. */
uint8_t *v6502_access(v6502_memory *memory, uint16_t offset);
/** @brief Convert a raw byte to its signed value */
int8_t v6502_signedValueOfByte(uint8_t byte);
/** @brief Convert a signed value to its raw byte */
uint8_t v6502_byteValueOfSigned(int8_t i);
/**@}*/

#endif
