//
//  symbols.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h> // as6502_printSymbolTable

#include "symbols.h"
#include "common.h"
#include "linectl.h"

// Address Table Lifecycle Functions
as6502_symbol_table *as6502_createSymbolTable() {
	as6502_symbol_table *table = malloc(sizeof(as6502_symbol_table));
	
	table->labelCount = 0;
	table->labels = NULL;
	table->varCount = 0;
	table->vars = NULL;
	
	return table;
}

void as6502_destroySymbolTable(as6502_symbol_table *table) {
	for (int i = 0; i < table->labelCount; i++) {
		free(table->labels[i]->name);
		free(table->labels[i]);
	}
	free(table->labels);
	
	for (int i = 0; i < table->varCount; i++) {
		free(table->vars[i]->name);
		free(table->vars[i]);
	}
	free(table->vars);
	
	free(table);
}

void as6502_printSymbolTable(as6502_symbol_table *table) {
	printf("Symbol table %p = {\n", table);
	for (int i = 0; i < table->varCount; i++) {
		printf("\tVar[%d] { name = \"%s\", addr = 0x%x, init = 0x%x }\n", i, table->vars[i]->name, table->vars[i]->address, table->vars[i]->init);
	}
	for (int i = 0; i < table->labelCount; i++) {
		printf("\tLabel[%d] { name = \"%s\", addr = 0x%x }\n", i, table->labels[i]->name, table->labels[i]->address);
	}
	printf("}\n");
}

// Symbol Table Accessors
void as6502_addLabelToTable(as6502_symbol_table *table, unsigned long line, const char *name, uint16_t address) {
	as6502_label *label = malloc(sizeof(as6502_label));
	if (!label) {
		die("label malloc in as6502_addLabelToTable");
	}
	label->line = line;
	label->address = address;

	size_t len = strlen(name) + 1;
	label->name = malloc(len);
	if (!label->name) {
		die("label name malloc in as6502_addLabelToTable");
	}
	memcpy(label->name, name, len);

	table->labels = realloc(table->labels, sizeof(as6502_label *) * table->labelCount + 1);
	if (!table->labels) {
		die("labels realloc in as6502_addLabelToTable");
	}
	table->labels[table->labelCount++] = label;
}

as6502_label *as6502_labelForString(as6502_symbol_table *table, const char *name) {
	size_t len = strlen(name);
	for (int i = 0; i < table->labelCount; i++) {
		if (strncmp(table->labels[i]->name, name, len)) {
			return table->labels[i];
		}
	}
	
	return 0;
}

uint16_t as6502_addressForLabel(as6502_symbol_table *table, const char *name) {
	return as6502_labelForString(table, name)->address;
}

as6502_var *as6502_varForString(as6502_symbol_table *table, const char *name) {
	size_t len = strlen(name);
	for (int i = 0; i < table->varCount; i++) {
		if (strncmp(table->vars[i]->name, name, len)) {
			return table->vars[i];
		}
	}
	
	return 0;
}

uint16_t as6502_addressForVar(as6502_symbol_table *table, const char *name) {
	return as6502_varForString(table, name)->address;
}

void as6502_addVarToTable(as6502_symbol_table *table, unsigned long line, const char *name, uint16_t address) {
	as6502_var *var = malloc(sizeof(as6502_var));
	if (!var) {
		die("var malloc in as6502_addVarToTable");
	}
	var->line = line;
	var->address = address;
	
	size_t len = strlen(name) + 1;
	var->name = malloc(len);
	if (!var->name) {
		die("var name malloc in as6502_addVarToTable");
	}
	memcpy(var->name, name, len);
	
	table->vars = realloc(table->vars, sizeof(as6502_var *) * table->varCount + 1);
	if (!table->vars) {
		die("vars realloc in as6502_addVarToTable");
	}
	table->vars[table->varCount++] = var;
}

// Easy Symbol Table Access

void as6502_addSymbolForLine(as6502_symbol_table *table, const char *line, unsigned long lineNumber, uint16_t offset) {
	size_t len = strlen(line) + 1;
	char *symbol = malloc(len);
	strncpy(symbol, line, len);
	trimgreedytaild(symbol); // symbol will only be the first phrase
	
	if (strchr(line, '=')) { // Variable
		// TODO: allocate variable addresses
		as6502_addVarToTable(table, lineNumber, symbol, 0);
	}
	else { // Label
		as6502_addLabelToTable(table, lineNumber, symbol, offset);
	}
	
	free(symbol);
}

static void as6502_replaceSymbolInLineAtLocationWithText(char *line, char *loc, const char *symbol, const char *text) {
	size_t symLen = strlen(symbol);
	size_t txtLen = strlen(text);
	long difference = txtLen - symLen;
	
	if (difference < 0) { // Shift string left
		for (/* difference */; difference < 0; difference++) {
			for (char *cur = loc + symLen - 1; *cur; cur++) {
				cur[0] = cur[1];
			}
		}
	}

	if (difference > 0) { // Shift string right (Unsafely?)
		fprintf(stderr, "as6502: Tried to desymbolicate label less than 4 chars! - FIXME\n");
	}
	
	// Strings are aligned, overwrite
	memcpy(loc, text, txtLen);
}

void as6502_desymbolicateLine(as6502_symbol_table *table, char *line) {
	// NOTE: Must be null terminated, this is not good, especially for cases where desymbolication will expand the line length
	// This is absurdly inefficient, but works, given the current symbol table implementation
	char *cur;
	int i;
	char addrString[7];
	
	for (i = 0; i < table->varCount; i++) {
		cur = strstr(line, table->vars[i]->name);
		if (cur) {
			// TODO: What actual address length are we going to use, can there be absolute addressed symbols?
			snprintf(addrString, 7, "0x%02x", table->vars[i]->address);
			as6502_replaceSymbolInLineAtLocationWithText(line, cur, table->vars[i]->name, addrString);
		}
	}
	for (i = 0; i < table->labelCount; i++) {
		cur = strstr(line, table->labels[i]->name);
		if (cur) {
			as6502_replaceSymbolInLineAtLocationWithText(line, cur, table->labels[i]->name, "");
		}
	}	
}
