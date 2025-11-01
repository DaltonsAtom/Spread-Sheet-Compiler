/*
 * --- Virtual Machine Header (Prompt 6.2) ---
 *
 * Defines the VM struct, stack size, and
 * the main execution function.
 */

#ifndef VM_H
#define VM_H

#include "ir.h"
#include "symtab.h"
#include "value.h"

// Define a fixed size for the VM's value stack
#define VM_STACK_SIZE 256

typedef struct {
    CodeArray* code;      // The bytecode to run
    SymbolTable* symtab;  // Global symbol table
    int pc;               // Program Counter
    int stack_top;        // Stack pointer
    Value stack[VM_STACK_SIZE]; // The value stack
    
    int trace;            // Flag for tracing execution
} VM;

/**
 * @brief Creates a new Virtual Machine.
 * @param code The bytecode array to execute.
 * @param table The symbol table for cell lookups.
 * @return A pointer to the new VM.
 */
VM* vm_create(CodeArray* code, SymbolTable* table);

/**
 * @brief Frees all memory associated with the VM.
 */
void vm_free(VM* vm);

/**
 * @brief Executes the VM's bytecode.
 * @return The final 'Value' result of the computation.
 */
Value vm_execute(VM* vm);


#endif // VM_H

