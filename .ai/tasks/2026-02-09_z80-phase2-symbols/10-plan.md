# Implementation Plan

**Task ID:** 2026-02-09_z80-phase2-symbols
**Created:** 2026-02-09
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Extend Z80 Universal Syntax Phase 1 implementation with symbol visibility directives (PUBLIC/EXTERN). This follows the same pattern as Phase 1 - extend `HandleDirective()` method, add symbol table tracking, and implement comprehensive tests using TDD methodology.

**Key Technical Decisions:**
- Extend Symbol class with visibility enum (private, public, extern)
- Add directive handlers for PUBLIC/GLOBAL/ENTRY and EXTERN/EXTRN/EXT
- Parse comma-separated symbol lists for multi-symbol directives
- Follow same TDD pattern as Phase 1 (RED → GREEN → REFACTOR)

**Patterns to Use:**
- Directive handler pattern (established in Phase 1)
- Enum for visibility tracking (Symbol::Visibility)
- Test-first development (write tests, then implementation)

---

## Critical Files Identified

### Files to Modify
```
1. include/xasm++/syntax/z80_universal_syntax.h
   - Current: Handles basic directives (DB, DW, ORG, EQU, etc.)
   - Changes: Add HandlePublic() and HandleExtern() methods

2. src/syntax/z80_universal/z80_universal_syntax.cpp
   - Current: Implements Phase 1 directives
   - Changes: Add PUBLIC/EXTERN handler implementations, extend HandleDirective()

3. tests/unit/test_z80_universal_syntax.cpp
   - Current: 26 tests for Phase 1
   - Changes: Add 10-12 tests for Phase 2 directives
```

### Files to Read (for context)
```
- include/xasm++/assembler/symbol.h - Check Symbol class API, extend if needed
- .ai/tasks/2026-02-09_z80-universal-syntax/ - Phase 1 reference
```

---

## Step-by-Step Implementation Plan

### Phase 1: Symbol Visibility Infrastructure (TDD RED)
```
Step 1.1: Extend Symbol class with visibility
  - Action: Add Symbol::Visibility enum (PRIVATE, PUBLIC, EXTERN)
  - Files: include/xasm++/assembler/symbol.h
  - Tests: Write unit tests for SetVisibility()/GetVisibility()
  - Verification: Tests compile and fail (RED)

Step 1.2: Write PUBLIC directive tests
  - Action: Add tests for PUBLIC/GLOBAL/ENTRY in test_z80_universal_syntax.cpp
  - Files: tests/unit/test_z80_universal_syntax.cpp
  - Tests: Test single symbol, multiple symbols, all synonyms
  - Verification: Tests compile and fail (RED)

Step 1.3: Write EXTERN directive tests
  - Action: Add tests for EXTERN/EXTRN/EXT
  - Files: tests/unit/test_z80_universal_syntax.cpp
  - Tests: Test single symbol, multiple symbols, all synonyms
  - Verification: Tests compile and fail (RED)
```

### Phase 2: Implementation (TDD GREEN)
```
Step 2.1: Implement Symbol visibility tracking
  - Action: Add visibility field, getter/setter to Symbol class
  - Files: include/xasm++/assembler/symbol.h, src/assembler/symbol.cpp
  - Tests: Symbol visibility tests should now pass
  - Verification: Symbol tests GREEN

Step 2.2: Implement PUBLIC directive handler
  - Action: Add HandlePublic() method, extend HandleDirective()
  - Files: z80_universal_syntax.h/cpp
  - Tests: PUBLIC tests should pass
  - Verification: PUBLIC tests GREEN

Step 2.3: Implement EXTERN directive handler
  - Action: Add HandleExtern() method, extend HandleDirective()
  - Files: z80_universal_syntax.h/cpp
  - Tests: EXTERN tests should pass
  - Verification: EXTERN tests GREEN
```

### Phase 3: Refactor and Verify
```
Step 3.1: Refactor and optimize
  - Action: Clean up code, extract common patterns if any
  - Files: All modified files
  - Tests: All tests (Phase 1 + Phase 2) pass
  - Verification: 36+ tests passing, zero warnings

Step 3.2: Integration verification
  - Action: Verify Phase 1 functionality unchanged
  - Files: Run all existing tests
  - Tests: All 26 Phase 1 tests still pass
  - Verification: No regressions
```

---

## Testing Strategy

### Unit Tests
```
□ PUBLIC directive - Test cases:
  - PUBLIC SYMBOL (single symbol)
  - PUBLIC SYM1, SYM2, SYM3 (multiple symbols)
  - GLOBAL synonym
  - ENTRY synonym
  - Error: undefined symbol marked public

□ EXTERN directive - Test cases:
  - EXTERN SYMBOL (single symbol)
  - EXTERN SYM1, SYM2, SYM3 (multiple symbols)
  - EXTRN synonym
  - EXT synonym
  - Extern symbol usable in expressions

□ Symbol visibility - Test cases:
  - Default visibility (private)
  - SetVisibility(PUBLIC)
  - GetVisibility() returns correct value
```

### Coverage Target
```
- Overall: 90%+
- New code: 90%+
- Critical paths: 100%
```

---

## Dependencies and Prerequisites

### Task Dependencies
```
✓ Z80 Universal Syntax Phase 1 (xasm++-1fn) - COMPLETE
```

### Knowledge Dependencies
```
□ Phase 1 implementation patterns
□ Symbol table architecture
□ TDD methodology (RED-GREEN-REFACTOR)
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Symbol class API may not support extensibility
- **Probability:** Low
- **Impact:** Medium
- **Mitigation:** Check Symbol.h early in Step 1.1, extend if needed
- **Contingency:** May need to modify Symbol class interface

**Risk 2:** Phase 1 regression
- **Probability:** Low
- **Impact:** High
- **Mitigation:** Run Phase 1 tests after each change, no modifications to existing handlers
- **Contingency:** Revert changes if regression detected

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. git restore (revert uncommitted changes)
2. Verify Phase 1 tests still pass
3. Document blockers
```

### Git Rollback
```
# If not committed
git restore include/xasm++/syntax/z80_universal_syntax.h \
           src/syntax/z80_universal/z80_universal_syntax.cpp \
           tests/unit/test_z80_universal_syntax.cpp

# Verify
cmake --build build && ctest --test-dir build -R test_z80_universal_syntax
```

---

## Success Metrics

### Completion Criteria
```
✓ All Phase 1 tests passing (26 tests)
✓ All Phase 2 tests passing (10-12 tests)
✓ Total: 36-38 tests passing
✓ Zero compiler warnings
✓ Code coverage ≥ 90%
✓ TDD methodology followed
```

### Quality Metrics
```
- Test coverage: 90%+
- Code quality: Google C++ Style Guide compliance
- Documentation: Doxygen comments complete
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [X] Author: Orchestrator [2026-02-09]
- [X] Requestor: User [2026-02-09]

**Revision History:**
```
v1.0 - 2026-02-09 - Initial plan
```

---

## Notes and Considerations

This plan follows the exact same pattern as Phase 1, which was successful (26/26 tests passing). The approach is proven and low-risk.

**Estimated effort:** 2-3 hours
**Batch size:** 3 files, 9K tokens (SAFE)
**Dependencies:** Phase 1 complete ✓

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-09
