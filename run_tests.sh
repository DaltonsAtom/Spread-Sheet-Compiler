#!/bin/bash

# --- Test Runner (Prompt 7.2) ---
#
# This script finds all 'test_*.txt' files in the 'tests/' subdirectories,
# runs the compiler against them, and compares the output to the
# corresponding '.expected' file.

COMPILER="./bin/compiler"
TEST_DIR="tests"
CELL_FILE="tests/cells/base_cells.txt" # Base data for most tests

# Colors
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
NC="\033[0m" # No Color

# Counters
total_tests=0
passed_tests=0

# Ensure compiler exists
if [ ! -f "$COMPILER" ]; then
    echo -e "${RED}Compiler not found at '$COMPILER'.${NC}"
    echo "Please run 'make' first."
    exit 1
fi

# Create a base cell file for tests
mkdir -p tests/cells
cat > "$CELL_FILE" << EOL
A1=10
A2=20
A3=30
Z1=0
Z2=0
EOL

echo "======================================"
echo " SPREADSHEET COMPILER TEST SUITE"
echo "======================================"

# Find all test input files
while read -r test_file; do
    total_tests=$((total_tests + 1))
    
    base_name="${test_file%.txt}"
    expected_file="${base_name}.expected"
    actual_file="${base_name}.actual"
    
    category=$(dirname "$test_file" | xargs basename)
    test_name=$(basename "$test_file")

    if [ ! -f "$expected_file" ]; then
        echo -e "${YELLOW}[SKIP]${NC} No expected file for $test_name"
        continue
    fi
    
    printf "Running test [%-10s] %-25s ... " "$category" "$test_name"
    
    # --- Run the Compiler ---
    
    # Special case for the "arithmetic" test to show full verbose output
    if [[ "$test_file" == *"syntax/test_arithmetic.txt"* ]]; then
        "$COMPILER" --input "$test_file" --cells "$CELL_FILE" --verbose --ast-tree --bytecode --trace > "$actual_file" 2>&1
    else
        # Default run: minimal output
        "$COMPILER" --input "$test_file" --cells "$CELL_FILE" --no-ast 2>&1 \
            | grep -vE "^(Setting up|Parsing formula|✓|Running semantic analysis|Compilation successful|---)" \
            | sed 's/^[[:space:]]*//;s/[[:space:]]*$//' \
            > "$actual_file"
    fi
        
    # --- Compare Output ---
    if diff -w "$actual_file" "$expected_file" > /dev/null; then
        passed_tests=$((passed_tests + 1))
        echo -e "${GREEN}[PASS]${NC}"
        rm "$actual_file" # Clean up on pass
    else
        echo -e "${RED}[FAIL]${NC}"
        echo "  - Expected output in: $expected_file"
        echo "  - Actual output in:   $actual_file"
    fi
done < <(find "$TEST_DIR" -name "test_*.txt")

echo "--------------------------------------"
echo " TEST SUMMARY"
echo "--------------------------------------"
if [ $total_tests -eq 0 ]; then
    echo -e "${YELLOW}No tests found.${NC}"
elif [ $passed_tests -eq $total_tests ]; then
    echo -e "${GREEN}All $total_tests tests passed!${NC}"
else
    echo -e "${RED}FAIL: $passed_tests / $total_tests tests passed.${NC}"
fi
echo "======================================"

