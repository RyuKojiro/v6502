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
	
	// Branch Instructions
	v6502_opcode_bcc		= 0x90,
	v6502_opcode_bcs		= 0xB0,
	v6502_opcode_beq		= 0xF0,
	v6502_opcode_bne		= 0xD0,
	v6502_opcode_bmi		= 0x30,
	v6502_opcode_bpl		= 0x10,
	v6502_opcode_bvc		= 0x50,
	v6502_opcode_bvs		= 0x70,
	
	// ADC
	v6502_opcode_adc_imm	= 0x69,
	v6502_opcode_adc_zpg	= 0x65,
	v6502_opcode_adc_zpgx	= 0x75,
	v6502_opcode_adc_abs	= 0x6D,
	v6502_opcode_adc_absx	= 0x7D,
	v6502_opcode_adc_absy	= 0x79,
	v6502_opcode_adc_indx	= 0x61,
	v6502_opcode_adc_indy	= 0x71,

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
	
	// CMP
	v6502_opcode_cmp_imm	= 0xC9,
	v6502_opcode_cmp_zpg	= 0xC5,
	v6502_opcode_cmp_zpgx	= 0xD5,
	v6502_opcode_cmp_abs	= 0xCD,
	v6502_opcode_cmp_absx	= 0xDD,
	v6502_opcode_cmp_absy	= 0xD9,
	v6502_opcode_cmp_indx	= 0xC1,
	v6502_opcode_cmp_indy	= 0xD1,

	// DEC
	v6502_opcode_dec_zpg	= 0xC6,
	v6502_opcode_dec_zpgx	= 0xD6,
	v6502_opcode_dec_abs	= 0xCE,
	v6502_opcode_dec_absx	= 0xDE,

	// EOR
	v6502_opcode_eor_imm	= 0x49,
	v6502_opcode_eor_zpg	= 0x45,
	v6502_opcode_eor_zpgx	= 0x55,
	v6502_opcode_eor_abs	= 0x4D,
	v6502_opcode_eor_absx	= 0x5D,
	v6502_opcode_eor_absy	= 0x59,
	v6502_opcode_eor_indx	= 0x41,
	v6502_opcode_eor_indy	= 0x51,

	// INC
	v6502_opcode_inc_zpg	= 0xE6,
	v6502_opcode_inc_zpgx	= 0xF6,
	v6502_opcode_inc_abs	= 0xEE,
	v6502_opcode_inc_absx	= 0xFE,

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
	
	// ROL
	v6502_opcode_rol_acc	= 0x2A,
	v6502_opcode_rol_zpg	= 0x26,
	v6502_opcode_rol_zpgx	= 0x36,
	v6502_opcode_rol_abs	= 0x2E,
	v6502_opcode_rol_absx	= 0x3E,
	
	// ROR
	v6502_opcode_ror_acc	= 0x6A,
	v6502_opcode_ror_zpg	= 0x66,
	v6502_opcode_ror_zpgx	= 0x76,
	v6502_opcode_ror_abs	= 0x6E,
	v6502_opcode_ror_absx	= 0x7E,

	// SBC
	v6502_opcode_sbc_imm	= 0xE9,
	v6502_opcode_sbc_zpg	= 0xE5,
	v6502_opcode_sbc_zpgx	= 0xF5,
	v6502_opcode_sbc_abs	= 0xED,
	v6502_opcode_sbc_absx	= 0xFD,
	v6502_opcode_sbc_absy	= 0xF9,
	v6502_opcode_sbc_indx	= 0xE1,
	v6502_opcode_sbc_indy	= 0xF1,

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
