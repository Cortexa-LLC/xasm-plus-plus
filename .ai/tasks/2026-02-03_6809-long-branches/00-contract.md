# Task Contract: 6809 Long Branch Instructions

**Task ID:** 2026-02-03_6809-long-branches
**Beads Task:** xasm++-8ew
**Created:** 2026-02-03
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement remaining 16-bit relative branch instructions for Motorola 6809 CPU to complete the basic instruction set.

### Background and Context

Phase 2B implemented 8-bit relative branches (BRA, BEQ, BNE, etc.) with offsets from -128 to +127 bytes. Long branches use 16-bit offsets, allowing branches to anywhere in the 64KB address space (-32768 to +32767 bytes).

### Current State

- 8-bit relative branches implemented and tested (Phase 2B complete)
- 78 instructions with 121 tests passing
- No long branch instructions implemented yet

### Desired State

- All 16 long branch instructions implemented
- Comprehensive unit tests
- Documentation updated
- Zero compiler warnings

---

## Success Criteria

```
✓ All 16 long branch instructions implemented in cpu_6809.cpp/h
✓ 16+ unit tests passing (one per instruction minimum)
✓ Big-endian byte order verified
✓ Zero compiler warnings
✓ Documentation complete (Doxygen comments)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ LBRA (Long Branch Always) - 0x10 0x16
□ LBRN (Long Branch Never) - 0x10 0x21
□ LBHI (Long Branch if Higher) - 0x10 0x22
□ LBLS (Long Branch if Lower or Same) - 0x10 0x23
□ LBCC/LBHS (Long Branch if Carry Clear) - 0x10 0x24
□ LBCS/LBLO (Long Branch if Carry Set) - 0x10 0x25
□ LBNE (Long Branch if Not Equal) - 0x10 0x26
□ LBEQ (Long Branch if Equal) - 0x10 0x27
□ LBVC (Long Branch if Overflow Clear) - 0x10 0x28
□ LBVS (Long Branch if Overflow Set) - 0x10 0x29
□ LBPL (Long Branch if Plus) - 0x10 0x2A
□ LBMI (Long Branch if Minus) - 0x10 0x2B
□ LBGE (Long Branch if Greater or Equal) - 0x10 0x2C
□ LBLT (Long Branch if Less Than) - 0x10 0x2D
□ LBGT (Long Branch if Greater Than) - 0x10 0x2E
□ LBLE (Long Branch if Less or Equal) - 0x10 0x2F
□ Big-endian byte order for 16-bit offset (use ToBigEndian() utility)
□ Method signatures: EncodeLBRA(int16_t offset), EncodeLBEQ(int16_t offset), etc.
```

### Quality Requirements
```
□ All tests passing (16+ new tests)
□ Edge case tests (min/max offsets: -32768, +32767)
□ Zero compiler warnings
□ Code review approved
□ Doxygen documentation complete
```

### Encoding Format

```
Opcode: 0x10 [condition_byte]
Offset: 16-bit signed big-endian
Total:  4 bytes

Example: LBEQ $1234 → 10 27 12 34
         LBRA -100  → 10 16 FF 9C
```

---

## Constraints and Dependencies

### Dependencies
```
□ Phase 2B complete (8-bit branches implemented) ✅
□ ToBigEndian() utility function available ✅
□ GoogleTest framework available ✅
```

### Out of Scope
```
✗ Long branches with indexed addressing (separate Phase 3 task)
✗ BSR/LBSR (Branch to Subroutine) - different instruction category
✗ EDTASM+ syntax parser integration (separate Phase 3 task)
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Number of files affected: 3 (cpu_6809.h, cpu_6809.cpp, test_cpu6809.cpp)
- Lines of code estimate: ~200 (16 methods × ~5 lines each + tests)
- New concepts/patterns: No (follows existing branch pattern)
- Integration complexity: Low (extends existing instruction set)
- Risk level: Low (straightforward implementation)

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 3 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
Your Task: 3 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
3 files × 3,000 tokens = 9,000 tokens

Agent Output Limit: 25K-32K tokens

Status: ✅ SAFE (<20K tokens)
```

### Work In Progress Planning

**Concurrent Execution Assessment:**
```
Planned WIP: 1 agent → ✅ IDEAL
```

### Decomposition Decision

**Proceed as single task packet?**
- [x] YES - Batch size = 3 files AND token budget = 9K AND WIP = 1

---

## Resources and References

### Relevant Files
```
- include/xasm++/cpu/cpu_6809.h - Method declarations
- src/cpu/m6809/cpu_6809.cpp - Implementation
- tests/unit/test_cpu6809.cpp - Unit tests
```

### Documentation
```
- docs/architecture/6809-data-models.md - Instruction encoding reference
- docs/architecture/6809-architecture.md - Overall architecture
- Motorola 6809 datasheet - Opcode reference
```

### Examples
```
- Existing short branch methods (EncodeBRA, EncodeBEQ, etc.)
- ToBigEndian() usage in LDX, LDY implementations
```

---

## Risk Assessment

### Identified Risks
```
1. Incorrect opcode values
   - Probability: Low
   - Impact: High
   - Mitigation: Verify against datasheet, comprehensive tests

2. Big-endian byte order errors
   - Probability: Low
   - Impact: Medium
   - Mitigation: Use ToBigEndian() utility, verify with tests
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: User [2026-02-03]
- [ ] Agent: Engineer [Pending]

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-03
