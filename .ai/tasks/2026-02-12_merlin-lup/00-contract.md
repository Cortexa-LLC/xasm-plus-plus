# Task Contract

**Task ID:** 2026-02-12_merlin-lup
**Beads Task:** xasm++-skhn
**Created:** 2026-02-12
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement Merlin LUP (loop) directive for code repetition.

### Background and Context

Merlin assembler's LUP directive repeats a code block N times. Similar to REPT in macro systems, but standalone. Essential for Merlin compatibility. Implementation should support nested loops and local label generation.

### Current State

- LUP directive already partially implemented
- Nested loop support may be incomplete
- Tests exist but coverage may be limited

### Desired State

LUP directive fully functional:
- Repeat code block N times
- Support nested loops
- Generate unique local labels per iteration

---

## Success Criteria

```
✓ LUP directive repeats code N times
✓ Nested loops supported
✓ Local labels unique per iteration
✓ All tests passing
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ LUP N repeats block N times
□ --^ terminator recognized
□ Nested LUP blocks work
□ Local labels uniquified
□ Expression evaluation for repeat count
```

### Quality Requirements
```
□ TDD process followed
□ Tests for nested loops
□ No reduction in test pass rate
```

---

## Technical Approach

**Files to Modify:**
- LUP directive handler (may already exist)
- Nested loop state tracking
- Tests

**Estimated:** 2-3 hours, 2 files, ~6K tokens

---

## Related Documents

- **Reference:** docs/planning/not-yet-implemented-items.md (Section 5)
- **Existing:** src/syntax/merlin_directives.cpp (may have partial implementation)
