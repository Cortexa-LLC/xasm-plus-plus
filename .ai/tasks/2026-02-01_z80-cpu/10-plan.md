# Implementation Plan: Z80 CPU Plugin

**Task:** xasm++-ocf
**Date:** 2026-02-01

---

## Approach Summary

Follow the established CPU plugin pattern from 6502 and 6809 implementations. Implement the Z80 instruction set with full support for IX/IY indexing and bit manipulation instructions.

**Key Technical Decisions:**
- Use opcode table approach (like 6502/6809)
- Separate IX/IY prefix handling (0xDD, 0xFD)
- Bit operations use CB prefix (0xCB)
- TDD approach: Write tests first, then implementation

---

## Critical Files Identified

### Files to Create

```
1. include/xasm++/cpu/cpu_z80.h
   - Z80CPU class declaration
   - Instruction encoding methods

2. src/cpu/z80/cpu_z80.cpp
   - Z80CPU implementation
   - Opcode table
   - Instruction handlers

3. tests/unit/test_cpu_z80.cpp
   - Unit tests for all instructions
   - Edge case testing
```

### Files to Reference

```
- include/xasm++/cpu/cpu_6502.h (pattern reference)
- src/cpu/m6502/cpu_6502.cpp (implementation pattern)
- include/xasm++/cpu/cpu_6809.h (complex addressing reference)
```

---

## Step-by-Step Implementation Plan

### Phase 1: Core Structure (3-4 hours)

**Step 1.1: Create header**
- Define Z80CPU class
- Declare instruction methods
- Documentation

**Step 1.2: Basic opcodes**
- 8-bit loads (LD r, n)
- 16-bit loads (LD rr, nn)
- Simple arithmetic (ADD, SUB, INC, DEC)

**Step 1.3: Tests**
- Unit tests for Phase 1 instructions
- Verify: Tests pass

### Phase 2: Advanced Instructions (5-6 hours)

**Step 2.1: Arithmetic/Logic**
- ADC, SBC, AND, OR, XOR, CP
- 16-bit arithmetic (ADD HL, rr)

**Step 2.2: Jumps/Calls**
- JP, JR, CALL, RET
- Conditional variants

**Step 2.3: Stack operations**
- PUSH, POP

### Phase 3: IX/IY Indexing (3-4 hours)

**Step 3.1: Index register loads**
- LD IX, nn / LD IY, nn
- LD (IX+d), n / LD (IY+d), n

**Step 3.2: Indexed operations**
- ADD/SUB/AND/OR/XOR with (IX+d)/(IY+d)

### Phase 4: Bit Operations (2-3 hours)

**Step 4.1: Bit testing**
- BIT b, r / BIT b, (HL)

**Step 4.2: Bit manipulation**
- SET b, r / RES b, r

**Step 4.3: Rotates/Shifts**
- RLC, RRC, RL, RR, SLA, SRA, SRL

### Phase 5: Block Instructions (1-2 hours)

**Step 5.1: Load blocks**
- LDI, LDIR, LDD, LDDR

**Step 5.2: Compare blocks**
- CPI, CPIR, CPD, CPDR

### Phase 6: Testing & Polish (3-4 hours)

**Step 6.1: Comprehensive testing**
- Test all 150 opcodes
- Edge cases
- Coverage >85%

**Step 6.2: Documentation**
- Doxygen comments
- Usage examples

---

## Testing Strategy

### Unit Tests

```
□ 8-bit loads (all registers)
□ 16-bit loads
□ Arithmetic operations
□ Logic operations
□ Jumps (absolute, relative, conditional)
□ Stack operations
□ IX/IY indexed addressing
□ Bit operations (BIT, SET, RES)
□ Rotates and shifts
□ Block instructions
```

### Test Coverage Target

```
- Overall: >85%
- Critical paths: 100%
```

---

## Implementation Notes

**Opcode Prefixes:**
- 0xCB: Bit operations
- 0xDD: IX prefix
- 0xED: Extended instructions
- 0xFD: IY prefix

**Instruction Encoding:**
- Most instructions: 1-4 bytes
- Indexed: 2-4 bytes (prefix + opcode + displacement)
- Bit ops: 2 bytes (0xCB + opcode) or 4 (with IX/IY)

**Reference Documentation:**
- Z80 User Manual (Zilog)
- Z80 opcode table
- Game Boy CPU manual (for variant)

---

**Status:** Ready for Implementation
**Next Step:** Begin Phase 1 - Core Structure
