# Task Contract: Z80 Phase 14 - Current Location Operator ($)

**Task ID:** 2026-02-10_z80-phase14-current-location
**Beads Task:** xasm++-i17r
**Created:** 2026-02-10
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement current location counter ($) operator in expression evaluator for calculating sizes and offsets dynamically.

### Background and Context

Phase 10 identified this as a limitation. The $ operator returns the current assembly address and is commonly used in Z80 code for calculating data structure sizes: `SIZE EQU ($-START)`

### Current State

Expression evaluator doesn't recognize $ symbol. Users must hand-calculate sizes.

### Desired State

$ returns current address in all expression contexts, enabling dynamic size calculations.

---

## Success Criteria

```
✓ $ operator returns current address
✓ ($-LABEL) calculates size correctly
✓ Works in EQU, DB/DW/DS operands
✓ All tests passing (97%+)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ $ in EQU: SIZE EQU $-START
□ $ in DB: DB ($>>8),($&$FF)
□ $ in expressions: VALUE EQU ($-TABLE)/2
□ Nested: DB ($-START)>>8, ($-START)&$FF
```

### Quality Requirements
```
□ All tests passing
□ No warnings
□ TDD followed
```

---

## Estimated Complexity

**Complexity:** Small

**Files:** 3 (expression evaluator, parser, tests)
**Tokens:** 9K → ✅ SAFE

---

## Dependencies

**Depends on:** Nothing
**Blocks:** Nothing

---

**Contract Approved:** ✓ Orchestrator 2026-02-10
