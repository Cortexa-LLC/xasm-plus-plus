# Implementation Plan

**Task ID:** 2026-01-24_merlin-advanced-7-8
**Created:** 2026-01-24
**Author:** Orchestrator
**Plan Version:** 1.0
**Depends On:**
- xasm++-vp5 (Phases 1-3) - REQUIRED
- xasm++-m94 (Phases 4-6) - REQUIRED

---

## Approach Summary

Complete Merlin syntax support with remaining directives and real-world validation using Prince of Persia source files.

---

## Files to Modify

- `src/syntax/merlin/merlin_syntax.cpp` - Add LST, ASC handlers
- `tests/unit/test_merlin_syntax.cpp` - Add tests
- `tests/integration/` - Add PoP integration tests (new directory)

---

## Implementation Steps

### Phase 7: Advanced Directives (~15 tests)
1. LST/LST OFF tests (RED → GREEN)
2. LSTDO tests (RED → GREEN)
3. TR tests (RED → GREEN)
4. ASC string tests (RED → GREEN)
5. REFACTOR

### Phase 8: Real-World Validation (~5 tests)
1. Create integration test directory
2. Test: Assemble PoP EQ.S (RED → GREEN)
3. Test: Assemble PoP SUBS.S (RED → GREEN)
4. Document any unsupported features
5. Final REFACTOR

---

## Testing Strategy

- ~20 unit tests total
- 2+ integration tests with real PoP files
- Coverage target: ≥85% overall

---

**Plan Status:** Approved
**Plan Version:** 1.0
**Last Updated:** 2026-01-24
