# Task Contract

**Task ID:** 2026-01-26_extract-encode-instructions
**Created:** 2026-01-26
**Beads Task:** xasm++-dbd
**Parent Task:** 2026-01-26_code-quality-refactoring (xasm++-hxm)
**Requestor:** Orchestrator (God Method Refactoring Phase 2)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract instruction encoding logic from the 309-line `Assembler::Assemble()` method into a focused private method `EncodeInstructions()`. This is subtask B2 of [M1] God Method Anti-Pattern from the comprehensive code quality review.

### Background and Context

Following the successful extraction of ResolveSymbols() (B1), the Assemble() method is now 309 lines. It still handles instruction encoding along with convergence checking and error handling. This task extracts ONLY the instruction encoding logic.

### Current State

- `Assemble()` is 309 lines (down from 344)
- Instruction encoding logic intermingled with other concerns
- All 451 tests passing
- ResolveSymbols() extraction complete

### Desired State

- `EncodeInstructions()` private method created
- Instruction encoding logic extracted (~80-100 lines)
- `Assemble()` calls `EncodeInstructions()` appropriately
- All 451 tests still passing (zero regressions)
- Behavior identical to before refactoring

---

## Success Criteria

```
✓ New private method EncodeInstructions() created
✓ Instruction encoding logic extracted from Assemble()
✓ Assemble() calls EncodeInstructions() appropriately
✓ All 451 tests passing (zero regressions)
✓ Zero new compiler warnings
```

---

## Technical Specification

### Method Signature
```cpp
private:
  void EncodeInstructions(std::vector<std::shared_ptr<Atom>>& atoms,
                          ConcreteSymbolTable& symbols,
                          uint32_t pass,
                          AssemblerResult& result);
```

### Scope
Extract instruction encoding logic from `Assemble()` - typically includes iterating atoms, encoding InstructionAtoms, calling CPU plugin encode methods. Do NOT extract convergence checking or error reporting.

### Constraints
- ZERO behavior changes (pure refactoring)
- ZERO test regressions (all 451 tests must pass)
- Method must be `private`
- Follow existing code style

---

## Lean Flow Analysis

**Files:** 2 files (assembler.h, assembler.cpp)
**Tokens:** ~6K tokens
**Status:** ✅ SAFE - Small batch, proceed

---

## Resources

- `src/core/assembler.cpp` - Implementation
- `include/xasm++/assembler.h` - Header
- `tests/unit/test_assembler.cpp` - Tests
- Previous: `.ai/tasks/2026-01-26_extract-resolve-symbols/` (B1)
- Parent: `.ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md`
