/*
 * --- Virtual Machine Implementation (Prompt 6.2) ---
 *
 * This is the execution engine that runs the stack-based
 * bytecode generated in Phase 5.
 *
 * FIX:
 * 1. Included parser.tab.h (for token names)
 * 2. Included ir.h (for print_instruction)
 * 3. Included value.h (for print_value_inline)
 * 4. Used correct operand union member 'func_call'
 * 5. Removed unused 'vm_peek' function
 */

#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// FIX: Include headers for missing definitions
#include "parser.tab.h" // For SUM, AVERAGE, MIN, MAX, NOT
#include "ir.h"         // For print_instruction
#include "value.h"      // For print_value_inline, get_numeric, etc.
#include "runtime.h"    // FIX: Added for ValueNode and rt_... functions


/* --- VM Helpers --- */
static void vm_push(VM* vm, Value val);
static Value vm_pop(VM* vm);
static void vm_print_stack(VM* vm);
static Value vm_run(VM* vm);

/* --- Public API --- */

VM* vm_create(CodeArray* code, SymbolTable* table) {
    VM* vm = (VM*)malloc(sizeof(VM));
    if (vm == NULL) {
        return NULL;
    }
    
    vm->code = code;
    vm->symtab = table;
    vm->pc = 0;
    vm->stack_top = 0;
    vm->trace = 0;
    
    return vm;
}

void vm_free(VM* vm) {
    if (vm != NULL) {
        // Free any values left on the stack
        for(int i = 0; i < vm->stack_top; i++) {
            free_value(vm->stack[i]);
        }
        free(vm);
    }
}

Value vm_execute(VM* vm) {
    if (vm->trace) {
        printf("--- VM TRACE ---\n");
    }
    
    Value result = vm_run(vm);
    
    if (vm->trace) {
        printf("--- END TRACE ---\n");
    }
    
    return result;
}


/* --- Stack Operations --- */

static void vm_push(VM* vm, Value val) {
    if (vm->stack_top >= VM_STACK_SIZE) {
        // This should be a runtime error, but we'll crash
        fprintf(stderr, "VM Error: Stack overflow\n");
        exit(1);
    }
    vm->stack[vm->stack_top++] = val;
}

static Value vm_pop(VM* vm) {
    if (vm->stack_top == 0) {
        fprintf(stderr, "VM Error: Stack underflow\n");
        exit(1);
    }
    // Note: We return the value, but if it was a string,
    // the *caller* is now responsible for freeing it.
    return vm->stack[--vm->stack_top];
}

/*
 * FIX: Removed unused vm_peek function
 */

/* --- Main Execution Loop --- */

static Value vm_run(VM* vm) {
    for (;;) {
        if (vm->pc >= vm->code->count) {
            return create_error_value("VM Error: PC out of bounds");
        }
        
        // Fetch
        Instruction instruction = vm->code->code[vm->pc];
        
        if (vm->trace) {
            printf("%04d: ", vm->pc);
            // FIX: print_instruction is declared in ir.h
            print_instruction(instruction, vm->pc);
            vm_print_stack(vm);
        }
        
        // Decode & Execute
        vm->pc++; // Increment *before* executing
        
        switch (instruction.opcode) {
            case OP_HALT: {
                if (vm->stack_top == 0) {
                    return create_error_value("VM Halted on empty stack");
                }
                Value final_result = vm_pop(vm);
                return final_result; // Success!
            }
            
            case OP_PUSH: {
                vm_push(vm, create_number_value(instruction.operand.number));
                break;
            }
            
            case OP_PUSH_CELL: {
                const char* cell_ref = instruction.operand.cell_ref;
                CellEntry* cell = symtab_get_cell(vm->symtab, cell_ref);
                if (cell != NULL && cell->is_defined) {
                    vm_push(vm, create_number_value(cell->value));
                } else {
                    vm_push(vm, create_number_value(0.0)); // Treat undefined as 0
                }
                break;
            }
            
            case OP_PUSH_RANGE: {
                // Push the range string itself. OP_CALL will handle it.
                vm_push(vm, create_string_value(instruction.operand.range_str));
                break;
            }


            // --- Binary Operators ---
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_POW:
            case OP_GT:
            case OP_LT:
            case OP_GTE:
            case OP_LTE:
            case OP_EQ:
            case OP_NEQ:
            case OP_AND:
            case OP_OR: {
                Value b = vm_pop(vm);
                Value a = vm_pop(vm);
                
                // FIX: get_numeric is in value.h
                double a_num = get_numeric(a);
                double b_num = get_numeric(b);
                
                Value result;
                switch(instruction.opcode) {
                    case OP_ADD: result = create_number_value(a_num + b_num); break;
                    case OP_SUB: result = create_number_value(a_num - b_num); break;
                    case OP_MUL: result = create_number_value(a_num * b_num); break;
                    case OP_DIV:
                        if (b_num == 0) result = create_error_value("Division by zero");
                        else result = create_number_value(a_num / b_num);
                        break;
                    case OP_POW: result = create_number_value(pow(a_num, b_num)); break;
                    
                    case OP_GT:  result = create_boolean_value(a_num > b_num); break;
                    case OP_LT:  result = create_boolean_value(a_num < b_num); break;
                    case OP_GTE: result = create_boolean_value(a_num >= b_num); break;
                    case OP_LTE: result = create_boolean_value(a_num <= b_num); break;
                    case OP_EQ:  result = create_boolean_value(a_num == b_num); break;
                    case OP_NEQ: result = create_boolean_value(a_num != b_num); break;

                    // FIX: is_truthy is in value.h
                    case OP_AND: result = create_boolean_value(is_truthy(a) && is_truthy(b)); break;
                    case OP_OR:  result = create_boolean_value(is_truthy(a) || is_truthy(b)); break;
                    
                    default: result = create_error_value("Unhandled binary op");
                }
                
                free_value(a);
                free_value(b);
                
                if (result.type == TYPE_ERROR) {
                    return result; // Propagate error
                }
                vm_push(vm, result);
                break;
            }

            // --- Unary Operators ---
            case OP_NEG:
            case OP_NOT: {
                Value a = vm_pop(vm);
                Value result;
                
                if (instruction.opcode == OP_NEG) {
                    result = create_number_value(-get_numeric(a));
                } else {
                    result = create_boolean_value(!is_truthy(a));
                }
                
                free_value(a);
                vm_push(vm, result);
                break;
            }
            
            // --- Control Flow ---
            case OP_JMP_IF_FALSE: {
                Value cond = vm_pop(vm);
                if (!is_truthy(cond)) {
                    vm->pc = instruction.operand.address; // JUMP
                }
                free_value(cond);
                break;
            }
            
            case OP_JMP: {
                vm->pc = instruction.operand.address; // JUMP
                break;
            }

            // --- Functions ---
            case OP_CALL: {
                // FIX: Use 'func_call' member
                int func_token = instruction.operand.func_call.token;
                int arg_count = instruction.operand.func_call.arg_count;
                
                // 1. Pop args off stack and build ValueNode list
                // (Args are pushed in order, so pop in reverse)
                ValueNode* arg_head = NULL;
                for (int i = 0; i < arg_count; i++) {
                    Value val = vm_pop(vm);
                    
                    // Handle ranges (which were pushed as strings)
                    if (val.type == TYPE_STRING) {
                        ValueNode* range_vals = rt_expand_range(val.as.string, vm->symtab);
                        if (range_vals != NULL) {
                            // It was a range. Find tail and append.
                            ValueNode* tail = range_vals;
                            while(tail->next != NULL) {
                                tail = tail->next;
                            }
                            tail->next = arg_head;
                            arg_head = range_vals;
                            free_value(val); // Free the range string
                            continue; // Move to next arg
                        }
                    }
                    
                    // Not a range, just prepend
                    ValueNode* new_node = (ValueNode*)malloc(sizeof(ValueNode));
                    new_node->value = val;
                    new_node->next = arg_head;
                    arg_head = new_node;
                }
                
                // 2. Call runtime function
                Value result;
                // FIX: Need parser.tab.h for these tokens
                switch (func_token) {
                    case SUM:     result = rt_sum(arg_head); break;
                    case AVERAGE: result = rt_average(arg_head); break;
                    case MIN:     result = rt_min(arg_head); break;
                    case MAX:     result = rt_max(arg_head); break;
                    case NOT:     result = rt_not(arg_head); break;
                    // IF is handled by JMP ops, not OP_CALL
                    
                    default:
                        result = create_error_value("Unknown function call in VM");
                }
                
                free_value_list(arg_head); // Clean up args
                vm_push(vm, result);       // Push result
                break;
            }
                
            case OP_NOP:
                // Do nothing
                break;

            default:
                return create_error_value("VM Error: Unknown opcode");
        }
    }
}


/* --- Debugging Helpers --- */

static void vm_print_stack(VM* vm) {
    printf("    STACK: [ ");
    for (int i = 0; i < vm->stack_top; i++) {
        // FIX: print_value_inline is declared in value.h
        print_value_inline(vm->stack[i]);
        printf(" ");
    }
    printf("]\n");
}

