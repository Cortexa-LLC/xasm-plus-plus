# Task Contract: Z80 - Implement Missing Opcodes

**Task ID:** 2026-02-10_z80-missing-opcodes
**Beads Task:** xasm++-18tk
**Created:** 2026-02-10
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement missing Z80 opcodes to increase coverage from 35% to 100% based on verification report findings.

### Background and Context

Opcode verification (xasm++-1491) found that opcodes_z80.h only contains 243 opcodes (~35% of full Z80 instruction set). All defined opcodes are correct, but many common opcodes are missing. The verification report identifies high-value missing opcodes needed for real Z80 code.

### Current State

- 243 opcodes defined (35% coverage)
- Missing: LD variants, RLD/RRD, I/O instructions, some arithmetic/logical ops
- All existing opcodes verified 100% correct

### Desired State

Complete Z80 opcode coverage with all missing opcodes implemented in opcodes_z80.h and cpu_z80.cpp.

---

## Success Criteria

```
✓ Missing LD variants added
✓ RLD/RRD digit rotate added
✓ Complete I/O instruction set
✓ All tests passing (97%+)
✓ Coverage documented
```

---

## Acceptance Criteria

### Functional Requirements
```
□ LD A,(BC) = $0A, LD A,(DE) = $1A
□ LD (BC),A = $02, LD (DE),A = $12
□ LD (HL),n = $36
□ LD HL,(nn) = $2A, LD (nn),HL = $22
□ LD SP,HL = $F9
□ RLD = ED6F, RRD = ED67
□ IN r,(C) for all registers (ED prefix)
□ OUT (C),r for all registers (ED prefix)
□ Integration with cpu_z80.cpp encoder
□ Comprehensive tests
```

### Quality Requirements
```
□ All tests passing
□ No warnings
□ TDD followed
```

---

## Estimated Complexity

**Complexity:** Medium

**Files:** 3 (opcodes_z80.h, cpu_z80.cpp, tests)
**Tokens:** 12K → ✅ SAFE

---

## Dependencies

**Reference:** docs/architecture/verification-z80-opcodes.md (gap analysis)

---

**Contract Approved:** ✓ Orchestrator 2026-02-10
