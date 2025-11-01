#ifndef AST_PRINTER_H
#define AST_PRINTER_H

/*
 * FIX: Include ast.h here so this file
 * knows what 'ASTNode' is.
 */
#include "ast.h"

/**
 * @brief Defines the different AST output formats.
 */
typedef enum {
    PRINT_NONE, // Don't print the AST
    PRINT_TREE, // Default box-drawing tree
    PRINT_DOT,  // Graphviz .dot format
    PRINT_LISP  // Lisp-style S-expression
} PrintFormat;


/**
 * @brief Public-facing function to print the AST.
 *
 * This function is the entry point and dispatches to the
 * correct private print function based on the format.
 *
 * @param node The root of the AST to print.
 * @param format The desired output format.
 */
void print_ast(ASTNode* node, PrintFormat format);


#endif // AST_PRINTER_H

