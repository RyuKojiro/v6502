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
#include <stddef.h>

#ifndef YES
/** @brief Boolean true */
#define YES 1
#endif

#ifndef NO
/** @brief Boolean false */
#define NO  0
#endif

/** @defgroup mem_boundaries Memory Map Definitions */
/**@{*/

// Memory Starts
/** @brief Start of work memory available for general program use */
#define v6502_memoryStartWorkMemory         0x0000
/** @brief Start of memory reserved for CPU stack */
#define v6502_memoryStartStack              0x0100
/** @brief Start of PPU registers for the RP2C02 chipset */
#define v6502_memoryStartPPURegisters       0x2000
/** @brief Start of APU registers for the RP2A03 chipset */
#define v6502_memoryStartAPURegisters       0x4000
/** @brief Start of memory mapped expansion ROM */
#define v6502_memoryStartExpansionRom       0x4020
/** @brief Start of SRAM backed memory region */
#define v6502_memoryStartSRAM               0x6000
/** @brief Start of memory mapped PRGROM */
#define v6502_memoryStartPRGROM             0x8000
/** @brief Start of memory mapped interrupt vectors */
#define v6502_memoryStartInterruptVectors   0xFFFA
/** @brief Maximum memory boundary */
#define v6502_memoryStartCeiling            0xFFFF

// Memory Blob Sizes
/** @brief Size of work memory available for general program use */
#define v6502_memorySizeWorkMemory          0x0800
/** @brief Size of PPU registers for the RP2C02 chipset */
#define v6502_memorySizePPURegisters        0x0008
/** @brief Size of the six interrupt vector bytes (Hint: It's six.) */
#define v6502_memorySizeInterruptVectors    (v6502_memoryStartCeiling - v6502_memoryStartInterruptVectors)

// Vector Locations
/** @brief The low byte location of the NMI vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorNMILow            0xFFFA
/** @brief The high byte location of the NMI vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorNMIHigh           0xFFFB
/** @brief The low byte location of the reset vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorResetLow          0xFFFC
/** @brief The high byte location of the reset vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorResetHigh         0xFFFD
/** @brief The low byte location of the normal interrupt vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorInterruptLow      0xFFFE
/** @brief The high byte location of the normal interrupt vector stored in the v6502_memory::interrupt_vectors */
#define v6502_memoryVectorInterruptHigh     0xFFFF
/**@}*/

/** @brief Maximum possible value of an 8-bit byte */
#define BYTE_MAX 0xFF

/** @cond STRUCT_FORWARD_DECLS */
/* Forward declaration needed for circular dependency of mapping function and structures */
struct _v6502_memory;
/** @endcond */

/** @ingroup mem_access */
/** @brief The function prototype for memory mapped accessors to be used by external virtual hardware. */
typedef uint8_t (v6502_readFunction)(struct _v6502_memory *memory, uint16_t offset, int trap, void *context);
/** @brief The function prototype for memory mapped accessors to be used by external virtual hardware. */
typedef void (v6502_writeFunction)(struct _v6502_memory *memory, uint16_t offset, uint8_t value, void *context);

/** @struct */
/** @brief Memory Map Range Record */
typedef struct {
	/** @brief Start address of mapped range */
	uint16_t start;
	/** @brief Byte-length of mapped range */
	size_t size;
	/** @brief Memory access callback for reading bytes within this memory range */
	v6502_readFunction *read;
	/** @brief Memory access callback for writing bytes within this memory range */
	v6502_writeFunction *write;
	/** @brief Context pointer, generally used to point to hardware data structures so that they can be referenced when called back to  */
	void *context;
} v6502_mappedRange;

/** @struct */
/** @brief Virtual Memory Object */
typedef struct /** @cond STRUCT_FORWARD_DECLS */ _v6502_memory /** @endcond */ {
	/** @brief Memory accessible as a byte-array */
	uint8_t *bytes;
	/** @brief Byte-length of memory object */
	size_t size;
	/** @brief Fault Callback Function */
	void(*fault_callback)(void *context, const char *reason);
	/** @brief Fault Callback Context */
	void *fault_context;
	/** @brief Array of memory map ranges */
	v6502_mappedRange *mappedRanges;
	/** @brief Number of memory map ranges in array */
	size_t rangeCount;
	/** @brief @ref mem_cache control */
	int mapCacheEnabled;
	/** @brief Memory map read cache (See: @ref mem_cache) */
	v6502_readFunction **readCache;
	/** @brief Memory map write cache (See: @ref mem_cache) */
	v6502_writeFunction **writeCache;
	/** @brief Memory map context cache (See: @ref mem_cache) */
	void **contextCache;
} v6502_memory;

/** @defgroup mem_lifecycle Memory Lifecycle Functions */
/**@{*/
/** @brief Create v6502_memory, type is a size_t so that you can alloc an entire 64k with 0x1,0000 */
v6502_memory *v6502_createMemory(size_t size);
/** @brief Destroy v6502_memory */
void v6502_destroyMemory(v6502_memory *memory);
/** @brief Load a binary blob of expansion ROM into a given v6502_memory */
void v6502_loadExpansionRomIntoMemory(v6502_memory *memory, uint8_t *rom, uint16_t size);
/**@}*/

/** @defgroup mem_access Memory Access */
/**@{*/
/** @brief Map an address in v6502_memory */
/** This works by registering an v6502_memoryAccessor as the handler for that range of v6502_memory. Anytime an access is made to that range of memory, the v6502_memoryAccessor is called instead, and is expected to return a byte ready for access. When this function is called, it is also assumed that an access is actually going to happen, which means it is safe to use calls to your callback as trap signals. This function returns YES if the mapping succeedsm, and NO if it fails. It is highly reccomended that you assert, or at least check the return code. */
int v6502_map(v6502_memory *memory, uint16_t start, size_t size, v6502_readFunction *read, v6502_writeFunction *write, void *context);
/** @brief Read a byte from v6502_memory */
/** All accesses made by the v6502_cpu should travel through these functions, so that they respect any hardware memory mapping.
	The trap argument should always be YES when accessed by the CPU, and always NO when accessed by any virtual hardware outside the CPU, or any VM construct (such as logging/introspection mechanisms.)
 */
uint8_t v6502_read(v6502_memory *memory, uint16_t offset, int trap);
/** @brief Write a byte to v6502_memory */
/** All accesses made by the v6502_cpu should travel through these functions, so that they respect any hardware memory mapping. */
void v6502_write(v6502_memory *memory, uint16_t offset, uint8_t value);
/** @brief Locate a v6502_mappedRange inside of v6502_memory, if it exists */
v6502_mappedRange *v6502_mappedRangeForOffset(v6502_memory *memory, uint16_t offset);
/** @brief Convert a raw byte to its signed value */
int8_t v6502_signedValueOfByte(uint8_t byte);
/** @brief Convert a signed value to its raw byte */
uint8_t v6502_byteValueOfSigned(int8_t i);
/**@}*/

#endif
