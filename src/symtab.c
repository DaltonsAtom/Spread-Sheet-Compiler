/*
 * --- Symbol Table Implementation ---
 *
 * Implements a hash table for cell storage.
 *
 * FIX:
 * 1. Added symtab_print() function.
 * 2. Updated symtab_check_circular_dep to use struct.
 */

#include "symtab.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --- Hash Function --- */
// djb2 hash algorithm
static unsigned long hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

/* --- Private: Find/Resize --- */
static CellEntry* find_entry(CellEntry* entries, int capacity, const char* key) {
    unsigned long hash = hash_string(key);
    int index = (int)(hash & (capacity - 1));
    
    for (;;) {
        CellEntry* entry = &entries[index];
        if (entry->key == NULL) {
            // Found an empty slot
            return entry;
        } else if (strcmp(key, entry->key) == 0) {
            // Found the key
            return entry;
        }
        
        index = (index + 1) & (capacity - 1); // Linear probe
    }
}

static void resize_table(SymbolTable* table) {
    int old_capacity = table->capacity;
    CellEntry* old_entries = table->entries;
    
    table->capacity = old_capacity < 8 ? 8 : old_capacity * 2;
    table->entries = (CellEntry*)calloc(table->capacity, sizeof(CellEntry));
    table->count = 0; // Will be recounted
    
    // Re-hash all old entries
    for (int i = 0; i < old_capacity; i++) {
        if (old_entries[i].key != NULL) {
            CellEntry* dest = find_entry(table->entries, table->capacity, old_entries[i].key);
            // Just move the data
            *dest = old_entries[i];
            table->count++;
        }
    }
    
    free(old_entries);
}


/* --- Public API --- */

SymbolTable* symtab_create() {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
    resize_table(table); // Initialize
    return table;
}

void symtab_free(SymbolTable* table) {
    for (int i = 0; i < table->capacity; i++) {
        if (table->entries[i].key != NULL) {
            // Free heap-allocated data
            free(table->entries[i].key);
            free(table->entries[i].formula_str);
            free(table->entries[i].dependencies);
        }
    }
    free(table->entries);
    free(table);
}

CellEntry* symtab_get_cell(SymbolTable* table, const char* key) {
    if (table->count == 0) return NULL;
    
    unsigned long hash = hash_string(key);
    int index = (int)(hash & (table->capacity - 1));
    
    for (;;) {
        CellEntry* entry = &table->entries[index];
        if (entry->key == NULL) {
            // Reached an empty slot, key is not present
            return NULL;
        }
        if (strcmp(key, entry->key) == 0) {
            return entry; // Found it
        }
        index = (index + 1) & (table->capacity - 1);
    }
}

void symtab_define_cell(SymbolTable* table, const char* key, double value, const char* formula, int line) {
    if (table->count + 1 > table->capacity * SYMTAB_LOAD_FACTOR) {
        resize_table(table);
    }
    
    CellEntry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) {
        // New entry
        table->count++;
        entry->key = strdup(key);
    } else {
        // Overwriting, free old formula
        free(entry->formula_str);
    }
    
    entry->value = value;
    entry->formula_str = (formula != NULL) ? strdup(formula) : NULL;
    entry->line = line;
    entry->is_defined = 1;
    // Note: Dependencies are managed by the semantic analyzer
}

void symtab_add_dependency(SymbolTable* table, const char* this_cell_key, const char* depends_on_key) {
    CellEntry* entry = symtab_get_cell(table, this_cell_key);
    if (entry == NULL) {
        // This shouldn't happen if semantic analysis is correct
        symtab_define_cell(table, this_cell_key, 0, NULL, 0);
        entry = symtab_get_cell(table, this_cell_key);
    }
    
    // Check if dependency already exists
    for (int i = 0; i < entry->dep_count; i++) {
        if (strcmp(entry->dependencies[i], depends_on_key) == 0) {
            return; // Already in list
        }
    }
    
    // Add new dependency
    entry->dep_count++;
    entry->dependencies = (char**)realloc(entry->dependencies, entry->dep_count * sizeof(char*));
    entry->dependencies[entry->dep_count - 1] = strdup(depends_on_key);
}

// Recursive helper for circular dependency check
static int check_dep_recursive(SymbolTable* table, const char* this_cell_key, const char* check_cell_key, ErrorSystem* errors) {
    if (strcmp(this_cell_key, check_cell_key) == 0) {
        return 1; // Found a circle!
    }
    
    CellEntry* check_cell = symtab_get_cell(table, check_cell_key);
    if (check_cell == NULL || check_cell->dep_count == 0) {
        return 0; // No dependencies, no circle
    }
    
    for (int i = 0; i < check_cell->dep_count; i++) {
        if (check_dep_recursive(table, this_cell_key, check_cell->dependencies[i], errors)) {
            // Log the dependency path as we unwind
            char msg[128];
            snprintf(msg, 128, " -> %s", check_cell_key);
            error_report_message(errors, msg);
            return 1;
        }
    }
    return 0;
}

int symtab_check_circular_dep(SymbolTable* table, const char* this_cell_key, const char* check_cell_key, ErrorSystem* errors) {
    if (check_dep_recursive(table, this_cell_key, check_cell_key, errors)) {
        // We found a circle, now create the full error message
        char final_msg[512];
        snprintf(final_msg, 512, "Circular dependency detected: %s -> %s", this_cell_key, check_cell_key);
        
        // Append the rest of the path from the error system
        strncat(final_msg, error_get_messages(errors), 512 - strlen(final_msg) - 1);
        error_clear_messages(errors); // Clear partial messages
        
        // Report the final, complete error
        error_report(errors, ERROR_SEMANTIC, 0, 0, final_msg, "Remove the dependency.");
        return 1;
    }
    return 0;
}

/**
 * @brief Prints the symbol table in a formatted way.
 */
void symtab_print(SymbolTable* table) {
    printf("Cell | Value   | Status\n");
    printf("-----|---------|----------\n");
    
    for (int i = 0; i < table->capacity; i++) {
        CellEntry* entry = &table->entries[i];
        if (entry->key != NULL) {
            // Only print defined cells for the clean output
            if(entry->is_defined && entry->formula_str != NULL) {
                printf("%-4s | %-7.2f | %s\n",
                    entry->key,
                    entry->value,
                    "DEFINED");
            }
        }
    }
}

