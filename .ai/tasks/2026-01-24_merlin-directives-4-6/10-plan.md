# Implementation Plan

**Task ID:** 2026-01-24_merlin-directives-4-6
**Created:** 2026-01-24
**Author:** Orchestrator
**Plan Version:** 1.0
**Depends On:** xasm++-vp5 (Merlin Phases 1-3) - MUST BE COMPLETE FIRST

---

## Approach Summary

Build on Phases 1-3 foundation to add file inclusion (PUT), data directives (DFB), and conditional assembly (DO/FIN). Each phase follows TDD: RED-GREEN-REFACTOR.

---

## Files to Modify

- `src/syntax/merlin/merlin_syntax.h` - Add PUT, DFB, DO/FIN methods
- `src/syntax/merlin/merlin_syntax.cpp` - Implement handlers  
- `tests/unit/test_merlin_syntax.cpp` - Add ~30 more tests

---

## Implementation Steps

### Phase 4: PUT Directive (~10 tests)
1. Write tests for PUT directive (RED)
2. Implement file reading and parsing (GREEN)
3. Add circular include detection (RED → GREEN)
4. Add nested include tests (RED → GREEN)
5. REFACTOR

### Phase 5: Data Directives (~10 tests)
1. Write tests for DFB (RED)
2. Implement DFB handler (GREEN)
3. Add expression support in data (RED → GREEN)
4. Add label reference tests (RED → GREEN)
5. REFACTOR

### Phase 6: Conditional Assembly (~10 tests)
1. Write tests for DO/FIN (RED)
2. Implement conditional block management (GREEN)
3. Add ELSE clause (RED → GREEN)
4. Add nested conditional tests (RED → GREEN)
5. REFACTOR

---

## Testing Strategy

- ~30 unit tests total (10 per phase)
- Integration test with all features
- Coverage target: ≥85% overall, ≥95% new code

---

**Plan Status:** Approved
**Plan Version:** 1.0
**Last Updated:** 2026-01-24
