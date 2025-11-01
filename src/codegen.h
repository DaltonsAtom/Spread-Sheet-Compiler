#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "symtab.h"
/*
 * FIX: Include ir.h to define CodeArray and other
 * instruction-related structures.
 */
#include "ir.h"

/*
 * Generates a CodeArray (bytecode) from a given
 * Abstract Syntax Tree.
 *
 * @param root The root node of the AST to compile.
 * @param table The symbol table (used for cell lookups, etc.)
 * @return A pointer to a new CodeArray, or NULL on failure.
 */
CodeArray* generate_code(ASTNode* root, SymbolTable* table);

#endif // CODEGEN_H

