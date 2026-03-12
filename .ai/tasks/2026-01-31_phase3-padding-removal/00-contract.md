# Task Contract

**Task ID:** 2026-01-31_phase3-padding-removal
**Beads Task:** xasm++-46v
**Priority:** P2 (High - final binary compatibility piece)
**Working Directory:** /Users/bryanw/Projects/Vintage/tools/xasm++

---

## Task Description

Remove leading zero padding so binary output starts at ORG address, not $0000.

### Background

vasm outputs binaries starting at the lowest ORG address specified in the source. xasm++ currently pads from $0000 to the ORG address with zeros, creating larger files.

### Current State

```
SEQTABLE.S:
- ORG $E930 (first address)
- xasm++: Outputs from $0000 to highest address (includes leading 59KB of zeros)
- vasm: Outputs from $E930 to highest address (no leading zeros)
```

### Root Cause

Binary output logic starts at address $0000 instead of the lowest ORG address in the section.

---

## Success Criteria

```
✓ SEQTABLE.S matches vasm (no leading zeros)
✓ Binary output starts at lowest ORG address
✓ All 10 POP files match vasm byte-for-byte (100% compatibility)
✓ All 734 tests still pass
✓ Binary compat test added for padding removal
```

---

## Acceptance Criteria

### Functional Requirements

```
□ Find lowest ORG address in section
□ Start binary output at that address
□ No leading zeros before first ORG
□ Handle multiple ORG directives correctly
```

### Quality Requirements

```
□ All tests passing (734/734 minimum)
□ Binary compat test added
□ No linting errors
□ TDD approach
```

---

## Constraints and Dependencies

### Constraints

```
□ Must not break existing functionality
□ Must preserve Phases 1 & 2 fixes
□ Handle edge cases (no ORG, multiple ORGs)
```

### Dependencies

```
□ Phase 1 complete (xasm++-l0q) ✅
□ Phase 2 complete (xasm++-ahk) ✅
```

### Out of Scope

```
✗ Other binary compatibility issues
✗ Output format changes beyond padding
```

---

## Estimated Complexity

**Complexity:** Small-Medium

**Rationale:**
- Number of files affected: 3-5
- Lines of code estimate: ~50-100
- New concepts/patterns: Address range calculation
- Integration complexity: Medium (binary output logic)
- Risk level: Medium (affects all binary output)

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 3-5 files

**Batch Size Evaluation:**
```
File Count Assessment:
└─ 3-5 files → ✅ IDEAL: Small batch, proceed
```

### Token Budget Estimation

**Conservative Estimate:**
```
5 × 2,500 tokens = 12,500 tokens
Agent Output Limit: 25K-32K tokens
Status: ✅ SAFE
```

---

## Resources and References

### Relevant Files

```
- src/output/binary_output.cpp - Binary output logic
- include/xasm++/output/binary_output.h - BinaryOutput class
- src/core/section.cpp - Section address tracking
- tests/unit/test_binary_compat.cpp - Add padding test
```

---

## Notes

- Final phase of 3-phase binary compatibility work
- Phases 1 & 2 complete
- Achieves 100% binary compatibility when complete
- Most complex phase (binary output logic)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-31
