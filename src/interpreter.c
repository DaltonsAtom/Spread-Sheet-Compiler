/*
 * --- AST Interpreter Implementation (Prompt 6.1) ---
 *
 * This file contains the logic to evaluate the AST directly
 * by recursively traversing the tree.
 *
 * FIX: Added trace_level for verbose logging.
 */

#include "interpreter.h"
#include "runtime.h"
#include "parser.tab.h" // For token definitions (e.g., PLUS, MINUS)
#include <stdio.h>
#include <math.h>

/* --- Private Helper Prototypes --- */
static Value eval_binary_op(Value left, Value right, int op_token);
static Value eval_unary_op(Value right, int op_token);
static Value eval_function_call(ASTNode* node, SymbolTable* table, int trace_level);
static ValueNode* eval_arg_list(ASTNode* arg_node, SymbolTable* table, int* arg_count, int trace_level);

// Helper for tracing
static void print_trace(const char* msg, int trace_level) {
    if (trace_level <= 0) return;
    for (int i = 0; i < (trace_level - 1) * 2; i++) printf(" "); // 2 spaces per level
    printf("%s\n", msg);
}
static void print_trace_result(Value val, int trace_level) {
    if (trace_level <= 0) return;
    for (int i = 0; i < (trace_level - 1) * 2; i++) printf(" ");
    printf("Result: ");
    print_value(val);
    printf("\n");
}


/* --- Public API --- */

Value interpreter_evaluate(ASTNode* node, SymbolTable* table, int trace_level) {
    if (node == NULL) {
        return create_error_value("Attempted to evaluate NULL node");
    }

    Value result;

    switch (node->type) {
        // --- Literals ---
        case NODE_NUMBER:
            if (trace_level > 0) {
                char msg[64];
                snprintf(msg, 64, "Evaluating NODE_NUMBER = %.2f", node->data.number);
                print_trace(msg, trace_level);
            }
            result = create_number_value(node->data.number);
            break;
        
        case NODE_STRING:
            if (trace_level > 0) print_trace("Evaluating NODE_STRING", trace_level);
            result = create_string_value(node->data.str_value);
            break;

        case NODE_CELL_REF: {
            CellEntry* cell = symtab_get_cell(table, node->data.str_value);
            double val = 0.0;
            if (cell != NULL && cell->is_defined) {
                val = cell->value;
            }
            if (trace_level > 0) {
                char msg[64];
                snprintf(msg, 64, "Evaluating NODE_CELL(%s) = %.2f", node->data.str_value, val);
                print_trace(msg, trace_level);
            }
            result = create_number_value(val);
            break;
        }
            
        case NODE_RANGE:
            if (trace_level > 0) print_trace("Evaluating NODE_RANGE", trace_level);
            result = create_string_value(node->data.str_value);
            break;

        // --- Operators ---
        case NODE_UNARY_OP: {
            if (trace_level > 0) print_trace("Evaluating NODE_UNARY_OP", trace_level);
            Value right = interpreter_evaluate(node->data.op.left, table, trace_level + 1);
            if (right.type == TYPE_ERROR) return right;
            result = eval_unary_op(right, node->data.op.op_token);
            free_value(right);
            break;
        }

        case NODE_BINARY_OP: {
            if (trace_level > 0) print_trace("Evaluating NODE_BINARY_OP", trace_level);
            Value left = interpreter_evaluate(node->data.op.left, table, trace_level + 1);
            if (left.type == TYPE_ERROR) return left;
            
            Value right = interpreter_evaluate(node->data.op.right, table, trace_level + 1);
            if (right.type == TYPE_ERROR) {
                free_value(left);
                return right;
            }
            
            result = eval_binary_op(left, right, node->data.op.op_token);
            free_value(left);
            free_value(right);
            break;
        }

        // --- Functions ---
        case NODE_FUNCTION_CALL:
            if (trace_level > 0) print_trace("Evaluating NODE_FUNCTION_CALL", trace_level);
            result = eval_function_call(node, table, trace_level + 1);
            break;
            
        case NODE_ARG_LIST:
            result = create_error_value("Cannot evaluate argument list directly");
            break;

        default:
            result = create_error_value("Unknown AST node type");
    }
    
    if (trace_level == 1) { // Print final result of this branch
         print_trace_result(result, trace_level);
    }
    return result;
}


/* --- Private Helper Implementations --- */

static Value eval_binary_op(Value left, Value right, int op_token) {
    double left_num = get_numeric(left);
    double right_num = get_numeric(right);

    switch(op_token) {
        case PLUS:     return create_number_value(left_num + right_num);
        case MINUS:    return create_number_value(left_num - right_num);
        case MULTIPLY: return create_number_value(left_num * right_num);
        case DIVIDE:
            if (right_num == 0) {
                return create_error_value("Division by zero");
            }
            return create_number_value(left_num / right_num);
        case POWER:
            return create_number_value(pow(left_num, right_num));
        case GT:     return create_boolean_value(left_num > right_num);
        case LT:     return create_boolean_value(left_num < right_num);
        case GTE:    return create_boolean_value(left_num >= right_num);
        case LTE:    return create_boolean_value(left_num <= right_num);
        case EQUALS: return create_boolean_value(left_num == right_num);
        case NE:     return create_boolean_value(left_num != right_num);
        case AND:    return create_boolean_value(is_truthy(left) && is_truthy(right));
        case OR:     return create_boolean_value(is_truthy(left) || is_truthy(right));
        default:     return create_error_value("Unknown binary operator");
    }
}

static Value eval_unary_op(Value right, int op_token) {
    switch(op_token) {
        case MINUS: return create_number_value(-get_numeric(right));
        case NOT:   return create_boolean_value(!is_truthy(right));
        default:    return create_error_value("Unknown unary operator");
    }
}

static Value eval_function_call(ASTNode* node, SymbolTable* table, int trace_level) {
    int func_token = node->data.func.function_token;
    
    if (func_token == IF) {
        ASTNode* arg_list = node->data.func.arguments;
        if (arg_list == NULL || arg_list->data.arg.next_arg == NULL || arg_list->data.arg.next_arg->data.arg.next_arg == NULL) {
            return create_error_value("IF requires 3 arguments");
        }
        
        ASTNode* cond_node = arg_list->data.arg.expression;
        Value cond_val = interpreter_evaluate(cond_node, table, trace_level + 1);
        if (cond_val.type == TYPE_ERROR) return cond_val;
        
        Value result;
        if (is_truthy(cond_val)) {
            ASTNode* true_node = arg_list->data.arg.next_arg->data.arg.expression;
            result = interpreter_evaluate(true_node, table, trace_level + 1);
        } else {
            ASTNode* false_node = arg_list->data.arg.next_arg->data.arg.next_arg->data.arg.expression;
            result = interpreter_evaluate(false_node, table, trace_level + 1);
        }
        
        free_value(cond_val);
        return result;
    }
    
    int arg_count = 0;
    ValueNode* arg_head = eval_arg_list(node->data.func.arguments, table, &arg_count, trace_level + 1);
    
    Value result;
    switch (func_token) {
        case SUM:     result = rt_sum(arg_head); break;
        case AVERAGE: result = rt_average(arg_head); break;
        case MIN:     result = rt_min(arg_head); break;
        case MAX:     result = rt_max(arg_head); break;
        case NOT:     result = rt_not(arg_head); break;
        default:      result = create_error_value("Unknown function");
    }
    
    free_value_list(arg_head);
    return result;
}

static ValueNode* eval_arg_list(ASTNode* arg_node, SymbolTable* table, int* arg_count, int trace_level) {
    if (arg_node == NULL) {
        *arg_count = 0;
        return NULL;
    }

    ValueNode* next_values = eval_arg_list(arg_node->data.arg.next_arg, table, arg_count, trace_level);
    Value current_val = interpreter_evaluate(arg_node->data.arg.expression, table, trace_level);
    
    if (current_val.type == TYPE_STRING) {
        ValueNode* range_head = rt_expand_range(current_val.as.string, table);
        if (range_head != NULL) {
            free_value(current_val);
            if (range_head == NULL) {
                return next_values;
            }
            ValueNode* tail = range_head;
            int range_count = 1;
            while (tail->next != NULL) {
                tail = tail->next;
                range_count++;
            }
            tail->next = next_values;
            *arg_count += range_count;
            return range_head;
        }
    }
    
    ValueNode* new_head = (ValueNode*)malloc(sizeof(ValueNode));
    new_head->value = current_val;
    new_head->next = next_values;
    *arg_count += 1;
    
    return new_head;
}

