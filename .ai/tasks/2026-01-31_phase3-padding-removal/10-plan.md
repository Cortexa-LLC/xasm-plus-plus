# Implementation Plan

**Task ID:** 2026-01-31_phase3-padding-removal
**Created:** 2026-01-31
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Modify binary output logic to start at the lowest ORG address in the section instead of $0000, eliminating leading zero padding.

**TDD Approach:**
1. Write failing test for SEQTABLE.S (has leading zeros)
2. Implement lowest address tracking and output range calculation
3. Verify all 10 POP files match vasm

**Key Technical Decisions:**
- Track lowest and highest addresses used in section
- Output only the range [lowest...highest]
- Preserve gap handling (zeros between ORG directives are intentional)

**Patterns to Use:**
- TDD (RED-GREEN-REFACTOR)
- Address range calculation
- Conditional output range

---

## Critical Files Identified

### Files to Modify

```
1. src/output/binary_output.cpp
   - Current: Outputs from $0000 to highest address
   - Changes: Output from lowest ORG to highest address

2. include/xasm++/output/binary_output.h
   - Current: BinaryOutput class
   - Changes: May need methods for address range

3. src/core/section.cpp (possibly)
   - Current: Section address tracking
   - Changes: Track lowest address used

4. tests/unit/test_binary_compat.cpp
   - Current: Equate-only and ASC tests
   - Changes: Add padding removal test
```

---

## Step-by-Step Implementation Plan

### Phase 1: Create Failing Tests (RED)

```
Step 1.1: Add padding removal test
  - Action: Create test with ORG at high address
  - Files: tests/unit/test_binary_compat.cpp
  - Tests: Verify no leading zeros before ORG
  - Verification: Test fails (currently has padding)

Step 1.2: Add SEQTABLE.S comparison test
  - Action: Compare xasm++ output size to vasm
  - Files: tests/unit/test_binary_compat.cpp
  - Tests: File sizes should match
  - Verification: Test fails (xasm++ larger due to padding)
```

### Phase 2: Implement Padding Removal (GREEN)

```
Step 2.1: Find binary output logic
  - Action: Locate where output range is determined
  - Files: src/output/binary_output.cpp
  - Tests: N/A (exploration)
  - Verification: Found output range calculation

Step 2.2: Track lowest address
  - Action: Find/add lowest address tracking in section
  - Files: src/core/section.cpp, include/xasm++/core/section.h
  - Tests: Existing tests pass
  - Verification: Lowest address correctly tracked

Step 2.3: Modify output range
  - Action: Change output to start at lowest address
  - Files: src/output/binary_output.cpp
  - Tests: Binary compat tests pass
  - Verification: No leading zeros

Step 2.4: Handle edge cases
  - Action: Handle files with no ORG (use $0000)
  - Files: src/output/binary_output.cpp
  - Tests: Existing tests still pass
  - Verification: Edge cases handled
```

### Phase 3: Verify 100% Compatibility

```
Step 3.1: Run full test suite
  - Action: ctest
  - Files: N/A
  - Tests: 734/734 pass
  - Verification: No regressions

Step 3.2: Verify all 10 POP files
  - Action: Build all POP files, compare with vasm
  - Files: tests/e2e/apple2/prince_of_persia/
  - Tests: All 10 match byte-for-byte
  - Verification: 100% binary compatibility achieved

Step 3.3: Update documentation
  - Action: Document completion of all 3 phases
  - Files: Work logs, acceptance docs
  - Tests: N/A
  - Verification: Documentation complete
```

---

## Testing Strategy

### Unit Tests

```
□ Binary compatibility - Padding removal:
  - TEST_CASE("No leading zeros before ORG")
  - ORG $E930; data → output starts at $E930
  - Verify first byte is at ORG address, not $0000

□ Binary compatibility - SEQTABLE.S:
  - TEST_CASE("SEQTABLE.S matches vasm size")
  - Compare file sizes (should be ~2.5KB, not 60KB)
  - Byte-for-byte comparison
```

### Integration Tests

```
□ All 10 POP files:
  - BGDATA.bin ✅ (already matches)
  - HRTABLES.bin ✅ (already matches)
  - EQ.bin ✅ (Phase 1 fixed)
  - GAMEEQ.bin ✅ (Phase 1 fixed)
  - HRPARAMS.bin ✅ (Phase 1 fixed)
  - VERSION.bin ✅ (Phase 2 verified)
  - SEQTABLE.bin → Fix with Phase 3
  - SEQDATA.bin → Check after Phase 3
  - MOVEDATA.bin → Check after Phase 3
  - SOUNDNAMES.bin → Check after Phase 3
```

### Regression Tests

```
□ Full test suite
  - Execute: ctest
  - Verify: 734/734 pass (minimum)
```

---

## Success Metrics

```
✓ SEQTABLE.S: 2.5KB (not 60KB)
✓ Output starts at first ORG address
✓ All 10 POP files match vasm (100%)
✓ All 734 tests pass
✓ Binary compat test added
```

---

## Implementation Notes

- **Find lowest address** - May already be tracked in Section
- **Output range calculation** - Change start from $0000 to lowest_address
- **Preserve gaps** - Zeros BETWEEN ORGs are intentional (don't remove)
- **Edge case** - Files with no ORG should output from $0000

---

**Plan Status:** Approved (proceeding to implementation)

**Reviewed By:**
- [x] Orchestrator (2026-01-31)

**Plan Version:** 1.0
**Last Updated:** 2026-01-31
