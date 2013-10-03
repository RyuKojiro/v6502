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

#include "object.h"
#include "error.h"
#include "flat.h"
#include "reverse.h"
#include "symbols.h"

#define MAX_FILENAME_LEN	255
#define MAX_LINE_LEN		80
#define MAX_SYM_LEN			25

static int _isBranchOpcode(v6502_opcode opcode) {
	switch (opcode) {
		case v6502_opcode_bcc:
		case v6502_opcode_bcs:
		case v6502_opcode_beq:
		case v6502_opcode_bmi:
		case v6502_opcode_bne:
		case v6502_opcode_bpl:
		case v6502_opcode_bvc:
		case v6502_opcode_bvs:
		case v6502_opcode_jmp_abs:
		case v6502_opcode_jmp_ind:
		case v6502_opcode_jsr:
			return YES;
		default:
			return NO;
	}
}

static void disassembleFile(FILE *in, FILE *out) {
	char line[MAX_LINE_LEN];
	char symbolName[MAX_SYM_LEN];
	int currentLabel = 1;
	uint16_t address;
	
	as6502_object *obj = as6502_createObject();
	as6502_readObjectFromFlatFile(obj, in);
	
	as6502_symbol_table *table = as6502_createSymbolTable();
	
	for (int i = 0; i < obj->count; i++) {
		as6502_object_blob *blob = &obj->blobs[i];

		// Build Symbol Table
		currentLineNum = 0;
		for (uint8_t offset = 0; offset < blob->len; offset += v6502_instructionLengthForOpcode(blob->data[offset])) {
			v6502_opcode opcode = blob->data[offset];
			if (_isBranchOpcode(opcode)) {				
				if (opcode == v6502_opcode_jmp_abs || opcode == v6502_opcode_jmp_ind || opcode == v6502_opcode_jsr) {
					address = (blob->data[offset + 2] << 8 | blob->data[offset + 1]) - v6502_memoryStartProgram;
				}
				else {
						address = offset + 2 + v6502_signedValueOfByte(blob->data[offset + 1]);
				}
				
				if (!as6502_symbolForAddress(table, address)) {
					snprintf(symbolName, MAX_SYM_LEN, "Label%d", currentLabel++);
					as6502_addSymbolToTable(table, currentLineNum, symbolName, address, as6502_symbol_type_label);
				}
			}
			currentLineNum++;
		}

		// Disassemble
		currentLineNum = 0;
		for (uint8_t offset = 0; offset < blob->len; offset += v6502_instructionLengthForOpcode(blob->data[offset])) {
			as6502_symbol *label = as6502_symbolForAddress(table, offset);
			if (label) {
				fprintf(out, "%s:\n", label->name);
			}
			
			as6502_stringForInstruction(line, MAX_LINE_LEN, blob->data[offset], blob->data[offset + 2], blob->data[offset + 1]);
			as6502_symbolicateLine(table, line, MAX_LINE_LEN, v6502_memoryStartProgram, offset);
			
			fprintf(out, "\t%s\n", line);
			currentLineNum++;
		}
	}
	
	as6502_destroyObject(obj);
}

static void outNameFromInName(char *out, int len, const char *in) {
	int c;
	for (c = 0; c < len && in[c] && in[c] != '.'; c++) {
		out[c] = in[c];
	}
	out[c] = '.';
	out[++c] = 's';
	out[++c] = '\0';
}

static void usage() {
	fprintf(stderr, "usage: dis6502 [file ...]\n");
}

int main(int argc, char * const argv[]) {
	FILE *in;
	FILE *out;
	char outName[MAX_FILENAME_LEN];
	
	for (int i = 1; i < argc; i++) {
		in = fopen(argv[i], "r");
		currentFileName = argv[i];
		outNameFromInName(outName, MAX_FILENAME_LEN, argv[i]);
		out = fopen(outName, "w");
		disassembleFile(in, out);
		fclose(in);
		fclose(out);
	}
}

