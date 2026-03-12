# Task Contract: Fix CPU 6502 Magic Opcodes

**Task ID:** 2026-02-14_cpu6502-opcodes
**Beads Task:** xasm++-lmzc
**Created:** 2026-02-14
**Assigned Role:** Engineer
**Workflow:** Refactor
**Priority:** P0 - CRITICAL

---

## Task Description

### Background
Architectural review found 72 magic hex opcode values and 5 magic radix values in CPU 6502 implementation, violating self-documenting code principles.

### Current State
```cpp
// WRONG: Magic values
if (opcode == 0xCB) { /* WAI */ }
if (opcode == 0xDB) { /* STP */ }
address = std::stoul(operand, nullptr, 16);  // Magic radix
```

### Desired State
```cpp
// CORRECT: Named constants
if (opcode == Opcodes::WAI) { /* WAI */ }
if (opcode == Opcodes::STP) { /* STP */ }
address = std::stoul(operand, nullptr, RADIX_HEXADECIMAL);
```

---

## Success Criteria

✓ Zero magic hex values in cpu_6502.cpp (72 → 0)
✓ Zero magic radix values (5 → 0)
✓ 100+ opcode constants added to opcodes_6502.h
✓ Binary output identical (byte-for-byte)
✓ All tests pass (1538/1538)

---

## Acceptance Criteria

### Functional Requirements
- [ ] All 72 magic hex values replaced with Opcodes:: constants
- [ ] Missing opcode constants added: WAI, STP, PHX, PLX, PHY, PLY, STZ, BRA, TRB, TSB
- [ ] All addressing mode variants defined
- [ ] Radix constants used: RADIX_HEXADECIMAL, RADIX_DECIMAL

### Quality Requirements
- [ ] Binary output identical to baseline
- [ ] All tests passing
- [ ] Zero compiler warnings
- [ ] Opcode constant naming follows Rockwell pattern

---

## Constraints and Dependencies

### Dependencies
**Blocks:** None (independent P0 task)
**Blocked By:** None

### Out of Scope
✗ CPU 6809/Z80 opcodes (separate tasks)
✗ Other 6502 magic values not related to opcodes

---

## Estimated Complexity

**Complexity:** Large
**Files affected:** 2 (cpu_6502.cpp, opcodes_6502.h)
**Constants to add:** ~100 opcodes
**Values to replace:** 77 total (72 hex + 5 radix)
**Est:** 20-30 hours

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

### Decomposition Decision
- [x] YES - Proceed as single task (2 files, safe token budget)

---

## Resources and References

### Relevant Files
- `src/cpu/cpu_6502.cpp` - Implementation with magic values
- `include/xasm++/cpu/opcodes_6502.h` - Opcode constants
- `src/cpu/cpu_rockwell.cpp` - Reference pattern (commit 7341e43)

### Documentation
- Architectural Review: `.ai/tasks/2026-02-14_architectural-review/30-review.md`
- Rockwell fix commit: 7341e43

---

## Risk Assessment

1. **Breaking instruction encoding**
   - Probability: Low
   - Impact: High
   - Mitigation: Binary output verification after each change

2. **Missing opcode variants**
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Cross-reference with 65C02/65816 instruction sets

---

## Approvals

**Contract Approved By:**
- [x] Orchestrator: AI Orchestrator (2026-02-14)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-14
