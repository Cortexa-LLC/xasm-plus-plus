# BUG-003: Missing Expression Support (<, >, +, - operators)

**Priority:** P1 - HIGH
**Status:** Open
**Discovered:** 2026-02-12
**Impact:** Blocks pointer manipulation and multi-byte addressing

## Summary

Essential expression operators are not supported:
- `<` (low byte extraction)
- `>` (high byte extraction)
- `+` (addition/offset)
- `-` (subtraction)

These operators are fundamental to 6502 programming for 16-bit address manipulation.

## Description

6502 assembly requires splitting 16-bit addresses into low/high bytes and computing offsets. Standard syntax uses operators that xasm++ currently doesn't support.

## Minimal Reproduction

### Low Byte Expression

```assembly
* File: bug003_lo_repro.S
        .OP 65C02
        .OR $2000
        
MESSAGE .AS "Hello"

START   LDA #<MESSAGE    ; Get low byte of MESSAGE address
        RTS
```

```bash
$ ./build/bin/xasm++ --syntax scmasm --cpu 65c02 bug003_lo_repro.S -o output.bin

error: Invalid argument for #<MESSAGE: Unsupported instruction: #<MESSAGE
```

### High Byte Expression

```assembly
* File: bug003_hi_repro.S
        .OP 65C02
        .OR $2000
        
MESSAGE .AS "Hello"

START   LDA #>MESSAGE    ; Get high byte of MESSAGE address
        RTS
```

```bash
$ ./build/bin/xasm++ --syntax scmasm --cpu 65c02 bug003_hi_repro.S -o output.bin

error: Invalid argument for #>MESSAGE: Unsupported instruction: #>MESSAGE
```

### Offset Expression

```assembly
* File: bug003_offset_repro.S
        .OP 65C02
        
ZPPTR   .EQ $80
        
        .OR $2000
        
START   LDA #$12
        STA ZPPTR+1      ; Store to ZPPTR+1 ($81)
        RTS
```

```bash
$ ./build/bin/xasm++ --syntax scmasm --cpu 65c02 bug003_offset_repro.S -o output.bin

error: Invalid argument for ZPPTR+1: Unsupported instruction: ZPPTR+1
```

## Expected Behavior

### Low Byte Operator: `<`

Extracts the low 8 bits (LSB) of a 16-bit value:

```assembly
MESSAGE .OR $C0DE    ; Address = $C0DE
        LDA #<MESSAGE    ; Should assemble as: A9 DE
```

**Formula:** `low_byte = value & 0xFF`

### High Byte Operator: `>`

Extracts the high 8 bits (MSB) of a 16-bit value:

```assembly
MESSAGE .OR $C0DE    ; Address = $C0DE
        LDA #>MESSAGE    ; Should assemble as: A9 C0
```

**Formula:** `high_byte = (value >> 8) & 0xFF`

### Addition Operator: `+`

Adds offset to a value:

```assembly
ZPPTR   .EQ $80
        STA ZPPTR+1      ; Address = $80 + 1 = $81
                         ; Should assemble as: 85 81
```

**Formula:** `result = left_value + right_value`

### Subtraction Operator: `-`

Subtracts offset from a value:

```assembly
BUFFER  .EQ $2000
        LDA BUFFER-1     ; Address = $2000 - 1 = $1FFF
                         ; Should assemble as: AD FF 1F
```

**Formula:** `result = left_value - right_value`

## Actual Behavior

All expression operators are treated as part of the symbol name:
- `#<MESSAGE` → Lookup symbol "#<MESSAGE" → Not found
- `ZPPTR+1` → Lookup symbol "ZPPTR+1" → Not found

The parser doesn't recognize operators as expression elements.

## Analysis

### Use Cases in A2oSX

**1. Pointer Setup (Low/High Bytes)**

```assembly
* Store 16-bit address into zero page pointer
MESSAGE .AS "Hello World"

        LDA #<MESSAGE      ; Low byte
        STA ZPPTR
        LDA #>MESSAGE      ; High byte
        STA ZPPTR+1
```

**2. Indirect Indexed Addressing**

```assembly
* Access string via zero page pointer
        LDY #0
LOOP    LDA (ZPPTR),Y      ; Uses 16-bit pointer at ZPPTR/ZPPTR+1
        BEQ DONE
        JSR PUTCHAR
        INY
        BNE LOOP
DONE    RTS
```

**3. Multi-Byte Table Indexing**

```assembly
* 4-byte entries in table
TABLE   .EQ $3000

        LDX INDEX
        LDA TABLE+0,X      ; Byte 0 of entry
        STA TEMP
        LDA TABLE+1,X      ; Byte 1 of entry
        STA TEMP+1
```

### Frequency in A2oSX

Expression patterns identified in analysis:
- `#<LABEL`: ~150 occurrences
- `#>LABEL`: ~150 occurrences  
- `LABEL+N`: ~200 occurrences
- `LABEL-N`: ~50 occurrences

**Total impact:** ~550 expressions (5.7% of code)

## Suggested Fix

### Implementation Approach

Add expression evaluator to operand parser:

```cpp
class ExpressionEvaluator {
public:
    int evaluate(const string& expr, SymbolTable& symbols) {
        // Handle low byte: <SYMBOL
        if (expr[0] == '<') {
            int value = evaluate(expr.substr(1), symbols);
            return value & 0xFF;
        }
        
        // Handle high byte: >SYMBOL
        if (expr[0] == '>') {
            int value = evaluate(expr.substr(1), symbols);
            return (value >> 8) & 0xFF;
        }
        
        // Handle addition: SYMBOL+N
        size_t plus_pos = expr.find('+');
        if (plus_pos != string::npos) {
            int left = evaluate(expr.substr(0, plus_pos), symbols);
            int right = evaluate(expr.substr(plus_pos + 1), symbols);
            return left + right;
        }
        
        // Handle subtraction: SYMBOL-N
        size_t minus_pos = expr.find('-');
        if (minus_pos != string::npos && minus_pos > 0) {
            int left = evaluate(expr.substr(0, minus_pos), symbols);
            int right = evaluate(expr.substr(minus_pos + 1), symbols);
            return left - right;
        }
        
        // Handle numeric literals
        if (isdigit(expr[0]) || expr[0] == '$' || expr[0] == '%') {
            return parse_number(expr);
        }
        
        // Handle symbol lookup
        return symbols.lookup(expr);
    }
};
```

### Operator Precedence

Standard 6502 assembler precedence:
1. `<` and `>` (unary, highest precedence)
2. `*` and `/` (multiplication, division)
3. `+` and `-` (addition, subtraction)
4. Parentheses for grouping

Example:
```assembly
VALUE   .EQ $1234
        LDA #<(VALUE+$10)  ; First add $10, then extract low byte
                           ; = <$1244 = $44
```

### Test Cases to Add

```cpp
TEST(Expressions, LowByteOperator) {
    const char* source = R"(
        .OR $C0DE
MESSAGE NOP
        LDA #<MESSAGE
    )";
    // MESSAGE = $C0DE, low byte = $DE
    // Should assemble to: A9 DE
}

TEST(Expressions, HighByteOperator) {
    const char* source = R"(
        .OR $C0DE
MESSAGE NOP
        LDA #>MESSAGE
    )";
    // MESSAGE = $C0DE, high byte = $C0
    // Should assemble to: A9 C0
}

TEST(Expressions, AdditionOperator) {
    const char* source = R"(
ZPPTR   .EQ $80
        STA ZPPTR+1
    )";
    // $80 + 1 = $81
    // Should assemble to: 85 81
}

TEST(Expressions, SubtractionOperator) {
    const char* source = R"(
        .OR $2000
TARGET  NOP
        JMP TARGET-1
    )";
    // $2000 - 1 = $1FFF
    // Should assemble to: 4C FF 1F
}

TEST(Expressions, Combined) {
    const char* source = R"(
        .OR $C000
TABLE   NOP
OFFSET  .EQ 5
        LDA #<(TABLE+OFFSET)
    )";
    // $C000 + 5 = $C005, low byte = $05
    // Should assemble to: A9 05
}
```

## Priority Justification

**P1 - HIGH** because:

1. **Fundamental to 6502 programming**
   - 16-bit pointers require low/high byte split
   - Multi-byte data requires offset arithmetic
   - Cannot work with pointers without these operators

2. **No reasonable workaround**
   - Cannot hardcode addresses (defeats purpose of symbols)
   - Cannot manually calculate (especially with relocatable code)
   - Would require extensive manual preprocessing

3. **High frequency in A2oSX**
   - ~550 expressions use these operators
   - 5.7% of source code affected
   - Essential for OS functionality (I/O, memory management)

4. **Blocks testing of BUG-001**
   - Even if symbol resolution fixed, expressions still fail
   - Must be fixed for real-world validation

## Related Issues

- Depends on BUG-001 being fixed (symbol resolution)
- May need to support more complex expressions:
  - Multiplication: `RECORD_SIZE*INDEX`
  - Division: `TABLE_SIZE/ENTRY_SIZE`
  - Bitwise operations: `VALUE&$FF`, `VALUE|$80`

## Verification

Once fixed, verify with:

```bash
cd tests/integration/a2osx
./validation_suite.py --all

# Should see these tests pass:
# - test_expr_lo.S
# - test_expr_hi.S
# - test_expr_offset.S
# - sample_simple.S (if other bugs also fixed)
```

## References

- 6502 pointer manipulation: Standard pattern in all 6502 code
- SCMASM syntax: Uses `<` and `>` for byte extraction
- A2oSX source: Heavy use of pointer expressions
- Other assemblers: ca65, xa65, ACME all support these operators

---

**Reporter:** Validation Engineer
**Date:** 2026-02-12
**Version:** xasm++ 0.1.0
