# Task Contract: God-Class Phase 4 - Conditional Assembler

**Task ID:** 2026-02-09_god-class-phase4-conditional
**Beads Task:** xasm++-01qi
**Created:** 2026-02-09
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract conditional assembly methods into shared ConditionalAssembler component to eliminate duplication across parsers.

### Background and Context

From god-class investigation, parsers have 14 conditional assembly methods duplicated:
- IF/ELSE/ENDIF conditionals
- IFDEF/IFNDEF symbol checks
- Comparison conditionals (IFEQ, IFNE, IFLT, IFGT, IFLE, IFGE)
- Pass conditionals (IF1, IF2)
- Blank checks (IFB, IFNB)
- String comparison (IFIDN, IFDIF)
- Nested conditional support

This represents ~350 LOC per parser of complex control flow logic.

### Current State

Each parser (Merlin, SCMASM, Z80) implements its own conditional assembly handling with duplicated logic for:
- Conditional stack management
- Expression evaluation in conditionals
- Nested conditional tracking
- Pass tracking (IF1/IF2)

### Desired State

Shared ConditionalAssembler component that all parsers can use:
- `include/xasm++/common/conditional_assembler.h`
- `src/common/conditional_assembler.cpp`
- Unit tests for all conditional types
- Integrated into all 3 parsers

---

## Success Criteria

```
✓ ConditionalAssembler component created and tested
✓ Supports all 14 conditional types
✓ Handles nested conditionals correctly
✓ All parser tests pass after integration
✓ Removes ~350 LOC per parser (total ~1,000 LOC reduction)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ ConditionalAssembler class handles IF/ELSE/ENDIF
□ Supports IFDEF/IFNDEF symbol checks
□ Supports comparison conditionals (6 types)
□ Supports pass conditionals (IF1/IF2)
□ Supports blank checks (IFB/IFNB)
□ Supports string comparison (IFIDN/IFDIF)
□ Maintains conditional stack for nesting
□ Integrates with ExpressionParser
```

### Quality Requirements
```
□ All tests passing (97%+ pass rate maintained)
□ Unit tests for ConditionalAssembler
□ No new compilation warnings
□ TDD process followed (RED-GREEN-REFACTOR)
```

### Non-Functional Requirements
```
□ Performance maintained (conditional evaluation fast)
□ Clear error messages for mismatched conditionals
□ Thread-safe if needed
```

---

## Estimated Complexity

**Complexity:** Medium-Large

**Rationale:**
- Files affected: 6-8 (3 parsers + new component + tests)
- Lines of code: ~1,500 total changes
- New concepts: Conditional stack management
- Integration complexity: Medium (parser integration)
- Risk level: Medium (complex control flow)

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 8 files

**Batch Size:** ✅ ACCEPTABLE (6-14 files)

**Justification:**
- High cohesion - conditional logic tightly coupled
- Single concern - conditional assembly processing
- Already minimal viable batch

**Token Budget:** ~24K tokens (8 × 3K) - ✅ SAFE

**WIP:** 1 agent - ✅ IDEAL

---

## Dependencies

**Depends on:**
- ✅ Phase 1: ExpressionParser (completed - xasm++-9nox)
- ✅ Phase 3: NumberParser (completed - xasm++-ai4z)

**Blocks:**
- None (independent of Phase 5 and 6)

---

## Resources

**Relevant Files:**
- Reference: `.ai/tasks/2026-02-09_god-class-investigation/`
- Parsers: `src/syntax/merlin/`, `src/syntax/scmasm/`, `src/syntax/edtasm_m80_plusplus/`

---

## Approvals

**Contract Approved By:**
- [X] Orchestrator: 2026-02-10

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-10
