# Task Contract: God-Class Phase 5 - Macro Processor

**Task ID:** 2026-02-09_god-class-phase5-macro
**Beads Task:** xasm++-n197
**Created:** 2026-02-09
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract macro processing methods into shared MacroProcessor component to eliminate duplication and isolate complex state management.

### Background and Context

From god-class investigation, parsers have 7 macro-related methods duplicated:
- Macro definition (MAC/MACRO)
- Macro end (EOM/ENDM)
- Macro expansion
- Parameter substitution
- Macro invocation tracking
- Nested macro support
- Macro symbol table management

Macro processing involves complex state management that's currently scattered across parsers.

### Current State

Each parser implements its own macro handling with duplicated logic for:
- Macro definition storage
- Parameter parsing and substitution
- Macro expansion during assembly
- Nested macro tracking
- Macro symbol management

### Desired State

Shared MacroProcessor component that all parsers can use:
- `include/xasm++/common/macro_processor.h`
- `src/common/macro_processor.cpp`
- Unit tests for macro definition, expansion, parameters
- Integrated into all parsers

---

## Success Criteria

```
✓ MacroProcessor component created and tested
✓ Supports macro definition and expansion
✓ Handles parameter substitution correctly
✓ Supports nested macros
✓ All parser tests pass after integration
✓ Isolates complex state management
```

---

## Acceptance Criteria

### Functional Requirements
```
□ MacroProcessor handles MAC/MACRO/ENDM/EOM
□ Parameter substitution works (\1, \2, etc.)
□ Macro expansion during assembly
□ Nested macro support
□ Macro symbol table integration
□ Local labels in macros
□ Macro invocation counting
```

### Quality Requirements
```
□ All tests passing (97%+ pass rate maintained)
□ Unit tests for MacroProcessor
□ No new compilation warnings
□ TDD process followed
```

### Non-Functional Requirements
```
□ Performance: Macro expansion efficient
□ Memory: Macro storage reasonable
□ Error messages: Clear macro errors
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Files affected: 6-8
- Lines of code: ~1,200 total changes
- State management complexity: Medium-High
- Integration complexity: Medium
- Risk level: Medium

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 7 files

**Batch Size:** ✅ ACCEPTABLE (6-14 files)

**Token Budget:** ~21K tokens - ✅ SAFE

**WIP:** 1 agent - ✅ IDEAL

---

## Dependencies

**Depends on:**
- ✅ Phase 1: ExpressionParser (completed)

**Blocks:**
- None (independent of other phases)

---

## Resources

**Relevant Files:**
- Reference: `.ai/tasks/2026-02-09_god-class-investigation/`
- Parsers: `src/syntax/merlin/`, `src/syntax/scmasm/`, `src/syntax/edtasm_m80_plusplus/`
- Existing macro code: `src/syntax/flex/macro_processor.cpp` (reference)

---

## Approvals

**Contract Approved By:**
- [X] Orchestrator: 2026-02-10

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-10
