%{
/*
 * --- Lexer Specification (Final) ---
 *
 * FIX: Added g_token_count and RETURN_TOKEN macro
 * to count all tokens that are processed.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

// Connect to global counter in parser.y
extern int g_token_count;

// Define a wrapper that counts and returns
#define RETURN_TOKEN(token) (g_token_count++, (token))

int yylex(void);
%}

/* --- Flex Options --- */
%option noyywrap
%option yylineno
%option noinput
%option nounput

/* --- Definitions --- */
DIGIT     [0-9]
LETTER    [A-Z]
WS        [ \t\n]+
NUMBER    ({DIGIT}+(\.{DIGIT}*)?|\.{DIGIT}+)
CELL_REF  {LETTER}{DIGIT}+
RANGE     {CELL_REF}:{CELL_REF}
STRING    \"([^"\\]|\\.)*\"

%%

{WS} {
    /* No action, just consume the input */
    /* Do not count whitespace */
    ;
}

{NUMBER} {
    yylval.num = atof(yytext);
    return RETURN_TOKEN(NUMBER);
}

{STRING} {
    yylval.str = strdup(yytext + 1);
    /* Remove the trailing quote */
    yylval.str[strlen(yylval.str) - 1] = '\0';
    /* TODO: Handle escaped quotes \" */
    return RETURN_TOKEN(STRING);
}

{RANGE} {
    yylval.str = strdup(yytext);
    return RETURN_TOKEN(RANGE);
}

{CELL_REF} {
    yylval.str = strdup(yytext);
    return RETURN_TOKEN(CELL_REF);
}


[sS][uU][mM]                 { return RETURN_TOKEN(SUM); }
[aA][vV][eE][rR][aA][gG][eE] { return RETURN_TOKEN(AVERAGE); }
[mM][iI][nN]                 { return RETURN_TOKEN(MIN); }
[mM][aA][xX]                 { return RETURN_TOKEN(MAX); }
[iI][fF]                     { return RETURN_TOKEN(IF); }
[aA][nN][dD]                 { return RETURN_TOKEN(AND); }
[oO][rR]                     { return RETURN_TOKEN(OR); }
[nN][oO][tT]                 { return RETURN_TOKEN(NOT); }


">=" { return RETURN_TOKEN(GTE); }
"<=" { return RETURN_TOKEN(LTE); }
"<>" { return RETURN_TOKEN(NE); }
">"  { return RETURN_TOKEN(GT); }
"<"  { return RETURN_TOKEN(LT); }


"+" { return RETURN_TOKEN(PLUS); }
"-" { return RETURN_TOKEN(MINUS); }
"*" { return RETURN_TOKEN(MULTIPLY); }
"/" { return RETURN_TOKEN(DIVIDE); }
"^" { return RETURN_TOKEN(POWER); }


"(" { return RETURN_TOKEN(LPAREN); }
")" { return RETURN_TOKEN(RPAREN); }
"=" { return RETURN_TOKEN(EQUALS); }
"," { return RETURN_TOKEN(COMMA); }
":" { return RETURN_TOKEN(COLON); }


. {
    fprintf(stderr, "Line %d: Unexpected character: %s\n", yylineno, yytext);
    yylval.str = strdup(yytext);
    return RETURN_TOKEN(ERROR);
}

%%
/* --- C Code Section --- */

