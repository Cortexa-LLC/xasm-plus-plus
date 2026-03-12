# Plan: Code Quality - Eliminate Magic Strings/Numbers

**Task:** xasm++-aq89
**Workflow:** Code Review → Refactoring
**Roles:** Reviewer (Phase 1) → Engineer (Phase 2)
**Approach:** Systematic review, then batch cleanup

---

## Execution Strategy

**Sequential two-phase approach:**
1. **Reviewer Phase:** Identify all magic values, categorize, prioritize
2. **Engineer Phase:** Implement fixes based on review findings

**Rationale:** Review must complete before cleanup to ensure systematic approach.

---

## Phase 1: Code Review (Reviewer Role)

**Duration:** 1-2 hours
**Deliverable:** Review report with categorized findings

### Step 1: Read Standards
Read `.ai-pack/quality/clean-code/magic-numbers.md` for criteria

### Step 2: Review Implementation Files

**File 1: `src/syntax/scmasm_directive_handlers.cpp`**

Review each handler function:
- `HandlePs()` - Check for 255, 0x80, delimiter checks
- `HandleInb()` - Check for error message strings
- `HandleCs()` - **HIGH PRIORITY** - Many escape sequence values
- `HandleCz()` - Check for 0x00 null terminator
- `HandleDummy()` - Check for state management values
- `HandleOp()` - Check for CPU type string literals
- `HandleTf()` - Check for metadata strings
- `HandleEp()` - Check for entry point values
- `HandleHx()` - Check for hex nibble values
- `HandleTa()` - Check for error/warning strings

**File 2: `include/xasm++/directives/scmasm_directive_constants.h`**

Review all constants:
- Directive name strings
- Any numeric constants
- String literals

**File 3: `include/xasm++/directives/scmasm_directive_handlers.h`**

Review declarations (usually clean)

**File 4: `tests/unit/test_scmasm_syntax.cpp`**

Review test values:
- Acceptable as magic in tests
- But note if constants would improve readability

### Step 3: Categorize Findings

Create review report with sections:
1. **P0 Violations** (must fix - blocking)
2. **P1 Violations** (should fix - important)
3. **P2 Suggestions** (nice to have)
4. **Acceptable Literals** (documented as OK)

### Step 4: Generate Recommendations

For each violation:
- Suggested constant name
- Suggested location (file/class/module level)
- Rationale for extraction

---

## Phase 2: Cleanup (Engineer Role)

**Duration:** 2-4 hours
**Deliverable:** Clean code with named constants

### Step 1: Create Constants File (if needed)

If many module-level constants needed:
```cpp
// File: include/xasm++/directives/scmasm_constants.h
namespace xasm {
namespace scmasm {

// Pascal String Directive (.PS)
constexpr uint8_t MAX_PASCAL_STRING_LENGTH = 255;
constexpr uint8_t HIGH_BIT_MASK = 0x80;

// Delimiter-based High-Bit Rule
constexpr char APOSTROPHE_DELIMITER = 0x27;

// ASCII Control Characters (non-standard escapes)
constexpr uint8_t ASCII_ESC = 0x1B;  // Escape character

} // namespace scmasm
} // namespace xasm
```

### Step 2: Implement P0 Fixes

Batch 1: Numeric constants
- Extract all repeated numbers
- Extract all non-obvious numbers
- Add to constants file or file-level

Batch 2: String constants
- Extract repeated error messages
- Document directive names

Batch 3: Character constants
- Replace magic hex with standard escapes where possible
- Extract non-standard escape values

**Test after each batch:**
```bash
cmake --build build && cd build && ctest -R SCMASM
```

### Step 3: Implement P1 Fixes (if time permits)

- Extract single-use but non-obvious values
- Improve test readability with constants

### Step 4: Documentation

Document constants with origin:
```cpp
// From vasm-ext syntax.c line 1440
constexpr uint8_t MAX_PASCAL_STRING_LENGTH = 255;

// SCASM high-bit rule: delimiters < apostrophe (0x27) set high bit
constexpr char APOSTROPHE_DELIMITER = 0x27;
```

---

## Common Magic Values to Look For

### High-Bit Manipulation
```cpp
0x80, 0xFF, 0x7F  // High-bit masks
0x27              // Apostrophe delimiter threshold
```

### String Lengths
```cpp
255               // Max Pascal string length
256               // Buffer sizes
```

### ASCII Control Characters
```cpp
0x0A, 0x0D        // \n \r (use standard escapes)
0x09, 0x0B        // \t \v (use standard escapes)
0x1B              // ESC (extract to constant)
0x07, 0x08, 0x0C  // \a \b \f (use standard escapes)
```

### Error Messages
```cpp
"Empty string in .PS directive"
"String length exceeds 255"
"Cannot open file"
```

---

## Verification Steps

After each batch of fixes:

1. **Build:**
   ```bash
   cmake --build build 2>&1 | grep -i warning
   ```
   Expected: Zero new warnings

2. **Test:**
   ```bash
   cd build && ctest -R SCMASM --output-on-failure
   ```
   Expected: 117/117 tests passing

3. **Full regression:**
   ```bash
   cd build && ctest --output-on-failure
   ```
   Expected: 1552+ tests passing

---

## Success Criteria

- ✅ Review report complete (Phase 1)
- ✅ All P0 violations fixed (Phase 2)
- ✅ All P1 violations fixed if time permits (Phase 2)
- ✅ Constants documented with origin
- ✅ All tests passing
- ✅ Zero new warnings
- ✅ Work log updated
- ✅ Beads task closed

---

## References

- **Standard:** `.ai-pack/quality/clean-code/magic-numbers.md`
- **vasm-ext:** `~/Projects/Vintage/tools/vasm-ext/syntax/scmasm/syntax.c`
- **Recent commits:** Phase 1-3 implementations

---

**Plan approved by:** Orchestrator
**Ready for execution by:** Reviewer → Engineer
