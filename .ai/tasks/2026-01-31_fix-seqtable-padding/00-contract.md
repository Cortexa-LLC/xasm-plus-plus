# Task Contract

**Task ID:** 2026-01-31_fix-seqtable-padding
**Beads Task:** xasm++-7gr
**Priority:** P2 (High - final binary compatibility fix)
**Working Directory:** /Users/bryanw/Projects/Vintage/tools/xasm++

---

## Task Description

Fix binary output to start at lowest ORG address instead of $0000, eliminating leading zero padding in SEQTABLE.bin.

### Background

SEQTABLE.bin has `ORG $3000` but xasm++ outputs 224 bytes of leading zeros from $0000-$00DF before the actual data. vasm starts output at $3000 directly.

### Current State

```
SEQTABLE.bin comparison:
- xasm++: Starts with 224 bytes of 0x00, then data at offset 0xE0
- vasm: Starts immediately with data (0xE9 0x30...)
- Both are 2.5KB total, but content differs due to padding offset
```

### Root Cause

Binary output logic in `src/output/binary_output.cpp` doesn't track the lowest address used and always starts output at $0000.

---

## Success Criteria

```
✓ SEQTABLE.bin matches vasm byte-for-byte
✓ No leading zeros before first data
✓ Binary output starts at lowest ORG address
✓ All 10 POP files match vasm (100% compatibility)
✓ All 736 tests still pass
```

---

## Acceptance Criteria

### Functional Requirements

```
□ Track lowest address across all atoms in section
□ Start binary output at lowest address, not $0000
□ SEQTABLE first bytes are 0xE9 0x30... (not 0x00 0x00...)
□ File size remains 2.5KB
```

### Quality Requirements

```
□ All tests passing (736/739 minimum)
□ No linting errors
□ TDD approach (update existing Phase 3 tests)
```

---

## Resources and References

### Relevant Files

```
- src/output/binary_output.cpp - Binary output logic (MAIN FIX)
- include/xasm++/output/binary_output.h - BinaryOutput class
- tests/unit/test_binary_compat.cpp - Existing tests to verify
- ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/build/SEQTABLE.bin - Test file
```

---

## Notes

- Final piece to achieve 100% binary compatibility (currently 9/10 = 90%)
- Phase 3 tests passed but didn't catch real-world SEQTABLE case
- Fix should be in binary output range calculation

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-31
