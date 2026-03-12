# Task Contract

**Task ID:** 2026-02-16_implement-directive-abbreviations
**Beads Task:** xasm++-bo6t
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement 3 missing S-C Macro Assembler directive abbreviations for complete A2osX compatibility:

1. **`.ED`** (End Dummy) - 23 uses in A2osX
2. **`.FI`** (Finish) - 1 use in A2osX
3. **`.EL`** (Else) - 1 use in A2osX

All three are simple aliases or validation wrappers for existing functionality.

### Background and Context

Comprehensive analysis (task xasm++-55bg) revealed xasm++ has excellent SCMASM directive coverage:
- **99.7% of A2osX directive usage** already supported
- **22 of 25 directives** implemented (88%)
- Only these 3 rare abbreviations missing (25 uses out of 9000+)

Without these abbreviations, A2osX assembly fails at line 23. Implementing them provides complete SCMASM compatibility with no source modifications needed.

### Current State

- A2osX.S.txt fails at line 23: `.ED` not recognized
- Full forms exist: `.FIN` and `.ELSE` work correctly
- `.DUMMY` directive works but doesn't recognize `.ED` end marker
- Conditional directives (`.DO`/`.ELSE`/`.FIN`) fully functional
- All existing 1679+ tests passing

### Desired State

- All 3 abbreviations recognized and functional
- A2osX.S.txt assembles successfully
- Complete SCMASM compatibility (100%)
- All existing 1679+ tests still passing

---

## Success Criteria

```
✓ .ED directive recognizes end of dummy section (validation + no-op)
✓ .FI directive works as alias to .FIN
✓ .EL directive works as alias to .ELSE
✓ All 3 directives have unit tests (TDD approach)
✓ Integration test with A2osX snippet passes
✓ A2osX.S.txt assembles past line 23
✓ All existing tests still passing (no regressions)
```

---

## Acceptance Criteria

### Functional Requirements

**`.ED` (End Dummy):**
```
□ Validates we're in a dummy section (throws error if not)
□ Acts as explicit end marker (no-op, validation only)
□ Works with multiple dummy sections in same file
□ Dummy section state tracked correctly
□ Unit tests cover valid use, invalid use, multiple sections
```

**`.FI` (Finish):**
```
□ Recognized as alias to .FIN in ParseLine()
□ Works in HandleDo() conditional scanning
□ Error messages mention both .FIN and .FI as valid
□ Works with nested conditionals
□ Unit tests cover simple use, with .EL, nested cases
```

**`.EL` (Else):**
```
□ Recognized as alias to .ELSE in ParseLine()
□ Works in HandleDo() conditional scanning
□ Error messages mention both .ELSE and .EL as valid
□ Conditional logic correct (else block executes when condition false)
□ Unit tests cover use with .FIN, use with .FI, error cases
```

### Quality Requirements
```
□ All unit tests pass (TDD: RED-GREEN-REFACTOR for each directive)
□ Integration test with all 3 abbreviations passes
□ No regressions in existing 1679+ tests
□ Code coverage maintained or improved
□ Comments and documentation updated
```

---

## Constraints and Dependencies

### Constraints
```
□ Must not break existing 1679+ tests
□ Must maintain control flow directive handling pattern
□ Must use TDD approach (write tests first)
□ Must handle both abbreviations and full forms simultaneously
□ Must follow existing code patterns and style
```

### Dependencies
```
□ Analysis complete (task xasm++-55bg)
□ Implementation plan available
□ Test framework (GoogleTest) ready
□ Build system (CMake) functional
```

### Out of Scope
```
✗ Implementing other SCMASM directives
✗ Modifying A2osX source files
✗ Changing behavior of existing directives
✗ Performance optimization
```

---

## Estimated Complexity

**Complexity:** Small to Medium

**Rationale:**
- Files affected: 6 files
- Lines of code estimate: ~150 lines (including tests)
- New concepts/patterns: No (following existing patterns)
- Integration complexity: Low (simple aliases and validation)
- Risk level: Low (well-tested, clear requirements)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 6 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 6 files → ⚠️ ACCEPTABLE
```

### Batch Size Justification

Files: 6 (within acceptable range but requires justification)

**Why not decomposed further:**
- High cohesion - all changes tightly coupled (directive handling)
- Single concern - implementing 3 related abbreviations
- Already minimal viable batch (cannot split without breaking functionality)
- Implementation is simple aliases and validation

**Contingency for token limits:**
- If token limit hit, will decompose into: .ED implementation (3 files) + .FI/.EL implementation (3 files)

**Estimated tokens:** 6 × 3,000 = 18,000 tokens
**Status:** Within 25K-32K limit? **YES** ✅

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
6 × 3,000 tokens = 18,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 18,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent ✅ IDEAL
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [x] YES - Batch size = 6 files (acceptable) AND token budget = 18K (safe) AND WIP = 1 (ideal)
- [ ] NO - MUST decompose

---

## Resources and References

### Relevant Files
```
1. include/xasm++/directives/scmasm_directive_constants.h - Add 3 constants
2. include/xasm++/syntax/scmasm_syntax.h - Add dummy section tracking
3. src/syntax/scmasm_directive_handlers.cpp - Implement HandleEd()
4. src/syntax/scmasm_syntax.cpp - Update ParseLine(), HandleDo(), registry
5. tests/unit/test_scmasm_directives.cpp - Add unit tests
6. tests/integration/a2osx/ - Add integration test
```

### Documentation
```
- .ai/tasks/2026-02-16_analyze-missing-directives/SUMMARY.md - Analysis summary
- .ai/tasks/2026-02-16_analyze-missing-directives/MISSING-DIRECTIVES.md - Quick reference
- .ai/tasks/2026-02-16_analyze-missing-directives/IMPLEMENTATION-PLAN.md - Detailed TDD guide
```

### Examples
```
- src/syntax/scmasm_syntax.cpp - HandleDo() for control flow patterns
- src/syntax/scmasm_directive_handlers.cpp - HandleDummy() for dummy section pattern
```

---

## Assumptions

```
1. Analysis from task xasm++-55bg is accurate and complete
2. TDD approach will be followed (RED-GREEN-REFACTOR)
3. Build and test infrastructure is working
4. A2osX.S.txt is available for validation testing
5. Control flow directives follow existing patterns (not in registry)
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Dummy section state synchronization
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Explicit state machine with start/end methods, comprehensive tests

2. Control flow abbreviations breaking existing .FIN/.ELSE
   - Probability: Low
   - Impact: High
   - Mitigation: Update all scanning logic, test both forms, no removal of existing checks

3. Nested conditional confusion
   - Probability: Low
   - Impact: Medium
   - Mitigation: Same nesting logic for abbreviations, test mixed forms
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: Orchestrator (2026-02-16)
- [ ] Agent: Engineer (to be approved upon claiming task)

**Changes to Contract:**
None

---

## Notes

Implementation follows detailed TDD guide in `.ai/tasks/2026-02-16_analyze-missing-directives/IMPLEMENTATION-PLAN.md`.

Estimated timeline:
- .ED implementation: 2-3 hours (state tracking + validation)
- .FI implementation: 15 minutes (simple alias)
- .EL implementation: 15 minutes (simple alias)
- Integration testing: 30 minutes
- **Total: 3-4 hours**

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
