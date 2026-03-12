# Task Contract

**Task ID:** 2026-01-27_extract-magic-numbers
**Created:** 2026-01-27
**Beads Task:** xasm++-epp
**Parent Task:** 2026-01-26_code-quality-refactoring (xasm++-hxm)
**Requestor:** Orchestrator (Code Quality Phase)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Replace hardcoded opcode magic numbers with named constants or enums. This is [M3] Magic Numbers from the comprehensive code quality review.

### Background and Context

The codebase contains many hardcoded opcodes like 0xA9, 0xF0, 0x20 without explanation, making code hard to understand and maintain. These should be replaced with descriptive named constants.

### Current State

- Hardcoded opcodes throughout assembler.cpp and cpu_6502.cpp
- No named constants or enums for opcodes
- Code difficult to understand (what is 0xA9?)
- All 451 tests passing

### Desired State

- All magic opcode numbers replaced with named constants
- Constants organized logically (enum class or namespace)
- Comments explain what each opcode does
- All 451 tests still passing
- Code self-documenting and maintainable

---

## Success Criteria

```
✓ All magic opcodes replaced with named constants
✓ Constants organized (enum class or const namespace)
✓ Comments explain each opcode
✓ All 451 tests passing (zero regressions)
✓ Zero new compiler warnings
```

---

## Technical Specification

### Approach

Create an enum class or constants namespace for 6502 opcodes:

```cpp
namespace Opcodes {
    constexpr uint8_t LDA_IMM = 0xA9;   // Load Accumulator Immediate
    constexpr uint8_t BEQ = 0xF0;       // Branch if Equal
    constexpr uint8_t JSR = 0x20;       // Jump to Subroutine
    // ... etc
}
```

Or:
```cpp
enum class Opcode : uint8_t {
    LDA_IMM = 0xA9,   // Load Accumulator Immediate
    BEQ = 0xF0,       // Branch if Equal  
    JSR = 0x20,       // Jump to Subroutine
    // ... etc
};
```

### Scope

Find all hardcoded opcodes in:
- `src/cpu/m6502/cpu_6502.cpp`
- `src/core/assembler.cpp`  
- Any other files with opcodes

Replace with named constants and add comments.

### Constraints

- ZERO behavior changes (pure refactoring)
- ZERO test regressions (all 451 tests must pass)
- Must be C++17 compatible
- Follow project naming conventions

---

## Lean Flow Analysis

**Files:** 3-5 files (cpu_6502.h, cpu_6502.cpp, assembler.cpp, possibly new opcodes.h)
**Tokens:** ~10K tokens
**Status:** ✅ SAFE - Small batch, proceed

---

## Resources

- `src/cpu/m6502/cpu_6502.cpp` - Main CPU implementation
- `include/xasm++/cpu/cpu_6502.h` - Header
- `src/core/assembler.cpp` - Assembler
- 6502 opcode reference: http://www.6502.org/tutorials/6502opcodes.html
- Parent: `.ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md`

---

## Acceptance Criteria

- [ ] Opcode constants defined (enum or namespace)
- [ ] All magic numbers replaced
- [ ] Comments added for each opcode
- [ ] All 451 tests passing
- [ ] Zero compiler warnings
- [ ] Code self-documenting
