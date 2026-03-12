# Task Contract: Fix FLEX ASM Phase 2 Macro Test Failures

**Task ID:** 2026-02-09_fix-flex-macro-tests
**Created:** 2026-02-09
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Bugfix
**Beads Task:** xasm++-iu8v

---

## Task Description

Fix 2 failing tests in FLEX ASM Phase 2 Macro System to achieve 100% test pass rate (18/18 tests).

### Background and Context

FLEX ASM Phase 2 (Macro System) implementation is complete but has 2 failing tests related to local label generation in macro expansion. This is blocking commit of the macro implementation and completion of Phase 2. FLEX Phase 3 (Conditionals) is already committed, but Phase 2 must be fixed and committed separately.

### Current State

- 16/18 tests passing (88.9%)
- 2 tests failing:
  - `FlexMacroIntegrationTest.MacroWithMultipleLocalLabels`
  - `FlexMacroIntegrationTest.CompleteProgram`
- Root cause identified: Local label generation in macro expansion
- Implementation files uncommitted: macro_processor.h/cpp, test_flex_macros.cpp

### Desired State

- 18/18 tests passing (100%)
- All tests green
- Code ready for commit
- FLEX Phase 2 complete and committable

---

## Success Criteria

```
✓ All 18 FLEX macro tests passing (100%)
✓ Zero regression in 16 currently passing tests
✓ Build completes with zero compiler warnings
✓ Code follows existing patterns
✓ Work log documents fix approach
```

---

## Acceptance Criteria

### Functional Requirements
```
□ MacroWithMultipleLocalLabels test passes
□ CompleteProgram test passes
□ All 16 previously passing tests still pass
□ Local labels generate unique symbols per macro expansion
□ Multiple local labels in one macro each get unique names
```

### Quality Requirements
```
□ All tests passing (18/18 = 100%)
□ Code coverage maintained
□ Zero compiler warnings
□ Code review approved
□ Work log updated with fix details
```

### Non-Functional Requirements
```
□ Follows Google C++ Style Guide
□ Implementation consistent with existing patterns
□ Error handling robust
□ Documentation complete
```

---

## Constraints and Dependencies

### Constraints
```
□ Must fix without breaking existing 16 tests
□ Must follow existing macro_processor architecture
□ Must not introduce new dependencies
```

### Dependencies
```
□ FLEX Phase 2 implementation (exists, needs fixing)
□ Google Test framework (available)
```

### Out of Scope
```
✗ Adding new macro features
✗ Refactoring unrelated code
✗ Performance optimizations (unless required)
✗ FLEX Phase 3 changes
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Number of files affected: 3 (macro_processor.h/cpp, test_flex_macros.cpp)
- Lines of code estimate: ~50-100 lines to modify
- New concepts/patterns: No (fix existing implementation)
- Integration complexity: Low (isolated to macro system)
- Risk level: Low (test-driven fix)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 3 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 3 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
3 × 3,000 tokens = 9,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 9,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (Engineer)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [x] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

---

## Resources and References

### Relevant Files
```
- include/xasm++/syntax/macro_processor.h - Macro processor interface
- src/syntax/flex/macro_processor.cpp - Macro expansion implementation
- tests/unit/test_flex_macros.cpp - Macro system tests (2 failing)
```

### Documentation
```
- FLEX ASM09 reference manual
- .ai/uncommitted-work-review.md - Investigation results
```

### Examples
```
- test_flex_macros.cpp - Working test cases for reference
```

---

## Assumptions

```
1. Root cause is correctly identified (local label generation)
2. Fix is localized to macro_processor.cpp
3. Test framework is working correctly
4. Other 16 tests validate correct behavior
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Fix breaks existing tests
   - Probability: Low
   - Impact: Medium
   - Mitigation: Run full test suite after fix, TDD approach

2. Root cause is more complex than identified
   - Probability: Low
   - Impact: Medium
   - Mitigation: Debug failing tests first, understand exact failure mode
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: Bryan Woodruff (Orchestrator) 2026-02-09
- [ ] Agent: Engineer [Pending]

---

## Notes

**Technical Context:**

FLEX ASM Macro System features:
- MACRO/ENDM directives define reusable code blocks
- Parameters passed to macros via \1, \2, etc.
- Local labels (prefixed with ?) are unique per expansion
- Multiple macro expansions must not generate conflicting symbols

**Known Issue:**
Current implementation likely has one of these problems:
1. Local label counter not properly scoped per expansion
2. Multiple local labels in one macro share the same generated suffix
3. Local label prefix/suffix algorithm doesn't ensure uniqueness

**Expected Fix Pattern:**
```cpp
// Current (broken):
local_label_counter++;  // Shared across all labels

// Fixed (likely):
expansion_counter++;  // Per macro expansion
local_label_index++;  // Per label within expansion
generated_name = "?" + original_name + "." + expansion_counter + "." + local_label_index;
```

**Example:**
```assembly
MyMacro MACRO
?start: NOP         ; Local label 1
       BRA ?end     ; Local label 2
?end:   RTS
       ENDM

MyMacro             ; Expansion 1: ?start.1.1, ?end.1.2
MyMacro             ; Expansion 2: ?start.2.1, ?end.2.2
```

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
