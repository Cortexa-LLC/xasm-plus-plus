# Task Contract: Z80 CPU - Replace Magic Numbers with Opcode Constants

**Task ID:** 2026-02-10_z80-cpu-opcode-constants
**Beads Task:** xasm++-vqis
**Created:** 2026-02-10
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Replace magic number opcodes in cpu_z80.cpp with named constants from opcodes_z80.h.

### Background and Context

Following the same pattern as Z80 syntax parser fix (xasm++-xvlw), cpu_z80.cpp currently uses magic numbers like 0x3E, 0x06, 0xC6 instead of named constants.

The opcodes_z80.h header is already included but constants are not being used.

### Current State

```cpp
return {0x3E, value};  // Magic number
return {0x06, value};  // Magic number
return {0xC6, value};  // Magic number
```

### Desired State

```cpp
return {LD_A_n, value};   // Named constant
return {LD_B_n, value};   // Named constant
return {ADD_A_n, value};  // Named constant
```

---

## Success Criteria

```
✓ All magic numbers replaced with named constants
✓ opcodes_z80.h constants used throughout
✓ All tests passing (97%+ maintained)
✓ Build passing with no warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Replace ~50+ magic numbers in cpu_z80.cpp
□ Use constants from opcodes_z80.h
□ All CPU encoding methods use constants
□ No new magic numbers introduced
```

### Quality Requirements
```
□ All tests passing
□ No compilation warnings
□ Code more readable
```

---

## Estimated Complexity

**Complexity:** Trivial

**Files:** 1 (cpu_z80.cpp)
**Token Budget:** ~3K tokens - ✅ SAFE
**Time:** <30 minutes

---

## Dependencies

**Depends on:**
- ✅ opcodes_z80.h already exists

---

## Approvals

**Contract Approved By:**
- [X] Orchestrator: 2026-02-10

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-10
