#include "semantic.h"
#include <stdio.h>
#include <string.h>

/*
 * FIX: Include parser.tab.h to get all token definitions
 * like PLUS, MINUS, IF, GT, etc.
 */
#include "parser.tab.h"

/* --- Private Context Structure --- */

/*
 * A context to pass down the recursive traversal,
 * holding all the systems we need.
 */
typedef struct {
    SymbolTable* table;
    ErrorSystem* errors;
    const char* this_cell_ref; // The cell we are currently defining (e.g., "C1")
    int error_count;
} SemanticContext;


/* --- Private Helper Prototypes --- */

// static ValueType get_node_type(ASTNode* node, SemanticContext* ctx); // REMOVED - This belongs to Phase 4/Evaluation
static void semantic_traverse(ASTNode* node, SemanticContext* ctx);
static void check_function_args(ASTNode* node, SemanticContext* ctx);
static void check_range(const char* range_str, int line, SemanticContext* ctx);

/* --- Public API --- */

int semantic_analysis(ASTNode* node, SymbolTable* table, ErrorSystem* errors, const char* this_cell_ref) {
    if (node == NULL || table == NULL || errors == NULL) {
        return 0; // Nothing to do
    }
    
    SemanticContext ctx;
    ctx.table = table;
    ctx.errors = errors;
    ctx.this_cell_ref = this_cell_ref;
    ctx.error_count = 0;

    printf("Running semantic analysis for cell %s...\n", this_cell_ref);

    // 1. Get or create the cell entry we are defining
    CellEntry* this_cell = symtab_get_cell(table, this_cell_ref);
    if (this_cell == NULL) {
        // This cell wasn't in the pre-defined list, so add it.
        symtab_define_cell(table, this_cell_ref, 0.0, NULL, 0); // Line 0 for now
        this_cell = symtab_get_cell(table, this_cell_ref);
    }
    this_cell->is_defined = 1; // We are now defining it

    // 2. Recursively traverse the AST to find all errors
    semantic_traverse(node, &ctx);

    // 3. After traversal, check for circular dependencies
    // We do this by checking all *direct* dependencies of this cell.
    if (ctx.error_count == 0 && this_cell->dep_count > 0) {
        printf("Checking circular dependencies for %s...\n", this_cell_ref);
        for (int i = 0; i < this_cell->dep_count; i++) {
            if (symtab_check_circular_dep(table, this_cell_ref, this_cell->dependencies[i], errors)) {
                ctx.error_count++;
                // Stop after the first circle is found
                break; 
            }
        }
    }

    // FIX: Use the correct function name
    return ctx.error_count + error_get_count(errors);
}


/* --- Recursive Traversal Function --- */

static void semantic_traverse(ASTNode* node, SemanticContext* ctx) {
    if (node == NULL) {
        return;
    }
    
    // Post-order traversal: check children first
    switch(node->type) {
        case NODE_UNARY_OP:
            semantic_traverse(node->data.op.left, ctx);
            break;
        case NODE_BINARY_OP:
            semantic_traverse(node->data.op.left, ctx);
            semantic_traverse(node->data.op.right, ctx);
            break;
        case NODE_FUNCTION_CALL:
            // Check arguments first
            if (node->data.func.arguments != NULL) {
                semantic_traverse(node->data.func.arguments, ctx);
            }
            // Then check the function itself
            check_function_args(node, ctx);
            break;
        case NODE_ARG_LIST:
            // Traverse this argument's expression
            semantic_traverse(node->data.arg.expression, ctx);
            // Traverse the next argument in the list
            semantic_traverse(node->data.arg.next_arg, ctx);
            break;
        
        default:
            // Literals (NUMBER, STRING) have no children
            // Cell/Range refs are checked below
            break;
    }

    // --- Check the current node ---
    
    // 1. Check for undefined cell references
    if (node->type == NODE_CELL_REF) {
        const char* ref = node->data.str_value;
        CellEntry* cell = symtab_get_cell(ctx->table, ref);
        
        if (cell == NULL || !cell->is_defined) {
            char msg[256];
            snprintf(msg, 256, "Undefined cell reference: '%s'.", ref);
            // FIX: Use node->line
            error_report(ctx->errors, ERROR_SEMANTIC, node->line, 0, msg, "Ensure this cell has a value.");
            ctx->error_count++;
        } else {
            // Add this as a dependency for the cell we are defining
            symtab_add_dependency(ctx->table, ctx->this_cell_ref, ref);
        }
    }
    
    // 2. Check for invalid ranges
    if (node->type == NODE_RANGE) {
        // FIX: Use node->line
        check_range(node->data.str_value, node->line, ctx);
    }
    
    // 3. Check for type mismatches
    // REMOVED - This logic belongs in Phase 4 (Evaluation)
    // and depends on value.h, which we don't have yet.
}


/* --- Type Inference Helper --- */
// REMOVED - This entire function (get_node_type) is
// postponed until Phase 4 (Evaluation).


/* --- Specific Check Helpers --- */

static void check_function_args(ASTNode* node, SemanticContext* ctx) {
    int arg_count = 0;
    ASTNode* arg = node->data.func.arguments;
    while (arg != NULL) {
        arg_count++;
        arg = arg->data.arg.next_arg;
    }

    int func = node->data.func.function_token;
    char msg[256];

    switch (func) {
        case IF:
            if (arg_count != 3) {
                snprintf(msg, 256, "Function 'IF' expects exactly 3 arguments, but got %d.", arg_count);
                // FIX: Use node->line
                error_report(ctx->errors, ERROR_SEMANTIC, node->line, 0, msg,
                    "The format is IF(condition, value_if_true, value_if_false).");
                ctx->error_count++;
            }
            break;
        
        case SUM:
        case AVERAGE:
        case MIN:
        case MAX:
            if (arg_count == 0) {
                 snprintf(msg, 256, "Function '%s' expects at least 1 argument, but got 0.", 
                    func == SUM ? "SUM" : (func == AVERAGE ? "AVERAGE" : (func == MIN ? "MIN" : "MAX")));
                // FIX: Use node->line
                error_report(ctx->errors, ERROR_SEMANTIC, node->line, 0, msg, "Provide a cell, range, or number.");
                ctx->error_count++;
            }
            break;
        default:
            break;
    }
}

static void check_range(const char* range_str, int line, SemanticContext* ctx) {
    char col_start, col_end;
    int row_start, row_end;
    
    // sscanf "A1:B10" -> A, 1, B, 10
    if (sscanf(range_str, "%c%d:%c%d", &col_start, &row_start, &col_end, &row_end) != 4) {
        char msg[256];
        snprintf(msg, 256, "Invalid range format: '%s'.", range_str);
        error_report(ctx->errors, ERROR_SEMANTIC, line, 0, msg, "Expected format like A1:B10.");
        ctx->error_count++;
        return;
    }

    // Check if start is before end
    if (col_start > col_end || row_start > row_end) {
        char msg[256];
        snprintf(msg, 256, "Invalid range: '%s'.", range_str);
        error_report(ctx->errors, ERROR_SEMANTIC, line, 0, msg, "Start of range must be top-left of end of range.");
        ctx->error_count++;
    }
}

