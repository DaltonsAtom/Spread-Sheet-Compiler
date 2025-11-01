# Spreadsheet Formula Grammar (BNF)

This document defines the formal grammar for the spreadsheet formula compiler, as required by Prompt 2.1.

## 1. Token Review

The grammar is built using these tokens (defined in Phase 1):

* **Literals:** `NUMBER`, `STRING`, `CELL_REF`, `RANGE`
* **Operators:** `PLUS`, `MINUS`, `MULTIPLY`, `DIVIDE`, `POWER`
* **Comparison:** `GT`, `LT`, `GTE`, `LTE`, `NE`, `EQUALS`
* **Logical:** `AND`, `OR`, `NOT`
* **Functions:** `IF`, `SUM`, `AVERAGE`, `MIN`, `MAX`
* **Delimiters:** `LPAREN`, `RPAREN`, `COMMA`, `COLON`

## 2. Operator Precedence & Associativity

This table defines the order of operations, from highest to lowest precedence.

| Precedence | Operator(s)        | Description              | Associativity   |
| ---------- | ------------------ | ------------------------ | --------------- |
| 7 (High)   | `( )`            | Parentheses (Grouping)   | N/A             |
| 6          | `^`(POWER)       | Exponentiation           | **Right** |
| 5          | `* /`            | Multiplication, Division | Left            |
| 4          | `+ -`            | Addition, Subtraction    | Left            |
| 3          | `> < >= <= <> =` | Comparison               | Left            |
| 2          | `NOT`            | Logical NOT              | Right           |
| 1          | `AND`            | Logical AND              | Left            |
| 0 (Low)    | `OR`             | Logical OR               | Left            |

## 3. Formal BNF Grammar

Here is the complete grammar in Backus-Naur Form (BNF).

```
<program> ::= <formula>
          | /* empty */

<formula> ::= "=" <expression>
            | <expression>

<expression> ::= <logical_or_expr>

<logical_or_expr> ::= <logical_and_expr>
                    | <logical_or_expr> "OR" <logical_and_expr>

<logical_and_expr> ::= <comparison_expr>
                     | <logical_and_expr> "AND" <comparison_expr>

<comparison_expr> ::= <add_sub_expr>
                    | <add_sub_expr> ">" <add_sub_expr>
                    | <add_sub_expr> "<" <add_sub_expr>
                    | <add_sub_expr> ">=" <add_sub_expr>
                    | <add_sub_expr> "<=" <add_sub_expr>
                    | <add_sub_expr> "<>" <add_sub_expr>
                    | <add_sub_expr> "=" <add_sub_expr>

<add_sub_expr> ::= <mul_div_expr>
                 | <add_sub_expr> "+" <mul_div_expr>
                 | <add_sub_expr> "-" <mul_div_expr>

<mul_div_expr> ::= <power_expr>
                 | <mul_div_expr> "*" <power_expr>
                 | <mul_div_expr> "/" <power_expr>

<power_expr> ::= <unary_expr>
               | <unary_expr> "^" <power_expr>  /* Right-associative */

<unary_expr> ::= <factor>
               | "-" <unary_expr> /* Unary minus */
               | "NOT" <unary_expr> /* Logical NOT */

<factor> ::= NUMBER
           | STRING
           | CELL_REF
           | RANGE
           | <function_call>
           | "(" <expression> ")"

<function_call> ::= ("SUM" | "AVERAGE" | "MIN" | "MAX") "(" <argument_list> ")"
                  | "IF" "(" <expression> "," <expression> "," <expression> ")"

<argument_list> ::= <expression>
                  | <argument_list> "," <expression>
```

## 4. Ambiguity and Resolution

1. **Operator Precedence:** Ambiguity like `A1 + B1 * C1` is resolved by splitting the grammar into separate non-terminals for each precedence level (e.g., `<add_sub_expr>`, `<mul_div_expr>`). `*` is handled by `<mul_div_expr>` first, which is nested inside `<add_sub_expr>`, forcing `*` to bind tighter than `+`.
2. **Operator Associativity:**
   * **Left-associativity** (for `+`, `-`, `*`, `/`, etc.) is handled by left-recursion.
     `e.g., <add_sub_expr> ::= <add_sub_expr> "+" <mul_div_expr>`
   * **Right-associativity** (for `^`) is handled by right-recursion.
     `e.g., <power_expr> ::= <unary_expr> "^" <power_expr>`
3. **Dangling Else:** The classic `IF` ambiguity isn't present here because our `IF` function requires a full `IF(...)` structure with parentheses and commas, making its arguments explicit.
4. **Unary Minus:** The ambiguity between binary minus (`A - B`) and unary minus (`-B`) is resolved by giving unary minus a high precedence, placing it in `<unary_expr>` near the `<factor>` level.

## 5. Example Derivations

### Example 1: `= (A1 + 10) * B2`

1. `<program>` -> `<formula>`
2. `<formula>` -> "=" `<expression>`
3. `<expression>` -> `<logical_or_expr>` -> `<logical_and_expr>` -> `<comparison_expr>` -> `<add_sub_expr>`
4. `<add_sub_expr>` -> `<mul_div_expr>`
5. `<mul_div_expr>` -> `<mul_div_expr> * <power_expr>`
6. `<mul_div_expr>` -> `<power_expr>` (for the left side)
7. `<power_expr>` -> `<unary_expr>` -> `<factor>`
8. `<factor>` -> `(` `<expression>` `)` (This is `(A1 + 10)`)
   * `<expression>` -> ... -> `<add_sub_expr> + <mul_div_expr>`
   * `<add_sub_expr>` -> ... -> `<factor>` -> `CELL_REF` (A1)
   * `<mul_div_expr>` -> ... -> `<factor>` -> `NUMBER` (10)
9. `*` (The operator from step 5)
10. `<power_expr>` -> `<unary_expr>` -> `<factor>`
11. `<factor>` -> `CELL_REF` (This is `B2`)

**Result:** `( (A1 + 10) * B2 )`

### Example 2: `= IF(A1 > B1, 10, 20)`

1. `<program>` -> `<formula>` -> "=" `<expression>`
2. `<expression>` -> ... -> `<factor>`
3. `<factor>` -> `<function_call>`
4. `<function_call>` -> "IF" `(` `<expression>` `,` `<expression>` `,` `<expression>` `)`
5. **Arg 1 (Condition):** `<expression>` -> ... -> `<comparison_expr>`
   * `<comparison_expr>` -> `<add_sub_expr> > <add_sub_expr>`
   * `<add_sub_expr>` -> ... -> `CELL_REF` (A1)
   * `<add_sub_expr>` -> ... -> `CELL_REF` (B1)
6. **Arg 2 (True-case):** `<expression>` -> ... -> `<factor>` -> `NUMBER` (10)
7. **Arg 3 (False-case):** `<expression>` -> ... -> `<factor>` -> `NUMBER` (20)

**Result:** `IF( (A1 > B1), 10, 20 )`
