# Task Contract: Phase 2 Integration Complete

**Task ID:** 2026-02-09_phase2-integration-complete
**Beads Task:** xasm++-k6y4
**Created:** 2026-02-09
**Priority:** P1

Complete Phase 2 integration: ExpressionParser into all 3 god-class parsers.

## Context

**Phase 1 Complete:** ExpressionParser created (xasm++-9nox)
**Phase 2 Prerequisites Complete:**
- ✅ Comparison operators added (xasm++-6nra)
- ✅ Z80 number format adapter created (xasm++-81es)
- ✅ All blockers resolved

## Objective

Integrate ExpressionParser into Z80Universal, Merlin, and SCMASM parsers, removing ~1,200 LOC of duplicated expression parsing logic.

## Success Criteria

✓ Z80Universal uses ExpressionParser (~450 LOC removed)
✓ Merlin uses ExpressionParser (~400 LOC removed)
✓ SCMASM uses ExpressionParser (~350 LOC removed)
✓ All existing tests pass
✓ Zero behavioral changes
✓ Total reduction: ~1,200 LOC (22% overall)
✓ Code duplication: 37% → <5%

## Acceptance Criteria

### Integration for Each Parser:
- [ ] Replace ParseExpression methods with ExpressionParser.Parse()
- [ ] Wire up Z80 number format adapter (if needed)
- [ ] Remove old expression parsing methods
- [ ] All tests pass
- [ ] Measure LOC reduction
- [ ] Commit checkpoint

### Final Verification:
- [ ] All 3 parsers integrated
- [ ] Full test suite passes
- [ ] Total LOC: 5,379 → ~4,179
- [ ] Duplication metrics: <5%

## Estimated Effort

8-12 hours (3 parsers × 3-4 hours each)

**Approved:** Orchestrator 2026-02-09
