/*
 * --- Intermediate Representation Header (Prompt 5.1) ---
 *
 * Defines the stack-based bytecode instruction set.
 *
 * FIX:
 * 1. Added 'func_call' struct to the union.
 * 2. Added 'print_instruction' function prototype.
 */

#ifndef IR_H
#define IR_H

#include <stdlib.h>

/* --- OpCodes --- */
typedef enum {
    OP_HALT,        // Stop execution
    OP_PUSH,        // Push constant number
    OP_PUSH_CELL,   // Push cell value
    OP_PUSH_RANGE,  // Push a string literal for a range (e.g., "A1:B10")
    
    // Binary Ops
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_POW,
    OP_EQ,          // Equals ==
    OP_NEQ,         // Not Equals !=
    OP_GT,
    OP_LT,
    OP_GTE,
    OP_LTE,
    OP_AND,
    OP_OR,

    // Unary Ops
    OP_NEG,         // Unary minus
    OP_NOT,         // Logical not
    
    // Control Flow
    OP_JMP,         // Unconditional jump
    OP_JMP_IF_FALSE,// Pop stack, jump if false
    
    // Functions
    OP_CALL,        // Call a built-in function
    
    // Optimizer
    OP_NOP          // No-operation
    
} OpCode;

/* --- Instruction Operand --- */

// Struct to hold function call info
typedef struct {
    int token;      // e.g., SUM, AVERAGE
    int arg_count;
} FuncCallInfo;


// An instruction is an opcode + an optional operand
typedef struct {
    OpCode opcode;
    int line; // For debugging
    
    union {
        double number;
        char *cell_ref; // For PUSH_CELL (e.g., "A1")
        char *range_str; // For PUSH_RANGE (e.g., "A1:B10")
        int address;    // For JMP targets
        FuncCallInfo func_call; // For OP_CALL
    } operand;
    
} Instruction;


/* --- Code Array (Chunk) --- */
typedef struct {
    Instruction *code;
    int capacity;
    int count;
} CodeArray;


/* --- Public Functions --- */

CodeArray* create_code_array();
void free_bytecode(CodeArray *code);

// Emitter functions
int emit_op(CodeArray* code, OpCode opcode, int line);
int emit_push(CodeArray* code, double number, int line);
int emit_push_cell(CodeArray* code, char* cell_ref, int line);
int emit_push_range(CodeArray* code, char* range_str, int line);
int emit_jump(CodeArray* code, OpCode opcode, int line);
int emit_call(CodeArray* code, int func_token, int arg_count, int line);
void patch_jump(CodeArray* code, int jump_instruction_index);

// Debugging
void print_bytecode(CodeArray* code);
void print_instruction(Instruction instruction, int index); // FIX: Added prototype


#endif // IR_H

