/** @brief Symbol table management */
/** @file symbols.h */

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

#ifndef as6502_symbols_h
#define as6502_symbols_h

#include <stdint.h>

/** @enum */
/** @brief as6502_symbol Type */
typedef enum {
	as6502_symbol_type_unknown = 0,
	as6502_symbol_type_label,
	as6502_symbol_type_variable
} as6502_symbol_type;

/** @struct */
/** @brief An individual symbol in a symbol table */
typedef struct _as6502_symbol {
	/** @brief The next variable in the linked list */
	struct _as6502_symbol *next;
	/** @brief The line number of the symbol in the source file */
	unsigned long line;
	/** @brief The symbol name */
	char *name;
	/** @brief The symbol's address */
	uint16_t address;
	/** @brief The symbol type */
	as6502_symbol_type type;
} as6502_symbol;

/** @struct */
/** @brief The assembler's per-object symbol table structure, which holds all symbols. */
typedef struct {
	/** @brief The number of symbols in the symbol table */
	long symbolCount;
	/** @brief The head of the linked list of labels */
	as6502_symbol *first_symbol;
} as6502_symbol_table;

/** @defgroup sym_lifecycle Symbol Table Lifecycle Functions */
/**@{*/
/** @brief Creates a new symbol table object */
as6502_symbol_table *as6502_createSymbolTable();
/** @brief Destroys a symbol table object */
void as6502_destroySymbolTable(as6502_symbol_table *table);
/** @brief Prints a human readable representation of a as6502_symbol_table for debugging */
void as6502_printSymbolTable(as6502_symbol_table *table);
/**@}*/

/** @defgroup sym_access Symbol Table Accessors */
/**@{*/
/** @brief Finds a as6502_symbol in a given as6502_symbol_table by name */
as6502_symbol *as6502_symbolForString(as6502_symbol_table *table, const char *name);
/** @brief Finds a as6502_symbol in a given as6502_symbol_table by address */
as6502_symbol *as6502_symbolForAddress(as6502_symbol_table *table, uint16_t address);
/** @brief Dereferences a label by name to retrieve its address */
uint16_t as6502_addressForLabel(as6502_symbol_table *table, const char *name);
/** @brief Dereferences a variable by name to retrieve its address */
uint16_t as6502_addressForVar(as6502_symbol_table *table, const char *name);
/** @brief Creates and adds a as6502_symbol to a as6502_symbol_table */
void as6502_addSymbolToTable(as6502_symbol_table *table, unsigned long line, const char *name, uint16_t address, as6502_symbol_type type);
/**@}*/

/** @defgroup sym_ez Easy Symbol Table Interaction */
/**@{*/
/** @brief Automatically detects symbols in a given line, then creates and inserts them into a given as6502_symbol_table */
as6502_symbol_type as6502_addSymbolForLine(as6502_symbol_table *table, const char *line, unsigned long lineNumber, uint16_t offset, uint16_t varLocation);

/** @brief Automatically detects symbols in a given line, then dereferences them and replaces them with their actual addresses
	@param table The as6502_symbol_table to search
	@param line The string to desymbolicate
	@param len Useable length of the line in chars
	@param pstart Program start address
	@param offset Address of current line
	@param caseSensitive Symbol search case sensitivity
 */
void as6502_desymbolicateLine(as6502_symbol_table *table, char *line, size_t len, uint16_t pstart, uint16_t offset, int caseSensitive);

/** @brief Searches for addresses in a given line and replaces them with their symbols in a given symbol table
	@param table The as6502_symbol_table to search
	@param line The string to desymbolicate
	@param len Useable length of the line in chars
	@param pstart Program start address
	@param offset Address of current line
 */
void as6502_symbolicateLine(as6502_symbol_table *table, char *line, size_t len, uint16_t pstart, uint16_t offset);
/**@}*/

/** @defgroup sym_rep Symbol Replacement */
/**@{*/
/** @brief Convenience function to replace a given string in another string, with its location already specified for performance */
void as6502_replaceSymbolInLineAtLocationWithText(char *line, size_t len, char *loc, char *symbol, const char *text);
/**@}*/

#endif
