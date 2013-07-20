//
/** @brief Symbol table management */
/** @file symbols.h */
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef as6502_symbols_h
#define as6502_symbols_h

#include <stdint.h>

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
void as6502_addSymbolForLine(as6502_symbol_table *table, const char *line, unsigned long lineNumber, uint16_t offset);
/** @brief Automatically detects symbols in a given line, then dereferences them and replaces them with their actual addresses */
void as6502_desymbolicateLine(as6502_symbol_table *table, char *line, size_t len, uint16_t pstart, uint16_t offset, int caseSensitive);
/**@}*/

/** @defgroup sym_rep Symbol Replacement */
/**@{*/
/** @brief Convenience function to replace a given string in another string, with its location already specified for performance */
void as6502_replaceSymbolInLineAtLocationWithText(char *line, size_t len, char *loc, char *symbol, const char *text);
/**@}*/

#endif
