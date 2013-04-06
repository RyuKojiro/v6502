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
	// Single Byte Instructions
	v6502_opcode_brk		= 0x00,
	v6502_opcode_nop		= 0xEA,
	v6502_opcode_clc		= 0x18,
	v6502_opcode_cld		= 0xD8,
	v6502_opcode_cli		= 0x58,
	v6502_opcode_clv		= 0xB8,
	v6502_opcode_sec		= 0x38,
	v6502_opcode_sed		= 0xF8,
	v6502_opcode_sei		= 0x78,
	v6502_opcode_dex		= 0xCA,
	v6502_opcode_dey		= 0x88,
	v6502_opcode_tax		= 0xAA,
	v6502_opcode_tay		= 0xA8,
	v6502_opcode_tsx		= 0xBA,
	v6502_opcode_txa		= 0x8A,
	v6502_opcode_txs		= 0x9A,
	v6502_opcode_tya		= 0x98,
	v6502_opcode_inx		= 0xE8,
	v6502_opcode_iny		= 0xC8,
	
	// AND
	v6502_opcode_and_imm	= 0x29,
	v6502_opcode_and_zpg	= 0x25,
	v6502_opcode_and_zpgx	= 0x35,
	v6502_opcode_and_abs	= 0x2D,
	v6502_opcode_and_absx	= 0x3D,
	v6502_opcode_and_absy	= 0x39,
	v6502_opcode_and_indx	= 0x21,
	v6502_opcode_and_indy	= 0x31,
	
	// ASL
	v6502_opcode_asl_acc	= 0x0A,
	v6502_opcode_asl_zpg	= 0x06,
	v6502_opcode_asl_zpgx	= 0x16,
	v6502_opcode_asl_abs	= 0x0E,
	v6502_opcode_asl_absx	= 0x1E,
	
	// EOR
	v6502_opcode_eor_imm	= 0x49,
	v6502_opcode_eor_zpg	= 0x45,
	v6502_opcode_eor_zpgx	= 0x55,
	v6502_opcode_eor_abs	= 0x4D,
	v6502_opcode_eor_absx	= 0x5D,
	v6502_opcode_eor_absy	= 0x59,
	v6502_opcode_eor_indx	= 0x41,
	v6502_opcode_eor_indy	= 0x51,

	// JMP
	v6502_opcode_jmp_abs	= 0x4C,
	v6502_opcode_jmp_ind	= 0x6C,
	
	// ORA
	v6502_opcode_ora_imm	= 0x09,
	v6502_opcode_ora_zpg	= 0x05,
	v6502_opcode_ora_zpgx	= 0x15,
	v6502_opcode_ora_abs	= 0x0D,
	v6502_opcode_ora_absx	= 0x1D,
	v6502_opcode_ora_absy	= 0x19,
	v6502_opcode_ora_indx	= 0x01,
	v6502_opcode_ora_indy	= 0x11,
	
	// LDA
	v6502_opcode_lda_imm	= 0xA9,
	v6502_opcode_lda_zpg	= 0xA5,
	v6502_opcode_lda_zpgx	= 0xB5,
	v6502_opcode_lda_abs	= 0xAD,
	v6502_opcode_lda_absx	= 0xBD,
	v6502_opcode_lda_absy	= 0xB9,
	v6502_opcode_lda_indx	= 0xA1,
	v6502_opcode_lda_indy	= 0xB1,

	// LDX
	v6502_opcode_ldx_imm	= 0xA2,
	v6502_opcode_ldx_zpg	= 0xA6,
	v6502_opcode_ldx_zpgy	= 0xB6,
	v6502_opcode_ldx_abs	= 0xAE,
	v6502_opcode_ldx_absy	= 0xBE,

	// LDY
	v6502_opcode_ldy_imm	= 0xA0,
	v6502_opcode_ldy_zpg	= 0xA4,
	v6502_opcode_ldy_zpgx	= 0xB4,
	v6502_opcode_ldy_abs	= 0xAC,
	v6502_opcode_ldy_absx	= 0xBC,
	
	// LSR
	v6502_opcode_lsr_acc	= 0x4A,
	v6502_opcode_lsr_zpg	= 0x46,
	v6502_opcode_lsr_zpgx	= 0x56,
	v6502_opcode_lsr_abs	= 0x4E,
	v6502_opcode_lsr_absx	= 0x5E,
	
	// STA
	v6502_opcode_sta_zpg	= 0x85,
	v6502_opcode_sta_zpgx	= 0x95,
	v6502_opcode_sta_abs	= 0x8D,
	v6502_opcode_sta_absx	= 0x9D,
	v6502_opcode_sta_absy	= 0x99,
	v6502_opcode_sta_indx	= 0x81,
	v6502_opcode_sta_indy	= 0x91,

	// STX
	v6502_opcode_stx_zpg	= 0x86,
	v6502_opcode_stx_zpgy	= 0x96,
	v6502_opcode_stx_abs	= 0x8E,
	
	// STY
	v6502_opcode_sty_zpg	= 0x84,
	v6502_opcode_sty_zpgx	= 0x94,
	v6502_opcode_sty_abs	= 0x8C,
	
	
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
