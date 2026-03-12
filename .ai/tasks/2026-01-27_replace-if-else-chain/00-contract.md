# Task Contract

**Task ID:** 2026-01-27_replace-if-else-chain
**Created:** 2026-01-27
**Beads Task:** xasm++-zd3
**Parent Task:** 2026-01-26_code-quality-refactoring (xasm++-hxm)
**Requestor:** Orchestrator (Code Quality Phase)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Replace the 208-line sequential if-else instruction dispatch chain with a hash map lookup table for O(1) performance. This is [M2] Massive If-Else Chain from the comprehensive code quality review.

### Background and Context

The current instruction dispatch in assembler.cpp uses a sequential if-else chain that checks instruction names one-by-one (O(n) lookup). This is slow and violates the Open-Closed Principle (hard to extend).

### Current State

- 208-line if-else chain for instruction dispatch
- O(n) lookup performance (sequential search)
- Hard to add new instructions
- All 451 tests passing

### Desired State

- Hash map lookup table (`std::unordered_map<std::string, InstructionHandler>`)
- O(1) lookup performance
- Easy to add new instructions
- All 451 tests still passing
- Measurable performance improvement

---

## Success Criteria

```
✓ If-else chain replaced with std::unordered_map
✓ O(1) instruction lookup (constant time)
✓ All 451 tests passing (zero regressions)
✓ Performance improved (benchmark shows improvement)
✓ Adding new instruction is simple (demonstrate)
✓ Zero new compiler warnings
```

---

## Technical Specification

### Approach

Replace the if-else chain with a hash map initialized at construction:

```cpp
class Assembler {
private:
  using InstructionHandler = std::function<void(...)>;
  std::unordered_map<std::string, InstructionHandler> instruction_handlers_;
  
  void InitializeInstructionHandlers();
};
```

### Scope

Find and replace the instruction dispatch if-else chain (likely in a parsing or encoding method). Convert to hash map lookup.

**DO NOT** change instruction encoding logic - only the dispatch mechanism.

### Constraints

- ZERO behavior changes (pure refactoring)
- ZERO test regressions (all 451 tests must pass)
- Must maintain case-sensitivity for instruction names
- Performance should improve (measure with benchmark)

---

## Performance Benchmark

Create a simple benchmark to demonstrate improvement:
- Assemble a file with 1000+ instructions
- Measure time before and after
- Document improvement in work log

---

## Lean Flow Analysis

**Files:** 2-3 files (assembler.h, assembler.cpp, possibly test)
**Tokens:** ~8K tokens
**Status:** ✅ SAFE - Small batch, proceed

---

## Resources

- `src/core/assembler.cpp` - Find if-else chain
- `include/xasm++/assembler.h` - Header
- `tests/unit/test_assembler.cpp` - Tests
- Parent: `.ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md`

---

## Acceptance Criteria

- [ ] If-else chain identified and replaced
- [ ] Hash map initialized at construction
- [ ] All 451 tests passing
- [ ] Performance benchmark shows improvement
- [ ] Zero compiler warnings
- [ ] Code compiles successfully
- [ ] Work log updated with benchmark results
