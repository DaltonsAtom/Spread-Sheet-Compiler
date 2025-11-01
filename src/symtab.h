/*
 * --- Symbol Table Header (Prompt 4.1) ---
 *
 * Defines the structures and functions for a hash table
 * to store cell values and metadata.
 *
 * FIX:
 * 1. Added symtab_print() declaration.
 * 2. Included error.h to get ErrorSystem type.
 * 3. Fixed prototype for symtab_check_circular_dep.
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include "error.h" // For ErrorSystem

#define SYMTAB_LOAD_FACTOR 0.75

/*
 * Represents a single cell (e.g., A1) in the table.
 */
typedef struct {
    char* key;          // The cell reference (e.g., "A1")
    char* formula_str;  // The raw formula string (e.g., "=B1+C1")
    double value;       // The last calculated numeric value
    int is_defined;     // 1 if this cell has a value/formula, 0 otherwise
    int line;           // Line number where this was defined (in cells file)
    
    // For dependency tracking (Prompt 4.2)
    char** dependencies; // Array of cell keys this cell depends on
    int dep_count;      // Number of dependencies
} CellEntry;


/*
 * The main symbol table structure (a hash table).
 */
typedef struct {
    int count;      // Number of entries
    int capacity;   // Size of the entries array
    CellEntry* entries; // The hash table array
} SymbolTable;


/* --- Public API --- */

/**
 * @brief Creates a new, empty symbol table.
 */
SymbolTable* symtab_create();

/**
 * @brief Frees all memory associated with the symbol table.
 */
void symtab_free(SymbolTable* table);

/**
 * @brief Gets a pointer to a cell entry by its key (e.g., "A1").
 * Returns NULL if the key is not found.
 */
CellEntry* symtab_get_cell(SymbolTable* table, const char* key);

/**
 * @brief Defines or updates a cell's value in the table.
 */
void symtab_define_cell(SymbolTable* table, const char* key, double value, const char* formula, int line);

/**
 * @brief Records that 'this_cell_key' depends on 'depends_on_key'.
 */
void symtab_add_dependency(SymbolTable* table, const char* this_cell_key, const char* depends_on_key);

/**
 * @brief Checks if evaluating 'this_cell_key' would cause a circular
 * dependency by following the chain to 'check_cell_key'.
 *
 * FIX: Changed 'struct ErrorSystem*' to 'ErrorSystem*'
 */
int symtab_check_circular_dep(SymbolTable* table, const char* this_cell_key, const char* check_cell_key, ErrorSystem* errors);

/**
 * @brief Prints the contents of the symbol table.
 */
void symtab_print(SymbolTable* table);


#endif // SYMTAB_H

