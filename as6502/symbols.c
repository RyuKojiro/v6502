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
#include <assert.h>

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
	if(!table) {
		return;
	}
	
	as6502_symbol *next;
	for (as6502_symbol *this = table->first_symbol; this; this = next) {
		next = this->next;
		free(this->name);
		free(this);
	}
	
	free(table);
}

void as6502_printSymbolTable(as6502_symbol_table *table) {
	if(!table) {
		printf("Symbol table is NULL\n");
		return;
	}
	
	printf("Symbol table %p = {\n", table);
	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		char *type;
		
		switch (this->type) {
			case as6502_symbol_type_unknown: {
				type = "???";
			} break;
			case as6502_symbol_type_label_unlinked:
			case as6502_symbol_type_label: {
				type = "Label";
			} break;
			case as6502_symbol_type_variable_unlinked:
			case as6502_symbol_type_variable: {
				type = "Var";
			} break;
		}
		
		if (as6502_symbolTypeIsLinked(this->type)) {
			printf("\t%s { name = \"%s\", addr = 0x%x, next = %p, line = %lu }\n", type, this->name, this->address, this->next, this->line);
		}
		else {
			printf("\t%s { name = \"%s\", not linked!, next = %p, line = %lu }\n", type, this->name, this->next, this->line);
		}
	}
	printf("}\n");
}

// Symbol Table Accessors
as6502_symbol *as6502_symbolForString(as6502_symbol_table *table, const char *name) {
	assert(table);

	size_t len = strlen(name);
	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		if (strncmp(this->name, name, len)) {
			return this;
		}
	}
	
	return NULL;
}

as6502_symbol *as6502_symbolForAddress(as6502_symbol_table *table, uint16_t address) {
	assert(table);

	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		if (this->address == address) {
			return this;
		}
	}
	
	return NULL;
}

uint16_t as6502_addressForLabel(as6502_symbol_table *table, const char *name) {
	assert(table);

	as6502_symbol *label = as6502_symbolForString(table, name);
	
	if (!label) {
		return 0;
	}
	
	return label->address;
}

uint16_t as6502_addressForVar(as6502_symbol_table *table, const char *name) {
	assert(table);

	as6502_symbol *var = as6502_symbolForString(table, name);
	
	if (!var) {
		return 0;
	}
	
	return var->address;
}

void as6502_addSymbolToTable(as6502_symbol_table *table, unsigned long line, const char *name, uint16_t address, as6502_symbol_type type) {
	assert(table);

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
			as6502_error(0, 0, v6502_DuplicateSymbolErrorText, name);
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

static void _removeConfirmedSymbol(as6502_symbol *symbol, as6502_symbol *previousSymbol) {
	// NOTE: Never call this with a NULL previousSymbol (i.e. the head)
	assert(previousSymbol);
	
	previousSymbol->next = symbol->next;
	free(symbol->name);
	free(symbol);
}

void as6502_removeSymbolFromTable(as6502_symbol_table *table, as6502_symbol *symbol) {
	assert(table);
	
	as6502_symbol *last = NULL;

	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		if (this == symbol) {
			if (!last) {
				table->first_symbol = this->next;
				free(this->name);
				free(this);
			}
			else {
				_removeConfirmedSymbol(this, last);
			}
			return;
		}
		last = this;
	}
}

void as6502_truncateTableToAddressSpace(as6502_symbol_table *table, uint16_t start, uint16_t len) {
	assert(table);
	
	as6502_symbol *last = NULL;
	
start_over:
	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		if ((this->address < start) || (this->address > start + len)) {
			if (!last) {
				table->first_symbol = this->next;
				free(this->name);
				free(this);
				goto start_over;
			}
			else {
				_removeConfirmedSymbol(this, last);
			}
		}
		last = this;
	}
}

// Easy Symbol Table Access

as6502_symbol_type as6502_addSymbolForLine(as6502_symbol_table *table, const char *line, unsigned long lineNumber, uint16_t offset, uint16_t varLocation) {
	assert(table);

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
	
	// TODO: implement variable declaration without equals signs
	if (strchr(line, '=')  || isByte) { // Variable
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
	assert(table);

	char *trimmed = trimhead(line, len);
	//len -= trimmed - line;
	
	if (((trimmed[0] == 'b' || trimmed[0] == 'B') && (strncasecmp(trimmed, "bit", 3) && strncasecmp(trimmed, "brk", 3))) || symbol[-1] == '(') {
		return 0;
	}

	return 1;
}

char *as6502_desymbolicateLine(as6502_symbol_table *table, const char *line, size_t len, uint16_t pstart, uint16_t offset, int caseSensitive, size_t *outLen) {
	/** FIXME: This needs to be smart about address formation, based on address mode
	 * This is absurdly inefficient, but works, given the current symbol table implementation
	 */
	assert(table);

	char *cur;
	int width;
	size_t last = 0;
	
	char *in = malloc(len + 1);
	char *out = NULL;
	size_t _outLen = 0;
	
	// Copy in string and ensure NULL termination for strstr
	strncpy(in, line, len + 1);
	
	// Shift offset for pre-branch program counter shift
	v6502_address_mode mode = as6502_addressModeForLine(in, len);
	offset += as6502_instructionLengthForAddressMode(mode);
	
	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		// Make sure searching is even possible
		if (!in[last]) {
			break;
		}
		if (strlen(this->name) > (len - last)) {
			continue;
		}
		
		// Search for symbol
		if (caseSensitive) {
			cur = strstr(in, this->name);
		}
		else {
			cur = strcasestr(in, this->name);
		}

		// Found a symbol! Swap in the address
		if (cur) {
			// Prevent partial symbol matches
			if (as6502_lengthOfToken(cur, len - (cur - line)) > strlen(this->name)) {
				continue;
			}
			if (!isspace(CTYPE_CAST cur[-1])) {
				continue;
			}

			// Copy up to the encountered symbol
			size_t lengthToCopy = cur - in - last;
			out = realloc(out, _outLen + lengthToCopy);
			strncpy(out + _outLen, in + last, lengthToCopy);
			last += lengthToCopy;
			_outLen += lengthToCopy;
			
			if (as6502_symbolTypeIsVariable(this->type)) {
				offset = 0;
				pstart = 0;
			}

			/** @todo FIXME: Should this detect if relative is an option, and do that instead? */
			width = as6502_doubleWidthForSymbolInLine(table, in, len, cur);
			if (width) {
				out = realloc(out, _outLen + 6);
				snprintf(out + _outLen, 6, "$%04x", pstart + this->address);
				_outLen += 5;
			}
			else {
				out = realloc(out, _outLen + 4);
				snprintf(out + _outLen, 4, "$%02x", v6502_byteValueOfSigned(this->address - offset));
				_outLen += 3;
			}
			
			last += strlen(this->name);
		}
	}
	
	// Empty anything that's left in the input buffer to the output
	if (last < len && in[last]) {
		size_t lengthToCopy = len - last;
		out = realloc(out, _outLen + lengthToCopy);
		lengthToCopy++; // Add one for guaranteed null termination, even if the in buffer isn't
		strncpy(out + _outLen, in + last, lengthToCopy);
		_outLen += lengthToCopy;
	}
	
	if (outLen) {
		*outLen = _outLen;
	}
	
	return out;
}

static int _symbolTypeIsAppropriateForInstruction(as6502_symbol_type type, char *line, size_t len) {
	line = trimhead(line, len);
	
	if (line[0] == 'b' || line[0] == 'B') {
		if (!asmeq(line, "bit")) {
			return as6502_symbolTypeIsLabel(type);
		}
	}
	
	if (line[0] == 'j' || line[0] == 'J') {
		return as6502_symbolTypeIsLabel(type);
	}
	
	// TODO: Variables
	
	return NO;
}

void as6502_symbolicateLine(as6502_symbol_table *table, char *line, size_t len, uint16_t pstart, uint16_t offset) {
	assert(table);

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
