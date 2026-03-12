# Task Contract

**Task ID:** 2026-01-31_phase2-asc-highbit
**Beads Task:** xasm++-ahk
**Priority:** P2 (High - part of binary compatibility)
**Working Directory:** /Users/bryanw/Projects/Vintage/tools/xasm++

---

## Task Description

Modify ASC directive to set bit 7 (0x80) for Apple II high-bit ASCII text encoding.

### Background

Apple II systems use high-bit ASCII where text characters have bit 7 set (0x80-0xFF range). Merlin assembler's ASC directive outputs high-bit ASCII to match this convention. Currently, xasm++ outputs plain ASCII (0x00-0x7F).

### Current State

```
VERSION.S with ASC "Prince of Persia":
xasm++: 0x50 0x72 0x69 0x6e... (plain ASCII "Prince of Persia")
vasm:   0xD0 0xF2 0xE9 0xEE... (high-bit ASCII 0x50+0x80, 0x72+0x80, etc.)
```

### Root Cause

The ASC directive handler in Merlin syntax outputs characters without setting bit 7.

---

## Success Criteria

```
✓ VERSION.S outputs high-bit ASCII (0xD0... not 0x50...)
✓ ASC directive sets bit 7 for all characters
✓ Existing files still work (BGDATA.bin, HRTABLES.bin unchanged)
✓ All 734 tests still pass (no regressions from Phase 1)
✓ Binary compat test added for ASC high-bit
```

---

## Acceptance Criteria

### Functional Requirements

```
□ ASC directive sets bit 7 (OR with 0x80)
□ High-bit ASCII only for Merlin syntax
□ Other syntaxes unchanged (plain ASCII)
□ VERSION.S matches vasm output byte-for-byte
```

### Quality Requirements

```
□ All tests passing (734/734 minimum)
□ Binary compat test added
□ No linting errors
□ TDD approach (test first)
```

---

## Constraints and Dependencies

### Constraints

```
□ Must not break existing functionality
□ High-bit ASCII only for Merlin syntax
□ Should preserve Phase 1 fixes
```

### Dependencies

```
□ Phase 1 complete (xasm++-l0q) ✅
```

### Out of Scope

```
✗ Padding removal (Phase 3)
✗ Other text encoding formats
✗ DCI or other Merlin text directives
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Number of files affected: 2-3
- Lines of code estimate: ~20-30
- New concepts/patterns: Bit manipulation (simple OR)
- Integration complexity: Low
- Risk level: Low

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 2-3 files

**Batch Size Evaluation:**
```
File Count Assessment:
└─ 2-3 files → ✅ IDEAL: Small batch, proceed
```

### Token Budget Estimation

**Conservative Estimate:**
```
3 × 2,000 tokens = 6,000 tokens
Agent Output Limit: 25K-32K tokens
Status: ✅ SAFE
```

---

## Resources and References

### Relevant Files

```
- src/syntax/merlin/merlin_syntax.cpp - ASC directive handler
- tests/unit/test_binary_compat.cpp - Add high-bit ASCII test
- tests/e2e/apple2/prince_of_persia/src/VERSION.S - Test input
```

---

## Notes

- Part of 3-phase binary compatibility work
- Phase 1 (equate-only) complete
- Phase 2 must complete before Phase 3 can start
- Simple bit manipulation (ch | 0x80)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-31
