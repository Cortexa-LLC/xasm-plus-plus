# Task Contract

**Task ID:** 2026-02-12_6809-indexed
**Beads Task:** xasm++-qtrx
**Created:** 2026-02-12
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement 11 6809 indexed addressing mode variants with post-byte encoding.

### Background and Context

The 6809 CPU has sophisticated indexed addressing modes that are fundamental for real programs. Currently only basic addressing modes are implemented. This task adds all indexed mode variants with proper post-byte encoding, offset calculation, and indirect mode support.

### Current State

- Basic 6809 instructions implemented
- Indexed addressing modes marked TODO
- Post-byte encoding not implemented

### Desired State

All 11 indexed addressing variants:
- Register variants: ,X ,Y ,U ,S ,PCR
- Offset sizes: 5-bit, 8-bit, 16-bit
- Indirect modes: [,X] etc.
- Post-byte encoding correct

---

## Success Criteria

```
✓ All 11 indexed variants implemented
✓ Post-byte encoding correct
✓ Offset calculation working (5/8/16-bit)
✓ Indirect modes supported
✓ All tests passing
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ ,X ,Y ,U ,S register indexed
□ ,PCR program counter relative
□ 5-bit offset (no post-byte)
□ 8-bit offset (post-byte + 1 byte)
□ 16-bit offset (post-byte + 2 bytes)
□ Indirect modes [,R]
□ Auto-increment/decrement
□ Post-byte encoding matches 6809 spec
```

### Quality Requirements
```
□ TDD process followed
□ Tests for all addressing variants
□ Opcode verification against 6809 docs
□ No reduction in test pass rate
```

---

## Technical Approach

**Files to Modify:**
- src/cpu/cpu_6809.cpp (indexed mode implementation)
- Post-byte generation logic
- Tests

**Estimated:** 4-6 hours, 2 files, ~12K tokens

---

## Related Documents

- **Reference:** docs/planning/not-yet-implemented-items.md (Section 6)
- **Architecture:** docs/reference/cpu/6809-levanthal.txt
- **Verification:** docs/architecture/verification-6809-opcodes.md
