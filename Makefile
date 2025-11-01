#
# --- Makefile for Spreadsheet Compiler (FINAL) ---
#
# This Makefile compiles all phases:
# 1. Lexer (flex)
# 2. Parser (bison)
# 3. AST & Printer
# 4. Symbol Table
# 5. Error Reporting
# 6. Semantic Analysis
# 7. Code Generation (IR)
# 8. Optimizer
# 9. Runtime, Interpreter, and VM (Phase 6)
#

# --- Tools ---
CC = gcc
CFLAGS = -Wall -g -I$(OBJDIR) -I$(SRCDIR)
LDFLAGS = -lm # Link math library for pow()
LEX = flex
YACC = bison
YFLAGS = -d # Generate header file

# --- Directories ---
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# --- Files ---
EXECUTABLE = $(BINDIR)/compiler

# Find all .c source files in src/
# FIX: Explicitly list sources to avoid compiling old/test files
SOURCES = \
    $(SRCDIR)/ast_printer.c \
    $(SRCDIR)/codegen.c \
    $(SRCDIR)/error.c \
    $(SRCDIR)/ir.c \
    $(SRCDIR)/optimizer.c \
    $(SRCDIR)/semantic.c \
    $(SRCDIR)/symtab.c \
    $(SRCDIR)/runtime.c \
    $(SRCDIR)/interpreter.c \
    $(SRCDIR)/vm.c

# Generated files
LEX_GEN_C = $(OBJDIR)/lex.yy.c
YACC_GEN_C = $(OBJDIR)/parser.tab.c
YACC_GEN_H = $(OBJDIR)/parser.tab.h

# Object files
# Create a list of .o files in obj/ based on SOURCES
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))
LEX_OBJ = $(patsubst %.c, %.o, $(LEX_GEN_C))
YACC_OBJ = $(patsubst %.c, %.o, $(YACC_GEN_C))

# --- Targets ---

# Default target
all: $(EXECUTABLE)

# Main executable
$(EXECUTABLE): $(OBJECTS) $(LEX_OBJ) $(YACC_OBJ)
	@echo "Linking main compiler: $@"
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) $(YACC_OBJ) $(LEX_OBJ) -o $@ $(LDFLAGS)

# Rule to compile .c files from src/ into .o files in obj/
# FIX: Added dependency on $(YACC_GEN_H)
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(YACC_GEN_H)
	@echo "Compiling C: $<"
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Generated File Rules ---

# Rule to compile generated lexer
$(LEX_OBJ): $(LEX_GEN_C) $(YACC_GEN_H)
	@echo "Compiling (generated): $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile generated parser
$(YACC_OBJ): $(YACC_GEN_C) $(YACC_GEN_H)
	@echo "Compiling (generated): $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to run Flex (Lex)
$(LEX_GEN_C): $(SRCDIR)/lexer.l $(YACC_GEN_H)
	@echo "Running Flex (Lex)..."
	@mkdir -p $(OBJDIR)
	$(LEX) -o $@ $<

# Rule to run Bison (Yacc)
# This rule creates *both* .c and .h files
$(YACC_GEN_C) $(YACC_GEN_H): $(SRCDIR)/parser.y
	@echo "Running Bison (Yacc)..."
	@mkdir -p $(OBJDIR)
	# This syntax is compatible with older bison versions.
	$(YACC) $(YFLAGS) -o $(YACC_GEN_C) $<


# --- Testing ---
test: $(EXECUTABLE)
	@echo "\n--- Running Tests ---"
	@echo "\nTest: Basic Arithmetic (VM)"
	@echo "=5+3*2" | ./$(EXECUTABLE)
	@echo "\nTest: Basic Arithmetic (AST Interpreter)"
	@echo "=5+3*2" | ./$(EXECUTABLE) --mode=ast
	@echo "\nTest: Optimization (VM)"
	@echo "=5+3*2" | ./$(EXECUTABLE) --optimize
	@echo "\nTest: Function Call (SUM)"
	@echo "=SUM(A1:A3)" | ./$(EXECUTABLE)
	@echo "\nTest: Function Call (IF)"
	@echo "=IF(A1 > 50, 1, 99)" | ./$(EXECUTABLE)
	@echo "\nTest: Semantic Error (Undefined Cell)"
	@echo "=A1 + B99" | ./$(EXECUTABLE)
	@echo "\nTest: Semantic Error (Circular Dependency)"
	@echo "=Z1" | ./$(EXECUTABLE)
	@echo "\nTest: VM Trace"
	@echo "=1+2" | ./$(EXECUTABLE) --trace
	@echo "\n--- Tests Complete ---"


# --- Cleanup ---
clean:
	@echo "Cleaning up..."
	rm -rf $(OBJDIR)
	rm -f $(BINDIR)/$(EXECUTABLE)
	@echo "Cleanup complete."

# --- Phony Targets ---
.PHONY: all clean test

