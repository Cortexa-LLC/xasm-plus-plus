# Next Steps for Bug Fixing

## For the Next Engineer

You've been assigned to fix bugs blocking A2oSX assembly. This guide gets you started quickly.

---

## Priority Order (MUST FIX IN THIS ORDER)

### 1. BUG-001 (P0 CRITICAL) - Symbol Resolution
**File:** `BUG-001-symbol-addressing.md`
**Time:** 1-2 days
**Why First:** Blocks 99% of assembly, nothing else matters until fixed

### 2. BUG-002 (P1 HIGH) - .BS Directive  
**File:** `BUG-002-bs-directive.md`
**Time:** Few hours
**Why Second:** Easier fix, builds confidence

### 3. BUG-003 (P1 HIGH) - Expression Support
**File:** `BUG-003-expression-support.md`
**Time:** 1-2 days
**Why Third:** Depends on BUG-001 being fixed first

---

## Quick Start

### Step 1: Read Documentation (15 min)
```bash
cd tests/integration/a2osx

# Quick overview
cat QUICKSTART.md

# Your specific bug
cat BUG-001-symbol-addressing.md
```

### Step 2: Run Current Tests (2 min)
```bash
# See what works now
./validation_suite.py

# See what's blocked
./validation_suite.py --all
```

### Step 3: Reproduce the Bug (5 min)
```bash
# BUG-001 example
cd /Users/bryanw/Projects/Vintage/tools/xasm++
./build/bin/xasm++ --syntax scmasm --cpu 65c02 \
  tests/integration/a2osx/test_symbols.S \
  -o test.bin

# You'll see: error: Unsupported instruction: ZPTEMP
```

### Step 4: Locate Code to Fix
Each bug report includes "Suggested Fix" section with:
- Files to investigate
- Functions to audit
- Proposed solution

### Step 5: Implement Fix
Follow TDD approach:
1. Write test that reproduces bug
2. Verify test fails
3. Implement fix
4. Verify test passes
5. Run full validation suite

### Step 6: Verify Fix
```bash
# After fixing BUG-001
./validation_suite.py --all

# Should see test_symbols.S pass now
```

---

## BUG-001: Where to Start

### Files to Investigate
```
src/parser/operand_parser.cpp    # Operand parsing logic
src/parser/symbol_table.cpp      # Symbol lookup
src/assembler/instruction.cpp    # Instruction assembly
```

### What's Wrong
When parser sees `STA ZPTEMP`, it treats "ZPTEMP" as instruction mnemonic, not symbol.

### Root Cause (Hypothesis)
```cpp
// Current (broken):
if (operand.starts_with("#")) {
    // Look up symbol ✅
} else {
    // Treat as instruction mnemonic ❌
}

// Should be:
if (operand.starts_with("#")) {
    // Immediate mode
} else if (symbols.exists(operand)) {
    // Symbol lookup ✅
} else if (is_number(operand)) {
    // Numeric literal
} else {
    // Unknown
}
```

### Quick Test
```cpp
// Add this test to test suite
TEST(SymbolResolution, ZeroPageAddressing) {
    const char* source = R"(
        ZPTEMP  .EQ $80
                STA ZPTEMP
    )";
    // Should assemble to: 85 80
    // Currently fails with: Unsupported instruction: ZPTEMP
}
```

---

## BUG-002: Where to Start

### Files to Investigate
```
src/parser/directive_handler.cpp    # .BS directive implementation
```

### What's Wrong
`.BS 256` is treated as binary digits (expects only 0/1).

### Quick Fix
```cpp
// Current (broken):
void handle_BS(const string& arg) {
    for (char c : arg) {
        if (c != '0' && c != '1') 
            error("Invalid binary digit");
        emit_byte(c - '0');
    }
}

// Should be (SCMASM-compatible):
void handle_BS(const string& arg) {
    int count = evaluate_expression(arg);
    for (int i = 0; i < count; i++) {
        emit_byte(0x00);
    }
}

// OR rename current .BS to .BIN, implement new .BS
```

---

## BUG-003: Where to Start

### Files to Investigate
```
src/parser/expression_evaluator.cpp    # Expression parsing
src/parser/operand_parser.cpp          # Operand evaluation
```

### What's Missing
Operators: `<` (low byte), `>` (high byte), `+` (add), `-` (subtract)

### Implementation Outline
```cpp
class ExpressionEvaluator {
public:
    int evaluate(const string& expr) {
        // Handle low byte: <SYMBOL
        if (expr[0] == '<') {
            return evaluate(expr.substr(1)) & 0xFF;
        }
        
        // Handle high byte: >SYMBOL
        if (expr[0] == '>') {
            return (evaluate(expr.substr(1)) >> 8) & 0xFF;
        }
        
        // Handle addition: SYMBOL+N
        if (size_t pos = expr.find('+'); pos != string::npos) {
            return evaluate(expr.substr(0, pos)) + 
                   evaluate(expr.substr(pos + 1));
        }
        
        // Similar for subtraction
        // ...
    }
};
```

---

## Validation Workflow

### After Each Bug Fix

```bash
# 1. Build
cd /Users/bryanw/Projects/Vintage/tools/xasm++
cmake --build build

# 2. Run unit tests (if you added any)
cd build
ctest

# 3. Run validation suite
cd ../tests/integration/a2osx
./validation_suite.py --all

# 4. Check specific test
./build/bin/xasm++ --syntax scmasm --cpu 65c02 \
  tests/integration/a2osx/test_symbols.S -o test.bin
xxd test.bin    # Verify output
```

### Success Criteria

**BUG-001 Fixed When:**
- ✅ `test_symbols.S` assembles successfully
- ✅ Output: 85 80 (STA $80)
- ✅ `sample_simple.S` progresses further

**BUG-002 Fixed When:**
- ✅ `test_bs.S` assembles successfully
- ✅ Creates 256 bytes of output (or advances location counter)
- ✅ No "Invalid binary digit" error

**BUG-003 Fixed When:**
- ✅ `test_expr_lo.S` assembles (low byte)
- ✅ `test_expr_hi.S` assembles (high byte)
- ✅ `test_expr_offset.S` assembles (offset)
- ✅ Output matches expected values

**All Fixed When:**
```bash
./validation_suite.py --all

# Should show:
# Total: 11
# Passed: 11
# Failed: 0
# Pass rate: 100%
```

---

## Getting Help

### If Stuck on BUG-001
- Check `src/parser/operand_parser.cpp` for symbol lookup
- Search codebase for `symbol_table.lookup()`
- Review how immediate mode (`#`) works (it's correct)
- Compare with how other assemblers handle symbols

### If Stuck on BUG-002
- Current `.BS` is for "binary string" (0/1 digits)
- Need to rename to `.BIN` or `.BINARY`
- Implement new `.BS` as "block storage" (reserve N bytes)
- Check SCMASM documentation for `.BS` semantics

### If Stuck on BUG-003
- Study expression evaluation in other assemblers (ca65, xa65)
- Implement operator precedence correctly
- Start with simple operators (`<`, `>`), then arithmetic (`+`, `-`)
- Test each operator independently before combining

---

## Commit Strategy

### After BUG-001 Fixed
```bash
git add src/parser/operand_parser.cpp
git add tests/integration/a2osx/
git commit -m "Fix BUG-001: Symbol resolution in addressing modes

Symbols now work in all addressing modes (ZP, ABS, indexed, indirect).
Previously only worked in immediate mode (#VALUE).

Fixes:
- test_symbols.S now passes
- sample_simple.S can progress further

Validation: ./tests/integration/a2osx/validation_suite.py --all
"
```

### Similar commits for BUG-002 and BUG-003

### After All Fixed
```bash
git commit -m "Complete A2oSX validation fixes

All 11 validation tests now pass:
- BUG-001: Symbol resolution (P0) ✅
- BUG-002: .BS directive (P1) ✅
- BUG-003: Expression support (P1) ✅

xasm++ is now ready for A2oSX assembly validation phase 2.

Validation: tests/integration/a2osx/validation_suite.py
Result: 11/11 tests pass (100%)
"
```

---

## Timeline Estimate

**BUG-001:** 1-2 days (most complex)
**BUG-002:** 2-4 hours (straightforward)
**BUG-003:** 1-2 days (medium complexity)

**Total:** 3-5 days for all three bugs

**After fixes:** 1 week for real A2oSX file validation

---

## Questions?

Refer to:
- `BUG-00*.md` - Detailed bug reports
- `README.md` - Full documentation
- `SUMMARY.md` - Executive overview
- Task packet: `.ai/tasks/2026-02-12_a2osx-validation/`

Good luck! The bugs are well-understood and fixes are straightforward.

---

**Prepared by:** Validation Engineer
**Date:** 2026-02-12
**Status:** Ready for bug fix phase
