# Task Contract

**Task ID:** 2026-01-27_reduce-code-duplication
**Created:** 2026-01-27
**Beads Task:** xasm++-2ug
**Parent Task:** 2026-01-26_code-quality-refactoring (xasm++-hxm)
**Requestor:** Orchestrator (Code Quality Phase)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract common patterns from 57 Encode methods that share 95% identical switch statement logic. This is [M7] Code Duplication from the comprehensive code quality review.

### Background and Context

The Cpu6502 class has 57 Encode methods (EncodeLDA, EncodeSTA, etc.) that each contain nearly identical switch statements for handling addressing modes. This massive duplication creates maintenance burden.

### Current State

- 57 Encode methods with similar structure
- ~95% code duplication in addressing mode switches
- Each method: ~20-30 lines, mostly identical
- Maintenance nightmare: changes require updating all 57 methods
- All 478 tests passing

### Desired State

- Common pattern extracted to helper function or template
- Duplication reduced from 95% to <10%
- Table-driven approach where possible
- Easier to add new instructions
- All 478 tests passing

---

## Success Criteria

```
✓ Duplication reduced from 95% to <10%
✓ Common patterns extracted to shared utilities
✓ All 478 tests passing (zero regressions)
✓ Easier to add new addressing modes (demonstrate)
✓ Zero new compiler warnings
```

---

## Technical Specification

### Current Pattern (Duplicated 57 times)

```cpp
std::vector<uint8_t> Cpu6502::EncodeLDA(uint16_t value, AddressingMode mode) {
    switch (mode) {
        case AddressingMode::Immediate:
            return {Opcode::LDA_IMM, static_cast<uint8_t>(value)};
        case AddressingMode::ZeroPage:
            return {Opcode::LDA_ZP, static_cast<uint8_t>(value)};
        case AddressingMode::Absolute:
            return {Opcode::LDA_ABS, 
                    static_cast<uint8_t>(value), 
                    static_cast<uint8_t>(value >> 8)};
        // ... more cases
    }
}
```

### Proposed Solution: Opcode Table

```cpp
// Opcode table indexed by [instruction][addressing_mode]
struct InstructionOpcodes {
    uint8_t immediate;
    uint8_t zero_page;
    uint8_t absolute;
    // ...
};

const InstructionOpcodes LDA_OPCODES = {
    .immediate = Opcode::LDA_IMM,
    .zero_page = Opcode::LDA_ZP,
    .absolute = Opcode::LDA_ABS,
    // ...
};

// Generic encode function
std::vector<uint8_t> EncodeWithAddressingMode(
    const InstructionOpcodes& opcodes,
    uint16_t value,
    AddressingMode mode
) {
    switch (mode) {
        case AddressingMode::Immediate:
            return {opcodes.immediate, static_cast<uint8_t>(value)};
        case AddressingMode::ZeroPage:
            return {opcodes.zero_page, static_cast<uint8_t>(value)};
        // ... ONE implementation for all instructions
    }
}

// Then each Encode method becomes:
std::vector<uint8_t> Cpu6502::EncodeLDA(uint16_t value, AddressingMode mode) {
    return EncodeWithAddressingMode(LDA_OPCODES, value, mode);
}
```

### Alternative: Template-Based

Could also use templates or function pointers, but table-driven is clearest.

### Scope

- Create opcode tables for all 57 instructions
- Extract common encoding logic to helper function
- Update all 57 Encode methods to use helper
- Verify tests pass

### Constraints

- ZERO behavior changes (pure refactoring)
- ZERO test regressions (all 478 tests must pass)
- Maintain same performance (inline helpers if needed)
- C++17 compatible

---

## Lean Flow Analysis

**WARNING:** This is a LARGE refactoring (57 methods, ~1000+ lines affected).

**Files:** 2 files (cpu_6502.h, cpu_6502.cpp)
**Lines:** ~1000+ lines of duplicated code
**Tokens:** High risk of hitting limits

**Recommendation:** Consider table-driven approach with automated generation
or decompose into multiple passes.

---

## Resources

- `src/cpu/m6502/cpu_6502.cpp` - 57 Encode methods
- `include/xasm++/cpu/cpu_6502.h` - Header
- `tests/unit/test_cpu6502.cpp` - Verify behavior unchanged
- Parent: `.ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md`

---

## Acceptance Criteria

- [ ] Common pattern extracted (table or helper)
- [ ] Duplication <10% (from 95%)
- [ ] All 57 methods use common logic
- [ ] All 478 tests passing
- [ ] Zero compiler warnings
- [ ] Adding new instruction is simple (demonstrate)
