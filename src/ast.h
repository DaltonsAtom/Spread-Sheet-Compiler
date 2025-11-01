/*
 * --- Abstract Syntax Tree Definitions ---
 *
 * Defines the NodeType enum, the ASTNode struct,
 * and all the constructor functions for building the tree.
 *
 * FIX:
 * 1. Added 'line' member to ASTNode.
 * 2. Updated all constructors to accept 'line'.
 * 3. Added 'g_node_count' to count all created nodes.
 */
#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Connect to global counter in parser.y
extern int g_node_count;

/* --- Node Type Enum --- */
typedef enum {
    NODE_NUMBER,
    NODE_STRING,
    NODE_CELL_REF,
    NODE_RANGE,
    NODE_UNARY_OP,
    NODE_BINARY_OP,
    NODE_FUNCTION_CALL,
    NODE_ARG_LIST
} NodeType;


/* --- AST Node Structure --- */
typedef struct ASTNode {
    NodeType type;
    int line; // Line number for error reporting

    union {
        double number;
        char* str_value; // For STRING, CELL_REF, RANGE

        struct {
            int op_token; // e.g., PLUS, MINUS, NOT
            struct ASTNode* left;
            struct ASTNode* right; // 'right' is NULL for unary ops
        } op;

        struct {
            int function_token; // e.g., SUM, IF
            struct ASTNode* arguments; // Pointer to an ARG_LIST node
        } func;
        
        struct {
            struct ASTNode* expression; // The expression for this arg
            struct ASTNode* next_arg;   // The next ARG_LIST node
        } arg;

    } data;
} ASTNode;


/* --- Private: Node Constructor --- */
static inline ASTNode* create_node(NodeType type, int line) {
    // FIX: Increment global node counter
    g_node_count++;
    
    // Use calloc to zero-initialize
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "Fatal: Out of memory\n");
        exit(1);
    }
    node->type = type;
    node->line = line;
    return node;
}


/* --- Public: Constructor Functions --- */

static inline ASTNode* create_number_node(double value, int line) {
    ASTNode* node = create_node(NODE_NUMBER, line);
    node->data.number = value;
    return node;
}

static inline ASTNode* create_string_node(char* value, int line) {
    ASTNode* node = create_node(NODE_STRING, line);
    node->data.str_value = value;
    return node;
}

static inline ASTNode* create_cell_ref_node(char* cell_ref, int line) {
    ASTNode* node = create_node(NODE_CELL_REF, line);
    node->data.str_value = cell_ref;
    return node;
}

static inline ASTNode* create_range_node(char* range_str, int line) {
    ASTNode* node = create_node(NODE_RANGE, line);
    node->data.str_value = range_str;
    return node;
}

static inline ASTNode* create_unary_op_node(int op_token, ASTNode* left, int line) {
    ASTNode* node = create_node(NODE_UNARY_OP, line);
    node->data.op.op_token = op_token;
    node->data.op.left = left;
    node->data.op.right = NULL;
    return node;
}

static inline ASTNode* create_binary_op_node(int op_token, ASTNode* left, ASTNode* right, int line) {
    ASTNode* node = create_node(NODE_BINARY_OP, line);
    node->data.op.op_token = op_token;
    node->data.op.left = left;
    node->data.op.right = right;
    return node;
}

static inline ASTNode* create_function_call_node(int func_token, ASTNode* args, int line) {
    ASTNode* node = create_node(NODE_FUNCTION_CALL, line);
    node->data.func.function_token = func_token;
    node->data.func.arguments = args;
    return node;
}

static inline ASTNode* create_arg_list_node(ASTNode* expr, ASTNode* next, int line) {
    ASTNode* node = create_node(NODE_ARG_LIST, line);
    node->data.arg.expression = expr;
    node->data.arg.next_arg = next;
    return node;
}


/* --- Utility Functions --- */

static inline void free_ast(ASTNode* node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case NODE_UNARY_OP:
            free_ast(node->data.op.left);
            break;
        case NODE_BINARY_OP:
            free_ast(node->data.op.left);
            free_ast(node->data.op.right);
            break;
        case NODE_FUNCTION_CALL:
            free_ast(node->data.func.arguments);
            break;
        case NODE_ARG_LIST:
            free_ast(node->data.arg.expression);
            free_ast(node->data.arg.next_arg);
            break;
        
        case NODE_STRING:
        case NODE_CELL_REF:
        case NODE_RANGE:
            free(node->data.str_value); // Free the string copied by strdup()
            break;

        case NODE_NUMBER:
            // No dynamic data
            break;
    }
    
    // Free the node itself
    free(node);
}

#endif // AST_H

