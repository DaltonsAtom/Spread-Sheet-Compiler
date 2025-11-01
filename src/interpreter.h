/*
 * --- AST Interpreter Header (Prompt 6.1) ---
 *
 * Declares the function for direct AST evaluation.
 *
 * FIX: Added trace_level parameter.
 */
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "symtab.h"
#include "value.h"

/**
 * @brief Recursively evaluates an AST node.
 * @param node The AST node to evaluate.
 * @param table The symbol table for cell lookups.
 * @param trace_level 0 for no trace, >0 for indentation.
 * @return The final Value (number, string, or error).
 */
Value interpreter_evaluate(ASTNode* node, SymbolTable* table, int trace_level);


#endif // INTERPRETER_H

