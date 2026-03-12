# Implementation Plan: Replace Magic Values in edtasm_m80_plusplus_syntax

**Task ID:** 2026-02-13_edtasm-m80-magic-values
**Workflow:** `.ai-pack/workflows/refactor.md`
**Role:** Engineer (delegated from Orchestrator)

---

## Workflow Selection

**Using:** Refactor workflow (behavior-preserving code improvement)

**Why:** Pure refactoring to improve code readability and maintainability.

---

## Approach

### Phase 1: Preparation & Analysis
1. Read Rockwell fix reference (commit 7341e43)
2. Read `src/syntax/edtasm_m80_plusplus_syntax.cpp`
3. Identify all magic values (already documented in investigation)
4. Verify tests passing baseline

### Phase 2: Add Named Constants
1. Add include for Z80 mnemonics: `#include "xasm++/cpu/opcodes_z80.h"`
2. Create anonymous namespace at top of file (after includes)
3. Add constant categories:
   - Radix prefixes (HEX_PREFIX_DOLLAR, etc.)
   - Radix values (RADIX_HEXADECIMAL, etc.)
   - String delimiters (SINGLE_QUOTE, DOUBLE_QUOTE)
   - Instruction sizes (INSTRUCTION_SIZE_SINGLE_BYTE, etc.)
   - Operand delimiters (OPERAND_DELIMITER)
4. Use existing Z80Mnemonics namespace (no new constants needed)
5. Group related constants with comments

### Phase 3: Replace Magic Values
Systematically replace in order:

**Group 1: Radix Prefixes (6 replacements)**
- Line 39: `'$'` → `HEX_PREFIX_DOLLAR`
- Line 45: `'x'` → `HEX_PREFIX_0X`
- Line 48: `'%'` → `BIN_PREFIX_PERCENT`
- Line 54: `'b'` → `BIN_PREFIX_0B`
- Line 57: `'@'` → `OCT_PREFIX_AT`
- Line 63: `'o'` → `OCT_PREFIX_0O`

**Group 2: Radix Values (3 replacements)**
- Line 39: `16` → `RADIX_HEXADECIMAL`
- Line 48: `2` → `RADIX_BINARY`
- Line 57: `8` → `RADIX_OCTAL`

**Group 3: String Delimiters (2 replacements)**
- Line 95: `'\''` → `SINGLE_QUOTE`
- Line 96: `'"'` → `DOUBLE_QUOTE`

**Group 4: Instruction Size Constants (4 replacements)**
- `return 1;` → `return INSTRUCTION_SIZE_SINGLE_BYTE;` (RST instructions)
- `return 2;` → `return INSTRUCTION_SIZE_TWO_BYTES;` (immediate operands, JR, DJNZ)
- `return 3;` → `return INSTRUCTION_SIZE_THREE_BYTES;` (JP, CALL)

**Group 5: Instruction Mnemonics (5 replacements)**
- `"JR"` → `Z80Mnemonics::JR`
- `"DJNZ"` → `Z80Mnemonics::DJNZ`
- `"JP"` → `Z80Mnemonics::JP`
- `"CALL"` → `Z80Mnemonics::CALL`
- `"RST"` → `Z80Mnemonics::RST`

**Group 6: Operand Delimiters (1 replacement)**
- `","` → `OPERAND_DELIMITER`

### Phase 4: Verification
1. Rebuild: `cmake --build build`
2. Run edtasm_m80_plusplus tests
3. Run full test suite
4. Verify 100% pass rate
5. Check no compiler warnings

### Phase 5: Commit
Single atomic commit with descriptive message

---

## Execution Strategy

**Sequential** - Replace in batches, test after each batch:
1. Add all constants (Phase 2)
2. Replace Group 1 (radix prefixes) - compile & test
3. Replace Group 2 (radix values) - compile & test
4. Replace Group 3 (string delimiters) - compile & test
5. Replace Group 4 (instruction sizes) - compile & test
6. Replace Group 5 (instruction mnemonics) - compile & test
7. Replace Group 6 (operand delimiter) - compile & test
8. Final verification (Phase 4)
9. Commit (Phase 5)

---

## File Impact Analysis

**Files Modified:**
- `src/syntax/edtasm_m80_plusplus_syntax.cpp` - Add constants, replace magic values (~25 lines added, ~21 lines modified)

**Files NOT Modified:**
- Tests (no behavioral changes)
- Headers (implementation detail only)

**Net Impact:** ~25 lines added (constants), ~21 lines modified (replacements)

---

## Detailed Constant Definitions

Add this at the top of `edtasm_m80_plusplus_syntax.cpp` after includes:

```cpp
namespace {

// Numeric literal prefixes
constexpr char HEX_PREFIX_DOLLAR = '$';    // $FF
constexpr char HEX_PREFIX_0X = 'x';        // 0xFF
constexpr char BIN_PREFIX_PERCENT = '%';   // %11010010
constexpr char BIN_PREFIX_0B = 'b';        // 0b11010010
constexpr char OCT_PREFIX_AT = '@';        // @377
constexpr char OCT_PREFIX_0O = 'o';        // 0o377

// Radix values
constexpr int RADIX_BINARY = 2;
constexpr int RADIX_OCTAL = 8;
constexpr int RADIX_HEXADECIMAL = 16;

// String delimiters
constexpr char SINGLE_QUOTE = '\'';
constexpr char DOUBLE_QUOTE = '"';

// Instruction size constants (Z80-specific)
constexpr int INSTRUCTION_SIZE_SINGLE_BYTE = 1;   // RST
constexpr int INSTRUCTION_SIZE_TWO_BYTES = 2;     // JR, DJNZ, immediate operands
constexpr int INSTRUCTION_SIZE_THREE_BYTES = 3;   // JP, CALL

// Note: Z80 mnemonics already defined in include/xasm++/cpu/opcodes_z80.h
// Use existing Z80Mnemonics namespace:
//   Z80Mnemonics::JR, Z80Mnemonics::DJNZ, Z80Mnemonics::JP,
//   Z80Mnemonics::CALL, Z80Mnemonics::RST

// Operand delimiters
constexpr char OPERAND_DELIMITER = ',';

} // anonymous namespace
```

---

## Example Transformations

### Before (parse_numeric_literal):
```cpp
int EdtasmM80PlusPlusSyntax::parse_numeric_literal(const std::string& text) {
    if (text[0] == '$') {
        return 16;  // Hexadecimal
    } else if (text[0] == '0' && text.length() > 1 && text[1] == 'x') {
        return 16;  // Hexadecimal (0x prefix)
    } else if (text[0] == '%') {
        return 2;   // Binary
    } else if (text[0] == '@') {
        return 8;   // Octal
    }
    // ...
}
```

### After:
```cpp
int EdtasmM80PlusPlusSyntax::parse_numeric_literal(const std::string& text) {
    if (text[0] == HEX_PREFIX_DOLLAR) {
        return RADIX_HEXADECIMAL;
    } else if (text[0] == '0' && text.length() > 1 && text[1] == HEX_PREFIX_0X) {
        return RADIX_HEXADECIMAL;
    } else if (text[0] == BIN_PREFIX_PERCENT) {
        return RADIX_BINARY;
    } else if (text[0] == OCT_PREFIX_AT) {
        return RADIX_OCTAL;
    }
    // ...
}
```

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Typo in replacement | High | Compile after each batch, immediate detection |
| Wrong constant name | Low | Follow naming convention exactly |
| Miss a magic value | Low | Systematic search and replace |
| Test failures | Low | Pure refactoring, no logic changes |

---

## Quality Gates

**Before starting:**
- [ ] All edtasm_m80_plusplus tests passing
- [ ] All total tests passing

**During refactoring:**
- [ ] Compile after adding constants
- [ ] Compile after each replacement group
- [ ] Run tests after each group

**Before commit:**
- [ ] All tests passing (100%)
- [ ] No compiler warnings
- [ ] Code review (verify all replacements)

---

## Reference: Rockwell Fix Pattern

Study commit 7341e43 for exact pattern to follow:
```bash
git show 7341e43
```

Key elements:
- Anonymous namespace for constants
- constexpr for compile-time constants
- Grouped by category with comments
- Descriptive constant names

---

## Estimated Timeline

- Preparation & analysis: 20 min
- Add constants: 20 min
- Replace Group 1 (radix prefixes): 30 min
- Replace Group 2 (radix values): 15 min
- Replace Group 3 (string delimiters): 10 min
- Replace Group 4 (instruction sizes): 15 min
- Replace Group 5 (instruction mnemonics): 20 min
- Replace Group 6 (operand delimiter): 10 min
- Verification: 30 min
- Commit: 10 min

**Total:** 3 hours

---

**Status:** READY FOR EXECUTION
