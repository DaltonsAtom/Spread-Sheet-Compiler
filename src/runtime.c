/*
 * --- Runtime Library Implementation (Prompt 6.3) ---
 *
 * Implements the logic for built-in functions
 * like SUM, AVERAGE, MIN, MAX.
 *
 * FIX: Removed local definitions of get_numeric and is_truthy,
 * as they are now provided by value.h.
 */

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // For fmin, fmax


/* --- Private Helpers --- */

// Helper for SUM, AVG
static void accumulate(ValueNode* args, double* sum, int* count) {
    *sum = 0;
    *count = 0;
    ValueNode* current = args;
    while (current != NULL) {
        // Only accumulate numeric types
        if (current->value.type == TYPE_NUMBER) {
            *sum += current->value.as.number;
            (*count)++;
        }
        current = current->next;
    }
}

/* --- Public Functions --- */

Value rt_sum(ValueNode* args) {
    double sum;
    int count;
    accumulate(args, &sum, &count);
    return create_number_value(sum);
}

Value rt_average(ValueNode* args) {
    double sum;
    int count;
    accumulate(args, &sum, &count);
    
    if (count == 0) {
        return create_error_value("AVERAGE divide by zero (no numeric args)");
    }
    return create_number_value(sum / count);
}

Value rt_min(ValueNode* args) {
    if (args == NULL) {
        return create_number_value(0.0); // Excel returns 0 for MIN()
    }
    
    double min_val = INFINITY;
    int numeric_found = 0;
    
    ValueNode* current = args;
    while (current != NULL) {
        if (current->value.type == TYPE_NUMBER) {
            if (!numeric_found) {
                min_val = current->value.as.number;
                numeric_found = 1;
            } else {
                min_val = fmin(min_val, current->value.as.number);
            }
        }
        current = current->next;
    }
    
    return create_number_value(numeric_found ? min_val : 0.0);
}

Value rt_max(ValueNode* args) {
    if (args == NULL) {
        return create_number_value(0.0); // Excel returns 0 for MAX()
    }
    
    double max_val = -INFINITY;
    int numeric_found = 0;
    
    ValueNode* current = args;
    while (current != NULL) {
        if (current->value.type == TYPE_NUMBER) {
            if (!numeric_found) {
                max_val = current->value.as.number;
                numeric_found = 1;
            } else {
                max_val = fmax(max_val, current->value.as.number);
            }
        }
        current = current->next;
    }
    
    return create_number_value(numeric_found ? max_val : 0.0);
}

Value rt_not(ValueNode* args) {
    if (args == NULL || args->next != NULL) {
        return create_error_value("NOT expects exactly 1 argument");
    }
    return create_boolean_value(!is_truthy(args->value));
}


/* --- Helper Implementations --- */

void free_value_list(ValueNode* head) {
    while (head != NULL) {
        ValueNode* temp = head;
        head = head->next;
        free_value(temp->value);
        free(temp);
    }
}

ValueNode* rt_expand_range(const char* range_str, SymbolTable* table) {
    char col_start, col_end;
    int row_start, row_end;
    
    if (sscanf(range_str, "%c%d:%c%d", &col_start, &row_start, &col_end, &row_end) != 4) {
        // Not a valid range, treat as a single string.
        return NULL;
    }
    
    ValueNode* head = NULL;
    ValueNode* tail = NULL;
    
    // Loop from A1 to B10 (e.g.)
    for (char c = col_start; c <= col_end; c++) {
        for (int r = row_start; r <= row_end; r++) {
            char cell_ref_str[16];
            snprintf(cell_ref_str, 16, "%c%d", c, r);
            
            CellEntry* cell = symtab_get_cell(table, cell_ref_str);
            Value val;
            if (cell != NULL && cell->is_defined) {
                val = create_number_value(cell->value);
            } else {
                val = create_number_value(0.0); // Undefined cells are 0
            }
            
            // Create a new list node
            ValueNode* new_node = (ValueNode*)malloc(sizeof(ValueNode));
            new_node->value = val;
            new_node->next = NULL;
            
            // Append to list
            if (head == NULL) {
                head = new_node;
                tail = new_node;
            } else {
                tail->next = new_node;
                tail = new_node;
            }
        }
    }
    
    return head;
}

