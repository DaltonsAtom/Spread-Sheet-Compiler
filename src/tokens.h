#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>

// --- Token Types Enum ---
// Defines all possible token types our lexer can produce.
typedef enum {
    // --- Operators ---
    TOKEN_PLUS,       // +
    TOKEN_MINUS,      // -
    TOKEN_MULTIPLY,   // *
    TOKEN_DIVIDE,     // /
    TOKEN_POWER,      // ^

    // --- Delimiters ---
    TOKEN_LPAREN,     // (
    TOKEN_RPAREN,     // )
    TOKEN_COMMA,      // ,
    TOKEN_COLON,      // :
    TOKEN_EQUALS,     // =

    // --- Comparison ---
    TOKEN_GT,         // >
    TOKEN_LT,         // <
    TOKEN_GTE,        // >=
    TOKEN_LTE,        // <=
    TOKEN_NE,         // <> (Not Equals)

    // --- Literals ---
    TOKEN_NUMBER,     // 123 or 45.67
    TOKEN_STRING,     // "hello"
    TOKEN_CELL_REF,   // A1, B22
    TOKEN_RANGE,      // A1:B10

    // --- Functions ---
    TOKEN_FUNC_SUM,
    TOKEN_FUNC_AVERAGE,
    TOKEN_FUNC_MIN,
    TOKEN_FUNC_MAX,
    TOKEN_FUNC_IF,
    TOKEN_FUNC_AND,
    TOKEN_FUNC_OR,
    TOKEN_FUNC_NOT,

    // --- Special ---
    TOKEN_EOF,        // End of File/Input
    TOKEN_ERROR       // Lexical error
} TokenType;

// --- Token Value Union ---
// A token's value can be of different types.
// A union is memory-efficient for this.
typedef union {
    double num;     // For TOKEN_NUMBER
    char* str;      // For TOKEN_STRING, TOKEN_CELL_REF, TOKEN_RANGE, TOKEN_ERROR
} TokenValue;

// --- Token Structure ---
// This is the main structure our lexer (yylex) will return.
// It will be the type for yylval.
typedef struct {
    TokenType type;
    TokenValue value;
    int line;
    int col;
} Token;

// --- Debugging Function ---
/*
 * * MODIFICATION:
 * * The function definition has been MOVED to test_lexer.c
 * * (and will later be in a shared .c file).
 * *
 * * This header now only contains the DECLARATION.
 * * This prevents "multiple definition" linker errors when
 * * multiple .c files include this header.
 */
const char* token_type_to_string(TokenType type);

#endif // TOKENS_H

