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

#define kDesymErrorText	"Could not shift string far enough while desymbolicating"

// Address Table Lifecycle Functions
as6502_symbol_table *as6502_createSymbolTable() {
	as6502_symbol_table *table = malloc(sizeof(as6502_symbol_table));
	
	table->labelCount = 0;
	table->first_label = NULL;
	table->varCount = 0;
	table->first_var = NULL;
	
	return table;
}

void as6502_destroySymbolTable(as6502_symbol_table *table) {
	as6502_var *next_var;
	for (as6502_var *this = table->first_var; this; this = next_var) {
		next_var = this->next;
		free(this->name);
		free(this);
	}
	
	as6502_label *next_label;
	for (as6502_label *this = table->first_label; this; this = next_label) {
		next_label = this->next;
		free(this->name);
		free(this);
	}
	
	free(table);
}

void as6502_printSymbolTable(as6502_symbol_table *table) {
	printf("Symbol table %p = {\n", table);
	for (as6502_var *this = table->first_var; this; this = this->next) {
		printf("\tVar { name = \"%s\", addr = 0x%x, init = 0x%x, next = %p }\n", this->name, this->address, this->init, this->next);
	}
	for (as6502_label *this = table->first_label; this; this = this->next) {
		printf("\tLabel { name = \"%s\", addr = 0x%x, next = %p }\n", this->name, this->address, this->next);
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

	// Add it to the table
	if (!table->first_label) {
		label->next = NULL;
		table->first_label = label;
	}
	else {
		for (as6502_label *this = table->first_label; this; this = this->next) {
			if (!this->next || strlen(this->next->name) < strlen(name)) {
				as6502_label *next = this->next;
				this->next = label;
				label->next = next;
				return;
			}
		}
	}
}

as6502_label *as6502_labelForString(as6502_symbol_table *table, const char *name) {
	size_t len = strlen(name);
	for (as6502_label *this = table->first_label; this; this = this->next) {
		if (strncmp(this->name, name, len)) {
			return this;
		}
	}
	
	return 0;
}

uint16_t as6502_addressForLabel(as6502_symbol_table *table, const char *name) {
	return as6502_labelForString(table, name)->address;
}

as6502_var *as6502_varForString(as6502_symbol_table *table, const char *name) {
	size_t len = strlen(name);
	for (as6502_var *this = table->first_var; this; this = this->next) {
		if (strncmp(this->name, name, len)) {
			return this;
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
	
	// Add it to the table
	if (!table->first_var) {
		var->next = NULL;
		table->first_var = var;
	}
	else {
		for (as6502_var *this = table->first_var; this; this = this->next) {
			if (!this->next || strlen(this->next->name) < strlen(name)) {
				as6502_var *next = this->next;
				this->next = var;
				var->next = next;
				return;
			}
		}
	}
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

static void as6502_replaceSymbolInLineAtLocationWithText(char *line, size_t len, char *loc, const char *symbol, const char *text) {
	size_t symLen = strlen(symbol);
	size_t txtLen = strlen(text);
	long difference = txtLen - symLen;
	
	if (difference < 0) { // Shift string left
		for (char *cur = loc + txtLen; *cur; cur++) {
			cur[0] = cur[0 - difference];
		}
	}

	if (difference > 0) { // Shift string right
		if (strlen(line) + difference < len) {
			char *start = line + strlen(line) + 1;
			for (char *cur = start + difference; cur > start; cur--) {
				*cur = *(cur - difference);
			}
		}
		else {
			die(kDesymErrorText);
		}
	}
	
	// Strings are aligned, overwrite
	memcpy(loc, text, txtLen);
}

void as6502_desymbolicateLine(as6502_symbol_table *table, char *line, size_t len) {
	// FIXME: This needs to be smart about address formation, based on address mode
	// This is absurdly inefficient, but works, given the current symbol table implementation
	char *cur;
	char addrString[7];
	
	// Ensure termination for strstr
	line[len - 1] = '\0';
	
	for (as6502_var *this = table->first_var; this; this = this->next) {
		cur = strstr(line, this->name);
		if (cur) {
			snprintf(addrString, 7, "$%04x", this->address);
			as6502_replaceSymbolInLineAtLocationWithText(line, len, cur, this->name, addrString);
		}
	}
	for (as6502_label *this = table->first_label; this; this = this->next) {
		cur = strstr(line, this->name);
		if (cur == line) {
			as6502_replaceSymbolInLineAtLocationWithText(line, len, cur, this->name, "");
		}
		else if (cur) {
			snprintf(addrString, 7, "$%04x", this->address);
			as6502_replaceSymbolInLineAtLocationWithText(line, len, cur, this->name, addrString);
		}
	}	
}
