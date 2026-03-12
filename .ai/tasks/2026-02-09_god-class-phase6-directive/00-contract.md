# Task Contract: God-Class Phase 6 - Directive Handler

**Task ID:** 2026-02-09_god-class-phase6-directive
**Beads Task:** xasm++-3mwi
**Created:** 2026-02-09
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract directive handling methods into DirectiveHandler registry pattern to enable directive plugins and improve long-term extensibility.

### Background and Context

From god-class investigation and recent DirectiveRegistry work:
- Parsers have 20+ directive handling methods
- DirectiveRegistry pattern already implemented for dispatch
- Directive constants already defined (xasm++-6y60)
- Need to complete the extraction by creating DirectiveHandler abstraction

This is the final phase of god-class refactoring, building on DirectiveRegistry foundation.

### Current State

Parsers use DirectiveRegistry for dispatch but:
- Handler logic still embedded in parser classes
- No abstraction for directive handlers
- Cannot add directives without modifying parsers
- No plugin architecture for directives

### Desired State

DirectiveHandler pattern that enables:
- Pluggable directive handlers
- Directive handlers as separate components
- Easy addition of new directives without parser changes
- Clear separation between dispatch and handling

---

## Success Criteria

```
✓ DirectiveHandler abstraction created
✓ Existing directives refactored to use handler pattern
✓ Plugin architecture enables adding directives
✓ All parser tests pass after refactoring
✓ Clear pathway for directive extensibility
```

---

## Acceptance Criteria

### Functional Requirements
```
□ DirectiveHandler base class/interface created
□ Common directives (ORG, EQU, DB, DW, DS) use handler pattern
□ Directive plugin registration mechanism
□ DirectiveRegistry integrated with handlers
□ Parser classes simplified (directive logic moved out)
```

### Quality Requirements
```
□ All tests passing (97%+ pass rate maintained)
□ Unit tests for DirectiveHandler
□ No new compilation warnings
□ TDD process followed
```

### Non-Functional Requirements
```
□ Performance: Handler dispatch efficient
□ Extensibility: Easy to add new directives
□ Maintainability: Clear handler separation
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Files affected: 8-10
- Lines of code: ~1,500 total changes
- Builds on existing DirectiveRegistry work
- Integration complexity: Medium
- Risk level: Low-Medium (registry already in place)

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 9 files

**Batch Size:** ✅ ACCEPTABLE (6-14 files)

**Token Budget:** ~27K tokens - ✅ SAFE

**WIP:** 1 agent - ✅ IDEAL

---

## Dependencies

**Depends on:**
- ✅ DirectiveRegistry (completed - recent work)
- ✅ Directive constants (completed - xasm++-6y60)

**Blocks:**
- None (final phase of god-class refactoring)

---

## Resources

**Relevant Files:**
- DirectiveRegistry: `include/xasm++/syntax/directive_registry.h`
- Directive constants: `include/xasm++/directives/*.h`
- Parsers: `src/syntax/merlin/`, `src/syntax/scmasm/`, `src/syntax/edtasm_m80_plusplus/`

---

## Approvals

**Contract Approved By:**
- [X] Orchestrator: 2026-02-10

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-10
