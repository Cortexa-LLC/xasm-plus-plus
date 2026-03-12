# Work Log: Fix A2oSX Critical Bugs

**Task ID:** 2026-02-13_a2osx-critical-bugs
**Beads Task:** xasm++-zmna
**Status:** In Progress
**Started:** 2026-02-13

---

## Session 1: 2026-02-13

### Initial Assessment

**Bugs to fix:**
1. BUG-001 (P0): Symbol resolution - symbols fail in addressing modes
2. BUG-002 (P1): .BS directive - treats as binary, should reserve bytes  
3. BUG-003 (P1): Expression operators - missing <, >, +, - operators

**Current implementation analysis:**

**BUG-002 (.BS directive):**
- Location: `src/syntax/scmasm_directive_handlers.cpp` HandleBs function (line ~414)
- Current behavior: Treats operand as binary digits (0/1), converts to bytes
- Required behavior: Parse operand as byte count, reserve N bytes
- Root cause: Confusion between Binary String vs Block Storage directives

**BUG-003 (Expression operators):**
- Need to check expression parser for < > + - support
- Test files show usage: `#<MESSAGE`, `#>MESSAGE`, `ZPPTR+1`

**BUG-001 (Symbol resolution):**
- Likely in operand parsing - symbols not looked up for non-immediate modes
- Need to examine how instructions are parsed

### Phase 1: Fix BUG-002 (.BS Directive) - ALREADY FIXED

**Discovery:** The .BS directive was already correctly implemented!
- Location: `src/syntax/scmasm_directive_handlers.cpp` lines 418-447
- Current behavior: Evaluates expression, reserves N zero bytes
- This matches expected behavior for Block Storage

**Verification:**
```bash
# Created test_bs.S with various .BS cases
./build/src/xasm++ --syntax scmasm --cpu 65c02 tests/integration/a2osx/test_bs.S -o /tmp/test_bs.bin
# Output: 299 bytes (10 + 256 + 32 + 1 NOP) ✅
```

BUG-002 is **NOT A BUG** - already working correctly.

---

### Phase 2: Fix BUG-001 (Symbol Resolution in Addressing Modes)

**Investigation:**
- Created `tests/integration/a2osx/test_symbols.S` to reproduce the issue
- Found that `STA ZPPTR+1` was assembling to `STA $00` instead of `STA $83`
- Root cause located in `src/core/assembler.cpp` lines 438-446

**Root Cause:**
When operand is not immediate (`#`), hex (`$`), or indirect (`(...)`), the code just does direct symbol lookup instead of parsing as expression:

```cpp
} else if (value_str != "A") {
  // Label reference - look up in symbol table (skip if accumulator "A")
  SymbolTable *lookup_table = symbols_ ? symbols_ : &symbols;
  if (lookup_table != nullptr) {
    int64_t symbol_value;
    if (lookup_table->Lookup(value_str, symbol_value)) {  // ❌ No expression parsing!
      value = static_cast<uint16_t>(symbol_value);
    }
  }
}
```

This works for simple symbols like `ZPPTR`, but fails for expressions like `ZPPTR+1`.

**Fix Applied:**
Changed to use `ParseExpression` for all label references, matching the pattern used for immediate and hex modes:

```cpp
} else if (value_str != "A") {
  // Label reference or expression - use ParseExpression to handle
  // both simple symbols and expressions like ZPPTR+1
  // BUG-003 FIX: Support expressions with +, -, <, > operators
  try {
    auto expr = ParseExpression(value_str, symbols);
    int64_t expr_value = expr->Evaluate(symbols);
    value = static_cast<uint16_t>(expr_value);
  } catch (const std::exception &e) {
    // Check if this is a parse error or an undefined symbol
    std::string msg(e.what());
    if (msg.find("Undefined symbol") != std::string::npos) {
      // Symbol undefined (forward reference) - use placeholder 0
      value = 0;
    } else {
      // Parse error - propagate the exception
      throw;
    }
  }
}
```

**Verification:**
```bash
./build/src/xasm++ --syntax scmasm --cpu 65c02 tests/integration/a2osx/test_symbols.S -o /tmp/test_symbols.bin
hexdump -C /tmp/test_symbols.bin
# 85 83 = STA $83 (ZPPTR+1) ✅ CORRECT!
```

Created comprehensive test `test_bug001.S` covering all addressing modes with symbols:
- Zero page: `LDA ZPTEMP` ✅
- Zero page indexed: `LDA ZPTEMP,X` ✅
- Absolute: `LDA BUFFER` ✅
- Absolute indexed: `LDA BUFFER,X` ✅
- Indirect indexed: `LDA (ZPPTR),Y` ✅
- Indexed indirect: `LDA (ZPPTR,X)` ✅

All tests pass! BUG-001 is **FIXED**.

---

### Phase 3: Fix BUG-003 (Expression Operators)

**Investigation:**
Created `test_bug003.S` to test all required operators:
- Low byte: `#<MESSAGE` 
- High byte: `#>MESSAGE`
- Addition: `ZPPTR+1`
- Subtraction: `MESSAGE-1`
- Complex: `#<(MESSAGE+$10)`

**Issue Found:**
When testing `#<(MESSAGE+$10)`, got error:
```
Logic error: Invalid hex digit ')' in hex string: '$10)'
```

**Root Cause:**
The `ParseExpression` function didn't handle parentheses for grouping. When processing `<(MESSAGE+$10)`:
1. Strips `<` → recursively parses `(MESSAGE+$10)`
2. Recursive call sees `(MESSAGE+$10)` but doesn't strip outer parentheses
3. When parsing `+` operator, right side is `$10)` (with closing paren)
4. Hex parser fails on `)` character

**Fix Applied:**
Added parentheses-stripping logic at the start of `ParseExpression`:

```cpp
// Strip outer parentheses for grouping: (EXPR)
// BUG-003 FIX: Support parentheses in complex expressions like <(MESSAGE+$10)
if (!trimmed.empty() && trimmed[0] == '(' && trimmed[trimmed.length() - 1] == ')') {
  // Check if these are matching outer parentheses
  int depth = 0;
  bool is_outer = true;
  for (size_t i = 0; i < trimmed.length(); ++i) {
    if (trimmed[i] == '(') depth++;
    if (trimmed[i] == ')') depth--;
    // If depth hits 0 before the end, these aren't outer parens
    if (depth == 0 && i < trimmed.length() - 1) {
      is_outer = false;
      break;
    }
  }
  if (is_outer && depth == 0) {
    // Strip outer parentheses and recurse
    trimmed = Trim(trimmed.substr(1, trimmed.length() - 2));
  }
}
```

**Verification:**
```bash
./build/src/xasm++ --syntax scmasm --cpu 65c02 tests/integration/a2osx/test_bug003.S -o /tmp/test_bug003.bin
hexdump -C /tmp/test_bug003.bin
```

All operators verified:
- `a9 de` = LDA #$DE (low byte of $C0DE) ✅
- `a9 c0` = LDA #$C0 (high byte of $C0DE) ✅
- `85 81` = STA $81 (ZPPTR+1) ✅
- `ad dd c0` = LDA $C0DD (MESSAGE-1) ✅
- `a9 ee` = LDA #$EE (low byte of $C0EE = MESSAGE+$10) ✅
- `a9 c0` = LDA #$C0 (high byte of $C000 = MESSAGE-$DE) ✅

BUG-003 is **FIXED**.

---

### Final Verification

**Full test suite:**
```bash
ctest --test-dir build --output-on-failure
# Result: 100% tests passed, 0 tests failed out of 1564 ✅
```

**Test files created:**
1. `tests/integration/a2osx/test_symbols.S` - Symbol resolution test
2. `tests/integration/a2osx/test_bug001.S` - Comprehensive addressing mode test
3. `tests/integration/a2osx/test_bug003.S` - Expression operator test
4. `tests/integration/a2osx/test_bs.S` - .BS directive test

---

## Summary

**Bugs Fixed:**
- ✅ BUG-001: Symbol resolution in addressing modes (FIXED - ParseExpression now used for all operands)
- ✅ BUG-002: .BS directive (NOT A BUG - already working correctly)
- ✅ BUG-003: Expression operators (FIXED - added parentheses grouping support)

**Code Changes:**
1. `src/core/assembler.cpp` lines 438-446: Changed label lookup to use ParseExpression
2. `src/core/assembler.cpp` lines 28-47: Added parentheses grouping support

**Tests:**
- All 1564 existing tests pass
- 4 new integration tests created for A2oSX compatibility

**Status:** ✅ COMPLETE

