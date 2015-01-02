//
//  main.c
//  v6502
//
//  Created by Daniel Loffgren on 1/2/15.
//  Copyright (c) 2015 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <sysexits.h>
#include "color.h"

#pragma mark Macro Support

#define TOTAL_TESTS (sizeof(testFunctions) / sizeof(testFunction))
#define TEST_START lastTest = __PRETTY_FUNCTION__

typedef int (* testFunction)(void);

static const char *lastTest;

#pragma mark - Tests

int test_sign() {
	TEST_START;
	
	return 1;
}

int test_pass() {
	TEST_START;
	
	return 0;
}

#pragma mark - Test Harness

testFunction testFunctions[] = {
	test_pass,
	test_sign,
};

int main(int argc, const char *argv[]) {
	int rc = EX_OK;
	
	for (int i = 0; i < TOTAL_TESTS; i++) {
		int lastResult = testFunctions[i]();
		if (lastResult) {
			printf(">>> " ANSI_COLOR_BRIGHT_RED "FAILURE" ANSI_COLOR_RESET " >>> " ANSI_COLOR_BRIGHT_WHITE "%s\n" ANSI_COLOR_RESET, lastTest);
			rc++;
		}
		else {
			printf("Passed test %s\n", lastTest);
		}
	}
	
	return rc;
}
