# Acceptance Criteria: Fix A2oSX Critical Bugs

**Task ID:** 2026-02-13_a2osx-critical-bugs
**Beads Task:** xasm++-zmna
**Date Completed:** 2026-02-13

---

## Requirements Met

### BUG-001: Symbol Resolution in Addressing Modes ✅

**Requirement:** Symbols must work in all 6502 addressing modes, including expressions like `ZPPTR+1`.

**Implementation:**
- Modified `src/core/assembler.cpp` lines 438-446
- Changed from simple symbol lookup to full expression parsing using `ParseExpression`
- Now supports both simple symbols (`ZPPTR`) and expressions (`ZPPTR+1`)

**Testing:**
- Created `tests/integration/a2osx/test_symbols.S` - Basic symbol test
- Created `tests/integration/a2osx/test_bug001.S` - Comprehensive addressing mode test
- Verified all addressing modes work with symbols:
  - Zero page: `LDA ZPTEMP` → `a5 80` ✅
  - Zero page indexed: `LDA ZPTEMP,X` → `b5 80` ✅
  - Absolute: `LDA BUFFER` → `ad 00 20` ✅
  - Absolute indexed: `LDA BUFFER,X` → `bd 00 20` ✅
  - Indirect indexed: `LDA (ZPPTR),Y` → `b1 82` ✅
  - Indexed indirect: `LDA (ZPPTR,X)` → `a1 82` ✅
- Verified expressions work: `STA ZPPTR+1` → `85 83` ✅

**Status:** ✅ COMPLETE

---

### BUG-002: .BS Directive ✅

**Requirement:** `.BS` directive should reserve N bytes, not treat operand as binary.

**Implementation:**
- No changes needed - already correctly implemented in `src/syntax/scmasm_directive_handlers.cpp`
- Current implementation evaluates expression and reserves N zero bytes
- This matches expected behavior for Block Storage directive

**Testing:**
- Created `tests/integration/a2osx/test_bs.S`
- Verified `.BS 10` reserves 10 bytes ✅
- Verified `.BS $100` reserves 256 bytes ✅
- Verified `.BS SIZE*4` with expression reserves correct bytes ✅
- Total output: 299 bytes (10 + 256 + 32 + 1 NOP) ✅

**Status:** ✅ ALREADY WORKING (NOT A BUG)

---

### BUG-003: Expression Operators ✅

**Requirement:** Support `<`, `>`, `+`, `-` operators in expressions, including complex nested expressions like `#<(MESSAGE+$10)`.

**Implementation:**
- Operators `<`, `>`, `+`, `-` were already implemented
- Issue was missing parentheses grouping support
- Modified `src/core/assembler.cpp` lines 28-47
- Added logic to strip outer parentheses before parsing expression
- Now correctly handles nested expressions with grouping

**Testing:**
- Created `tests/integration/a2osx/test_bug003.S`
- Verified low byte operator: `#<MESSAGE` → `a9 de` ✅
- Verified high byte operator: `#>MESSAGE` → `a9 c0` ✅
- Verified addition: `ZPPTR+1` → `85 81` ✅
- Verified subtraction: `MESSAGE-1` → `ad dd c0` ✅
- Verified complex expression: `#<(MESSAGE+$10)` → `a9 ee` ✅
- Verified complex expression: `#>(MESSAGE-$DE)` → `a9 c0` ✅

**Status:** ✅ COMPLETE

---

## Code Quality

### Standards Compliance
- ✅ Follows C++ coding standards
- ✅ Proper error handling with exceptions
- ✅ Comments added explaining BUG fixes
- ✅ No compiler warnings (build clean)

### Testing
- ✅ All 1564 existing tests pass (100% pass rate)
- ✅ 4 new integration tests created for A2oSX compatibility
- ✅ Tests cover all addressing modes
- ✅ Tests cover all expression operators
- ✅ Tests cover edge cases (nested expressions, complex expressions)

### Documentation
- ✅ Work log updated with detailed progress
- ✅ Code comments added at fix locations
- ✅ Test files include documentation comments
- ✅ Acceptance criteria documented

---

## Files Modified

### Source Code
1. `src/core/assembler.cpp`
   - Lines 28-47: Added parentheses grouping support (BUG-003)
   - Lines 438-446: Changed to ParseExpression for all operands (BUG-001)

### Test Files Created
1. `tests/integration/a2osx/test_symbols.S` - Basic symbol resolution test
2. `tests/integration/a2osx/test_bug001.S` - Comprehensive addressing mode test
3. `tests/integration/a2osx/test_bug003.S` - Expression operator test
4. `tests/integration/a2osx/test_bs.S` - .BS directive test

---

## Verification Results

### Build Status
```bash
cmake --build build --target xasm++
# Result: Clean build with 0 warnings ✅
```

### Test Status
```bash
ctest --test-dir build --output-on-failure
# Result: 100% tests passed, 0 tests failed out of 1564 ✅
```

### Integration Tests
```bash
# BUG-001: Symbol resolution
./build/src/xasm++ --syntax scmasm --cpu 65c02 tests/integration/a2osx/test_bug001.S -o /tmp/test_bug001.bin
# Result: Assembly successful, all opcodes correct ✅

# BUG-002: .BS directive
./build/src/xasm++ --syntax scmasm --cpu 65c02 tests/integration/a2osx/test_bs.S -o /tmp/test_bs.bin
# Result: Assembly successful, correct byte count (299) ✅

# BUG-003: Expression operators
./build/src/xasm++ --syntax scmasm --cpu 65c02 tests/integration/a2osx/test_bug003.S -o /tmp/test_bug003.bin
# Result: Assembly successful, all operators work ✅
```

---

## Completion Checklist

- ✅ All acceptance criteria met
- ✅ All tests passing (1564/1564)
- ✅ Code follows standards
- ✅ Build passes with ZERO WARNINGS
- ✅ Code formatted per language standards
- ✅ No TODO/FIXME left unaddressed
- ✅ Work log updated with final status
- ✅ Integration tests created for all bugs
- ✅ Ready for review

---

## Summary

All three A2oSX critical bugs have been successfully addressed:

1. **BUG-001 (P0):** Symbol resolution now works in all addressing modes, including expressions
2. **BUG-002 (P1):** .BS directive was already working correctly (not a bug)
3. **BUG-003 (P1):** Expression operators now support parentheses grouping

The fixes are minimal, focused, and do not break any existing functionality. All 1564 tests pass, and new integration tests ensure the bugs do not regress.

**Status:** ✅ TASK COMPLETE

Beads Task: xasm++-zmna [READY TO CLOSE]
