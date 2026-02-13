# BUG-001: Symbol Resolution Failure in Addressing Modes

**Priority:** P0 - CRITICAL
**Status:** Open
**Discovered:** 2026-02-12
**Impact:** Blocks A2oSX assembly (190 source files unusable)

## Summary

Symbols defined with `.EQ` directive work in immediate addressing mode but fail in all other addressing modes with error "Unsupported instruction: <symbol>".

## Description

When a symbol is defined with `.EQ` and then used in an instruction's operand, the symbol is only recognized in immediate mode (`#VALUE`). In all other addressing modes, the assembler reports the symbol as an "Unsupported instruction" rather than resolving it to its defined value.

## Minimal Reproduction

```assembly
* File: bug001_repro.S
        .OP 65C02
        
ZPTEMP  .EQ $80
        
        .OR $2000
        
START   LDA #ZPTEMP      ; ✅ Works - assembles as A9 80
        STA ZPTEMP       ; ❌ Fails - error: Unsupported instruction: ZPTEMP
        RTS
```

### Steps to Reproduce

```bash
$ ./build/src/xasm++ --syntax scmasm --cpu 65c02 bug001_repro.S -o output.bin

error: Invalid argument for ZPTEMP: Unsupported instruction: ZPTEMP
```

## Expected Behavior

The symbol `ZPTEMP` should be resolved to its defined value `$80` in all addressing modes:

```assembly
ZPTEMP  .EQ $80

LDA #ZPTEMP      ; Should assemble as: A9 80  (immediate)
STA ZPTEMP       ; Should assemble as: 85 80  (zero page)
LDA ZPTEMP       ; Should assemble as: A5 80  (zero page)
LDA ZPTEMP,X     ; Should assemble as: B5 80  (zero page,X)
```

## Actual Behavior

Only immediate mode works:
- ✅ `LDA #ZPTEMP` - Correctly assembles to A9 80
- ❌ `STA ZPTEMP` - Error: "Unsupported instruction: ZPTEMP"
- ❌ `LDA ZPTEMP` - Error: "Unsupported instruction: ZPTEMP"
- ❌ `LDA ZPTEMP,X` - Error: "Unsupported instruction: ZPTEMP"

## Analysis

The issue appears to be in operand parsing. When the assembler sees:
- `#ZPTEMP` - Recognizes `#` as immediate operator, looks up ZPTEMP ✅
- `ZPTEMP` - Treats ZPTEMP as an instruction mnemonic, fails ❌

### Likely Root Cause

The instruction parser probably:
1. Checks if operand starts with `#` → if yes, evaluate expression
2. Otherwise, treats entire operand string as instruction mnemonic
3. Never consults symbol table for non-immediate operands

### Correct Behavior Should Be

1. Parse instruction mnemonic (e.g., "STA")
2. Determine addressing mode from operand syntax
3. **Look up symbol in symbol table**
4. Evaluate expression to numeric value
5. Generate appropriate opcode + operand bytes

## Impact Assessment

**Severity:** CRITICAL

This bug makes xasm++ **completely unusable** for real-world 6502 assembly:

1. **A2oSX Impact:** Cannot assemble any of 190 source files
   - Zero page addressing used extensively for performance
   - ~40% of instructions use ZP addressing
   
2. **General 6502 Impact:** 
   - Zero page addressing is fundamental to 6502 programming
   - Indexed addressing required for arrays/tables
   - Absolute addressing needed for memory-mapped I/O

3. **Workaround:** None viable
   - Cannot use hardcoded values (maintainability nightmare)
   - Cannot use immediate mode for memory access
   - Would require rewriting all source code

## Suggested Fix

### Location to Investigate

`src/parser/operand_parser.cpp` (or equivalent)
- Function that parses instruction operands
- Symbol table lookup logic

### Proposed Solution

When parsing an operand:
1. Check if it's an immediate value (`#` prefix)
2. Check if it's a numeric literal (`$`, `%`, or decimal)
3. **Check if it's a symbol (lookup in symbol table)**
4. Check if it's an expression (contains operators)
5. Evaluate and resolve to numeric value

### Test Cases to Add

```cpp
TEST(SymbolResolution, ZeroPageAddressing) {
    const char* source = R"(
        ZPTEMP  .EQ $80
                LDA ZPTEMP
    )";
    // Should assemble to: A5 80
}

TEST(SymbolResolution, ZeroPageIndexed) {
    const char* source = R"(
        ZPBASE  .EQ $80
                LDA ZPBASE,X
    )";
    // Should assemble to: B5 80
}

TEST(SymbolResolution, AbsoluteAddressing) {
    const char* source = R"(
        TARGET  .EQ $C000
                JMP TARGET
    )";
    // Should assemble to: 4C 00 C0
}

TEST(SymbolResolution, IndirectIndexed) {
    const char* source = R"(
        ZPPTR   .EQ $80
                LDA (ZPPTR),Y
    )";
    // Should assemble to: B1 80
}
```

## Related Issues

- This bug blocks BUG-003 (expression support) from being tested
- May be related to forward reference resolution

## Verification

Once fixed, verify with:

```bash
# Run validation suite
cd tests/integration/a2osx
./validation_suite.py --all

# Should see test_symbols.S pass
```

## References

- A2oSX source code: Uses ZP addressing extensively
- 6502 addressing modes: https://www.masswerk.at/6502/6502_instruction_set.html
- SCMASM syntax: Original assembler documentation

---

**Reporter:** Validation Engineer
**Date:** 2026-02-12
**Version:** xasm++ 0.1.0
