/** @brief Virtual CPU */
/** @file cpu.h */

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

#ifndef v6502_cpu_h
#define v6502_cpu_h

#include <stdint.h>

#include <v6502/mem.h>

/** @struct */
/** @brief Virtual CPU Object */
typedef struct {
	/** @brief Program counter (16-bit) */
	uint16_t pc;
	/** @brief Accumulator (8-bit) */
	uint8_t a;
	/** @brief X register (8-bit) */
	uint8_t x;
	/** @brief Y register (8-bit) */
	uint8_t y;
	/** @brief Status register (8-bit) */
	uint8_t sr;
	/** @brief Stack Pointer (8-bit) */
	uint8_t sp;
	/** @brief Virtual Memory */
	v6502_memory *memory;
	/** @brief Fault Callback Function */
	void(*fault_callback)(void *context, const char *reason);
	/** @brief Fault Callback Context */
	void *fault_context;
} v6502_cpu;

/** @enum */
/** @brief Instruction Set */
typedef enum {
	// Single Byte Instructions
	v6502_opcode_brk        = 0x00, // BRK - Break
	v6502_opcode_nop        = 0xEA, // NOP - No-op
	v6502_opcode_clc        = 0x18, // CLC - Clear Carry Flag
	v6502_opcode_cld        = 0xD8, // CLD - Clear Decimal Flag
	v6502_opcode_cli        = 0x58, // CLI - Clear Interrupt Flag
	v6502_opcode_clv        = 0xB8, // CLV - Clear Overflow Flag
	v6502_opcode_sec        = 0x38, // SEC - Set Carry Flag
	v6502_opcode_sed        = 0xF8, // SED - Set Decimal Flag
	v6502_opcode_sei        = 0x78, // SEI - Set Interrupt Flag
	v6502_opcode_dex        = 0xCA, // DEX - Decrement X
	v6502_opcode_dey        = 0x88, // DEY - Decrement Y
	v6502_opcode_tax        = 0xAA, // TAX - Transfer Accumulator to X
	v6502_opcode_tay        = 0xA8, // TAY - Transfer Accumulator to Y
	v6502_opcode_tsx        = 0xBA, // TSX - Transfer Status Flags to X
	v6502_opcode_txa        = 0x8A, // TXA - Transfer X to Accumulator
	v6502_opcode_txs        = 0x9A, // TXS - Transfer X to Status Flags
	v6502_opcode_tya        = 0x98, // TYA - Transfer Y to Accumulator
	v6502_opcode_inx        = 0xE8, // INX - Increment X
	v6502_opcode_iny        = 0xC8, // INY - Increment Y

	// Stack Instructions
	v6502_opcode_jsr        = 0x20, // JSR - Call Subroutine
	v6502_opcode_rti        = 0x40, // RTI - Return from Interrupt
	v6502_opcode_rts        = 0x60, // RTS - Return from Subroutine
	v6502_opcode_pha        = 0x48, // PHA - Push Accumulator to Stack
	v6502_opcode_php        = 0x08, // PHP - Push Program Counter to Stack
	v6502_opcode_pla        = 0x68, // PLA - Pull Accumulator from Stack
	v6502_opcode_plp        = 0x28, // PLP - Pull Program Counter from Stack

	// Branch Instructions
	v6502_opcode_bcc        = 0x90, // BCC - Branch if Carry Clear
	v6502_opcode_bcs        = 0xB0, // BCS - Branch if Carry Set
	v6502_opcode_beq        = 0xF0, // BEQ - Branch if Equal (Zero Set)
	v6502_opcode_bne        = 0xD0, // BNE - Branch if Not Equal (Zero Clear)
	v6502_opcode_bmi        = 0x30, // BMI - Branch if Minus (Negative Set)
	v6502_opcode_bpl        = 0x10, // BPL - Branch if Plus (Negative Clear)
	v6502_opcode_bvc        = 0x50, // BVC - Branch if Overflow Clear
	v6502_opcode_bvs        = 0x70, // BVS - Branch if Overflow Set

	// ADC - Add With Carry
	v6502_opcode_adc_imm    = 0x69,
	v6502_opcode_adc_zpg    = 0x65,
	v6502_opcode_adc_zpgx   = 0x75,
	v6502_opcode_adc_abs    = 0x6D,
	v6502_opcode_adc_absx   = 0x7D,
	v6502_opcode_adc_absy   = 0x79,
	v6502_opcode_adc_indx   = 0x61,
	v6502_opcode_adc_indy   = 0x71,

	// AND - Bitwise And
	v6502_opcode_and_imm    = 0x29,
	v6502_opcode_and_zpg    = 0x25,
	v6502_opcode_and_zpgx   = 0x35,
	v6502_opcode_and_abs    = 0x2D,
	v6502_opcode_and_absx   = 0x3D,
	v6502_opcode_and_absy   = 0x39,
	v6502_opcode_and_indx   = 0x21,
	v6502_opcode_and_indy   = 0x31,

	// ASL - Arithmetic Shift Left
	v6502_opcode_asl_acc    = 0x0A,
	v6502_opcode_asl_zpg    = 0x06,
	v6502_opcode_asl_zpgx   = 0x16,
	v6502_opcode_asl_abs    = 0x0E,
	v6502_opcode_asl_absx   = 0x1E,

	// BIT - Bit Test
	v6502_opcode_bit_zpg    = 0x24,
	v6502_opcode_bit_abs    = 0x2C,

	// CMP - Compare Accumulator
	v6502_opcode_cmp_imm    = 0xC9,
	v6502_opcode_cmp_zpg    = 0xC5,
	v6502_opcode_cmp_zpgx   = 0xD5,
	v6502_opcode_cmp_abs    = 0xCD,
	v6502_opcode_cmp_absx   = 0xDD,
	v6502_opcode_cmp_absy   = 0xD9,
	v6502_opcode_cmp_indx   = 0xC1,
	v6502_opcode_cmp_indy   = 0xD1,

	// CPX - Compare X
	v6502_opcode_cpx_imm    = 0xE0,
	v6502_opcode_cpx_zpg    = 0xE4,
	v6502_opcode_cpx_abs    = 0xEC,

	// CPY - Compare Y
	v6502_opcode_cpy_imm    = 0xC0,
	v6502_opcode_cpy_zpg    = 0xC4,
	v6502_opcode_cpy_abs    = 0xCC,

	// DEC - Decrement Accumulator
	v6502_opcode_dec_zpg    = 0xC6,
	v6502_opcode_dec_zpgx   = 0xD6,
	v6502_opcode_dec_abs    = 0xCE,
	v6502_opcode_dec_absx   = 0xDE,

	// EOR - Bitwise Exclusive Or
	v6502_opcode_eor_imm    = 0x49,
	v6502_opcode_eor_zpg    = 0x45,
	v6502_opcode_eor_zpgx   = 0x55,
	v6502_opcode_eor_abs    = 0x4D,
	v6502_opcode_eor_absx   = 0x5D,
	v6502_opcode_eor_absy   = 0x59,
	v6502_opcode_eor_indx   = 0x41,
	v6502_opcode_eor_indy   = 0x51,

	// INC - Increment Accumulator
	v6502_opcode_inc_zpg    = 0xE6,
	v6502_opcode_inc_zpgx   = 0xF6,
	v6502_opcode_inc_abs    = 0xEE,
	v6502_opcode_inc_absx   = 0xFE,

	// JMP - Unconditional Jump
	v6502_opcode_jmp_abs    = 0x4C,
	v6502_opcode_jmp_ind    = 0x6C,

	// ORA - Bitwise Or
	v6502_opcode_ora_imm    = 0x09,
	v6502_opcode_ora_zpg    = 0x05,
	v6502_opcode_ora_zpgx   = 0x15,
	v6502_opcode_ora_abs    = 0x0D,
	v6502_opcode_ora_absx   = 0x1D,
	v6502_opcode_ora_absy   = 0x19,
	v6502_opcode_ora_indx   = 0x01,
	v6502_opcode_ora_indy   = 0x11,

	// LDA - Load Accumulator
	v6502_opcode_lda_imm    = 0xA9,
	v6502_opcode_lda_zpg    = 0xA5,
	v6502_opcode_lda_zpgx   = 0xB5,
	v6502_opcode_lda_abs    = 0xAD,
	v6502_opcode_lda_absx   = 0xBD,
	v6502_opcode_lda_absy   = 0xB9,
	v6502_opcode_lda_indx   = 0xA1,
	v6502_opcode_lda_indy   = 0xB1,

	// LDX - Load X
	v6502_opcode_ldx_imm    = 0xA2,
	v6502_opcode_ldx_zpg    = 0xA6,
	v6502_opcode_ldx_zpgy   = 0xB6,
	v6502_opcode_ldx_abs    = 0xAE,
	v6502_opcode_ldx_absy   = 0xBE,

	// LDY - Load Y
	v6502_opcode_ldy_imm    = 0xA0,
	v6502_opcode_ldy_zpg    = 0xA4,
	v6502_opcode_ldy_zpgx   = 0xB4,
	v6502_opcode_ldy_abs    = 0xAC,
	v6502_opcode_ldy_absx   = 0xBC,

	// LSR - Logical Shift Right
	v6502_opcode_lsr_acc    = 0x4A,
	v6502_opcode_lsr_zpg    = 0x46,
	v6502_opcode_lsr_zpgx   = 0x56,
	v6502_opcode_lsr_abs    = 0x4E,
	v6502_opcode_lsr_absx   = 0x5E,

	// ROL - Rotate Left
	v6502_opcode_rol_acc    = 0x2A,
	v6502_opcode_rol_zpg    = 0x26,
	v6502_opcode_rol_zpgx   = 0x36,
	v6502_opcode_rol_abs    = 0x2E,
	v6502_opcode_rol_absx   = 0x3E,

	// ROR - Rotate Right
	v6502_opcode_ror_acc    = 0x6A,
	v6502_opcode_ror_zpg    = 0x66,
	v6502_opcode_ror_zpgx   = 0x76,
	v6502_opcode_ror_abs    = 0x6E,
	v6502_opcode_ror_absx   = 0x7E,

	// SBC - Subtract with Carry
	v6502_opcode_sbc_imm    = 0xE9,
	v6502_opcode_sbc_zpg    = 0xE5,
	v6502_opcode_sbc_zpgx   = 0xF5,
	v6502_opcode_sbc_abs    = 0xED,
	v6502_opcode_sbc_absx   = 0xFD,
	v6502_opcode_sbc_absy   = 0xF9,
	v6502_opcode_sbc_indx   = 0xE1,
	v6502_opcode_sbc_indy   = 0xF1,

	// STA - Store Accumulator
	v6502_opcode_sta_zpg    = 0x85,
	v6502_opcode_sta_zpgx   = 0x95,
	v6502_opcode_sta_abs    = 0x8D,
	v6502_opcode_sta_absx   = 0x9D,
	v6502_opcode_sta_absy   = 0x99,
	v6502_opcode_sta_indx   = 0x81,
	v6502_opcode_sta_indy   = 0x91,

	// STX - Store X
	v6502_opcode_stx_zpg    = 0x86,
	v6502_opcode_stx_zpgy   = 0x96,
	v6502_opcode_stx_abs    = 0x8E,

	// STY - Store Y
	v6502_opcode_sty_zpg    = 0x84,
	v6502_opcode_sty_zpgx   = 0x94,
	v6502_opcode_sty_abs    = 0x8C,

	// WAI - Wait
	v6502_opcode_wai        = 0xCB
} v6502_opcode;

/** @enum */
/** @brief Status Register Flags */
typedef enum {
	v6502_cpu_status_carry      = 1 << 0,
	v6502_cpu_status_zero       = 1 << 1,
	v6502_cpu_status_interrupt  = 1 << 2,
	v6502_cpu_status_decimal    = 1 << 3,
	v6502_cpu_status_break      = 1 << 4,
	v6502_cpu_status_ignored    = 1 << 5,
	v6502_cpu_status_overflow   = 1 << 6,
	v6502_cpu_status_negative   = 1 << 7,
} v6502_cpu_status;

/** @enum */
/** @brief Address Modes */
typedef enum {
	v6502_address_mode_symbol = -2, // Added for external code, like assemblers
	v6502_address_mode_unknown = -1,
	v6502_address_mode_implied = 0, // Or none
	v6502_address_mode_accumulator = 1,
	v6502_address_mode_immediate = 2,
	v6502_address_mode_absolute = 3,
	v6502_address_mode_absolute_x = 4,
	v6502_address_mode_absolute_y = 5,
	v6502_address_mode_indirect = 6,
	v6502_address_mode_indirect_x = 7,
	v6502_address_mode_indirect_y = 8,
	v6502_address_mode_relative = 9,
	v6502_address_mode_zeropage = 10,
	v6502_address_mode_zeropage_x = 11,
	v6502_address_mode_zeropage_y = 12,
} v6502_address_mode;

/** @defgroup cpu_lifecycle CPU Lifecycle Functions */
/**@{*/
/** @brief Create a v6502_cpu */
v6502_cpu *v6502_createCPU(void);
/** @brief Destroy a v6502_cpu */
void v6502_destroyCPU(v6502_cpu *cpu);
/**@}*/

/** @defgroup cpu_exec Instruction Execution */
/**@{*/
/** @brief Return the byte-length of an instruction based on the opcode (See: @ref cpu_kmap) */
int v6502_instructionLengthForOpcode(v6502_opcode opcode);
/** @brief Return the v6502_address_mode of an instruction based on the opcode (See: @ref cpu_kmap) */
v6502_address_mode v6502_addressModeForOpcode(v6502_opcode opcode);
/** @brief Execute an instruction on a v6502_cpu. */
/** It is important to note that this does not alter the program counter, \ref v6502_step
 is required in order for that to happen. This is because some operations (like
 the interrupt sequence) actually inject instructions directly into the cpu,
 mid-execution.  */
void v6502_execute(v6502_cpu *cpu, uint8_t opcode, uint8_t low, uint8_t high);
/** @brief Single step a v6502_cpu */
void v6502_step(v6502_cpu *cpu);
/** @brief Hardware reset a v6502_cpu */
void v6502_reset(v6502_cpu *cpu);
/** @brief Send an NMI to a v6502_cpu */
void v6502_nmi(v6502_cpu *cpu);
/**@}*/

#endif
