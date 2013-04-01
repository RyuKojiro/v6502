//
//  cpu.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_cpu_h
#define v6502_cpu_h

// Use this to work headless, ignoring faults
//#define v6502_fault	(void)

#include <stdint.h>

#include "mem.h"

// CPU Object
typedef struct {
	uint16_t pc;	// program counter
	uint8_t ac;		// accumulator
	uint8_t x;		// X register
	uint8_t y;		// Y register
	uint8_t sr;		// Status register
	uint8_t sp;		// Stack Pointer
	
	v6502_memory *memory;
} v6502_cpu;

// Instruction Set
typedef enum {
	v6502_opcode_brk		= 0x00,
	v6502_opcode_nop		= 0xEA,
	v6502_opcode_asl_zpg	= 0x06,

	v6502_opcode_jmp_abs	= 0x4C,
	v6502_opcode_jmp_ind	= 0x6C,
	
	v6502_opcode_ora_imm	= 0x09,
	v6502_opcode_ora_zpg	= 0x05,
	v6502_opcode_ora_zpgx	= 0x15,
	v6502_opcode_ora_abs	= 0x0D,
	v6502_opcode_ora_absx	= 0x1D,
	v6502_opcode_ora_absy	= 0x19,
	v6502_opcode_ora_indx	= 0x01,
	v6502_opcode_ora_indy	= 0x11,
} v6502_opcode;

// Status Register Values
typedef enum {
	v6502_cpu_status_carry		= 1,
	v6502_cpu_status_zero		= 2,
	v6502_cpu_status_interrupt	= 4,
	v6502_cpu_status_decimal	= 8,
	v6502_cpu_status_break		= 16,
	v6502_cpu_status_ignored	= 32,
	v6502_cpu_status_overflow	= 64,
	v6502_cpu_status_negative	= 128,
} v6502_cpu_status;

// CPU object lifecycle
v6502_cpu *v6502_createCPU(void);
void v6502_destroyCPU(v6502_cpu *cpu);

// Execution
void v6502_execute(v6502_cpu *cpu, uint8_t opcode, uint8_t low, uint8_t high);
void v6502_step(v6502_cpu *cpu);
void v6502_reset(v6502_cpu *cpu);

#endif
