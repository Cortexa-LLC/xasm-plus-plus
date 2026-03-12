# Task Contract: Fix Lambda Anti-pattern in Core Directive Handlers

**Task ID:** 2026-02-14_core-directive-lambdas
**Beads Task:** xasm++-cws7
**Created:** 2026-02-14
**Assigned Role:** Engineer
**Workflow:** Refactor
**Priority:** P0 - CRITICAL

---

## Task Description

### Background and Context
The architectural review identified that `src/syntax/core_directive_handlers.cpp` uses lambda wrappers instead of direct function assignment, violating the Merlin reference pattern used by all other syntax parsers.

### Current State
```cpp
// WRONG: Lambda wrapper pattern
registry.Register(INC, [&](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  HandleIncDirective(operand, label, *ctx.section, *ctx.symbols);
});
```

### Desired State
```cpp
// CORRECT: Direct assignment pattern (Merlin reference)
namespace xasm { namespace core {
void HandleInc(const std::string &label, const std::string &operand, DirectiveContext &context);
}}

registry.Register(INC, core::HandleInc);  // No lambda
```

---

## Success Criteria

✓ All handlers use signature: `(label, operand, context)`
✓ Zero lambda wrappers in `RegisterCoreDirectiveHandlers()`
✓ Pattern matches Merlin reference (src/syntax/merlin_directive_handlers.cpp:458-500)
✓ All tests pass (1538/1538)
✓ Binary output identical

---

## Acceptance Criteria

### Functional Requirements
- [ ] All handlers refactored to `(label, operand, DirectiveContext&)` signature
- [ ] All handlers access state through `DirectiveContext` members
- [ ] `RegisterCoreDirectiveHandlers()` uses direct assignment only

### Quality Requirements
- [ ] All unit tests passing
- [ ] All integration tests passing
- [ ] Binary compatibility verified
- [ ] Zero compiler warnings

---

## Constraints and Dependencies

### Dependencies
**Blocks:** P1 Task 5 (Parameter Order Consistency)
**Blocked By:** None

### Out of Scope
✗ Other syntax parsers (already converted)
✗ Parameter order standardization (P1 task)

---

## Estimated Complexity

**Complexity:** Medium
**Files affected:** 2 (core_directive_handlers.cpp + .h)
**Handlers to refactor:** ~12 handlers
**Lines of code:** ~200-300 LOC
**Risk level:** Low (pattern well-established)

---

## Lean Flow Analysis

### Batch Size Assessment
**Estimated Files:** 2 files
**Status:** ✅ IDEAL (Small batch)

### Token Budget Estimation
```
2 files × 3,000 tokens = 6,000 tokens
Status: ✅ SAFE (<20K limit)
```

### Work In Progress Planning
**Agents:** 1 (this task only)
**WIP Status:** ✅ Within limits

### Decomposition Decision
- [x] YES - Proceed as single task (2 files, 6K tokens, WIP=1)

---

## Resources and References

### Relevant Files
- `src/syntax/core_directive_handlers.cpp` - Implementation to refactor
- `include/xasm++/directives/core_directive_handlers.h` - Declarations
- `src/syntax/merlin_directive_handlers.cpp:458-500` - Reference pattern

### Documentation
- `.ai/tasks/2026-02-14_architectural-review/30-review.md` - Issue details
- `.ai/tasks/2026-02-14_architectural-review/NEXT-STEPS.md` - Task breakdown

---

## Risk Assessment

### Identified Risks
1. **Breaking directive dispatch**
   - Probability: Low
   - Impact: High
   - Mitigation: Run tests after each handler conversion

2. **DirectiveContext access errors**
   - Probability: Low
   - Impact: Medium
   - Mitigation: Follow Merlin reference strictly

---

## Approvals

**Contract Approved By:**
- [x] Orchestrator: AI Orchestrator (2026-02-14)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-14
