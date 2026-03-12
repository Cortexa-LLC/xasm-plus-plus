# Task Contract

**Task ID:** 2026-01-27_split-cpu6502-god-class
**Created:** 2026-01-27
**Beads Task:** xasm++-ahu
**Parent Task:** 2026-01-26_code-quality-refactoring (xasm++-hxm)
**Requestor:** Orchestrator (Code Quality Phase)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Split the 100+ method Cpu6502 God Class into focused classes following Single Responsibility Principle. This is [M6] God Class from the comprehensive code quality review.

### Background and Context

The Cpu6502 class has 100+ public methods and violates both Single Responsibility Principle (SRP) and Interface Segregation Principle (ISP). It handles instruction encoding, addressing mode decoding, branch relaxation, and validation all in one massive class.

### Current State

- Cpu6502 class: 100+ public methods
- Single class with multiple responsibilities
- 1353 lines in cpu_6502.cpp, 222 lines in header
- All 478 tests passing

### Desired State

- Split into 3-4 focused classes with <30 public methods each
- Clear separation of concerns:
  * Core instruction encoding
  * Addressing mode logic
  * Branch handling & relaxation
  * Validation
- All 478 tests still passing
- No performance regression

---

## Success Criteria

```
✓ Cpu6502 split into 3-4 focused classes
✓ Each class has <30 public methods
✓ Clear separation of concerns
✓ All 478 tests passing (zero regressions)
✓ Zero new compiler warnings
✓ No performance regression
```

---

## Technical Specification

### Proposed Class Structure

**WARNING:** This is a LARGE refactoring (1575 lines). HIGH RISK of token limits.

**Recommended Approach:** This task should be DECOMPOSED into subtasks.

### Option A: Complete in One Task (RISKY)

Split into:
1. **Cpu6502** - Main interface, delegates to components
2. **Cpu6502Encoder** - Instruction encoding (Encode* methods)
3. **Cpu6502AddressingModes** - Addressing mode logic
4. **Cpu6502BranchHandler** - Branch relaxation & complement

Use composition in Cpu6502:
```cpp
class Cpu6502 {
private:
    Cpu6502Encoder encoder_;
    Cpu6502AddressingModes addressing_;
    Cpu6502BranchHandler branch_handler_;
public:
    // Delegate to components
    std::vector<uint8_t> EncodeLDA(uint16_t value, AddressingMode mode) {
        return encoder_.EncodeLDA(value, mode);
    }
};
```

### Option B: Decompose into Subtasks (RECOMMENDED)

Given the size (1575 lines), this should be split into subtasks:

**Subtask M6.1:** Extract Cpu6502AddressingModes (smaller, ~100 lines)
**Subtask M6.2:** Extract Cpu6502BranchHandler (~150 lines)
**Subtask M6.3:** Extract Cpu6502Encoder (largest, ~1000 lines)
**Subtask M6.4:** Refactor Cpu6502 to use composition

Each subtask can be completed within token limits.

---

## Decomposition Decision

**Assessment:**
- Files: 2 files (cpu_6502.h, cpu_6502.cpp)
- Total lines: 1575
- Estimated tokens: 1575 × 3 = ~4.7K tokens to read
- Modifications: Extensive refactoring = ~20-40K tokens
- Agent output: Complex restructuring = ~30-50K tokens
- **TOTAL ESTIMATED: 60-100K tokens**

**Decision:** ⚠️ HIGH RISK - Should decompose

**Recommendation:** Create subtasks M6.1 - M6.4 as described above.

---

## Constraints

- ZERO behavior changes (pure refactoring)
- ZERO test regressions (all 478 tests must pass)
- Maintain binary compatibility if possible
- No performance degradation
- C++17 compatible

---

## Resources

- `include/xasm++/cpu/cpu_6502.h` - Header (222 lines)
- `src/cpu/m6502/cpu_6502.cpp` - Implementation (1353 lines)
- `tests/unit/test_cpu6502.cpp` - Tests
- Parent: `.ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md`

---

## Acceptance Criteria

- [ ] Classes split with clear responsibilities
- [ ] Each class <30 public methods
- [ ] Composition used in Cpu6502
- [ ] All 478 tests passing
- [ ] Zero compiler warnings
- [ ] No performance regression (benchmark if possible)
