# Task Contract: Fix CPU 6809 Magic Opcodes

**Task ID:** 2026-02-14_cpu6809-opcodes
**Beads Task:** xasm++-oi41
**Created:** 2026-02-14
**Assigned Role:** Engineer
**Workflow:** Refactor
**Priority:** P0 - CRITICAL

---

## Task Description

### Background
Architectural review found 215 magic hex opcode values in CPU 6809 implementation - the largest concentration of magic values in the codebase.

### Current State
```cpp
// WRONG: Magic values everywhere
if (opcode == 0x00) { /* NEG direct */ }
if (opcode == 0x01) { /* OIM direct */ }
// ... 213 more magic values
```

### Desired State
```cpp
// CORRECT: Named constants
namespace Opcodes {
  constexpr uint8_t NEG_DIRECT = 0x00;
  constexpr uint8_t OIM_DIRECT = 0x01;
  // ... etc
}
if (opcode == Opcodes::NEG_DIRECT) { /* NEG direct */ }
```

---

## Success Criteria

✓ Zero magic hex values in cpu_6809.cpp (215 → 0)
✓ 200+ opcode constants created in opcodes_6809.h
✓ Binary output identical (byte-for-byte)
✓ All tests pass (1538/1538)
✓ Addressing mode documentation added

---

## Acceptance Criteria

### Functional Requirements
- [ ] All 215 magic hex values replaced with Opcodes:: constants
- [ ] Comprehensive opcodes_6809.h created with 200+ constants
- [ ] Addressing mode encoding documented in header
- [ ] Naming convention: MNEMONIC_ADDRESSINGMODE

### Quality Requirements
- [ ] Binary output identical to baseline
- [ ] All tests passing
- [ ] Zero compiler warnings
- [ ] Pattern matches Rockwell/Z80 reference

---

## Constraints and Dependencies

### Dependencies
**Blocks:** None (independent P0 task)
**Blocked By:** None

### Out of Scope
✗ CPU 6502/Z80 opcodes (separate tasks)
✗ Other 6809 magic values not related to opcodes

---

## Estimated Complexity

**Complexity:** Very Large
**Files affected:** 2 (cpu_6809.cpp, opcodes_6809.h)
**Constants to add:** ~200 opcodes
**Values to replace:** 215 magic hex values
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
- [x] YES - Proceed as single task (2 files, safe despite large constant count)

---

## Resources and References

### Relevant Files
- `src/cpu/cpu_6809.cpp` - Implementation with 215 magic values
- `include/xasm++/cpu/opcodes_6809.h` - Opcode constants (to be created)
- `include/xasm++/cpu/opcodes_z80.h` - Reference pattern (good example)
- `src/cpu/cpu_rockwell.cpp` - Reference pattern (commit 7341e43)

### Documentation
- Architectural Review: `.ai/tasks/2026-02-14_architectural-review/30-review.md`
- 6809 Programming Manual for opcode reference

---

## Risk Assessment

1. **Breaking instruction encoding**
   - Probability: Low
   - Impact: High
   - Mitigation: Binary output verification, incremental refactor

2. **Opcode collision/duplication**
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Careful naming with addressing mode suffixes

3. **Large change volume**
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Change one instruction group at a time, test frequently

---

## Approvals

**Contract Approved By:**
- [x] Orchestrator: AI Orchestrator (2026-02-14)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-14
