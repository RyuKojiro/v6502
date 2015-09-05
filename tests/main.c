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

#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>

#include <as6502/color.h>
#include <v6502/cpu.h>
#include <v6502/log.h>
#include <as6502/parser.h>

#pragma mark Test Harness

#define TOTAL_TESTS		(sizeof(testFunctions) / sizeof(testFunction))
#define TEST_START		lastTest = __PRETTY_FUNCTION__
#define TEST_ASM(a)		as6502_executeAsmLineOnCPU(cpu, a, sizeof(a))

typedef int (* testFunction)(void);

static const char *lastTest;

uint8_t returnLow(struct _v6502_memory *memory, uint16_t offset, int trap, void *context) {
	return 0;
}

uint8_t returnHigh(struct _v6502_memory *memory, uint16_t offset, int trap, void *context) {
	return ~0;
}

v6502_address_mode bruteForce_addressModeForOpcode(v6502_opcode opcode) {
	switch (opcode) {
		case v6502_opcode_brk:
		case v6502_opcode_nop:
		case v6502_opcode_clc:
		case v6502_opcode_cld:
		case v6502_opcode_cli:
		case v6502_opcode_clv:
		case v6502_opcode_sec:
		case v6502_opcode_sed:
		case v6502_opcode_sei:
		case v6502_opcode_dex:
		case v6502_opcode_dey:
		case v6502_opcode_tax:
		case v6502_opcode_tay:
		case v6502_opcode_tsx:
		case v6502_opcode_txa:
		case v6502_opcode_txs:
		case v6502_opcode_tya:
		case v6502_opcode_inx:
		case v6502_opcode_iny:
		case v6502_opcode_rti:
		case v6502_opcode_rts:
		case v6502_opcode_wai:
		case v6502_opcode_pha:
		case v6502_opcode_pla:
		case v6502_opcode_php:
		case v6502_opcode_plp:
			return v6502_address_mode_implied;
		case v6502_opcode_bcc:
		case v6502_opcode_bcs:
		case v6502_opcode_beq:
		case v6502_opcode_bne:
		case v6502_opcode_bmi:
		case v6502_opcode_bpl:
		case v6502_opcode_bvc:
		case v6502_opcode_bvs:
			return v6502_address_mode_relative;
		case v6502_opcode_asl_acc:
		case v6502_opcode_lsr_acc:
		case v6502_opcode_rol_acc:
		case v6502_opcode_ror_acc:
			return v6502_address_mode_accumulator;
		case v6502_opcode_adc_imm:
		case v6502_opcode_and_imm:
		case v6502_opcode_cmp_imm:
		case v6502_opcode_cpx_imm:
		case v6502_opcode_cpy_imm:
		case v6502_opcode_eor_imm:
		case v6502_opcode_ora_imm:
		case v6502_opcode_lda_imm:
		case v6502_opcode_ldx_imm:
		case v6502_opcode_ldy_imm:
		case v6502_opcode_sbc_imm:
			return v6502_address_mode_immediate;
		case v6502_opcode_adc_abs:
		case v6502_opcode_and_abs:
		case v6502_opcode_asl_abs:
		case v6502_opcode_bit_abs:
		case v6502_opcode_cmp_abs:
		case v6502_opcode_cpx_abs:
		case v6502_opcode_cpy_abs:
		case v6502_opcode_dec_abs:
		case v6502_opcode_eor_abs:
		case v6502_opcode_inc_abs:
		case v6502_opcode_jmp_abs:
		case v6502_opcode_ora_abs:
		case v6502_opcode_lda_abs:
		case v6502_opcode_ldx_abs:
		case v6502_opcode_ldy_abs:
		case v6502_opcode_lsr_abs:
		case v6502_opcode_rol_abs:
		case v6502_opcode_ror_abs:
		case v6502_opcode_sbc_abs:
		case v6502_opcode_sta_abs:
		case v6502_opcode_stx_abs:
		case v6502_opcode_sty_abs:
		case v6502_opcode_jsr:
			return v6502_address_mode_absolute;
		case v6502_opcode_adc_absx:
		case v6502_opcode_and_absx:
		case v6502_opcode_asl_absx:
		case v6502_opcode_cmp_absx:
		case v6502_opcode_dec_absx:
		case v6502_opcode_eor_absx:
		case v6502_opcode_inc_absx:
		case v6502_opcode_ora_absx:
		case v6502_opcode_lda_absx:
		case v6502_opcode_ldy_absx:
		case v6502_opcode_lsr_absx:
		case v6502_opcode_rol_absx:
		case v6502_opcode_ror_absx:
		case v6502_opcode_sbc_absx:
		case v6502_opcode_sta_absx:
			return v6502_address_mode_absolute_x;
		case v6502_opcode_adc_absy:
		case v6502_opcode_and_absy:
		case v6502_opcode_cmp_absy:
		case v6502_opcode_eor_absy:
		case v6502_opcode_ora_absy:
		case v6502_opcode_lda_absy:
		case v6502_opcode_ldx_absy:
		case v6502_opcode_sbc_absy:
		case v6502_opcode_sta_absy:
			return v6502_address_mode_absolute_y;
		case v6502_opcode_jmp_ind:
			return v6502_address_mode_indirect;
		case v6502_opcode_adc_indx:
		case v6502_opcode_and_indx:
		case v6502_opcode_cmp_indx:
		case v6502_opcode_eor_indx:
		case v6502_opcode_ora_indx:
		case v6502_opcode_lda_indx:
		case v6502_opcode_sbc_indx:
		case v6502_opcode_sta_indx:
			return v6502_address_mode_indirect_x;
		case v6502_opcode_adc_indy:
		case v6502_opcode_and_indy:
		case v6502_opcode_cmp_indy:
		case v6502_opcode_eor_indy:
		case v6502_opcode_ora_indy:
		case v6502_opcode_lda_indy:
		case v6502_opcode_sbc_indy:
		case v6502_opcode_sta_indy:
			return v6502_address_mode_indirect_y;
		case v6502_opcode_adc_zpg:
		case v6502_opcode_and_zpg:
		case v6502_opcode_asl_zpg:
		case v6502_opcode_bit_zpg:
		case v6502_opcode_cmp_zpg:
		case v6502_opcode_cpx_zpg:
		case v6502_opcode_cpy_zpg:
		case v6502_opcode_dec_zpg:
		case v6502_opcode_eor_zpg:
		case v6502_opcode_inc_zpg:
		case v6502_opcode_ora_zpg:
		case v6502_opcode_lda_zpg:
		case v6502_opcode_ldx_zpg:
		case v6502_opcode_ldy_zpg:
		case v6502_opcode_lsr_zpg:
		case v6502_opcode_rol_zpg:
		case v6502_opcode_ror_zpg:
		case v6502_opcode_sbc_zpg:
		case v6502_opcode_sta_zpg:
		case v6502_opcode_stx_zpg:
		case v6502_opcode_sty_zpg:
			return v6502_address_mode_zeropage;
		case v6502_opcode_adc_zpgx:
		case v6502_opcode_and_zpgx:
		case v6502_opcode_asl_zpgx:
		case v6502_opcode_cmp_zpgx:
		case v6502_opcode_dec_zpgx:
		case v6502_opcode_eor_zpgx:
		case v6502_opcode_inc_zpgx:
		case v6502_opcode_ora_zpgx:
		case v6502_opcode_lda_zpgx:
		case v6502_opcode_ldy_zpgx:
		case v6502_opcode_lsr_zpgx:
		case v6502_opcode_rol_zpgx:
		case v6502_opcode_ror_zpgx:
		case v6502_opcode_sbc_zpgx:
		case v6502_opcode_sta_zpgx:
		case v6502_opcode_sty_zpgx:
			return v6502_address_mode_zeropage_x;
		case v6502_opcode_ldx_zpgy:
		case v6502_opcode_stx_zpgy:
			return v6502_address_mode_zeropage_y;
		default:
			return v6502_address_mode_unknown;
	}
}

#pragma mark - Tests

int test_addressModeForOpcode() {
	TEST_START;
	int rc = 0;
	
	for(v6502_opcode opcode = v6502_opcode_brk; opcode < 0xFF; opcode++) {
		v6502_address_mode knownMode = bruteForce_addressModeForOpcode(opcode);
		// Make sure we actually even support this instruction
		if(knownMode != v6502_address_mode_unknown) {
			if (v6502_addressModeForOpcode(opcode) != knownMode) {
				printf("Bad address mode for opcode %02x!\n", opcode);
				rc++;
			}
		}
	}
	return rc;
}

int test_sbc() {
	TEST_START;
	int rc = 0;
	
	v6502_cpu before;
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	v6502_map(cpu->memory, v6502_memoryStartWorkMemory, v6502_memoryStartCeiling, returnLow, NULL, NULL);
	
	printf("Testing SBC instruction...\n");
	
	v6502_reset(cpu);
	cpu->ac = 0xff;
	
	memcpy(&before, cpu, sizeof(v6502_cpu));
	v6502_execute(cpu, v6502_opcode_sbc_imm, 0x04, 0x00);
	if (!(cpu->ac == 0xfa && cpu->sr & v6502_cpu_status_carry)) {
		rc++;
		v6502_printCpuState(&before);
		v6502_printCpuState(cpu);
	}
	
	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	
	return rc;
}

int test_signedUnderflow() {
	TEST_START;
	int rc = 0;
	
	v6502_cpu before;
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	v6502_map(cpu->memory, v6502_memoryStartWorkMemory, v6502_memoryStartCeiling, returnLow, NULL, NULL);
	
	printf("Testing signed underflow...\n");
	
	v6502_reset(cpu);
	cpu->ac = 0x04;
	memcpy(&before, cpu, sizeof(v6502_cpu));
	v6502_execute(cpu, v6502_opcode_sbc_imm, 0xf0, 0x00);
	if (!(cpu->ac == 0x13 && !(cpu->sr & v6502_cpu_status_carry))) {
		rc++;
		v6502_printCpuState(&before);
		v6502_printCpuState(cpu);
	}
	
	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	
	return rc;
}

int test_jumpInstructionLength() {
	TEST_START;
	
	printf("Making sure that the jump instruction is calculated as 3 bytes...\n");

	return v6502_instructionLengthForOpcode(v6502_opcode_jmp_abs) != 3;
}

int test_wideJumpWithParsing() {
	TEST_START;
	int rc = 0;
	
	v6502_cpu before;
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	v6502_map(cpu->memory, v6502_memoryStartWorkMemory, v6502_memoryStartCeiling, returnLow, NULL, NULL);
	
	printf("Testing wide jump with inline assembler parsing...\n");
	
	v6502_reset(cpu);
	cpu->ac = 0x04;
	memcpy(&before, cpu, sizeof(v6502_cpu));
	TEST_ASM("jmp $7b7b");
	if (cpu->pc != 0x7b7b - 3 /* instruction length */) {
		rc++;
		v6502_printCpuState(&before);
		v6502_printCpuState(cpu);
	}
	
	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	
	return rc;
}

int test_intersectingMemoryMapping() {
	TEST_START;
	
	printf("Making sure the memory controller doesn't allow overlapping mapped regions...\n");
	
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	if (v6502_map(cpu->memory, 100, 100, returnLow, NULL, NULL)) {
		printf("Couldn't map the first range!\n");
		return 1;
	}
	
	if (v6502_map(cpu->memory, 50, 100, returnHigh, NULL, NULL)) {
		return 0;
	}
	
	printf("Second range was allowed!\n");

	return 1;
}

int test_contiguousMemoryMapping() {
	TEST_START;
	
	printf("Making sure the memory controller allows overlapping mapped regions...\n");
	
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	if (v6502_map(cpu->memory, 100, 100, returnLow, NULL, NULL)) {
		printf("Couldn't map the first range!\n");
		return 1;
	}
	
	if (v6502_map(cpu->memory, 200, 100, returnHigh, NULL, NULL)) {
		printf("Second range was not allowed!\n");
		return 1;
	}
	
	return 0;
}

#pragma mark - Test Harness

/* All you have to do to add a test is make a function that returns int,
 * starts with TEST_START, returns 1 (or more) for failure, and has been
 * appended to this array. The array length is automagically recalculated.
 */
static testFunction testFunctions[] = {
	test_sbc,
	test_signedUnderflow,
	test_jumpInstructionLength,
	test_wideJumpWithParsing,
	test_intersectingMemoryMapping,
	test_contiguousMemoryMapping,
	test_addressModeForOpcode
};

int main(int argc, const char *argv[]) {
	int rc = EX_OK;
	int color = 0;//isatty(fileno(stdin));
	
	for (size_t i = 0; i < TOTAL_TESTS; i++) {
		int lastResult = testFunctions[i]();
		if (lastResult) {
			if (color) {
				printf(">>> " ANSI_COLOR_BRIGHT_RED "FAILURE" ANSI_COLOR_RESET " >>> " ANSI_COLOR_BRIGHT_WHITE "%s" ANSI_COLOR_RESET "\n", lastTest);
			}
			else {
				printf(">>> FAILURE >>> %s\n", lastTest);
			}
			
			rc++;
		}
		else {
			printf("Passed test %s\n", lastTest);
		}
	}
	
	printf("Compiled unit test results: %lu/%lu tests passed, %lu%% pass rate.\n", (unsigned long)TOTAL_TESTS - rc, (unsigned long)TOTAL_TESTS, 100UL * (TOTAL_TESTS - rc) / TOTAL_TESTS);
	
	return rc;
}
