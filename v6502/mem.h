//
/** @brief Virtual Memory */
/** @file mem.h */
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_mem_h
#define v6502_mem_h

#include <stdint.h>

/** @defgroup mem_boundaries Memory Boundary Definitions */
/**@{*/

// Memory Starts
/** @brief Start of work memory available for general program use */
#define v6502_memoryStartWorv6502_memory	0x0000
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
/** @brief Raise an exception regarding v6502_memory */
#define v6502_mfault(a)	if (memory && memory->fault_callback) { \
							memory->fault_callback(memory->fault_context, a); \
						} \
/**@}*/

#endif
