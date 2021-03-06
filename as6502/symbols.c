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

#include <v6502/mem.h>
#include <v6502/cpu.h>

#include "symbols.h"
#include "error.h"
#include "linectl.h"
#include "parser.h"

#define v6502_DesymbolicationErrorText	"Could not shift string far enough while desymbolicating"
#define v6502_DuplicateSymbolErrorText	"Encountered duplicate symbol declaration '%s'"
#define v6502_DuplicateSymbolNoteText	"Previous declaration was here"

#define MAX_ADDRESS_TEXT_LEN			6

// Address Table Lifecycle Functions
as6502_symbol_table *as6502_createSymbolTable() {
	return calloc(1, sizeof(as6502_symbol_table));
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

void as6502_printSymbolScript(as6502_symbol_table *table, FILE *out) {
	if(!table) {
		fprintf(stderr, "Symbol table is NULL\n");
		return;
	}

	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		switch (this->type) {
			case as6502_symbol_type_label: {
				fprintf(out, "label %s %#x\n", this->name, this->address);
			} break;
			case as6502_symbol_type_variable: {
				fprintf(out, "var %s %#x\n", this->name, this->address);
			} break;
			default: break;
		}
	}
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
			default:
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
			printf("\t%s { name = \"%s\", addr = %#x, next = %p, line = %lu }\n", type, this->name, this->address, this->next, this->line);
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

	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		if (!strcmp(this->name, name)) {
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

uint16_t as6502_addressForSymbolByName(as6502_symbol_table *table, const char *name) {
	assert(table);

	as6502_symbol *label = as6502_symbolForString(table, name);

	if (!label) {
		return 0;
	}

	return label->address;
}

void as6502_addSymbolToTable(as6502_symbol_table *table, unsigned long line, const char *name, uint16_t address, as6502_symbol_type type) {
	assert(table);

	// Ensure the table doesn't already contain this symbol
	as6502_symbol *found = as6502_symbolForString(table, name);
	if (found && found->address != address) {
		as6502_error(0, 0, v6502_DuplicateSymbolErrorText, name);
		as6502_note(found->line, v6502_DuplicateSymbolNoteText);
		return;
	}

	// Make the symol object
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
	strncpy(sym->name, name, len);

	// Insert it into the table, in descending order of length
	for (as6502_symbol **this = &table->first_symbol;; this = &((*this)->next)) {
		if (!*this || strlen((*this)->name) < strlen(name)) {
			as6502_symbol *next = *this;
			*this = sym;
			sym->next = next;
			return;
		}
	}
}

static void _removeConfirmedSymbol(as6502_symbol_table *table, as6502_symbol *symbol, as6502_symbol *previousSymbol) {
	if (previousSymbol) {
		previousSymbol->next = symbol->next;
		free(symbol->name);
		free(symbol);
	}
	else {
		table->first_symbol = symbol->next;
		free(symbol->name);
		free(symbol);
	}
}

void as6502_removeSymbolFromTable(as6502_symbol_table *table, as6502_symbol *symbol) {
	assert(table);

	as6502_symbol *last = NULL;

	for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
		if (this == symbol) {
			_removeConfirmedSymbol(table, this, last);
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
			_removeConfirmedSymbol(table, this, last);

			if (!last) {
				goto start_over;
			}
			else {
				this = last;
			}
		}
		last = this;
	}
}

// Easy Symbol Table Access

void as6502_replaceSymbolInLineAtLocationWithText(char *line, size_t len, char *loc, const char * restrict symbol, const char * restrict text) {
	size_t symLen = strlen(symbol);
	size_t txtLen = strlen(text);
	long difference = txtLen - symLen;

	if (difference < 0) { // Shift string left
		for (char *cur = loc + txtLen; cur < line + len && cur < loc + symLen; cur++) {
			cur[0] = cur[0 - difference];
		}
	}

	if (difference > 0) { // Shift string right
		if (strlen(line) + difference < len) {
			char *start = strstr(line, symbol);
			assert(start);
			for (char *cur = start + txtLen + difference; cur > start; cur--) {
				cur[0] = cur[0 - difference];
			}
		}
		else {
			as6502_fatal(v6502_DesymbolicationErrorText);
		}
	}

	// Strings are aligned, overwrite
	memcpy(loc, text, txtLen);
}

int as6502_symbolShouldBeReplacedDoubleWidth(as6502_token *instruction) {
	const char *trimmed = instruction->text;
	//len -= trimmed - line;

	if (((trimmed[0] == 'b' || trimmed[0] == 'B') && (strncasecmp(trimmed, "bit", 3) && strncasecmp(trimmed, "brk", 3))) || as6502_tokenListFindTokenLiteral(instruction, ")")) {
		return 0;
	}

	return 1;
}

as6502_token *as6502_desymbolicateExpression(as6502_symbol_table *table, as6502_token *head, uint16_t offset, int caseSensitive) {
	/** FIXME: This needs to be smart about address formation, based on address mode
	 * This is absurdly inefficient, but works, given the current symbol table implementation
	 */
	assert(table);

	// Shift offset for pre-branch program counter shift
	v6502_address_mode mode = as6502_addressModeForExpression(head);
	offset += as6502_instructionLengthForAddressMode(mode);

	// Decide whether use relative or absolute when swapping in addresses
	int width = as6502_instructionLengthForAddressMode(mode) - 2;

	as6502_token *newHead = NULL;
	as6502_token *newTail = NULL;
	while (head) {
		as6502_token *cur = NULL;

		// Find it in the symbol table and create a new token with it's address, or just copy it into the new token
		for (as6502_symbol *this = table->first_symbol; this; this = this->next) {
			if (!strncmp(this->name, head->text, head->len + 1)) {
				char address[MAX_ADDRESS_TEXT_LEN];

				if (width) {
					/* If the variable falls in zeropage, make it a zeropage call.
					 * All instructions that aren't either implied-only or
					 * relative-only, have zero page modes for all possible register
					 * combinations.
					 */
					if (this->address <= 0xFF) {
						snprintf(address, MAX_ADDRESS_TEXT_LEN, "*$%02x", this->address);
					}
					else {
						snprintf(address, MAX_ADDRESS_TEXT_LEN, "$%04x", this->address);
					}
				}
				else {
					if (mode == v6502_address_mode_relative) {
						snprintf(address, MAX_ADDRESS_TEXT_LEN, "$%02x", v6502_byteValueOfSigned(this->address - offset));
					}
					else {
						snprintf(address, MAX_ADDRESS_TEXT_LEN, "$%02x", v6502_byteValueOfSigned(this->address));
					}
				}

				cur = as6502_tokenCreate(address, head->loc, strlen(address));
				cur->type = as6502_token_type_value;
				break;
			}
		}

		if (!cur) {
			cur = as6502_tokenCopy(head);
		}

		// Insert into new token list
		if (!newHead) {
			newHead = cur;
		}
		else {
			newTail->next = cur;
		}
		newTail = cur;

		head = head->next;
	}

	return newHead;
}

static int _symbolTypeIsAppropriateForInstruction(as6502_symbol_type type, char *line, size_t len) {
	line = trimhead(line, len);

	if (line[0] == 'b' || line[0] == 'B') {
		if (strncasecmp(line, "bit", 3)) {
			return as6502_symbolTypeIsLabel(type);
		}
	}

	if (line[0] == 'j' || line[0] == 'J') {
		return as6502_symbolTypeIsLabel(type);
	}

	// TODO: Variables

	return NO;
}

// TODO: Make this work on token lists, and build a token list when disassembling
// TOOD: Add relative jump symbolication
void as6502_symbolicateLine(as6502_symbol_table *table, char *line, size_t len, uint16_t offset) {
	assert(table);

	// Iterate through tokens
	for (char *cur = line; *cur && ((size_t)(cur - line) < len); cur = (trimheadtospc(cur, len - (cur - line)) + 1)) {

		int wide;
		uint16_t address = as6502_valueForString(&wide, cur, len - (cur - line));
		as6502_symbol *symbol = as6502_symbolForAddress(table, address);

		// If we couldn't find one at that address, try a pstart offset symbol.
		if (!symbol && wide) {
			symbol = as6502_symbolForAddress(table, address);
		}

		if (symbol && _symbolTypeIsAppropriateForInstruction(symbol->type, line, len)) {
			char buf[80];

			int x;
			for (x = 0; cur[x] && !isspace(CTYPE_CAST cur[x]); x++) {
				buf[x] = cur[x];
			}
			buf[x] = '\0';

			as6502_replaceSymbolInLineAtLocationWithText(line, len, cur, buf, symbol->name);
		}
	}
}
