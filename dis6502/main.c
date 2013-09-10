//
//  main.c
//  dis6502
//
//  Created by Daniel Loffgren on H.25/09/10.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>

#include "object.h"
#include "error.h"
#include "flat.h"
#include "reverse.h"

#define MAX_LINE_LEN		80
#define MAX_FILENAME_LEN	255

static void disassembleFile(FILE *in, FILE *out) {
	char line[MAX_LINE_LEN];
	
	as6502_object *obj = as6502_createObject();
	as6502_readObjectFromFlatFile(obj, in);
	
	
	as6502_object_blob blob = obj->blobs[0];
	for (uint8_t offset = 0; offset < blob.len; offset++) {
		as6502_stringForInstruction(line, MAX_LINE_LEN, blob.data[0], blob.data[2], blob.data[1]);
		offset += v6502_instructionLengthForOpcode(blob.data[0]);
		fprintf(out, "%s\n", line);
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

