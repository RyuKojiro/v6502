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

#include <as6502/token.h>

/** @defgroup sym_type_macros Symbol Type Test Macros */
/**@{*/
/** @brief Return YES if a given as6502_symbol_type has a low link bit */
#define as6502_symbolTypeIsLinked(_type)		(_type == as6502_symbol_type_label || _type == as6502_symbol_type_variable)
/** @brief Return YES if a given as6502_symbol_type is a label type, regardless of linkage */
#define as6502_symbolTypeIsLabel(_type)			(_type == as6502_symbol_type_label || _type == as6502_symbol_type_label_unlinked)
/** @brief Return YES if a given as6502_symbol_type is a variable type, regardless of linkage */
#define as6502_symbolTypeIsVariable(_type)		(_type == as6502_symbol_type_variable || _type == as6502_symbol_type_variable_unlinked)
/**@}*/

/** @enum */
/** @brief as6502_symbol Type */
/** <B>If you need to test type or linkage, use the provided @ref sym_type_macros.</B> If for some reason you need to do more advanced manipulation of the type field: Label types are odd, variable types are even, so symbol type (regardless of linkage) can be tested with a simple or mask against the unlinked type that you want. For example: @code (type | as6502_symbol_type_variable_unlinked) == as6502_symbol_type_variable_unlinked @endcode Linkage can be tested by masking against the linkage you want, always using label, as it carries a type bit of zero. */
typedef enum {
	as6502_symbol_type_unknown = 0,
	as6502_symbol_type_label = 2,
	as6502_symbol_type_variable = 3,
	as6502_symbol_type_label_unlinked = 4,
	as6502_symbol_type_variable_unlinked = 5
} as6502_symbol_type;

/** @struct */
/** @brief An individual symbol in a symbol table */
typedef struct /** @cond STRUCT_FORWARD_DECLS */ _as6502_symbol /** @endcond */ {
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
	/** @brief The head of the linked list of labels */
	as6502_symbol *first_symbol;
} as6502_symbol_table;

/** @defgroup sym_lifecycle Symbol Table Lifecycle Functions */
/**@{*/
/** @brief Creates a new symbol table object */
as6502_symbol_table *as6502_createSymbolTable(void);
/** @brief Destroys a symbol table object */
void as6502_destroySymbolTable(as6502_symbol_table *table);
/** @brief Prints a human readable representation of a as6502_symbol_table for debugging */
void as6502_printSymbolTable(as6502_symbol_table *table);
/** @brief Generates a debugger script containing all of the commands necessary for loading the symbol table generated during assembly into the debugger */
void as6502_printSymbolScript(as6502_symbol_table *table, FILE *out);
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
/** @brief Removes a as6502_symbol from a as6502_symbol_table */
void as6502_removeSymbolFromTable(as6502_symbol_table *table, as6502_symbol *symbol);
/** @brief Remove all as6502_symbol's that lie outside the address region specified. */
void as6502_truncateTableToAddressSpace(as6502_symbol_table *table, uint16_t start, uint16_t len);
/**@}*/

/** @defgroup sym_ez Easy Symbol Table Interaction */
/**@{*/

/** @brief Automatically detects symbols in a given line, then dereferences them and replaces them with their actual addresses
	@param table The as6502_symbol_table to search
	@param head The as6502_token list to desymbolicate
	@param offset Address of current line
	@param caseSensitive Symbol search case sensitivity (This argument is currently ignored, and it's always case sensitive)
	@return A freshly malloc'ed, null terminated, string of length outLen, containing the desymbolicated line
 */
as6502_token *as6502_desymbolicateExpression(as6502_symbol_table *table, as6502_token *head, uint16_t offset, int caseSensitive);

/** @brief Searches for addresses in a given line and replaces them with their symbols in a given symbol table
	@param table The as6502_symbol_table to search
	@param line The string to desymbolicate
	@param len Useable length of the line in chars
	@param offset Address of current line
 */
void as6502_symbolicateLine(as6502_symbol_table *table, char *line, size_t len, uint16_t offset);
/**@}*/

/** @defgroup sym_rep Symbol Replacement */
/**@{*/
/** @brief Convenience function to replace a given string in another string, with its location already specified for performance */
void as6502_replaceSymbolInLineAtLocationWithText(char *line, size_t len, char *loc, const char * restrict symbol, const char * restrict text);
/** @brief This indicates if a symbol following an instruction should be replaced with a relative or absolute address */
int as6502_symbolShouldBeReplacedDoubleWidth(as6502_token *instruction);
/**@}*/

#endif
