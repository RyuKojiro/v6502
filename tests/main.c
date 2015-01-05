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
#include "color.h"
#include "cpu.h"
#include "log.h"

#pragma mark Test Harness

#define TOTAL_TESTS (sizeof(testFunctions) / sizeof(testFunction))
#define TEST_START lastTest = __PRETTY_FUNCTION__

typedef int (* testFunction)(void);

static const char *lastTest;

uint8_t returnLow(struct _v6502_memory *memory, uint16_t offset, int trap, void *context) {
	return 0;
}

uint8_t returnHigh(struct _v6502_memory *memory, uint16_t offset, int trap, void *context) {
	return ~0;
}

#pragma mark - Tests

int test_sbc() {
	TEST_START;
	int rc = 0;
	
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	v6502_map(cpu->memory, v6502_memoryStartWorkMemory, v6502_memoryStartCeiling, returnLow, NULL, NULL);
	
	printf("Testing SBC instruction...\n");
	
	v6502_reset(cpu);
	cpu->ac = 0xff;
	//v6502_printCpuState(cpu);
	v6502_execute(cpu, v6502_opcode_sbc_imm, 0x04, 0x00);
	//v6502_printCpuState(cpu);
	if (!(cpu->ac == 0xfa && cpu->sr & v6502_cpu_status_carry)) {
		rc++;
	}
	
	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	
	return rc;
}

int test_signedUnderflow() {
	TEST_START;
	int rc = 0;
	
	v6502_cpu *cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(0);
	v6502_map(cpu->memory, v6502_memoryStartWorkMemory, v6502_memoryStartCeiling, returnLow, NULL, NULL);
	
	printf("Testing signed underflow...\n");
	
	v6502_reset(cpu);
	cpu->ac = 0x04;
	//v6502_printCpuState(cpu);
	v6502_execute(cpu, v6502_opcode_sbc_imm, 0xf0, 0x00);
	//v6502_printCpuState(cpu);
	if (!(cpu->ac == 0x13 && !(cpu->sr & v6502_cpu_status_carry))) {
		rc++;
	}
	
	v6502_destroyMemory(cpu->memory);
	v6502_destroyCPU(cpu);
	
	return rc;
}

#pragma mark - Test Harness

static testFunction testFunctions[] = {
	test_sbc,
	test_signedUnderflow
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
	
	printf("Compiled unit test results: %lu/%lu tests passed, %lu%% pass rate.\n", TOTAL_TESTS - rc, TOTAL_TESTS, 100 * (TOTAL_TESTS - rc) / TOTAL_TESTS);
	
	return rc;
}