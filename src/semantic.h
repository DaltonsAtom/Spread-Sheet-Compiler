#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtab.h"
/*
 * FIX: Include error.h so we know what
 * 'ErrorSystem' is and get the prototype
 * for 'error_get_count'.
 */
#include "error.h" 

/**
 * @brief Runs all semantic analysis checks on the AST.
 *
 * Checks for:
 * - Undefined cell references
 * - Type mismatches (e.g., "A1" + 5)
 * - Incorrect function argument counts (e.g., IF(A1, B1))
 * - Circular dependencies (e.g., A1=B1, B1=A1)
 *
 * @param node The root of the AST.
 * @param table The symbol table to use for lookups.
 * @param errors The error reporting system to log errors.
 * @param this_cell_ref The reference of the cell we are defining (e.g., "C1").
 * @return int The total number of semantic errors found.
 */
int semantic_analysis(ASTNode* node, SymbolTable* table, ErrorSystem* errors, const char* this_cell_ref);


#endif // SEMANTIC_H

