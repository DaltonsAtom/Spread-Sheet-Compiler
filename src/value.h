/*
 * --- Value System Header ---
 *
 * Defines the 'Value' struct, which is the
 * universal data type used by the VM and Interpreter
 * for all calculations.
 *
 * FIX:
 * 1. Added 'print_value_inline' function prototype.
 * 2. Added 'is_truthy' function prototype.
 * 3. Added 'get_numeric' function prototype.
 */

#ifndef VALUE_H
#define VALUE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- Value Type Enum --- */
typedef enum {
    TYPE_NUMBER,
    TYPE_BOOLEAN,
    TYPE_STRING,
    TYPE_ERROR
} ValueType;


/* --- Value Struct --- */
typedef struct {
    ValueType type;
    union {
        double number;
        int boolean;
        char* string; // Dynamically allocated
    } as;
} Value;


/* --- Constructor Functions --- */

static inline Value create_number_value(double num) {
    Value val;
    val.type = TYPE_NUMBER;
    val.as.number = num;
    return val;
}

static inline Value create_boolean_value(int b) {
    Value val;
    val.type = TYPE_BOOLEAN;
    val.as.boolean = (b != 0);
    return val;
}

static inline Value create_string_value(const char* str) {
    Value val;
    val.type = TYPE_STRING;
    val.as.string = strdup(str); // Copy the string
    return val;
}

static inline Value create_error_value(const char* msg) {
    Value val;
    val.type = TYPE_ERROR;
    val.as.string = strdup(msg); // Store error message in string
    return val;
}

/* Free any dynamic data (like strings) */
static inline void free_value(Value val) {
    if (val.type == TYPE_STRING || val.type == TYPE_ERROR) {
        if (val.as.string != NULL) {
            free(val.as.string);
        }
    }
}


/* --- Helper Functions --- */

/**
 * @brief Checks if a value is "truthy" (not 0 or false).
 */
static inline int is_truthy(Value val) {
    switch (val.type) {
        case TYPE_NUMBER:  return val.as.number != 0;
        case TYPE_BOOLEAN: return val.as.boolean;
        case TYPE_STRING:  return val.as.string[0] != '\0'; // Not empty
        case TYPE_ERROR:   return 0; // Errors are false
        default:           return 0;
    }
}

/**
 * @brief Gets the numeric representation of a value.
 */
static inline double get_numeric(Value val) {
    if (val.type == TYPE_NUMBER) {
        return val.as.number;
    }
    if (val.type == TYPE_BOOLEAN) {
        return val.as.boolean ? 1.0 : 0.0;
    }
    return 0.0; // Strings, errors, etc., are 0
}


/* --- Printing Functions --- */

/**
 * @brief Prints a full, user-facing representation of the value.
 */
static inline void print_value(Value val) {
    switch (val.type) {
        case TYPE_NUMBER:
            printf("%f", val.as.number);
            break;
        case TYPE_BOOLEAN:
            printf(val.as.boolean ? "TRUE" : "FALSE");
            break;
        case TYPE_STRING:
            printf("\"%s\"", val.as.string);
            break;
        case TYPE_ERROR:
            printf("#ERROR: %s", val.as.string);
            break;
        default:
            printf("UNKNOWN_VALUE");
            break;
    }
}

/**
 * @brief Prints a compact, inline representation for debugging.
 * FIX: Added prototype.
 */
static inline void print_value_inline(Value val) {
     switch (val.type) {
        case TYPE_NUMBER:
            printf("%g", val.as.number);
            break;
        case TYPE_BOOLEAN:
            printf(val.as.boolean ? "T" : "F");
            break;
        case TYPE_STRING:
            printf("\"%.10s...\"", val.as.string);
            break;
        case TYPE_ERROR:
            printf("#ERR");
            break;
        default:
            printf("?");
            break;
    }
}


#endif // VALUE_H

