# Task Contract: Phase 6c.1 - Merlin Handler Extraction

**Task ID:** 2026-02-11_god-class-phase6c1-merlin
**Beads Task:** xasm++-kksh
**Created:** 2026-02-11
**Requestor:** Orchestrator (from Phase 6c decomposition)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract ~26 remaining Merlin directive handlers following Phase 6b pattern. Eliminate all magic numbers and magic strings using named constants.

### Background and Context

- Phase 6b extracted 5 core directives (ORG, EQU, DB, DW, DS) and established the pattern
- Merlin has ~31 total directives, ~26 remain to be extracted
- This task applies the proven pattern to remaining Merlin directives

### Current State

- Merlin: ~26 directives as lambda functions in merlin_syntax.cpp
- Phase 6b pattern established and working
- directive_constants.h exists with 5 directive names

### Desired State

All Merlin directives extracted, registered, and tested. Zero magic numbers/strings.

---

## Success Criteria

```
✓ ~26 Merlin directives extracted as free functions
✓ All handlers registered with DirectiveRegistry
✓ Zero magic numbers (use opcodes_*.h)
✓ Zero magic strings (use directive_constants.h)
✓ All tests passing (97%+)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Extract all remaining Merlin directives
□ Follow Phase 6b pattern exactly:
  - Free functions for logic
  - Wrapper functions matching DirectiveHandler signature
  - Registration with DirectiveRegistry
□ Verify directives work identically to before
```

### Code Quality Requirements (CRITICAL)
```
□ NO magic numbers - use opcodes_6502.h constants
□ NO magic strings - use/extend directive_constants.h
□ Add new directive names to directive_constants.h
□ Use constexpr for all constants
```

### Quality Requirements
```
□ All tests passing (97%+)
□ No warnings
□ TDD followed
□ Code documented
```

---

## Estimated Complexity

**Complexity:** Medium
**Files:** 8-10
**Tokens:** ~27K → ⚠️ Approaching limit, acceptable

---

## Dependencies

**Depends On:** xasm++-uqyd (Phase 6b) - ✓ COMPLETE
**Reference:**
- `.ai/tasks/2026-02-10_god-class-phase6b-core/ENGINEER-REPORT.md` - Pattern to follow
- `.ai/tasks/2026-02-10_god-class-phase6c-full/00-contract.md` - Parent task

---

**Contract Approved:** ✓ Orchestrator 2026-02-11
