//
//  symbols.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef as6502_symbols_h
#define as6502_symbols_h

#include <stdint.h>

// Types

typedef struct {
	unsigned long line;
	char *name;
	uint16_t address;
} as6502_label;

typedef struct {
	unsigned long line;
	char *name;
	uint16_t address;
	uint8_t init;
} as6502_var;

typedef struct {
	int labelCount;
	as6502_label **labels;
	int varCount;
	as6502_var **vars;
} as6502_symbol_table;

// Address Table Lifecycle Functions
as6502_symbol_table *as6502_createSymbolTable();
void as6502_destroySymbolTable(as6502_symbol_table *table);
void as6502_printSymbolTable(as6502_symbol_table *table);

// Symbol Table Accessors
void as6502_addLabelToTable(as6502_symbol_table *table, unsigned long line, const char *name, uint16_t address);
uint16_t as6502_addressForLabel(as6502_symbol_table *table, const char *name);
void as6502_addVarToTable(as6502_symbol_table *table, unsigned long line, const char *name, uint16_t address);
uint16_t as6502_addressForVar(as6502_symbol_table *table, const char *name);

// Easy Symbol Table Interaction
void as6502_addSymbolForLine(as6502_symbol_table *table, const char *line, unsigned long lineNumber, uint16_t offset);
void as6502_desymbolicateLine(as6502_symbol_table *table, char *line);

#endif