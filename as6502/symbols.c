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

#define v6502_DesymbolicationErrorText	"Could not shift string far enough while desymbolicating"
#define v6502_DuplicateSymbolErrorText	"Encountered duplicate symbol declaration '%s'"
#define v6502_DuplicateSymbolNoteText	"Previous declaration was here"

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
			as6502_error(v6502_DuplicateSymbolErrorText, name);
			as6502_note((*this)->line, v6502_DuplicateSymbolNoteText);
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

as6502_symbol_type as6502_addSymbolForLine(as6502_symbol_table *table, const char *line, unsigned long lineNumber, uint16_t offset, uint16_t varLocation) {
	as6502_symbol_type type;
	size_t len = strlen(line) + 1;
	char *symbol_true = malloc(len);
	strncpy(symbol_true, line, len);
	char *symbol = symbol_true;
	int isByte = NO;
	
	if (!strncasecmp(".byte", symbol, 5)) {
		// .byte will only be the last phrase
		symbol = trimheadtospc(symbol, len);
		symbol = trimhead(symbol, len);
		isByte = YES;
	}
	else  {
		// Labels will only be the first phrase
		trimgreedytaild(symbol);
	}
	
	trimtaild(symbol);
	trimtailchard(symbol, ':'); // If there is a colon, truncate there
	
	if (strchr(line, '=') || isByte) { // Variable
		/** TODO: @todo allocate variable addresses */
		type = as6502_symbol_type_variable;
		offset = varLocation;
	}
	else { // Label
		type = as6502_symbol_type_label;
	}
	
	as6502_addSymbolToTable(table, lineNumber, symbol, offset, type);
	
	free(symbol_true);
	
	return type;
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
			as6502_fatal(v6502_DesymbolicationErrorText);
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
			if (!isspace(CTYPE_CAST cur[-1])) {
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

int _symbolTypeIsAppropriateForInstruction(as6502_symbol_type type, char *line, size_t len) {
	line = trimhead(line, len);
	
	if (line[0] == 'b' || line[0] == 'B') {
		if (!asmeq(line, "bit")) {
			return (type == as6502_symbol_type_label);
		}
	}
	
	if (line[0] == 'j' || line[0] == 'J') {
		return (type == as6502_symbol_type_label);
	}
	
	// TODO: Variables
	
	return NO;
}

void as6502_symbolicateLine(as6502_symbol_table *table, char *line, size_t len, uint16_t pstart, uint16_t offset) {
	// Iterate through tokens
	for (char *cur = line; *cur && ((size_t)(cur - line) < len); cur = (trimheadtospc(cur, len - (cur - line)) + 1)) {
		
		int wide;
		uint16_t address = as6502_valueForString(&wide, cur);
		as6502_symbol *symbol = as6502_symbolForAddress(table, address);
		
		// If we couldn't find one at that address, try a pstart offset symbol.
		if (!symbol && wide) {
			symbol = as6502_symbolForAddress(table, address - pstart);
		}
		
		if (symbol && _symbolTypeIsAppropriateForInstruction(symbol->type, line, len)) {
			char buf[80];
			
			int x;
			for (x = 0; cur[x] && !isspace(CTYPE_CAST cur[x]); x++) {
				buf[x] = cur[x];
			}
			buf[x + 1] = '\0';
			
			as6502_replaceSymbolInLineAtLocationWithText(line, len, cur, buf, symbol->name);
		}
	}
}
