# Task Contract

**Task ID:** 2026-01-27_extract-branch-handler
**Created:** 2026-01-27
**Beads Task:** xasm++-hpl
**Parent Task:** 2026-01-26_code-quality-refactoring (xasm++-hxm)
**Requestor:** Orchestrator (God Class Decomposition M6.1)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract branch relaxation and complement logic from Cpu6502 into a focused Cpu6502BranchHandler class. This is subtask M6.1 of the God Class refactoring.

### Background and Context

The Cpu6502 class is a large class with multiple responsibilities with 100+ methods. This subtask extracts ONLY the branch-related logic (~150 lines) into a separate handler class.

### Current State

- Branch relaxation logic embedded in Cpu6502
- Methods: RelaxBranch(), GetBranchComplement(), branch encoding
- All 478 tests passing

### Desired State

- Cpu6502BranchHandler class created
- Branch relaxation logic extracted
- Cpu6502 delegates to branch handler
- All 478 tests passing

---

## Success Criteria

```
✓ Cpu6502BranchHandler class created
✓ RelaxBranch() and GetBranchComplement() extracted
✓ Cpu6502 uses composition with branch handler
✓ All 478 tests passing (zero regressions)
✓ Zero new compiler warnings
```

---

## Technical Specification

### New Class Structure

```cpp
// include/xasm++/cpu/cpu_6502_branch_handler.h
class Cpu6502BranchHandler {
public:
    std::vector<uint8_t> RelaxBranch(uint8_t branch_opcode, int16_t offset);
    uint8_t GetBranchComplement(uint8_t opcode) const;
};
```

### Modified Cpu6502

```cpp
class Cpu6502 {
private:
    Cpu6502BranchHandler branch_handler_;  // New member
public:
    // Delegate to branch handler
    std::vector<uint8_t> RelaxBranch(uint8_t opcode, int16_t offset) {
        return branch_handler_.RelaxBranch(opcode, offset);
    }
};
```

### Scope

Extract from cpu_6502.cpp:
- RelaxBranch() method (~50 lines)
- GetBranchComplement() method (~50 lines)
- Related branch encoding logic

Create:
- include/xasm++/cpu/cpu_6502_branch_handler.h (new file)
- src/cpu/m6502/cpu_6502_branch_handler.cpp (new file)

Modify:
- include/xasm++/cpu/cpu_6502.h (add member, delegate)
- src/cpu/m6502/cpu_6502.cpp (remove extracted code)

### Constraints

- ZERO behavior changes (pure refactoring)
- ZERO test regressions
- Keep branch relaxation logic identical

---

## Lean Flow Analysis

**Files:** 4 files (2 new, 2 modified)
**Lines:** ~150 lines extracted, ~50 lines added for delegation
**Tokens:** ~6K tokens
**Status:** ✅ SAFE - Small focused refactoring

---

## Resources

- `include/xasm++/cpu/cpu_6502.h` - Header to modify
- `src/cpu/m6502/cpu_6502.cpp` - Extract from here
- `tests/unit/test_cpu6502.cpp` - Verify tests pass
- Parent: `.ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md`
