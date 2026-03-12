# Contract: Code Quality - Eliminate Magic Strings/Numbers in SCMASM

**Beads Task:** xasm++-aq89
**Created:** 2026-02-13
**Owner:** Orchestrator → Reviewer + Engineer
**Priority:** P1 (blocks Phase 3 commit)

---

## Executive Summary

Review Phase 1-3 SCMASM implementation for magic strings, numbers, and characters. Replace with named constants following clean code principles from `.ai-pack/quality/clean-code/magic-numbers.md`.

**Scope:** All SCMASM directive handler code from Phases 1-3
**Goal:** Zero magic values, all literals properly named or documented

---

## Background

**Phases implemented:**
- Phase 1: .PS, .INB, .LIST
- Phase 2: .DUMMY, .OP
- Phase 3: .CS, .CZ, .TF, .EP, .HX, .TA, .DO/.FIN

**Current state:** Implementation complete, but may contain magic values that reduce code maintainability.

---

## Requirements

### 1. Code Review (Reviewer Role)

**Review these files for magic values:**
- `src/syntax/scmasm_directive_handlers.cpp` (~850 lines)
- `include/xasm++/directives/scmasm_directive_handlers.h` (~50 lines)
- `include/xasm++/directives/scmasm_directive_constants.h` (~40 lines)
- `tests/unit/test_scmasm_syntax.cpp` (~450 lines)

**Magic value categories to identify:**

#### Category A: Magic Numbers
```cpp
// ❌ BAD: Magic numbers
if (str_len > 255) throw "String too long";
data[0] = 0x80;
if (delim < 0x27) set_high_bit = true;

// ✅ GOOD: Named constants
constexpr uint8_t MAX_PASCAL_STRING_LENGTH = 255;
constexpr uint8_t HIGH_BIT_MASK = 0x80;
constexpr char APOSTROPHE_DELIMITER = 0x27;

if (str_len > MAX_PASCAL_STRING_LENGTH) throw "String too long";
data[0] = HIGH_BIT_MASK;
if (delim < APOSTROPHE_DELIMITER) set_high_bit = true;
```

#### Category B: Magic Strings
```cpp
// ❌ BAD: Magic strings in error messages
throw std::runtime_error("Empty string in .PS directive");
throw std::runtime_error(".PS string length exceeds 255");

// ✅ GOOD: Named or documented
// Error message is self-documenting directive name
throw std::runtime_error("Empty string in .PS directive");

// OR: Extract to constant if repeated
constexpr const char* ERR_PS_EMPTY = "Empty string in .PS directive";
throw std::runtime_error(ERR_PS_EMPTY);
```

#### Category C: Magic Characters
```cpp
// ❌ BAD: Magic escape characters
case 'n': c = 0x0A; break;
case 't': c = 0x09; break;

// ✅ GOOD: Named constants or use standard constants
case 'n': c = '\n'; break;  // Standard escape
case 't': c = '\t'; break;  // Standard escape
// OR for non-standard:
constexpr uint8_t ASCII_ESC = 0x1B;
case 'e': c = ASCII_ESC; break;
```

#### Category D: Acceptable Literals
```cpp
// ✅ ACCEPTABLE: Self-evident values
result.size() > 0          // Zero is self-evident
data.push_back(0x00)       // Null byte is self-evident
for (int i = 0; i < 10; i++) // Loop bounds are self-evident
```

### 2. Cleanup Implementation (Engineer Role)

**For each identified magic value:**

1. **Determine if it needs extraction:**
   - Used multiple times? → Extract to constant
   - Complex or non-obvious? → Extract to constant
   - Self-documenting? → May leave as-is with comment

2. **Choose appropriate location:**
   - **File-level constants:** Static constexpr in .cpp file
   - **Module-level constants:** In `scmasm_directive_constants.h`
   - **Class-level constants:** If part of class interface

3. **Name using domain language:**
   - Use SCASM terminology where applicable
   - Follow existing naming conventions
   - Document origin if from vasm-ext reference

**Example cleanup:**
```cpp
// BEFORE (magic-numbers.md violation)
if (result.size() > 255) {
  throw std::runtime_error(".PS string length exceeds 255");
}
data.push_back(static_cast<uint8_t>(result.size()));

// AFTER (clean code)
constexpr uint8_t MAX_PASCAL_STRING_LENGTH = 255;

if (result.size() > MAX_PASCAL_STRING_LENGTH) {
  throw std::runtime_error(
    ".PS string length exceeds " +
    std::to_string(MAX_PASCAL_STRING_LENGTH)
  );
}
data.push_back(static_cast<uint8_t>(result.size()));
```

---

## Acceptance Criteria

### Reviewer Deliverables

1. **Review report** documenting:
   - [ ] All magic values identified (by category)
   - [ ] Recommendations for each (extract vs leave as-is)
   - [ ] Priority ranking (P0 = must fix, P1 = should fix, P2 = optional)

2. **Categorization:**
   - [ ] Magic numbers → named constants
   - [ ] Magic strings → documented or extracted
   - [ ] Magic characters → standard escapes or named
   - [ ] Acceptable literals → documented as acceptable

### Engineer Deliverables

1. **Code cleanup:**
   - [ ] All P0 magic values eliminated
   - [ ] All P1 magic values eliminated (time permitting)
   - [ ] Named constants added to appropriate headers
   - [ ] Code remains functionally identical

2. **Testing:**
   - [ ] All 117 SCMASM tests still passing
   - [ ] All 1552+ total tests still passing
   - [ ] Zero new compiler warnings
   - [ ] Zero regressions

3. **Documentation:**
   - [ ] Constants documented with origin (vasm-ext reference lines)
   - [ ] Work log updated with cleanup summary

---

## Quality Standards Reference

**Primary standard:** `.ai-pack/quality/clean-code/magic-numbers.md`

**Key principles:**
1. Named constants improve code readability
2. Self-documenting code reduces cognitive load
3. Domain language enhances maintainability
4. Repeated literals become coupling points

---

## Files to Review

### High Priority (Core Implementation)

1. **`src/syntax/scmasm_directive_handlers.cpp`**
   - HandlePs: 0x80 high-bit, 255 max length
   - HandleInb: File I/O error strings
   - HandleCs: Escape sequence values (0x0A, 0x09, 0x1B, etc.)
   - HandleCz: Null terminator (0x00)
   - HandleDummy: State management values
   - HandleOp: CPU type strings

2. **`include/xasm++/directives/scmasm_directive_constants.h`**
   - Directive name strings
   - Any numeric constants

### Medium Priority (Interface)

3. **`include/xasm++/directives/scmasm_directive_handlers.h`**
   - Function declarations (usually clean)

### Low Priority (Tests)

4. **`tests/unit/test_scmasm_syntax.cpp`**
   - Test values (acceptable as magic in tests)
   - But constants can improve test readability

---

## Execution Strategy

**Two-phase approach:**

### Phase 1: Review (Reviewer Role)
1. Read magic-numbers.md standard
2. Review each file for violations
3. Categorize and prioritize findings
4. Document recommendations

### Phase 2: Cleanup (Engineer Role)
1. Implement P0 fixes (blocking violations)
2. Implement P1 fixes (important but not blocking)
3. Run tests after each batch
4. Update documentation

**Sequential execution:** Review completes before cleanup begins

---

## Estimated Effort

**Review phase:** 1-2 hours
**Cleanup phase:** 2-4 hours
**Total:** 3-6 hours

---

## Success Criteria

- ✅ Review report complete with categorized findings
- ✅ All P0 magic values eliminated
- ✅ All P1 magic values eliminated (time permitting)
- ✅ Named constants documented with origin
- ✅ All tests still passing
- ✅ Zero new warnings
- ✅ Code quality improved, maintainability enhanced

---

## References

- **Clean Code Standard:** `.ai-pack/quality/clean-code/magic-numbers.md`
- **vasm-ext Reference:** `~/Projects/Vintage/tools/vasm-ext/syntax/scmasm/syntax.c`
- **Phase 1-3 Implementations:** Recent commits

---

**Contract approved by:** Orchestrator
**Ready for:** Reviewer → Engineer (sequential)
