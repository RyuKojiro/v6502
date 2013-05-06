//
//  symbols.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdlib.h>

#include "symbols.h"

// Address Table Lifecycle Functions
as6502_symbol_table *as6502_createLabelTable() {
	as6502_symbol_table *table = malloc(sizeof(as6502_symbol_table));
	
	table->labelCount = 0;
	table->labels = NULL;
	table->varCount = 0;
	table->vars = NULL;
	
	return table;
}

void as6502_destroySymbolTable(as6502_symbol_table *table) {
	free(table->labels);
	free(table->vars);
	free(table);
}

void as6502_populateSymbolTableWithFile(as6502_symbol_table *table, FILE *file) {
	// TODO: automatic population
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
void as6502_addLabelToTable(as6502_symbol_table *table, const char name, uint16_t address);
uint16_t as6502_addressForLabel(as6502_symbol_table *table, const char name);
void as6502_addVarToTable(as6502_symbol_table *table, const char name, uint16_t address);
uint16_t as6502_addressForVar(as6502_symbol_table *table, const char name);
