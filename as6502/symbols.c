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
#include <ctype.h> // isspace

#include "symbols.h"
#include "error.h"
#include "linectl.h"
#include "parser.h"

#include "mem.h"
#include "cpu.h"

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
	as6502_symbol *next_var;
	for (as6502_symbol *this = table->first_var; this; this = next_var) {
		next_var = this->next;
		free(this->name);
		free(this);
	}
	
	as6502_symbol *next_label;
	for (as6502_symbol *this = table->first_label; this; this = next_label) {
		next_label = this->next;
		free(this->name);
		free(this);
	}
	
	free(table);
}

void as6502_printSymbolTable(as6502_symbol_table *table) {
	printf("Symbol table %p = {\n", table);
	for (as6502_symbol *this = table->first_var; this; this = this->next) {
		printf("\tVar { name = \"%s\", addr = 0x%x, next = %p }\n", this->name, this->address, this->next);
	}
	for (as6502_symbol *this = table->first_label; this; this = this->next) {
		printf("\tLabel { name = \"%s\", addr = 0x%x, next = %p }\n", this->name, this->address, this->next);
	}
	printf("}\n");
}

// Symbol Table Accessors
void as6502_addLabelToTable(as6502_symbol_table *table, unsigned long line, const char *name, uint16_t address) {
	as6502_symbol *label = malloc(sizeof(as6502_symbol));
	if (!label) {
		as6502_fatal("label malloc in as6502_addLabelToTable");
	}
	label->line = line;
	label->address = address;

	size_t len = strlen(name) + 1;
	label->name = malloc(len);
	if (!label->name) {
		as6502_fatal("label name malloc in as6502_addLabelToTable");
	}
	memcpy(label->name, name, len);

	// Add it to the table
	for (as6502_symbol **this = &table->first_label;; this = &((*this)->next)) {
		if (!*this || strlen((*this)->name) < strlen(name)) {
			as6502_symbol *next = *this;
			*this = label;
			label->next = next;
			return;
		}
	}
}

as6502_symbol *as6502_labelForString(as6502_symbol_table *table, const char *name) {
	size_t len = strlen(name);
	for (as6502_symbol *this = table->first_label; this; this = this->next) {
		if (strncmp(this->name, name, len)) {
			return this;
		}
	}
	
	return NULL;
}

as6502_symbol *as6502_labelForAddress(as6502_symbol_table *table, uint16_t address) {
	for (as6502_symbol *this = table->first_label; this; this = this->next) {
		if (this->address == address) {
			return this;
		}
	}
	
	return NULL;
}

uint16_t as6502_addressForLabel(as6502_symbol_table *table, const char *name) {
	return as6502_labelForString(table, name)->address;
}

as6502_symbol *as6502_varForString(as6502_symbol_table *table, const char *name) {
	size_t len = strlen(name);
	for (as6502_symbol *this = table->first_var; this; this = this->next) {
		if (strncmp(this->name, name, len)) {
			return this;
		}
	}
	
	return NULL;
}

uint16_t as6502_addressForVar(as6502_symbol_table *table, const char *name) {
	return as6502_varForString(table, name)->address;
}

void as6502_addVarToTable(as6502_symbol_table *table, unsigned long line, const char *name, uint16_t address) {
	as6502_symbol *var = malloc(sizeof(as6502_symbol));
	if (!var) {
		as6502_fatal("var malloc in as6502_addVarToTable");
	}
	var->line = line;
	var->address = address;
	
	size_t len = strlen(name) + 1;
	var->name = malloc(len);
	if (!var->name) {
		as6502_fatal("var name malloc in as6502_addVarToTable");
	}
	memcpy(var->name, name, len);
	
	// Add it to the table
	for (as6502_symbol **this = &table->first_var;; this = &((*this)->next)) {
		if (!*this || strlen((*this)->name) < strlen(name)) {
			as6502_symbol *next = *this;
			*this = var;
			var->next = next;
			return;
		}
	}
}

// Easy Symbol Table Access

void as6502_addSymbolForLine(as6502_symbol_table *table, const char *line, unsigned long lineNumber, uint16_t offset) {
	size_t len = strlen(line) + 1;
	char *symbol = malloc(len);
	strncpy(symbol, line, len);
	trimgreedytaild(symbol); // symbol will only be the first phrase
	trimtailchard(symbol, ':'); // If there is a colon, truncate there
	
	if (strchr(line, '=')) { // Variable
		/** TODO: @todo allocate variable addresses */
		as6502_addVarToTable(table, lineNumber, symbol, 0);
	}
	else { // Label
		as6502_addLabelToTable(table, lineNumber, symbol, offset);
	}
	
	free(symbol);
}

void as6502_replaceSymbolInLineAtLocationWithText(char *line, size_t len, char *loc, char *symbol, const char *text) {
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
			as6502_fatal(kDesymErrorText);
		}
	}
	
	// Strings are aligned, overwrite
	memcpy(loc, text, txtLen);
}

static int as6502_doubleWidthForSymbolInLine(as6502_symbol_table *table, char *line, size_t len, char *symbol) {
	char *trimmed = trimhead(line);
	//len -= trimmed - line;
	
	if (((trimmed[0] == 'b' || trimmed[0] == 'B') && (strncasecmp(trimmed, "bit", 3) && strncasecmp(trimmed, "brk", 3))) || symbol[-1] == '(') {
		return 0;
	}

	return 1;
}

void as6502_desymbolicateLine(as6502_symbol_table *table, char *line, size_t len, uint16_t pstart, uint16_t offset, int caseSensitive) {
	/** FIXME: This needs to be smart about address formation, based on address mode
	 * This is absurdly inefficient, but works, given the current symbol table implementation
	 */
	char *cur;
	char addrString[7];
	int width;
	
	// Ensure termination for strstr
	line[len - 1] = '\0';
	
	cur = strnchr(line, ':', len);
	if (cur) {
		as6502_replaceSymbolInLineAtLocationWithText(line, len, cur, ":", "");
	}
	
	// Shift offset for pre-branch program counter shift
	v6502_address_mode mode = as6502_addressModeForLine(line);
	offset += as6502_instructionLengthForAddressMode(mode);
	
	for (as6502_symbol *this = table->first_var; this; this = this->next) {
		// Search for symbol
		if (caseSensitive) {
			cur = strstr(line, this->name);
		}
		else {
			cur = strcasestr(line, this->name);
		}
		
		// Swap in address
		if (cur) {
			if (!isspace(cur[-1]) && cur[-1] != '(') {
				// Partial symbol match
				continue;
			}
			
			width = as6502_doubleWidthForSymbolInLine(table, line, len, cur);
			snprintf(addrString, 7, width ? "$%04x" : "$%02x", this->address);
			as6502_replaceSymbolInLineAtLocationWithText(line, len, cur, this->name, addrString);
		}
	}
	for (as6502_symbol *this = table->first_label; this; this = this->next) {
		// Search for symbol
		if (caseSensitive) {
			cur = strstr(line, this->name);
		}
		else {
			cur = strcasestr(line, this->name);
		}

		// Swap in address
		if (cur == line) {
			as6502_replaceSymbolInLineAtLocationWithText(line, len, cur, this->name, "");
		}
		else if (cur) {
			if (!isspace(cur[-1])) {
				// Partial symbol match
				continue;
			}
			
			width = as6502_doubleWidthForSymbolInLine(table, line, len, cur);
			if (width) {
				snprintf(addrString, 7, "$%04x", pstart + this->address);
			}
			else {
				snprintf(addrString, 5, "$%02x", v6502_byteValueOfSigned(this->address - offset));
			}
			as6502_replaceSymbolInLineAtLocationWithText(line, len, cur, this->name, addrString);
		}
	}	
}
