# Implementation Plan

**Task ID:** 2026-01-31_phase1-equate-detection
**Created:** 2026-01-31
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Add simple flag tracking to detect when no bytes are emitted during assembly. If flag remains false, output 0-byte file instead of padding with zeros.

**TDD Approach:**
1. Write failing test for equate-only files (EQ.S, GAMEEQ.S, HRPARAMS.S)
2. Implement byte emission tracking
3. Verify all tests pass

**Key Technical Decisions:**
- Use boolean flag in assembler state (simple, fast)
- Check flag after assembly, before binary output
- Skip binary output if no bytes emitted

**Patterns to Use:**
- TDD (RED-GREEN-REFACTOR)
- State tracking (flag set on any data/code emission)
- Conditional output (skip if equate-only)

---

## Critical Files Identified

### Files to Modify

```
1. src/syntax/merlin/merlin_syntax.cpp
   - Current: ASC directive outputs data
   - Changes: Set flag when outputting data

2. include/xasm++/syntax/merlin_syntax.h
   - Current: MerlinSyntax class declaration
   - Changes: Add bytes_emitted flag member

3. src/main.cpp (or binary output module)
   - Current: Always outputs binary
   - Changes: Check flag, skip output if no bytes

4. tests/unit/test_binary_compat.cpp (NEW)
   - Purpose: Binary compat regression tests
   - Changes: Add tests for equate-only detection
```

---

## Step-by-Step Implementation Plan

### Phase 1: Create Failing Tests (RED)

```
Step 1.1: Add binary compat test file
  - Action: Create tests/unit/test_binary_compat.cpp
  - Files: tests/unit/test_binary_compat.cpp (new)
  - Tests: Test cases for EQ.S, GAMEEQ.S, HRPARAMS.S → expect 0 bytes
  - Verification: Tests fail (currently output 7592, 3401, 2500 bytes)

Step 1.2: Update CMakeLists.txt
  - Action: Add test_binary_compat.cpp to unit test target
  - Files: tests/unit/CMakeLists.txt
  - Tests: Build succeeds
  - Verification: New tests compile and run (but fail)
```

### Phase 2: Implement Tracking (GREEN)

```
Step 2.1: Add flag to assembler state
  - Action: Add bool bytes_emitted member, initialize to false
  - Files: include/xasm++/syntax/merlin_syntax.h, src/syntax/merlin/merlin_syntax.cpp
  - Tests: Existing tests still pass
  - Verification: Build succeeds, no regressions

Step 2.2: Set flag on data emission
  - Action: Set bytes_emitted = true when outputting bytes (data, code, ASC, etc.)
  - Files: src/syntax/merlin/merlin_syntax.cpp
  - Tests: Binary compat tests pass (partially)
  - Verification: Flag tracking works

Step 2.3: Skip output if no bytes
  - Action: Check flag before binary output, skip if false
  - Files: src/main.cpp (or binary output module)
  - Tests: All binary compat tests pass
  - Verification: EQ.S, GAMEEQ.S, HRPARAMS.S produce 0 bytes
```

### Phase 3: Verify and Refactor

```
Step 3.1: Run full test suite
  - Action: ctest (all 729 tests)
  - Files: N/A
  - Tests: 729/729 pass
  - Verification: No regressions

Step 3.2: Verify data files still work
  - Action: Check BGDATA.bin, HRTABLES.bin still output correctly
  - Files: tests/e2e/apple2/prince_of_persia/
  - Tests: Binary comparison with vasm references
  - Verification: Still byte-for-byte identical

Step 3.3: Refactor if needed
  - Action: Clean up code, add comments
  - Files: Modified files
  - Tests: All tests still pass
  - Verification: Code is clean and maintainable
```

---

## Testing Strategy

### Unit Tests

```
□ Binary compatibility - Equate-only files:
  - TEST_CASE("Equate-only files produce 0-byte output")
  - Assemble EQ.S → verify 0 bytes
  - Assemble GAMEEQ.S → verify 0 bytes
  - Assemble HRPARAMS.S → verify 0 bytes

□ Binary compatibility - Data files:
  - TEST_CASE("Data files still produce output")
  - Assemble BGDATA.S → verify 491 bytes
  - Assemble HRTABLES.S → verify 2480 bytes
```

### Integration Tests

```
□ POP test suite
  - Setup: tests/e2e/apple2/prince_of_persia/
  - Execute: pytest test_pop.py (existing tests)
  - Verify: All POP tests pass
```

### Regression Tests

```
□ Full test suite
  - Execute: ctest
  - Verify: 729/729 pass (100%)
```

---

## Success Metrics

```
✓ EQ.S → 0 bytes
✓ GAMEEQ.S → 0 bytes
✓ HRPARAMS.S → 0 bytes
✓ BGDATA.bin → 491 bytes (unchanged)
✓ HRTABLES.bin → 2480 bytes (unchanged)
✓ All 729 tests pass
✓ Binary compat test added
```

---

## Implementation Notes

- **Start with test** - TDD approach, write failing test first
- **Simple implementation** - Just a boolean flag, no complex analysis
- **Verify no regressions** - Check data files still work
- **Update work log** - Document progress as you go

---

**Plan Status:** Approved (proceeding to implementation)

**Reviewed By:**
- [x] Orchestrator (2026-01-31)

**Plan Version:** 1.0
**Last Updated:** 2026-01-31
