# Task Contract: Phase 2b - Merlin ExpressionParser Integration

**Task ID:** 2026-02-10_phase2b-merlin-integration
**Beads Task:** xasm++-abz2
**Created:** 2026-02-10
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Integrate shared ExpressionParser into Merlin parser, removing ~1,200 LOC of duplicated expression parsing code.

### Background and Context

Phase 1 created shared ExpressionParser (xasm++-9nox). Phase 2a completed Z80Universal integration. Now need to complete Merlin integration.

Merlin has 140 LOC of custom expression logic that must be preserved while integrating the shared parser.

### Current State

Merlin parser has its own expression parsing implementation duplicated from other parsers.

### Desired State

Merlin uses shared ExpressionParser with custom Merlin-specific extensions preserved.

---

## Success Criteria

```
✓ ExpressionParser integrated into Merlin
✓ All Merlin tests passing (97%+ maintained)
✓ Custom Merlin logic preserved
✓ ~1,200 LOC removed
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Merlin parser uses ExpressionParser
□ Custom Merlin expression features work
□ All existing Merlin tests pass
□ No regression in functionality
```

### Quality Requirements
```
□ All tests passing
□ No new warnings
□ TDD process followed
```

---

## Estimated Complexity

**Complexity:** Small-Medium

**Files:** 4-6
**Token Budget:** ~18K tokens - ✅ SAFE
**WIP:** 1 agent - ✅ IDEAL

---

## Dependencies

**Depends on:**
- ✅ Phase 1: ExpressionParser (completed)

---

## Approvals

**Contract Approved By:**
- [X] Orchestrator: 2026-02-10

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-10
