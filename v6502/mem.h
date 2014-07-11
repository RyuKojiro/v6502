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
/** @brief Maximum memory boundary */
#define v6502_memoryStartCeiling			0xFFFF

// Memory Blob Sizes
/** @brief Size of work memory available for general program use */
#define v6502_memorySizeWorkMemory			0x0800
/** @brief Size of PPU registers for the RP2C02 chipset */
#define v6502_memorySizePPURegisters		0x0008

// Vector Locations
/** @brief The low byte location of the v6502_memory::nmi_vector */
#define v6502_memoryVectorNMILow			0xFFFA
/** @brief The high byte location of the v6502_memory::nmi_vector */
#define v6502_memoryVectorNMIHigh			0xFFFB
/** @brief The low byte location of the v6502_memory::reset_vector */
#define v6502_memoryVectorResetLow			0xFFFC
/** @brief The high byte location of the v6502_memory::reset_vector */
#define v6502_memoryVectorResetHigh			0xFFFD
/** @brief The low byte location of the v6502_memory::interrupt_vector */
#define v6502_memoryVectorInterruptLow		0xFFFE
/** @brief The high byte location of the v6502_memory::interrupt_vector */
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
	/** @brief NMI Vector, mapped by mem.c to v6502_memoryVectorNMILow and v6502_memoryVectorNMIHigh. */
	uint16_t nmi_vector;
	/** @brief Reset Vector, mapped by mem.c to $FFFC (low) and $FFFD (high). */
	uint16_t reset_vector;
	/** @brief Interrupt Vector, mapped by mem.c to $FFFE (low) and $FFFF (high). */
	uint16_t interrupt_vector;
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
/** @brief Map an address in v6502_memory */
uint8_t *v6502_map(v6502_memory *memory, uint16_t offset);
/** @brief Convert a raw byte to its signed value */
int8_t v6502_signedValueOfByte(uint8_t byte);
/** @brief Convert a signed value to its raw byte */
uint8_t v6502_byteValueOfSigned(int8_t i);
/**@}*/

#endif
