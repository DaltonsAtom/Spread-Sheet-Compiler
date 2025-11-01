/*
 * --- Error Reporting Header (Prompt 4.3) ---
 *
 * Defines the structures and functions for a
 * comprehensive error reporting system.
 *
 * FIX:
 * 1. Added helper functions for circular dependency message building.
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h" // For ASTNode

/* --- Error Types --- */
typedef enum {
    ERROR_SYNTAX,
    ERROR_SEMANTIC,
    ERROR_RUNTIME
} ErrorType;

/* --- Error Structure --- */
typedef struct Error {
    ErrorType type;
    int line;
    int column;
    char* message;
    char* suggestion;
    struct Error* next; // For linked list
} Error;

/* --- Error System --- */
typedef struct {
    Error* head;
    Error* tail;
    int error_count;
    const char* source_code; // Not fully used yet
    
    // Buffer for building circular dependency messages
    char message_buffer[1024]; 
} ErrorSystem;


/* --- Public API --- */

ErrorSystem* error_system_create(const char* source_code);
void error_system_free(ErrorSystem* system);

/**
 * @brief Reports a new error.
 */
void error_report(ErrorSystem* system, ErrorType type, int line, int col, const char* message, const char* suggestion);

/**
 * @brief Prints all collected errors to stderr.
 */
void error_print_all(ErrorSystem* system);

/**
 * @brief Gets the total number of errors reported.
 */
int error_get_count(ErrorSystem* system);


/* --- Helper Functions for Circular Dep --- */

/**
 * @brief Appends a message to the internal buffer (for circ. dep.)
 */
void error_report_message(ErrorSystem* system, const char* msg_part);

/**
 * @brief Gets the content of the internal message buffer.
 */
const char* error_get_messages(ErrorSystem* system);

/**
 * @brief Clears the internal message buffer.
 */
void error_clear_messages(ErrorSystem* system);


#endif // ERROR_H

