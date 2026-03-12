# Implementation Plan

**Task ID:** 2026-01-31_fix-seqtable-padding
**Created:** 2026-01-31
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Modify `BinaryOutput::WriteOutput()` to track the lowest address used across all atoms and start binary output at that address instead of $0000.

**TDD Approach:**
1. Add test for SEQTABLE (ORG $3000 case)
2. Implement lowest address tracking
3. Modify output range to start at lowest address
4. Verify all 10 POP files match

**Key Technical Decisions:**
- Track min/max addresses while processing atoms
- Output range: [min_address, max_address] (not [0, max_address])
- Default min_address to 0 if no ORG directives exist

---

## Critical Files

### Files to Modify

```
1. src/output/binary_output.cpp (PRIMARY)
   - Current: Outputs from address 0 to max
   - Changes: Track min address, output from min to max

2. include/xasm++/output/binary_output.h (possibly)
   - Current: BinaryOutput class
   - Changes: May need member variables for address tracking
```

---

## Step-by-Step Implementation

### Phase 1: Add SEQTABLE Test (RED)

```
Step 1.1: Add real-world test
  - Action: Test SEQTABLE.bin comparison (ORG $3000 case)
  - Files: tests/unit/test_binary_compat.cpp
  - Verification: Test fails (first byte is 0x00 not 0xE9)
```

### Phase 2: Implement Fix (GREEN)

```
Step 2.1: Track address range in atoms
  - Action: Scan atoms to find min/max addresses
  - Files: src/output/binary_output.cpp
  - Verification: Addresses tracked correctly

Step 2.2: Adjust output range
  - Action: Start output at min_address instead of 0
  - Files: src/output/binary_output.cpp
  - Verification: SEQTABLE test passes

Step 2.3: Handle edge cases
  - Action: Default to 0 if no addresses set
  - Files: src/output/binary_output.cpp
  - Verification: All existing tests still pass
```

### Phase 3: Verify 100% Compatibility

```
Step 3.1: Build all POP files
  - Action: make ASM=xasm binaries in POP directory
  - Verification: All 10 files assemble

Step 3.2: Binary comparison
  - Action: Compare all 10 files with vasm
  - Verification: 10/10 match (100%)

Step 3.3: Full test suite
  - Action: ctest
  - Verification: 736/739 pass (no regressions)
```

---

## Testing Strategy

### Unit Tests

```
□ Add SEQTABLE binary comparison test
□ Verify first bytes are 0xE9 0x30... (not 0x00 0x00...)
□ Verify file size is 2.5KB
□ Existing Phase 3 tests still pass
```

### Integration Tests

```
□ All 10 POP files:
  - Build with xasm++
  - Compare with vasm references
  - Verify 10/10 match (100%)
```

---

## Success Metrics

```
✓ SEQTABLE.bin: First bytes 0xE9 0x30 (not 0x00)
✓ SEQTABLE.bin: Matches vasm byte-for-byte
✓ All 10 POP files match vasm (100%)
✓ All 736 tests pass
✓ Build clean (0 warnings)
```

---

## Implementation Notes

- **Scan atoms first** - Find min/max addresses before output
- **Simple solution** - Just adjust start offset in WriteOutput()
- **Preserve gaps** - Zeros BETWEEN addresses are intentional
- **Edge case** - No ORG directives → start at 0 (default behavior)

---

**Plan Status:** Approved (proceeding to implementation)

**Reviewed By:**
- [x] Orchestrator (2026-01-31)

**Plan Version:** 1.0
**Last Updated:** 2026-01-31
