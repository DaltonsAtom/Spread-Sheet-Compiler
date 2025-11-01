#include "optimizer.h"
#include <stdio.h>

/*
 * --- Constant Folding ---
 *
 * Looks for patterns like:
 * PUSH <num1>
 * PUSH <num2>
 * <OP>
 * And replaces them with:
 * PUSH <result>
 * NOP
 * NOP
 */
static void fold_constants(CodeArray* code) {
    int instructions_folded = 0;
    for (int i = 0; i < code->count - 2; i++) {
        Instruction* inst1 = &code->code[i];
        Instruction* inst2 = &code->code[i+1];
        Instruction* inst3 = &code->code[i+2];

        // Look for PUSH, PUSH, OP
        if (inst1->opcode == OP_PUSH && inst2->opcode == OP_PUSH) {
            double num1 = inst1->operand.number;
            double num2 = inst2->operand.number;
            double result = 0.0;
            OpCode op = inst3->opcode;

            int folded = 1; // Assume we'll fold it

            switch (op) {
                case OP_ADD: result = num1 + num2; break;
                case OP_SUB: result = num1 - num2; break;
                case OP_MUL: result = num1 * num2; break;
                case OP_DIV: 
                    if (num2 != 0) {
                        result = num1 / num2; 
                    } else {
                        folded = 0; // Don't fold division by zero
                    }
                    break;
                // We could add more ops (GT, LT, etc.)
                default:
                    folded = 0; // Not an op we can fold
                    break;
            }

            if (folded) {
                // Success! Replace the pattern.
                inst1->opcode = OP_PUSH;
                inst1->operand.number = result;
                
                inst2->opcode = OP_NOP;
                inst3->opcode = OP_NOP;
                
                instructions_folded += 2;
                i += 2; // Skip the instructions we just NOP'd
            }
        }
    }
    if (instructions_folded > 0) {
        printf("Optimizer: Constant folding pass complete. %d instructions folded.\n", instructions_folded);
    }
}


/* --- Public API --- */

void optimize_bytecode(CodeArray* code) {
    if (code == NULL) return;
    
    printf("Running Optimizer...\n");
    
    // We can add more optimization passes here
    fold_constants(code);
    
    // ...
}

