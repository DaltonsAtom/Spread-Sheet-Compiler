/*
 * --- Runtime Library Header (Prompt 6.3) ---
 *
 * Declares the built-in functions that can be called
 * by the Interpreter or the VM.
 *
 * FIX: Removed declarations for get_numeric and is_truthy,
 * as they are now provided by value.h.
 */

#ifndef RUNTIME_H
#define RUNTIME_H

#include "value.h"      // Provides Value struct and helpers
#include "symtab.h"     // For expanding ranges

/* --- Argument List Struct --- */
/*
 * A simple linked list of values, used to pass
 * evaluated arguments to runtime functions.
 */
typedef struct ValueNode {
    Value value;
    struct ValueNode* next;
} ValueNode;

/* --- Built-in Function Implementations --- */

Value rt_sum(ValueNode* args);
Value rt_average(ValueNode* args);
Value rt_min(ValueNode* args);
Value rt_max(ValueNode* args);
Value rt_not(ValueNode* args);
// Note: IF, AND, OR are handled by interpreter/VM logic
// for lazy evaluation.

/* --- Helper Functions --- */

/**
 * @brief Expands a range string (e.g., "A1:B10") into a list
 * of ValueNodes by looking up each cell in the symbol table.
 */
ValueNode* rt_expand_range(const char* range_str, SymbolTable* table);

/**
 * @brief Frees an entire ValueNode list.
 */
void free_value_list(ValueNode* head);


#endif // RUNTIME_H

