# Task Contract

**Task ID:** 2026-01-26_extract-check-convergence
**Created:** 2026-01-26
**Beads Task:** xasm++-53d
**Parent Task:** 2026-01-26_code-quality-refactoring (xasm++-hxm)
**Requestor:** Orchestrator (God Method Refactoring Phase 2)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract convergence checking logic from the 53-line `Assembler::Assemble()` method into a focused private method `CheckConvergence()`. This is subtask B3 of [M1] God Method Anti-Pattern.

### Background

After ResolveSymbols() (B1) and EncodeInstructions() (B2) extractions, Assemble() is now 53 lines. This task extracts ONLY the convergence checking logic.

### Current State

- Assemble() is 53 lines (down from 344 originally)
- Convergence checking logic still in Assemble()
- All 451 tests passing

### Desired State

- CheckConvergence() private method created
- Convergence checking logic extracted (~20 lines)
- Assemble() calls CheckConvergence()
- All 451 tests passing (zero regressions)

---

## Success Criteria

```
✓ New private method CheckConvergence() created
✓ Convergence checking logic extracted
✓ All 451 tests passing (zero regressions)
✓ Zero new compiler warnings
```

---

## Technical Specification

### Method Signature
```cpp
private:
  bool CheckConvergence(const std::vector<size_t>& previous_sizes,
                        const std::vector<size_t>& current_sizes) const;
```

### Scope
Extract convergence checking logic - comparing previous and current instruction sizes to determine if another pass is needed.

### Constraints
- ZERO behavior changes (pure refactoring)
- ZERO test regressions
- Method must be `private` and `const`

---

## Lean Flow Analysis

**Files:** 2 files
**Tokens:** ~6K
**Status:** ✅ SAFE

---

## Resources

- `src/core/assembler.cpp` - Implementation
- `include/xasm++/assembler.h` - Header
- Previous: `.ai/tasks/2026-01-26_extract-encode-instructions/` (B2)
