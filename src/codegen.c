/*
 * --- Code Generator Implementation (Prompt 5.2) ---
 *
 * Traverses the AST (post-order) to generate stack-based
 * bytecode.
 *
 * FIX:
 * 1. Added 'parser.tab.h' include for token names.
 * 2. Renamed emit_constant -> emit_push
 * 3. Renamed emit_cell_ref -> emit_push_cell
 * 4. Renamed emit_range -> emit_push_range
 * 5. Passed 'node->line' to all emitter functions.
 */

#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h" // For token enums (PLUS, MINUS, etc.)

/* --- Private Helper Prototypes --- */
static void generate_expr(ASTNode* node, CodeArray* code, SymbolTable* table);

/* --- Recursive Traversal Function --- */

static void generate_expr(ASTNode* node, CodeArray* code, SymbolTable* table) {
    if (node == NULL) {
        return;
    }

    // Use node->line for all emitted instructions
    int line = node->line;

    switch (node->type) {
        case NODE_NUMBER:
            // FIX: Was emit_constant
            emit_push(code, node->data.number, line);
            break;
            
        case NODE_STRING:
            // Note: We don't have string ops, but we push it for functions.
            // This would be OP_PUSH_STRING in a fuller VM.
            // For now, we'll push 0.0 as a placeholder.
            emit_push(code, 0.0, line); // Placeholder
            break;
            
        case NODE_CELL_REF:
            // FIX: Was emit_cell_ref
            emit_push_cell(code, node->data.str_value, line);
            break;
            
        case NODE_RANGE:
            // Ranges are only valid as function args. We push the string.
            // FIX: Was emit_range_str
            emit_push_range(code, node->data.str_value, line);
            break;

        case NODE_UNARY_OP:
            // 1. Generate code for the child
            generate_expr(node->data.op.left, code, table);
            // 2. Emit the operator
            if (node->data.op.op_token == MINUS) {
                // FIX: Added line number
                emit_op(code, OP_NEG, line);
            } else if (node->data.op.op_token == NOT) {
                // FIX: Added line number
                emit_op(code, OP_NOT, line);
            }
            break;

        case NODE_BINARY_OP:
            // 1. Generate code for left child
            generate_expr(node->data.op.left, code, table);
            // 2. Generate code for right child
            generate_expr(node->data.op.right, code, table);
            // 3. Emit the operator
            switch(node->data.op.op_token) {
                // FIX: Added line number to all
                case PLUS:     emit_op(code, OP_ADD, line); break;
                case MINUS:    emit_op(code, OP_SUB, line); break;
                case MULTIPLY: emit_op(code, OP_MUL, line); break;
                case DIVIDE:   emit_op(code, OP_DIV, line); break;
                case POWER:    emit_op(code, OP_POW, line); break;
                case GT:       emit_op(code, OP_GT, line); break;
                case LT:       emit_op(code, OP_LT, line); break;
                case GTE:      emit_op(code, OP_GTE, line); break;
                case LTE:      emit_op(code, OP_LTE, line); break;
                case NE:       emit_op(code, OP_NEQ, line); break;
                case EQUALS:   emit_op(code, OP_EQ, line); break;
                case AND:      emit_op(code, OP_AND, line); break;
                case OR:       emit_op(code, OP_OR, line); break;
                default:
                    // Should not happen
                    break;
            }
            break;

        case NODE_FUNCTION_CALL: {
            int func_token = node->data.func.function_token;
            
            if (func_token == IF) {
                // Special case: IF(cond, true_branch, false_branch)
                ASTNode* args = node->data.func.arguments;
                ASTNode* cond_node = args->data.arg.expression;
                ASTNode* true_node = args->data.arg.next_arg->data.arg.expression;
                ASTNode* false_node = args->data.arg.next_arg->data.arg.next_arg->data.arg.expression;

                // 1. Generate code for condition
                generate_expr(cond_node, code, table);
                
                // 2. Emit JMP_IF_FALSE. We'll patch the address later.
                // FIX: Added line number
                int false_jump_idx = emit_jump(code, OP_JMP_IF_FALSE, line);
                
                // 3. Generate code for true branch
                generate_expr(true_node, code, table);
                
                // 4. Emit JMP (to skip the false branch). Patch later.
                // FIX: Added line number
                int end_jump_idx = emit_jump(code, OP_JMP, line);
                
                // 5. Patch the false_jump to point to *here*
                patch_jump(code, false_jump_idx);
                
                // 6. Generate code for false branch
                generate_expr(false_node, code, table);
                
                // 7. Patch the end_jump to point to *here*
                patch_jump(code, end_jump_idx);

            } else {
                // Standard function call: SUM, AVG, etc.
                // 1. Generate code for all arguments
                ASTNode* arg = node->data.func.arguments;
                int arg_count = 0;
                while (arg != NULL) {
                    generate_expr(arg->data.arg.expression, code, table);
                    arg_count++;
                    arg = arg->data.arg.next_arg;
                }
                
                // 2. Emit the CALL instruction
                // FIX: Added line number
                emit_call(code, func_token, arg_count, line);
            }
            break;
        }

        case NODE_ARG_LIST:
            // This node is handled by the function call logic,
            // so we don't do anything here.
            break;
            
        default:
            fprintf(stderr, "Code-gen error: Unknown AST node type %d\n", node->type);
            break;
    }
}


/* --- Public API --- */

CodeArray* generate_code(ASTNode* root, SymbolTable* table) {
    if (root == NULL) {
        return NULL;
    }

    CodeArray* code = create_code_array();
    
    // Start recursive generation
    generate_expr(root, code, table);
    
    // 3. Finish with HALT
    // FIX: Added line (use root->line as the "end" line)
    emit_op(code, OP_HALT, root->line);
    
    return code;
}

