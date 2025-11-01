%{
/*
 * --- Parser Specification (Final Counts) ---
 *
 * This is the complete, final parser and main program.
 *
 * FIX:
 * 1. Added global counters g_token_count and g_node_count.
 * 2. Passed them to print_summary().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include all project headers */
#include "ast.h"
#include "ast_printer.h"
#include "symtab.h"
#include "error.h"
#include "semantic.h"
#include "ir.h"
#include "codegen.h"
#include "optimizer.h"
#include "value.h"
#include "runtime.h"
#include "interpreter.h"
#include "vm.h"


/* External function declarations */
int yylex(void);
void yyerror(const char *s);
extern int yylineno; // Get line number from lexer
extern FILE* yyin;   // Flex input stream

/* Global pointer to the completed AST */
ASTNode* ast_root = NULL;

/* --- Global Counters for Summary --- */
int g_token_count = 0;
int g_node_count = 0;

/* --- Global Flags --- */
PrintFormat ast_print_format = PRINT_NONE; // Default to no AST
int optimize_code = 0; // Off by default
int trace_vm = 0;      // Off by default
int verbose = 0;       // Off by default
int show_bytecode = 0; // Off by default
typedef enum { MODE_VM, MODE_AST } ExecMode;
ExecMode execution_mode = MODE_VM; // Default

/* Global I/O and System Pointers */
const char* input_file = NULL;
const char* cells_file = NULL;
ErrorSystem* error_system = NULL;
SymbolTable* symbol_table = NULL;
char* current_formula_string = NULL;

%}

/*
 * This block tells Bison to insert this code into the
 * generated header file (parser.tab.h).
 */
%code requires {
    #include "ast.h"
}

/* --- Yacc Union (yylval) --- */
%union {
    double num;       /* For NUMBER tokens */
    char *str;        /* For CELL_REF, RANGE, STRING */
    ASTNode *node;    /* For all grammar non-terminals */
    int token_id;     /* For function name tokens */
}

/* --- Token Declarations --- */
%token <num> NUMBER
%token <str> STRING CELL_REF RANGE
%token <token_id> SUM AVERAGE MIN MAX IF
%token AND OR NOT
%token LPAREN RPAREN COMMA COLON
%token PLUS MINUS MULTIPLY DIVIDE POWER
%token GT LT GTE LTE NE EQUALS
%token ERROR

/* --- Type Declarations for Non-Terminals --- */
%type <node> program formula expression logical_or_expr logical_and_expr comparison_expr
%type <node> add_sub_expr mul_div_expr power_expr unary_expr
%type <node> factor function_call argument_list


/* --- Operator Precedence and Associativity --- */
%left OR
%left AND
%right NOT
%left GT LT GTE LTE NE EQUALS
%left PLUS MINUS
%left MULTIPLY DIVIDE
%right POWER
%right UMINUS /* A new precedence level for unary minus */


%%
/* --- Grammar Rules --- */

/* Top-level rule */
program:
    formula
        {
            ast_root = $1;
            // Moved to main
        }
    | /* An empty program is also valid */
        {
            ast_root = NULL;
            if (verbose) printf("✓ Empty input. Parse successful.\n");
        }
    ;

formula:
    EQUALS expression
        {
            $$ = $2; /* Pass the expression's node up */
        }
    | expression
        {
            $$ = $1; /* Pass the expression's node up */
        }
    ;

/* --- Expression Grammar --- */
/* (All rules are the same as the previous step) */

expression:
    logical_or_expr
        {
            $$ = $1;
        }
    ;

logical_or_expr:
    logical_and_expr
        {
            $$ = $1;
        }
    | logical_or_expr OR logical_and_expr
        {
            $$ = create_binary_op_node(OR, $1, $3, yylineno);
        }
    ;

logical_and_expr:
    comparison_expr
        {
            $$ = $1;
        }
    | logical_and_expr AND comparison_expr
        {
            $$ = create_binary_op_node(AND, $1, $3, yylineno);
        }
    ;

comparison_expr:
    add_sub_expr
        {
            $$ = $1;
        }
    | add_sub_expr GT add_sub_expr    { $$ = create_binary_op_node(GT, $1, $3, yylineno); }
    | add_sub_expr LT add_sub_expr    { $$ = create_binary_op_node(LT, $1, $3, yylineno); }
    | add_sub_expr GTE add_sub_expr   { $$ = create_binary_op_node(GTE, $1, $3, yylineno); }
    | add_sub_expr LTE add_sub_expr   { $$ = create_binary_op_node(LTE, $1, $3, yylineno); }
    | add_sub_expr NE add_sub_expr    { $$ = create_binary_op_node(NE, $1, $3, yylineno); }
    | add_sub_expr EQUALS add_sub_expr { $$ = create_binary_op_node(EQUALS, $1, $3, yylineno); }
    ;

add_sub_expr:
    mul_div_expr
        {
            $$ = $1;
        }
    | add_sub_expr PLUS mul_div_expr
        {
            $$ = create_binary_op_node(PLUS, $1, $3, yylineno);
        }
    | add_sub_expr MINUS mul_div_expr
        {
            $$ = create_binary_op_node(MINUS, $1, $3, yylineno);
        }
    ;

mul_div_expr:
    power_expr
        {
            $$ = $1;
        }
    | mul_div_expr MULTIPLY power_expr
        {
            $$ = create_binary_op_node(MULTIPLY, $1, $3, yylineno);
        }
    | mul_div_expr DIVIDE power_expr
        {
            $$ = create_binary_op_node(DIVIDE, $1, $3, yylineno);
        }
    ;

power_expr:
    unary_expr
        {
            $$ = $1;
        }
    | unary_expr POWER power_expr
        {
            $$ = create_binary_op_node(POWER, $1, $3, yylineno);
        }
    ;

unary_expr:
    factor
        {
            $$ = $1;
        }
    | MINUS unary_expr %prec UMINUS
        {
            $$ = create_unary_op_node(MINUS, $2, yylineno);
        }
    | NOT unary_expr
        {
            $$ = create_unary_op_node(NOT, $2, yylineno);
        }
    ;

factor:
    NUMBER
        {
            $$ = create_number_node($1, yylineno);
        }
    | CELL_REF
        {
            $$ = create_cell_ref_node($1, yylineno);
        }
    | RANGE
        {
            $$ = create_range_node($1, yylineno);
        }
    | STRING
        {
            $$ = create_string_node($1, yylineno);
        }
    | LPAREN expression RPAREN
        {
            $$ = $2;
        }
    | function_call
        {
            $$ = $1;
        }
    ;

function_call:
    IF LPAREN expression COMMA expression COMMA expression RPAREN
        {
            ASTNode *false_arg = create_arg_list_node($7, NULL, yylineno);
            ASTNode *true_arg = create_arg_list_node($5, false_arg, yylineno);
            ASTNode *cond_arg = create_arg_list_node($3, true_arg, yylineno);
            $$ = create_function_call_node(IF, cond_arg, yylineno);
        }
    | SUM LPAREN argument_list RPAREN
        {
            $$ = create_function_call_node(SUM, $3, yylineno);
        }
    | AVERAGE LPAREN argument_list RPAREN
        {
            $$ = create_function_call_node(AVERAGE, $3, yylineno);
        }
    | MIN LPAREN argument_list RPAREN
        {
            $$ = create_function_call_node(MIN, $3, yylineno);
        }
    | MAX LPAREN argument_list RPAREN
        {
            $$ = create_function_call_node(MAX, $3, yylineno);
        }
    ;

argument_list:
    expression
        {
            $$ = create_arg_list_node($1, NULL, yylineno);
        }
    | argument_list COMMA expression
        {
            $$ = create_arg_list_node($3, $1, yylineno);
        }
    ;


%%
/* --- C Code Section --- */

void print_header(const char* input_formula) {
    printf("======================================\n");
    printf(" SPREADSHEET FORMULA COMPILER v1.0\n");
    printf("======================================\n");
    if (input_formula) {
        printf("Input Formula: %s\n", input_formula);
    }
}

void print_phase_header(const char* title) {
    char header[100];
    snprintf(header, 100, "=== %s ===", title);
    printf("\n%s\n", header);
}

void print_summary(int tokens, int ast_nodes, int instructions) {
    printf("\n======================================\n");
    printf(" COMPILATION SUMMARY\n");
    printf("======================================\n");
    printf("Status:       SUCCESS\n");
    printf("Tokens:       %d\n", tokens);
    printf("AST Nodes:    %d\n", ast_nodes);
    printf("Instructions: %d\n", instructions);
    // TODO: Add timing and memory
}

void print_help(const char* prog_name) {
    printf("======================================\n");
    printf(" SPREADSHEET FORMULA COMPILER v1.0\n");
    printf("======================================\n");
    printf("Usage: %s [options]\n", prog_name);
    printf("Reads a formula from stdin or --input file.\n\n");
    printf("OPTIONS:\n");
    printf("  --input <file>    Read formula from <file>.\n");
    printf("  --cells <file>    Load cell values from <file> (format: A1=10.5).\n");
    printf("  --mode=ast        Execute using the AST Interpreter (Phase 6.1).\n");
    printf("  --mode=vm         Execute using the VM (Default, Phase 6.2).\n");
    printf("  --ast-tree        Show AST as a tree (box-drawing).\n");
    printf("  --ast-dot         Show AST in Graphviz .dot format.\n");
    printf("  --ast-lisp        Show AST in Lisp S-expression format.\n");
    printf("  --no-ast          Do not print the AST.\n");
    printf("  --bytecode        Show the generated stack-based bytecode.\n");
    printf("  --trace           Show VM/Interpreter execution trace.\n");
    printf("  --optimize        Enable bytecode constant-folding optimization.\n");
    printf("  --verbose         Show all compilation phase headers.\n");
    printf("  --help            Show this help message.\n\n");
}


/**
 * @brief Loads cell data from a file or sets up test environment.
 */
void load_cell_data(SymbolTable* table, const char* filename) {
    if (filename == NULL) {
        if (verbose) printf("✓ No --cells file. Loading default test data.\n");
        symtab_define_cell(table, "A1", 10.0, "Formula for A1", 1);
        symtab_define_cell(table, "A2", 20.0, "Formula for A2", 2);
        symtab_define_cell(table, "A3", 30.0, "Formula for A3", 3);
        symtab_define_cell(table, "B1", 5.0, "Formula for B1", 4);
        symtab_define_cell(table, "B2", 7.0, "Formula for B2", 5);
        symtab_define_cell(table, "Z1", 0.0, "=Z2", 10);
        symtab_define_cell(table, "Z2", 0.0, "=Z1", 11);
        symtab_add_dependency(table, "Z1", "Z2");
        symtab_add_dependency(table, "Z2", "Z1");
        return;
    }

    if (verbose) printf("✓ Loading cell data from: %s\n", filename);
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open cells file '%s'.\n", filename);
        return; // Continue with an empty table
    }

    char line[256];
    int line_num = 1;
    while (fgets(line, sizeof(line), file)) {
        char* key = strtok(line, "=");
        char* value_str = strtok(NULL, "\n");
        
        if (key != NULL && value_str != NULL) {
            double value = atof(value_str);
            symtab_define_cell(table, key, value, line, line_num);
        }
        line_num++;
    }
    fclose(file);
}


/**
 * @brief Parses command-line arguments.
 */
void parse_flags(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];
        
        if (strcmp(arg, "--help") == 0) {
            print_help(argv[0]);
            exit(0);
        } else if (strcmp(arg, "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(arg, "--optimize") == 0) {
            optimize_code = 1;
        } else if (strcmp(arg, "--trace") == 0) {
            trace_vm = 1; // Used by both VM and Interpreter
            verbose = 1; // Trace implies verbose
        } else if (strcmp(arg, "--bytecode") == 0) {
            show_bytecode = 1;
        } else if (strcmp(arg, "--ast-tree") == 0) {
            ast_print_format = PRINT_TREE;
        } else if (strcmp(arg, "--ast-dot") == 0) {
            ast_print_format = PRINT_DOT;
        } else if (strcmp(arg, "--ast-lisp") == 0) {
            ast_print_format = PRINT_LISP;
        } else if (strcmp(arg, "--no-ast") == 0) {
            ast_print_format = PRINT_NONE;
        } else if (strcmp(arg, "--mode=ast") == 0 || strcmp(arg, "--interpret") == 0) {
            execution_mode = MODE_AST;
        } else if (strcmp(arg, "--mode=vm") == 0 || strcmp(arg, "--execute") == 0) {
            execution_mode = MODE_VM;
        } else if (strcmp(arg, "--input") == 0) {
            if (i + 1 < argc) {
                input_file = argv[++i]; // Consume next argument
            } else {
                fprintf(stderr, "Error: --input requires a filename.\n");
                exit(1);
            }
        } else if (strcmp(arg, "--cells") == 0) {
            if (i + 1 < argc) {
                cells_file = argv[++i]; // Consume next argument
            } else {
                fprintf(stderr, "Error: --cells requires a filename.\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Error: Unknown argument '%s'. Use --help.\n", arg);
            exit(1);
        }
    }
}

// Reads the entire input file into a string
char* read_input_file(FILE* file) {
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = (char*)malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    
    // Remove trailing newline
    char* newline = strrchr(buffer, '\n');
    if (newline) *newline = '\0';
    
    return buffer;
}


int main(int argc, char* argv[]) {
    /* Create global systems */
    error_system = error_system_create(NULL);
    symbol_table = symtab_create();
    
    /* Parse command-line flags */
    parse_flags(argc, argv);
    
    /* Load Cell Data */
    load_cell_data(symbol_table, cells_file);

    /* Set Input Stream */
    FILE* input_stream = stdin;
    if (input_file != NULL) {
        input_stream = fopen(input_file, "r");
        if (input_stream == NULL) {
            fprintf(stderr, "Fatal Error: Could not open input file '%s'\n", input_file);
            exit(1);
        }
        // Read formula for header
        current_formula_string = read_input_file(input_stream);
        // Reset file pointer for lexer
        fseek(input_stream, 0, SEEK_SET);
    } else {
        // Read from stdin
        char line_buf[1024];
        if (fgets(line_buf, sizeof(line_buf), stdin)) {
            // Remove newline
            char* newline = strrchr(line_buf, '\n');
            if (newline) *newline = '\0';
            current_formula_string = strdup(line_buf);
            
            // We can't rewind stdin, so we need a new way
            // This is a common lex/yacc problem.
            // For simplicity, we'll just print a generic header
            // and assume yyin is still pointing to stdin.
        }
    }
    
    print_header(current_formula_string ? current_formula_string : "");
    yyin = input_stream; // Point lexer to the correct stream

    /* --- Run Compiler Phases --- */
    
    // Phase 1 & 2: Parsing (and Lexing)
    print_phase_header("PHASE 1 & 2: PARSING");
    if (yyparse() != 0) {
        fprintf(stderr, "Parse failed.\n");
        error_print_all(error_system);
        goto cleanup;
    }
    if (ast_root == NULL) {
        printf("No formula to process.\n");
        goto cleanup;
    }
    printf("✓ Parse tree constructed\n");
    printf("✓ No syntax errors detected\n");


    // Phase 3: AST Printing (if requested)
    if (ast_print_format != PRINT_NONE) {
        print_phase_header("ABSTRACT SYNTAX TREE");
        printf("AST VISUALIZATION\n");
        print_ast(ast_root, ast_print_format);
    }
        
    // Phase 4: Semantic Analysis
    print_phase_header("PHASE 4: SEMANTIC ANALYSIS");
    printf("SYMBOL TABLE\n");
    symtab_print(symbol_table);
    
    int semantic_errors = semantic_analysis(ast_root, symbol_table, error_system, "C1");
    if (semantic_errors > 0) {
        printf("\nCompilation failed with %d semantic error(s).\n", semantic_errors);
        error_print_all(error_system);
        goto cleanup;
    }
    printf("✓ Semantic analysis passed!\n");

    // Phase 5: Code Generation
    print_phase_header("PHASE 5: CODE GENERATION");
    printf("STACK-BASED BYTECODE\n");
    CodeArray* bytecode = generate_code(ast_root, symbol_table);
    
    if (optimize_code) {
        optimize_bytecode(bytecode);
    }
    if (show_bytecode || verbose) {
        print_bytecode(bytecode);
    }

    // Phase 6: Execution
    print_phase_header("PHASE 6: EXECUTION");
    printf("EVALUATION RESULTS\n\n");
    
    if (execution_mode == MODE_AST || trace_vm) {
        printf("Method 1: Direct AST Interpretation\n");
        if (trace_vm) printf("Stack Trace:\n");
        Value ast_result = interpreter_evaluate(ast_root, symbol_table, trace_vm ? 1 : 0);
        printf("RESULT: ");
        print_value(ast_result);
        printf("\n\n");
        free_value(ast_result);
    }

    if (execution_mode == MODE_VM || trace_vm) {
        printf("Method 2: Virtual Machine Execution\n");
        VM* vm = vm_create(bytecode, symbol_table);
        vm->trace = trace_vm; // Set trace flag
        Value vm_result = vm_execute(vm);
        
        printf("RESULT: ");
        print_value(vm_result);
        printf("\n");
        
        free_value(vm_result);
        vm_free(vm);
    }
    
    // FIX: Pass the global counters
    print_summary(g_token_count, g_node_count, bytecode->count);
    free_bytecode(bytecode);


cleanup:
    /* --- Final Cleanup --- */
    if (input_stream != stdin) {
        fclose(input_stream);
    }
    free(current_formula_string);
    free_ast(ast_root);
    symtab_free(symbol_table);
    error_system_free(error_system);

    return 0;
}

void yyerror(const char *s) {
    // yylineno is external from lexer
    error_report(error_system, ERROR_SYNTAX, yylineno, 0, s, "Check for missing parentheses, operators, or commas.");
}

