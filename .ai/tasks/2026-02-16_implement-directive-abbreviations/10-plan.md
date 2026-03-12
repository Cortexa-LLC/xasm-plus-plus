# Implementation Plan

**Task ID:** 2026-02-16_implement-directive-abbreviations
**Created:** 2026-02-16
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

This task implements 3 S-C Macro Assembler directive abbreviations (`.ED`, `.FI`, `.EL`) using Test-Driven Development (TDD). The implementation is straightforward:

- **`.ED`** requires adding dummy section state tracking and validation
- **`.FI`** and **`.EL`** are simple aliases to existing `.FIN` and `.ELSE` directives

All three follow existing patterns in the codebase. `.ED` needs new state management, while `.FI` and `.EL` only require updates to conditional scanning logic.

**Key Technical Decisions:**
- Use explicit state machine for dummy sections (`StartDummySection()`, `EndDummySection()`, `InDummySection()`)
- Control flow abbreviations handled in `ParseLine()` and `HandleDo()` (not directive registry)
- Implement using TDD: RED (failing test) → GREEN (minimal implementation) → REFACTOR (cleanup)

**Patterns to Use:**
- State machine pattern for dummy section tracking (like `.DUMMY` handler)
- Alias pattern for conditional abbreviations (update scanning logic in `HandleDo()`)
- Validation pattern for `.ED` (ensure in dummy section before no-op)

**Detailed Implementation Guide:**
See `.ai/tasks/2026-02-16_analyze-missing-directives/IMPLEMENTATION-PLAN.md` for complete TDD steps, test cases, and code examples.

---

## Critical Files Identified

### Files to Modify
```
1. include/xasm++/directives/scmasm_directive_constants.h
   - Current: Defines all directive constants (.DUMMY, .FIN, .ELSE, etc.)
   - Changes: Add ED, FI, EL constants

2. include/xasm++/syntax/scmasm_syntax.h
   - Current: ScmasmSyntaxParser class declaration
   - Changes: Add dummy section state tracking (in_dummy_section_, StartDummySection(), EndDummySection(), InDummySection())

3. src/syntax/scmasm_directive_handlers.cpp
   - Current: Implements directive handlers (HandleDummy, HandleDo, etc.)
   - Changes: Add HandleEd() implementation, update HandleDummy() to call StartDummySection()

4. src/syntax/scmasm_syntax.cpp
   - Current: Parser implementation, ParseLine(), HandleDo()
   - Changes:
     - Update InitializeDirectiveRegistry() to register .ED
     - Update ParseLine() to recognize .FI and .EL as mismatched directives
     - Update HandleDo() to scan for .FI and .EL when finding block boundaries

5. tests/unit/test_scmasm_directives.cpp (or test_scmasm_syntax.cpp)
   - Current: Unit tests for all directives
   - Changes: Add TDD tests for .ED, .FI, .EL (9+ test cases)

6. tests/integration/a2osx/ (new integration test)
   - Current: May not exist yet
   - Changes: Create integration test with all abbreviations
```

### Files to Read (for context)
```
- .ai/tasks/2026-02-16_analyze-missing-directives/IMPLEMENTATION-PLAN.md - Complete TDD guide
- .ai/tasks/2026-02-16_analyze-missing-directives/MISSING-DIRECTIVES.md - Quick reference
- src/syntax/scmasm_directive_handlers.cpp - HandleDummy() pattern
- src/syntax/scmasm_syntax.cpp - HandleDo() control flow pattern
```

---

## Step-by-Step Implementation Plan

### Phase 1: Implement .ED (End Dummy) - 2-3 hours

```
Step 1.1: RED - Write failing tests for .ED
  - Action: Create 3 unit tests (valid use, error case, multiple sections)
  - Files: tests/unit/test_scmasm_directives.cpp
  - Tests:
    * EdAfterDummySucceeds - .DUMMY ... .ED (should pass)
    * EdWithoutDummyErrors - .ED without .DUMMY (should throw)
    * MultipleEdInDifferentSections - multiple .DUMMY/.ED pairs
  - Verification: Tests fail (directive not recognized)

Step 1.2: GREEN - Add .ED constant and state tracking
  - Action:
    * Add ED constant to scmasm_directive_constants.h
    * Add in_dummy_section_ member and methods to ScmasmSyntaxParser
    * Initialize in_dummy_section_ = false in constructor
  - Files:
    * include/xasm++/directives/scmasm_directive_constants.h
    * include/xasm++/syntax/scmasm_syntax.h
    * src/syntax/scmasm_syntax.cpp
  - Verification: Code compiles

Step 1.3: GREEN - Implement HandleEd() and update HandleDummy()
  - Action:
    * Create HandleEd() that validates dummy section state
    * Update HandleDummy() to call StartDummySection()
    * Register .ED in InitializeDirectiveRegistry()
  - Files:
    * src/syntax/scmasm_directive_handlers.cpp
    * src/syntax/scmasm_syntax.cpp
  - Verification: Tests pass

Step 1.4: REFACTOR - Add comments and cleanup
  - Action: Document state machine, ensure dummy section auto-ends
  - Files: All modified files
  - Verification: Tests still pass, code readable
```

### Phase 2: Implement .FI (Finish) - 15 minutes

```
Step 2.1: RED - Write failing tests for .FI
  - Action: Create 2 unit tests (.DO/.FI, .DO/.EL/.FI)
  - Files: tests/unit/test_scmasm_directives.cpp
  - Tests:
    * FiEndsConditional - .DO ... .FI
    * AllAbbreviations - .DO ... .EL ... .FI
  - Verification: Tests fail (directive not recognized)

Step 2.2: GREEN - Add .FI constant and update scanning
  - Action:
    * Add FI constant
    * Update ParseLine() to recognize .FI
    * Update HandleDo() to scan for .FI
  - Files:
    * include/xasm++/directives/scmasm_directive_constants.h
    * src/syntax/scmasm_syntax.cpp (ParseLine and HandleDo)
  - Verification: Tests pass

Step 2.3: REFACTOR - Update error messages
  - Action: Ensure errors mention both .FIN and .FI
  - Files: src/syntax/scmasm_syntax.cpp
  - Verification: Error messages clear
```

### Phase 3: Implement .EL (Else) - 15 minutes

```
Step 3.1: RED - Write failing tests for .EL
  - Action: Create 1 unit test (.DO/.EL/.FIN)
  - Files: tests/unit/test_scmasm_directives.cpp
  - Tests:
    * ElInConditional - .DO ... .EL ... .FIN (verify else block executes)
  - Verification: Tests fail (directive not recognized)

Step 3.2: GREEN - Add .EL constant and update scanning
  - Action:
    * Add EL constant
    * Update ParseLine() to recognize .EL (already done in Phase 2 Step 2.2)
    * Update HandleDo() to scan for .EL (already done in Phase 2 Step 2.2)
  - Files:
    * include/xasm++/directives/scmasm_directive_constants.h
    * src/syntax/scmasm_syntax.cpp (if not already updated)
  - Verification: Tests pass

Step 3.3: REFACTOR - Update comments
  - Action: Document abbreviations
  - Files: src/syntax/scmasm_syntax.cpp
  - Verification: Comments clear
```

### Phase 4: Integration Testing - 30 minutes

```
Step 4.1: Create integration test file
  - Action: Create test with all 3 abbreviations
  - Files: tests/integration/a2osx/abbreviations.S.txt
  - Content: Dummy sections with .ED, conditionals with .EL/.FI
  - Verification: File created

Step 4.2: Run integration test
  - Action: Assemble integration test file
  - Verification: Assembly succeeds, output correct

Step 4.3: Test with A2osX
  - Action: Attempt to assemble A2osX.S.txt
  - Verification: Assembles past line 23 (previous failure point)

Step 4.4: Run full test suite
  - Action: cmake --build build && ctest --output-on-failure
  - Verification: All 1679+ tests pass (no regressions)
```

---

## Testing Strategy

### Unit Tests (TDD Approach)

```
□ .ED directive - Test cases:
  - Valid: .DUMMY ... .ED (no error)
  - Error: .ED without .DUMMY (throws exception)
  - Multiple: Multiple .DUMMY/.ED pairs
  - State: Dummy section state correct after .ED

□ .FI directive - Test cases:
  - Simple: .DO ... .FI (ends conditional)
  - With .EL: .DO ... .EL ... .FI (both abbreviations)
  - Nested: .DO ... .DO ... .FI ... .FI (nesting works)

□ .EL directive - Test cases:
  - Simple: .DO ... .EL ... .FIN (else clause)
  - With .FI: .DO ... .EL ... .FI (abbreviation combo)
  - Logic: Verify else block executes when condition false
  - Error: .EL without .DO (throws exception)
```

### Integration Tests

```
□ All abbreviations together
  - Setup: Create test file with .ED, .FI, .EL
  - Execute: xasm++ --cpu 6502 --syntax scmasm test.asm
  - Verify: Assembly succeeds, output matches expected

□ A2osX compatibility
  - Setup: A2osX.S.txt with abbreviations
  - Execute: xasm++ --cpu 65C02 --syntax scmasm A2osX.S.txt
  - Verify: Assembles past line 23, no errors
```

### Coverage Target

```
- Overall: Maintain existing 99%+ coverage
- New code (.ED handler, state tracking): 100%
- Modified code (ParseLine, HandleDo): 100%
```

---

## Dependencies and Prerequisites

### Technical Dependencies
```
□ C++20 compiler (Clang/GCC)
□ CMake 3.20+ build system
□ GoogleTest test framework
□ Existing xasm++ codebase with 1679+ passing tests
```

### Task Dependencies
```
□ Analysis complete (task xasm++-55bg) ✅
□ Build system functional ✅
□ Test framework operational ✅
```

### Knowledge Dependencies
```
□ Understanding of S-C Macro Assembler directive syntax ✅ (from analysis)
□ Familiarity with xasm++ directive handling patterns ✅
□ TDD methodology (RED-GREEN-REFACTOR) ✅
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Dummy section state gets out of sync
- **Probability:** Medium
- **Impact:** Medium (symbols in dummy sections incorrect)
- **Mitigation:** Explicit state machine, validation in .ED handler
- **Contingency:** Add debug logging, comprehensive tests

**Risk 2:** .FI/.EL break existing .FIN/.ELSE functionality
- **Probability:** Low
- **Impact:** High (regressions in conditionals)
- **Mitigation:** Update all scanning logic, test both forms
- **Contingency:** Rollback, investigate HandleDo() logic carefully

**Risk 3:** Control flow nesting gets confused
- **Probability:** Low
- **Impact:** Medium (nested conditionals fail)
- **Mitigation:** Same nesting logic for abbreviations, test mixed forms
- **Contingency:** Add nesting depth logging, simplify test cases

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. Revert all commits: git reset --hard HEAD~N (where N = number of commits)
2. Verify tests pass: ctest --output-on-failure
3. Verify A2osX still fails at line 23 (expected)
```

**Rollback Verification:**
```
✓ All 1679+ tests pass
✓ xasm++ functionality unchanged
✓ No build warnings
```

### Git Rollback
```
# If changes committed
git log --oneline  # Find commit hashes
git revert <commit-hash>  # Or git reset --hard <before-hash>

# If not committed
git restore .

# Verify
cmake --build build
ctest --output-on-failure
```

---

## Performance Considerations

### Expected Performance Impact

```
- Directive parsing: Negligible (3 additional constant comparisons)
- State tracking: Negligible (1 boolean member + 3 trivial methods)
- Memory: Negligible (1 byte per parser instance)
```

### Performance Targets

```
- Assembly time: No measurable increase (<1% overhead)
- Memory usage: No measurable increase
- Test execution: No measurable increase
```

---

## Alternative Approaches Considered

### Alternative 1: Modify A2osX Source Files (Option B from analysis)

**Pros:**
- Zero implementation time
- No xasm++ code changes
- Works immediately

**Cons:**
- Source modifications required (23 .ED, 1 .FI, 1 .EL)
- Deviates from original SCMASM syntax
- Less convenient for users
- Not a general solution

**Why Not Chosen:**
User wants complete SCMASM compatibility, no source modifications

### Alternative 2: Hybrid Approach (Option C from analysis)

**Pros:**
- Minimal effort (test first, implement only if needed)
- Pragmatic

**Cons:**
- .ED likely needed (23 uses)
- Still requires implementation eventually
- Delays complete solution

**Why Not Chosen:**
User requested comprehensive solution upfront, not iterative

---

## Timeline Estimate

**Note:** Estimates are for planning only.

```
Phase 1 (.ED implementation): 2-3 hours
- Step 1.1 (RED): 30 min
- Step 1.2 (GREEN - constants): 15 min
- Step 1.3 (GREEN - handlers): 1.5 hours
- Step 1.4 (REFACTOR): 30 min

Phase 2 (.FI implementation): 15 min
- Step 2.1 (RED): 5 min
- Step 2.2 (GREEN): 5 min
- Step 2.3 (REFACTOR): 5 min

Phase 3 (.EL implementation): 15 min
- Step 3.1 (RED): 5 min
- Step 3.2 (GREEN): 5 min (mostly done in Phase 2)
- Step 3.3 (REFACTOR): 5 min

Phase 4 (Integration testing): 30 min
- Step 4.1: 10 min
- Step 4.2: 5 min
- Step 4.3: 10 min
- Step 4.4: 5 min

Total: 3-4 hours
```

---

## Success Metrics

### Completion Criteria

```
✓ All TDD tests written and passing (9+ test cases)
✓ Integration test passes (all abbreviations)
✓ A2osX.S.txt assembles past line 23
✓ Full test suite passes (1679+ tests, no regressions)
✓ Code coverage maintained (99%+)
✓ Comments and documentation updated
```

### Quality Metrics

```
- Test coverage: 99%+ (maintain existing)
- New code coverage: 100%
- Build warnings: 0
- Code review: Approved
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [x] Author: Orchestrator (2026-02-16)
- [ ] Engineer: (to be approved upon claiming task)

**Revision History:**
```
v1.0 - 2026-02-16 - Initial plan
```

---

## Notes and Considerations

- **Detailed TDD guide available:** `.ai/tasks/2026-02-16_analyze-missing-directives/IMPLEMENTATION-PLAN.md` contains complete code examples, test cases, and implementation details
- **Small batch size:** Only 6 files, 18K tokens estimated (safe)
- **Low risk:** Following existing patterns, simple aliases
- **High value:** Completes SCMASM compatibility, enables A2osX assembly

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
