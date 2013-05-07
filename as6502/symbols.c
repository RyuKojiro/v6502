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

uint16_t as6502_addressForLabel(as6502_symbol_table *table, const char *name) {
	size_t len = strlen(name);
	for (int i = 0; i < table->labelCount; i++) {
		if (strncmp(table->labels[i]->name, name, len)) {
			return table->labels[i]->address;
		}
	}
	
	return 0;
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

uint16_t as6502_addressForVar(as6502_symbol_table *table, const char *name) {
	size_t len = strlen(name);
	for (int i = 0; i < table->varCount; i++) {
		if (strncmp(table->vars[i]->name, name, len)) {
			return table->vars[i]->address;
		}
	}
	
	return 0;
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

void as6502_desymbolicateLine(as6502_symbol_table *table, char *line) {
	
}

