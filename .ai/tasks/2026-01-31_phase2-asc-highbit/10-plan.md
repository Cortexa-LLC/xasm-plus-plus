# Implementation Plan

**Task ID:** 2026-01-31_phase2-asc-highbit
**Created:** 2026-01-31
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Modify ASC directive handler to set bit 7 (0x80) for all characters, producing Apple II high-bit ASCII.

**TDD Approach:**
1. Write failing test for ASC high-bit output
2. Implement bit 7 setting (ch | 0x80)
3. Verify all tests pass

**Key Technical Decisions:**
- Simple OR operation: `ch | 0x80`
- Apply to all characters in ASC strings
- Merlin syntax only (preserve other syntaxes)

**Patterns to Use:**
- TDD (RED-GREEN-REFACTOR)
- Bit manipulation
- Character transformation

---

## Critical Files Identified

### Files to Modify

```
1. src/syntax/merlin/merlin_syntax.cpp
   - Current: ASC outputs plain ASCII (0x00-0x7F)
   - Changes: Set bit 7 for all characters (ch | 0x80)

2. tests/unit/test_binary_compat.cpp
   - Current: Tests for equate-only files
   - Changes: Add test for ASC high-bit ASCII

3. tests/unit/test_merlin_syntax.cpp (possibly)
   - Current: Merlin-specific tests
   - Changes: May need to update expected values if tests check ASC output
```

---

## Step-by-Step Implementation Plan

### Phase 1: Create Failing Tests (RED)

```
Step 1.1: Add high-bit ASCII test
  - Action: Add test case to test_binary_compat.cpp
  - Files: tests/unit/test_binary_compat.cpp
  - Tests: Assemble simple ASC string, verify high-bit set
  - Verification: Test fails (currently outputs 0x50... not 0xD0...)

Step 1.2: Add VERSION.S comparison test
  - Action: Compare xasm++ output to vasm reference
  - Files: tests/unit/test_binary_compat.cpp
  - Tests: VERSION.S byte-for-byte comparison
  - Verification: Test fails (bytes don't match)
```

### Phase 2: Implement High-Bit ASCII (GREEN)

```
Step 2.1: Find ASC handler
  - Action: Locate HandleASC or similar in merlin_syntax.cpp
  - Files: src/syntax/merlin/merlin_syntax.cpp
  - Tests: N/A (exploration)
  - Verification: Found ASC directive handler

Step 2.2: Add bit 7 setting
  - Action: Modify character loop to OR with 0x80
  - Files: src/syntax/merlin/merlin_syntax.cpp
  - Tests: Binary compat tests pass
  - Verification: ASC outputs high-bit ASCII

Step 2.3: Verify existing tests
  - Action: Run full test suite
  - Files: N/A
  - Tests: 734/734 pass (minimum)
  - Verification: No regressions
```

### Phase 3: Verify and Document

```
Step 3.1: Compare with vasm
  - Action: Assemble VERSION.S, compare with vasm output
  - Files: tests/e2e/apple2/prince_of_persia/
  - Tests: Byte-for-byte match
  - Verification: VERSION.S matches vasm

Step 3.2: Update work log
  - Action: Document implementation and results
  - Files: .ai/tasks/2026-01-31_phase2-asc-highbit/20-work-log.md
  - Tests: N/A
  - Verification: Work log complete
```

---

## Testing Strategy

### Unit Tests

```
□ Binary compatibility - ASC high-bit:
  - TEST_CASE("ASC directive sets high bit")
  - ASC "ABC" → 0xC1 0xC2 0xC3 (not 0x41 0x42 0x43)
  - ASC "Prince of Persia" → 0xD0 0xF2... (not 0x50 0x72...)

□ Binary compatibility - VERSION.S:
  - TEST_CASE("VERSION.S matches vasm")
  - Byte-for-byte comparison with vasm reference
```

### Regression Tests

```
□ Full test suite
  - Execute: ctest
  - Verify: 734/734 pass (minimum, no regressions)
```

---

## Success Metrics

```
✓ ASC "test" → 0xF4 0xE5 0xF3 0xF4 (high-bit)
✓ VERSION.S matches vasm byte-for-byte
✓ All 734 tests pass
✓ Binary compat test added
```

---

## Implementation Notes

- **Find ASC handler first** - Locate where ASC directive processes characters
- **Simple change** - Just OR each character with 0x80
- **Test with VERSION.S** - Real-world verification
- **Watch for existing tests** - May need to update expected values

---

**Plan Status:** Approved (proceeding to implementation)

**Reviewed By:**
- [x] Orchestrator (2026-01-31)

**Plan Version:** 1.0
**Last Updated:** 2026-01-31
