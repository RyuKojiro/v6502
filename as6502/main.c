//
//  main.c
//  as6502
//
//  Created by Daniel Loffgren on H.25/04/10.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <ctype.h>

#include "linectl.h"
#include "parser.h"
#include "symbols.h"
#include "object.h"
#include "codegen.h"

#define MAX_LINE_LEN		80
#define MAX_FILENAME_LEN	255

static unsigned long currentLineNum;
static const char *currentFileName;

void as6502_error(const char *error) {
	fprintf(stderr, "%s:%lu: error: ", currentFileName, currentLineNum);
	fprintf(stderr, "%s", error);
	if (error[strlen(error)] != '\n') {
		fprintf(stderr, "\n");
	}
}

void as6502_warn(const char *warning) {
	fprintf(stderr, "%s:%lu: warning: %s", currentFileName, currentLineNum, warning);
	if (warning[strlen(warning)] != '\n') {
		fprintf(stderr, "\n");
	}
}

static uint16_t assembleLine(as6502_object_blob *blob, const char *line, size_t len) {
	uint8_t opcode, low, high;
	int addrLen;
	v6502_address_mode mode;

	as6502_instructionForLine(&opcode, &low, &high, &mode, line, len);
	addrLen = as6502_instructionLengthForAddressMode(mode);
	
	// TODO: Write machine code to object, not directly to file
	if (addrLen >= 1) {
		as6502_appendByteToBlob(blob, opcode);
	}
	if (addrLen >= 2) {
		as6502_appendByteToBlob(blob, low);
	}
	if (addrLen >= 3) {
		as6502_appendByteToBlob(blob, high);
	}
	
	return addrLen;
}

static void assembleFile(FILE *in, FILE *out) {
	char line[MAX_LINE_LEN];
	char *trimmedLine;
	v6502_address_mode mode;
	uint16_t address = 0;
	currentLineNum = 1;
	int newline;
	as6502_symbol_table *table = as6502_createSymbolTable();
	size_t lineLen;
	
	// First pass, build symbol table
	do {
		newline = NO;
		fgets(line, MAX_LINE_LEN, in);
		if (strchr(line, '\n')) {
			newline = YES;
		}
		
		// Check for symbols
		trimmedLine = trimheadchar(line, '\n');
		if (isalnum(trimmedLine[0])) {
			as6502_addSymbolForLine(table, line, currentLineNum, address);
		}
		
		// Increment offset
		mode = as6502_addressModeForLine(line);
		address += as6502_instructionLengthForAddressMode(mode);
		
		// Check if we are on the next line yet
		if (newline) {
			currentLineNum++;
		}
	} while (!feof(in));
	
	// Reset for pass 2
	rewind(in);
	address = 0;
	currentLineNum = 1;
	
	// Prepare object structure
	as6502_object_context *ctx = as6502_createObjectContext();
	
	// Final pass, parse file to bitcode
	do {
		newline = NO;
		fgets(line, MAX_LINE_LEN, in);
		if (strchr(line, '\n')) {
			newline = YES;
		}
		
		// Truncate at comments
		trimgreedytailchard(line, ';');
		
		// Trim trailing whitespace
		trimtaild(line);
		
		// Convert symbols to hard addresses from symbol table
		as6502_desymbolicateLine(table, line, MAX_LINE_LEN, NO);
		
		// Check for Variable Declarations and Arithmetic
		as6502_resolveArithmetic(line, MAX_LINE_LEN);
		
		// Handle dot directives
		if (line[0] == '.') {
			as6502_processObjectDirectiveForLine(ctx, line, MAX_LINE_LEN);
			continue;
		}
		
		// Trim leading whitespace
		trimmedLine = trimhead(line);
		lineLen = strlen(trimmedLine);

		if (as6502_resolveVariableDeclaration(table, as6502_currentBlobInContext(ctx), assembleLine, trimmedLine, lineLen)) {
			continue;
		}

		// Assemble whatever is left
		if (lineLen) {
			address += assembleLine(as6502_currentBlobInContext(ctx), trimmedLine, lineLen);
		}
		
		// Check if we are on the next line yet
		if (newline) {
			currentLineNum++;
		}
	} while (!feof(in));
	
	as6502_writeObjectToFile(ctx->obj, out);
	
	as6502_destroyObjectContext(ctx);
	as6502_destroySymbolTable(table);
}

static void outNameFromInName(char *out, int len, const char *in) {
	int c;
	for (c = 0; c < len && in[c] && in[c] != '.'; c++) {
		out[c] = in[c];
	}
	out[c] = '.';
	out[++c] = 'o';
	out[++c] = '\0';
}

int main(int argc, const char * argv[]) {
	FILE *in;
	FILE *out;
	char outName[MAX_FILENAME_LEN];
	
	if (argc < 2) {
		currentFileName = "stdin";
		currentLineNum = 0;
		
		as6502_warn("Assembling from stdin does not support symbols");
		
		assembleFile(stdin, stdout);
		return 0;
	}
	
	for (int i = 1; i < argc; i++) {
		in = fopen(argv[i], "r");
		currentFileName = argv[i];
		outNameFromInName(outName, MAX_FILENAME_LEN, argv[i]);
		out = fopen(outName, "w");
		assembleFile(in, out);
		fclose(in);
		fclose(out);
	}
}

