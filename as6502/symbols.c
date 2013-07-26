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

#define kDesymbolicationErrorText	"Could not shift string far enough while desymbolicating"
#define kDuplicateSymbolErrorText	"Encountered duplicate symbol declaration '%s'"
#define kDuplicateSymbolNoteText	"Previous declaration was here"

// Address Table Lifecycle Functions
as6502_symbol_table *as6502_createSymbolTable() {
	as6502_symbol_table *table = malloc(sizeof(as6502_symbol_table));
	
	table->symbolCount = 0;
	table->first_symbol = NULL;
	
	return table;
}

void as6502_destroySymbolTable(as6502_symbol_table *table) {
	as6502_symbol *next;
	for (as6502_symbol *this = table->first_symbol; this; this = next) {
		next = this->next;
		free(this->name);
		free(this);
	}
	
	free(table);
}

void as6502_printSymbolTable(as6502_symbol_table *table) {
	printf("Symbol table %p = {\n", table);
	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		char *type;
		
		switch (this->type) {
			case as6502_symbol_type_unknown: {
				type = "???";
			} break;
			case as6502_symbol_type_label: {
				type = "Label";
			} break;
			case as6502_symbol_type_variable: {
				type = "Var";
			} break;
		}
		
		printf("\t%s { name = \"%s\", addr = 0x%x, next = %p, line = %lu }\n", type, this->name, this->address, this->next, this->line);
	}
	printf("}\n");
}

// Symbol Table Accessors
as6502_symbol *as6502_symbolForString(as6502_symbol_table *table, const char *name) {
	if (!table) {
		return NULL;
	}
	size_t len = strlen(name);
	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		if (strncmp(this->name, name, len)) {
			return this;
		}
	}
	
	return NULL;
}

as6502_symbol *as6502_symbolForAddress(as6502_symbol_table *table, uint16_t address) {
	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		if (this->address == address) {
			return this;
		}
	}
	
	return NULL;
}

uint16_t as6502_addressForLabel(as6502_symbol_table *table, const char *name) {
	as6502_symbol *label = as6502_symbolForString(table, name);
	
	if (!label) {
		return 0;
	}
	
	return label->address;
}

uint16_t as6502_addressForVar(as6502_symbol_table *table, const char *name) {
	as6502_symbol *var = as6502_symbolForString(table, name);
	
	if (!var) {
		return 0;
	}
	
	return var->address;
}

void as6502_addSymbolToTable(as6502_symbol_table *table, unsigned long line, const char *name, uint16_t address, as6502_symbol_type type) {
	as6502_symbol *sym = malloc(sizeof(as6502_symbol));
	if (!sym) {
		as6502_fatal("symbol malloc in as6502_addVarToTable");
	}
	sym->line = line;
	sym->address = address;
	sym->type = type;
	
	size_t len = strlen(name) + 1;
	sym->name = malloc(len);
	if (!sym->name) {
		as6502_fatal("symbol name malloc in as6502_addVarToTable");
	}
	memcpy(sym->name, name, len);
	
	// Add it to the table
	for (as6502_symbol **this = &table->first_symbol;; this = &((*this)->next)) {
		if (*this && !strncmp((*this)->name, name, len)) {
			as6502_error(kDuplicateSymbolErrorText, name);
			as6502_note((*this)->line, kDuplicateSymbolNoteText);
			return;
		}
		if (!*this || strlen((*this)->name) < strlen(name)) {
			as6502_symbol *next = *this;
			*this = sym;
			sym->next = next;
			return;
		}
	}
}

// Easy Symbol Table Access

void as6502_addSymbolForLine(as6502_symbol_table *table, const char *line, unsigned long lineNumber, uint16_t offset) {
	as6502_symbol_type type;
	size_t len = strlen(line) + 1;
	char *symbol = malloc(len);
	strncpy(symbol, line, len);
	trimgreedytaild(symbol); // symbol will only be the first phrase
	trimtailchard(symbol, ':'); // If there is a colon, truncate there
	
	if (strchr(line, '=')) { // Variable
		/** TODO: @todo allocate variable addresses */
		type = as6502_symbol_type_variable;
	}
	else { // Label
		type = as6502_symbol_type_label;
	}
	
	as6502_addSymbolToTable(table, lineNumber, symbol, offset, type);
	
	free(symbol);
}

void as6502_replaceSymbolInLineAtLocationWithText(char *line, size_t len, char *loc, char *symbol, const char *text) {
	size_t symLen = strlen(symbol);
	size_t txtLen = strlen(text);
	long difference = txtLen - symLen;
	
	if (difference < 0) { // Shift string left
		for (char *cur = loc + txtLen; cur < line + len; cur++) {
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
			as6502_fatal(kDesymbolicationErrorText);
		}
	}
	
	// Strings are aligned, overwrite
	memcpy(loc, text, txtLen);
}

static int as6502_doubleWidthForSymbolInLine(as6502_symbol_table *table, char *line, size_t len, char *symbol) {
	char *trimmed = trimhead(line, len);
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
		
	// Shift offset for pre-branch program counter shift
	v6502_address_mode mode = as6502_addressModeForLine(line, len);
	offset += as6502_instructionLengthForAddressMode(mode);
	
	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		// Search for symbol
		if (caseSensitive) {
			cur = strstr(line, this->name);
		}
		else {
			cur = strcasestr(line, this->name);
		}

		// Swap in address
		if (cur) {
			if (!isspace(cur[-1])) {
				// Partial symbol match @todo Double check this logic
				continue;
			}
			
			if (this->type == as6502_symbol_type_variable) {
				offset = 0;
				pstart = 0;
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
