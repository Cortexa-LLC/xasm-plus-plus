# Task Contract: Phase 2c - SCMASM ExpressionParser Integration

**Task ID:** 2026-02-10_phase2c-scmasm-integration
**Beads Task:** xasm++-ksvm
**Created:** 2026-02-10
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Integrate shared ExpressionParser into SCMASM parser, completing Phase 2 integration across all 3 parsers.

### Background and Context

Phase 1 created shared ExpressionParser. Phase 2a completed Z80Universal. Phase 2b will complete Merlin. This task completes SCMASM integration.

### Current State

SCMASM parser has its own expression parsing implementation duplicated from other parsers.

### Desired State

SCMASM uses shared ExpressionParser, completing the god-class Phase 2 work across all parsers.

---

## Success Criteria

```
✓ ExpressionParser integrated into SCMASM
✓ All SCMASM tests passing (97%+ maintained)
✓ Phase 2 complete across all 3 parsers
✓ Expression duplication eliminated
```

---

## Acceptance Criteria

### Functional Requirements
```
□ SCMASM parser uses ExpressionParser
□ All existing SCMASM tests pass
□ No regression in functionality
□ Phase 2 complete
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
