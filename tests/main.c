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

#include "brute.h"

#pragma mark Test Harness

#define TOTAL_TESTS		(sizeof(testFunctions) / sizeof(testFunction))
#define TEST_START		lastTest = __PRETTY_FUNCTION__
#define TEST_ASM(a)		as6502_executeAsmLineOnCPU(cpu, a, sizeof(a))

typedef int (* testFunction)(void);

static const char *lastTest;

static uint8_t returnLow(struct _v6502_memory *memory, uint16_t offset, int trap, void *context) {
	return 0;
}

static uint8_t returnHigh(struct _v6502_memory *memory, uint16_t offset, int trap, void *context) {
	return ~0;
}

#pragma mark - Tests

static int test_addressModeForOpcode() {
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

static int test_instructionLengthForOpcode() {
	TEST_START;
	int rc = 0;

	for(v6502_opcode opcode = v6502_opcode_brk; opcode < 0xFF; opcode++) {
		int knownLen = bruteForce_instructionLengthForOpcode(opcode);
		// Make sure we actually even support this instruction
		if(knownLen) {
			if (v6502_instructionLengthForOpcode(opcode) != knownLen) {
				printf("Bad instruction length for opcode %02x!\n", opcode);
				rc++;
			}
		}
	}
	return rc;
}

static int test_sbc() {
	TEST_START;
	int rc = 0;

	v6502_cpu before;
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	v6502_map(cpu->memory, v6502_memoryStartWorkMemory, v6502_memoryStartCeiling, returnLow, NULL, NULL);

	printf("Testing SBC instruction...\n");

	v6502_reset(cpu);
	TEST_ASM("lda #$ff");

	memcpy(&before, cpu, sizeof(v6502_cpu));
	TEST_ASM("sbc #$04");
	if (!(cpu->ac == 0xfa &&
		  cpu->sr & v6502_cpu_status_carry &&
		  cpu->sr & v6502_cpu_status_negative)) {
		rc++;
		v6502_printCpuState(stderr, &before);
		v6502_printCpuState(stderr, cpu);
	}

	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);

	return rc;
}

static int test_signedUnderflow() {
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
		v6502_printCpuState(stderr, &before);
		v6502_printCpuState(stderr, cpu);
	}

	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);

	return rc;
}

static int test_jumpInstructionLength() {
	TEST_START;

	printf("Making sure that the jump instruction is calculated as 3 bytes...\n");

	return v6502_instructionLengthForOpcode(v6502_opcode_jmp_abs) != 3;
}

static int test_wideJumpWithParsing() {
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
		v6502_printCpuState(stderr, &before);
		v6502_printCpuState(stderr, cpu);
	}

	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);

	return rc;
}

static int test_intersectingMemoryMapping() {
	TEST_START;
	int rc = 0;

	printf("Making sure the memory controller doesn't allow overlapping mapped regions...\n");

	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	if (!v6502_map(cpu->memory, 100, 100, returnLow, NULL, NULL)) {
		printf("Couldn't map the first range!\n");
		rc++;
	}

	if (v6502_map(cpu->memory, 50, 100, returnHigh, NULL, NULL)) {
		printf("Second range was allowed!\n");
		rc++;
	}

	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	return rc;
}

static int test_contiguousMemoryMapping() {
	TEST_START;
	int rc = 0;

	printf("Making sure the memory controller allows contiguously mapped regions...\n");

	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	if (!v6502_map(cpu->memory, 100, 100, returnLow, NULL, NULL)) {
		printf("Couldn't map the first range!\n");
		rc++;
	}

	if (!v6502_map(cpu->memory, 200, 100, returnHigh, NULL, NULL)) {
		printf("Second range was not allowed!\n");
		rc++;
	}

	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	return rc;
}

static int test_ceilingMemoryMapping() {
	TEST_START;
	int rc = 0;

	printf("Making sure the memory controller protects mapping near the end of the address space...\n");

	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);

	if (v6502_map(cpu->memory, 0xFFFF, 2, returnLow, NULL, NULL)) {
		printf("Mapped beyond the end!\n");
		rc++;
	}

	if (!v6502_map(cpu->memory, 0xFFFF, 1, returnLow, NULL, NULL)) {
		printf("Couldn't map the last byte!\n");
		rc++;
	}

	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	return rc;
}

static int test_cmpCarrySet() {
	TEST_START;
	int rc = 0;

	v6502_cpu before;
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	v6502_map(cpu->memory, v6502_memoryStartWorkMemory, v6502_memoryStartCeiling, returnLow, NULL, NULL);

	printf("Making sure 0xB4 (cmp) 0x8D sets the carry flag...\n");

	v6502_reset(cpu);
	TEST_ASM("lda #$b4");
	memcpy(&before, cpu, sizeof(v6502_cpu));
	TEST_ASM("cmp #$8d");
	if (!(cpu->sr & v6502_cpu_status_carry)) {
		rc++;
		v6502_printCpuState(stderr, &before);
		v6502_printCpuState(stderr, cpu);
	}

	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);

	return rc;
}

static int test_adc1() {
	TEST_START;
	int rc = 0;

	v6502_cpu before;
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	v6502_map(cpu->memory, v6502_memoryStartWorkMemory, v6502_memoryStartCeiling, returnLow, NULL, NULL);

	printf("Testing 0xFD (adc) 0x06 for carry, overflow, and result...\n");

	v6502_reset(cpu);
	TEST_ASM("lda #253");
	memcpy(&before, cpu, sizeof(v6502_cpu));
	TEST_ASM("adc #6");
	if (!(cpu->sr & v6502_cpu_status_overflow &&
		  cpu->ac == 3)) {
		rc++;
		v6502_printCpuState(stderr, &before);
		v6502_printCpuState(stderr, cpu);
	}

	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);

	return rc;
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
	test_ceilingMemoryMapping,
	test_addressModeForOpcode,
	test_instructionLengthForOpcode,
	test_cmpCarrySet,
	test_adc1,
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
