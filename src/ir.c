/*
 * --- Intermediate Representation Implementation ---
 *
 * Implements the functions for creating, freeing,
 * and emitting instructions into a CodeArray.
 *
 * FIX:
 * 1. Implemented all 'emit_...' functions.
 * 2. Added 'line' parameter to all emitters.
 * 3. Used the correct 'func_call' union member.
 */

#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h" // For token names

/* --- Private Helper --- */

// Gets the string name for a function token
static const char* get_func_name(int func_token) {
    switch(func_token) {
        case SUM:     return "SUM";
        case AVERAGE: return "AVERAGE";
        case MIN:     return "MIN";
        case MAX:     return "MAX";
        case IF:      return "IF";
        default:      return "UNKNOWN_FUNC";
    }
}

// Resizes the instruction array
static void resize_code_array(CodeArray* code) {
    int old_cap = code->capacity;
    code->capacity = old_cap < 8 ? 8 : old_cap * 2;
    code->code = (Instruction*)realloc(code->code, sizeof(Instruction) * code->capacity);
    if (code->code == NULL) {
        fprintf(stderr, "Fatal: Out of memory resizing code array\n");
        exit(1);
    }
}

// Writes a single instruction and returns its index
static int write_instruction(CodeArray* code, Instruction inst) {
    if (code->count >= code->capacity) {
        resize_code_array(code);
    }
    code->code[code->count] = inst;
    return code->count++; // Return index of this new instruction
}


/* --- Public API --- */

CodeArray* create_code_array() {
    CodeArray* code = (CodeArray*)malloc(sizeof(CodeArray));
    code->capacity = 0;
    code->count = 0;
    code->code = NULL;
    resize_code_array(code); // Initialize with default capacity
    return code;
}

void free_bytecode(CodeArray *code) {
    if (code == NULL) return;
    
    // Free any heap-allocated strings inside instructions
    for (int i = 0; i < code->count; i++) {
        OpCode op = code->code[i].opcode;
        if (op == OP_PUSH_CELL || op == OP_PUSH_RANGE) {
            // Note: We don't free here, as the string is
            // owned by the AST, which is freed separately.
        }
    }
    
    free(code->code);
    free(code);
}

/* --- Emitter Functions --- */

int emit_op(CodeArray* code, OpCode opcode, int line) {
    Instruction inst;
    inst.opcode = opcode;
    inst.line = line;
    return write_instruction(code, inst);
}

int emit_push(CodeArray* code, double number, int line) {
    Instruction inst;
    inst.opcode = OP_PUSH;
    inst.line = line;
    inst.operand.number = number;
    return write_instruction(code, inst);
}

int emit_push_cell(CodeArray* code, char* cell_ref, int line) {
    Instruction inst;
    inst.opcode = OP_PUSH_CELL;
    inst.line = line;
    inst.operand.cell_ref = cell_ref;
    return write_instruction(code, inst);
}

int emit_push_range(CodeArray* code, char* range_str, int line) {
    Instruction inst;
    inst.opcode = OP_PUSH_RANGE;
    inst.line = line;
    inst.operand.range_str = range_str;
    return write_instruction(code, inst);
}

int emit_jump(CodeArray* code, OpCode opcode, int line) {
    Instruction inst;
    inst.opcode = opcode;
    inst.line = line;
    inst.operand.address = -1; // Placeholder, to be patched
    return write_instruction(code, inst);
}

int emit_call(CodeArray* code, int func_token, int arg_count, int line) {
    Instruction inst;
    inst.opcode = OP_CALL;
    inst.line = line;
    // FIX: Use the 'func_call' member
    inst.operand.func_call.token = func_token;
    inst.operand.func_call.arg_count = arg_count;
    return write_instruction(code, inst);
}

void patch_jump(CodeArray* code, int jump_instruction_index) {
    if (jump_instruction_index < 0 || jump_instruction_index >= code->count) {
        fprintf(stderr, "Error: Invalid jump index to patch.\n");
        return;
    }
    // Set the jump target to the *next* instruction's address
    int target_address = code->count;
    code->code[jump_instruction_index].operand.address = target_address;
}


/* --- Debugging --- */

void print_instruction(Instruction inst, int index) {
    printf("%04d: ", index);
    switch(inst.opcode) {
        case OP_HALT:         printf("HALT\n"); break;
        case OP_PUSH:         printf("PUSH %f\n", inst.operand.number); break;
        case OP_PUSH_CELL:    printf("PUSH_CELL %s\n", inst.operand.cell_ref); break;
        case OP_PUSH_RANGE:   printf("PUSH_RANGE %s\n", inst.operand.range_str); break;
        case OP_ADD:          printf("ADD\n"); break;
        case OP_SUB:          printf("SUB\n"); break;
        case OP_MUL:          printf("MUL\n"); break;
        case OP_DIV:          printf("DIV\n"); break;
        case OP_POW:          printf("POW\n"); break;
        case OP_EQ:           printf("EQ\n"); break;
        case OP_NEQ:          printf("NEQ\n"); break;
        case OP_GT:           printf("GT\n"); break;
        case OP_LT:           printf("LT\n"); break;
        case OP_GTE:          printf("GTE\n"); break;
        case OP_LTE:          printf("LTE\n"); break;
        case OP_AND:          printf("AND\n"); break;
        case OP_OR:           printf("OR\n"); break;
        case OP_NEG:          printf("NEG\n"); break;
        case OP_NOT:          printf("NOT\n"); break;
        case OP_JMP:          printf("JMP -> %d\n", inst.operand.address); break;
        case OP_JMP_IF_FALSE: printf("JMP_IF_FALSE -> %d\n", inst.operand.address); break;
        case OP_CALL:
            printf("CALL %s (Args: %d)\n",
                // FIX: Use the 'func_call' member
                get_func_name(inst.operand.func_call.token),
                inst.operand.func_call.arg_count);
            break;
        case OP_NOP:
            printf("NOP\n");
            break;
        default:
            printf("UNKNOWN (0x%X)\n", inst.opcode);
            break;
    }
}

void print_bytecode(CodeArray* code) {
    printf("--- Bytecode ---\n");
    for (int i = 0; i < code->count; i++) {
        print_instruction(code->code[i], i);
    }
    printf("----------------\n");
}

