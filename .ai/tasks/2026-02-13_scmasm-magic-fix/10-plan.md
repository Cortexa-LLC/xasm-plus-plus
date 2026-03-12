# Plan: Fix Magic Strings in SCMASM

**Role:** Engineer
**Approach:** Direct fix with TDD verification

---

## Steps

1. Find all magic string comparisons (grep for `== "\.`)
2. Extract ASCII escape constants to scmasm_constants.h
3. Replace all occurrences
4. Run tests to verify
5. Commit

---

## Implementation

### Step 1: Extract ASCII Constants

Add to `include/xasm++/directives/scmasm_constants.h`:
```cpp
// ASCII Control Characters for escape sequences
namespace ascii {
  constexpr uint8_t BELL = 0x07;        // \a
  constexpr uint8_t BACKSPACE = 0x08;   // \b
  constexpr uint8_t TAB = 0x09;         // \t
  constexpr uint8_t NEWLINE = 0x0A;     // \n
  constexpr uint8_t VTAB = 0x0B;        // \v
  constexpr uint8_t FORMFEED = 0x0C;    // \f
  constexpr uint8_t CR = 0x0D;          // \r
  constexpr uint8_t ESCAPE = 0x1B;      // \e (non-standard)
}
```

### Step 2: Replace Escape Sequences

In ParseEscapeSequence function, replace hex literals with constants.

### Step 3: Fix Directive String Comparisons

Find and replace with constants from scmasm_directive_constants.h.

---

## Success Criteria

- All magic strings eliminated
- All tests passing
- Zero warnings
