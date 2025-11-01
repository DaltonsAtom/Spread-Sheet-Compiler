/*
 * --- Error Reporting Implementation (Prompt 4.3) ---
 *
 * Implements the functions for creating, storing,
 * and printing compiler errors.
 *
 * FIX:
 * 1. Fixed printf format string for %s -> %d
 * 2. Implemented helper functions for circular dependency.
 */

#include "error.h"

// Colors for printing
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


/* --- Public API --- */

ErrorSystem* error_system_create(const char* source_code) {
    ErrorSystem* system = (ErrorSystem*)malloc(sizeof(ErrorSystem));
    system->head = NULL;
    system->tail = NULL;
    system->error_count = 0;
    system->source_code = (source_code != NULL) ? strdup(source_code) : NULL;
    
    // Clear message buffer
    system->message_buffer[0] = '\0';
    return system;
}

void error_system_free(ErrorSystem* system) {
    Error* current = system->head;
    while (current != NULL) {
        Error* next = current->next;
        free(current->message);
        free(current->suggestion);
        free(current);
        current = next;
    }
    free((void*)system->source_code);
    free(system);
}


void error_report(ErrorSystem* system, ErrorType type, int line, int col, const char* message, const char* suggestion) {
    if (system == NULL) return;

    // Create new error
    Error* err = (Error*)malloc(sizeof(Error));
    err->type = type;
    err->line = line;
    err->column = col;
    err->message = (message != NULL) ? strdup(message) : strdup("Unknown error");
    err->suggestion = (suggestion != NULL) ? strdup(suggestion) : NULL;
    err->next = NULL;
    
    // Append to list
    system->error_count++;
    if (system->tail == NULL) {
        system->head = err;
        system->tail = err;
    } else {
        system->tail->next = err;
        system->tail = err;
    }
}

int error_get_count(ErrorSystem* system) {
    return (system != NULL) ? system->error_count : 0;
}


void error_print_all(ErrorSystem* system) {
    if (system == NULL || system->head == NULL) {
        return; // No errors
    }
    
    Error* err = system->head;
    
    while(err != NULL) {
        const char* type_str;
        const char* color;
        
        switch(err->type) {
            case ERROR_SYNTAX:
                type_str = "Syntax Error";
                color = ANSI_COLOR_YELLOW;
                break;
            case ERROR_SEMANTIC:
                type_str = "Semantic Error";
                color = ANSI_COLOR_RED;
                break;
            case ERROR_RUNTIME:
                type_str = "Runtime Error";
                color = ANSI_COLOR_RED;
                break;
            default:
                type_str = "Error";
                color = ANSI_COLOR_RED;
                break;
        }

        // FIX: Changed %s to %d for err->line
        fprintf(stderr, "%s[Line %d] %s: %s%s\n",
                color,
                err->line,
                type_str,
                err->message,
                ANSI_COLOR_RESET);
        
        if (err->suggestion != NULL) {
            fprintf(stderr, "    %sSuggestion: %s%s\n",
                    ANSI_COLOR_CYAN,
                    err->suggestion,
                    ANSI_COLOR_RESET);
        }
        
        err = err->next;
    }
}


/* --- Helper Functions for Circular Dep --- */

void error_report_message(ErrorSystem* system, const char* msg_part) {
    if (system == NULL) return;
    strncat(system->message_buffer, msg_part, 1024 - strlen(system->message_buffer) - 1);
}

const char* error_get_messages(ErrorSystem* system) {
    if (system == NULL) return "";
    return system->message_buffer;
}

void error_clear_messages(ErrorSystem* system) {
    if (system != NULL) {
        system->message_buffer[0] = '\0';
    }
}

