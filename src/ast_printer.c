#include "ast_printer.h"
#include "parser.tab.h" // For token names (e.g., PLUS, MINUS)
#include <stdio.h>

/* --- Private Function Prototypes --- */
static void print_ast_tree(ASTNode* node, const char* prefix, int is_last);
static void print_ast_dot(ASTNode* node);
static void print_ast_lisp(ASTNode* node);


/* --- Public API Function --- */

void print_ast(ASTNode* node, PrintFormat format) {
    if (node == NULL) {
        printf("AST is NULL.\n");
        return;
    }
    
    switch (format) {
        case PRINT_TREE:
            print_ast_tree(node, "", 1);
            break;
        case PRINT_DOT:
            printf("digraph AST {\n");
            printf("  node [fontname=\"Arial\"];\n");
            print_ast_dot(node);
            printf("}\n");
            break;
        case PRINT_LISP:
            print_ast_lisp(node);
            printf("\n");
            break;
        case PRINT_NONE:
            // Do nothing
            break;
    }
}


/* --- Private Implementation: Box-Drawing Tree --- */

/**
 * @brief Recursively prints the AST with box-drawing characters.
 *
 * @param node The current node to print.
 * @param prefix The line prefix (contains │ and spaces).
 * @param is_last Is this the last child of its parent?
 */
static void print_ast_tree(ASTNode* node, const char* prefix, int is_last) {
    if (node == NULL) return;

    // Print the prefix and the connector (├── or └──)
    printf("%s", prefix);
    printf(is_last ? "└── " : "├── ");

    // Print the node's content
    switch (node->type) {
        case NODE_NUMBER:
            printf("NUMBER (%f)\n", node->data.number);
            break;
        case NODE_STRING:
            printf("STRING (\"%s\")\n", node->data.str_value);
            break;
        case NODE_CELL_REF:
            printf("CELL_REF (%s)\n", node->data.str_value);
            break;
        case NODE_RANGE:
            printf("RANGE (%s)\n", node->data.str_value);
            break;
        case NODE_UNARY_OP:
            printf("UNARY_OP (%s)\n", node->data.op.op_token == MINUS ? "-" : "NOT");
            break;
        case NODE_BINARY_OP:
            printf("BINARY_OP (%s)\n", 
                node->data.op.op_token == PLUS ? "+" :
                node->data.op.op_token == MINUS ? "-" :
                node->data.op.op_token == MULTIPLY ? "*" :
                node->data.op.op_token == DIVIDE ? "/" :
                node->data.op.op_token == POWER ? "^" :
                node->data.op.op_token == GT ? ">" :
                node->data.op.op_token == LT ? "<" :
                node->data.op.op_token == GTE ? ">=" :
                node->data.op.op_token == LTE ? "<=" :
                node->data.op.op_token == EQUALS ? "=" :
                node->data.op.op_token == NE ? "<>" :
                node->data.op.op_token == AND ? "AND" : "OR");
            break;
        case NODE_FUNCTION_CALL:
             printf("FUNCTION (%s)\n", 
                node->data.func.function_token == IF ? "IF" :
                node->data.func.function_token == SUM ? "SUM" :
                node->data.func.function_token == AVERAGE ? "AVERAGE" :
                node->data.func.function_token == MIN ? "MIN" : "MAX");
            break;
        case NODE_ARG_LIST:
            printf("ARG\n");
            break;
        default:
            printf("UNKNOWN_NODE\n");
    }

    // Prepare the prefix for the children
    char new_prefix[512];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    // Recurse on children
    switch (node->type) {
        case NODE_UNARY_OP:
            print_ast_tree(node->data.op.left, new_prefix, 1);
            break;
        case NODE_BINARY_OP:
            print_ast_tree(node->data.op.left, new_prefix, 0);
            print_ast_tree(node->data.op.right, new_prefix, 1);
            break;
        case NODE_FUNCTION_CALL:
            print_ast_tree(node->data.func.arguments, new_prefix, 1);
            break;
        case NODE_ARG_LIST:
            print_ast_tree(node->data.arg.expression, new_prefix, 0);
            print_ast_tree(node->data.arg.next_arg, new_prefix, 1);
            break;
        default:
            // No children
            break;
    }
}


/* --- Private Implementation: DOT (Graphviz) --- */

// Helper to get a unique ID for each node (its pointer address)
static long get_node_id(ASTNode* node) {
    return (long)node;
}

/**
 * @brief Recursively prints the AST in DOT format.
 */
static void print_ast_dot(ASTNode* node) {
    if (node == NULL) return;

    long id = get_node_id(node);

    // 1. Define the current node
    switch (node->type) {
        case NODE_NUMBER:
            printf("  node%ld [label=\"NUMBER\\n(%f)\"];\n", id, node->data.number);
            break;
        case NODE_STRING:
            printf("  node%ld [label=\"STRING\\n(\\\"%s\\\")\"];\n", id, node->data.str_value);
            break;
        case NODE_CELL_REF:
            printf("  node%ld [label=\"CELL_REF\\n(%s)\"];\n", id, node->data.str_value);
            break;
        case NODE_RANGE:
            printf("  node%ld [label=\"RANGE\\n(%s)\"];\n", id, node->data.str_value);
            break;
        case NODE_UNARY_OP:
            printf("  node%ld [label=\"UNARY_OP\\n(%s)\"];\n", id, node->data.op.op_token == MINUS ? "-" : "NOT");
            break;
        case NODE_BINARY_OP:
            printf("  node%ld [label=\"BINARY_OP\\n(%s)\"];\n", id, 
                node->data.op.op_token == PLUS ? "+" :
                node->data.op.op_token == MINUS ? "-" :
                node->data.op.op_token == MULTIPLY ? "*" :
                node->data.op.op_token == DIVIDE ? "/" :
                node->data.op.op_token == POWER ? "^" :
                node->data.op.op_token == GT ? ">" :
                node->data.op.op_token == LT ? "<" :
                node->data.op.op_token == GTE ? ">=" :
                node->data.op.op_token == LTE ? "<=" :
                node->data.op.op_token == EQUALS ? "=" :
                node->data.op.op_token == NE ? "<>" :
                node->data.op.op_token == AND ? "AND" : "OR");
            break;
        case NODE_FUNCTION_CALL:
             printf("  node%ld [label=\"FUNCTION\\n(%s)\"];\n", id, 
                node->data.func.function_token == IF ? "IF" :
                node->data.func.function_token == SUM ? "SUM" :
                node->data.func.function_token == AVERAGE ? "AVERAGE" :
                node->data.func.function_token == MIN ? "MIN" : "MAX");
            break;
        case NODE_ARG_LIST:
             printf("  node%ld [label=\"ARG\"];\n", id);
            break;
        default:
            printf("  node%ld [label=\"UNKNOWN\"];\n", id);
    }

    // 2. Define relationships and recurse
    switch (node->type) {
        case NODE_UNARY_OP:
            if (node->data.op.left) {
                printf("  node%ld -> node%ld;\n", id, get_node_id(node->data.op.left));
                print_ast_dot(node->data.op.left);
            }
            break;
        case NODE_BINARY_OP:
            if (node->data.op.left) {
                printf("  node%ld -> node%ld [label=\"L\"];\n", id, get_node_id(node->data.op.left));
                print_ast_dot(node->data.op.left);
            }
            if (node->data.op.right) {
                printf("  node%ld -> node%ld [label=\"R\"];\n", id, get_node_id(node->data.op.right));
                print_ast_dot(node->data.op.right);
            }
            break;
        case NODE_FUNCTION_CALL:
            if (node->data.func.arguments) {
                printf("  node%ld -> node%ld [label=\"Args\"];\n", id, get_node_id(node->data.func.arguments));
                print_ast_dot(node->data.func.arguments);
            }
            break;
        case NODE_ARG_LIST:
             if (node->data.arg.expression) {
                printf("  node%ld -> node%ld [label=\"Expr\"];\n", id, get_node_id(node->data.arg.expression));
                print_ast_dot(node->data.arg.expression);
            }
            if (node->data.arg.next_arg) {
                printf("  node%ld -> node%ld [label=\"Next\"];\n", id, get_node_id(node->data.arg.next_arg));
                print_ast_dot(node->data.arg.next_arg);
            }
            break;
        default:
            // No children
            break;
    }
}


/* --- Private Implementation: Lisp-Style S-Expression --- */

// Helper to get operator symbol
static const char* get_op_symbol(int op_token) {
    switch(op_token) {
        case PLUS: return "+";
        case MINUS: return "-";
        case MULTIPLY: return "*";
        case DIVIDE: return "/";
        case POWER: return "^";
        case GT: return ">";
        case LT: return "<";
        case GTE: return ">=";
        case LTE: return "<=";
        case EQUALS: return "=";
        case NE: return "<>";
        case AND: return "AND";
        case OR: return "OR";
        case NOT: return "NOT";
        default: return "?";
    }
}

// Helper to get function name
static const char* get_func_name(int func_token) {
    switch(func_token) {
        case IF: return "IF";
        case SUM: return "SUM";
        case AVERAGE: return "AVERAGE";
        case MIN: return "MIN";
        case MAX: return "MAX";
        default: return "?FUNC";
    }
}

/**
 * @brief Recursively prints the AST in Lisp-style S-Expression format.
 */
static void print_ast_lisp(ASTNode* node) {
    if (node == NULL) {
        printf("NIL");
        return;
    }

    switch (node->type) {
        case NODE_NUMBER:
            printf("%f", node->data.number);
            break;
        case NODE_STRING:
            printf("\"%s\"", node->data.str_value);
            break;
        case NODE_CELL_REF:
            printf("(CELL_REF %s)", node->data.str_value);
            break;
        case NODE_RANGE:
            printf("(RANGE %s)", node->data.str_value);
            break;
        case NODE_UNARY_OP:
            printf("(%s ", get_op_symbol(node->data.op.op_token));
            print_ast_lisp(node->data.op.left);
            printf(")");
            break;
        case NODE_BINARY_OP:
            printf("(%s ", get_op_symbol(node->data.op.op_token));
            print_ast_lisp(node->data.op.left);
            printf(" ");
            print_ast_lisp(node->data.op.right);
            printf(")");
            break;
        case NODE_FUNCTION_CALL:
            printf("(%s ", get_func_name(node->data.func.function_token));
            print_ast_lisp(node->data.func.arguments);
            printf(")");
            break;
        case NODE_ARG_LIST:
            printf("(ARG ");
            print_ast_lisp(node->data.arg.expression);
            if (node->data.arg.next_arg) {
                printf(" ");
                print_ast_lisp(node->data.arg.next_arg);
            }
            printf(")");
            break;
        default:
            printf("UNKNOWN");
    }
}

