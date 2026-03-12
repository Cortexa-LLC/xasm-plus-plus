# Task Contract: Z80 CPU Plugin

**Beads Task:** xasm++-ocf
**Date:** 2026-02-01
**Priority:** P4 (Nice to Have)

---

## Objective

Implement Zilog Z80 CPU plugin for xasm++ to support assembling Z80 assembly code.

## Background

**Target Systems:**
- Game Boy (modified Z80)
- ZX Spectrum
- TRS-80 Model I
- CP/M systems

**Current State:**
- ❌ No Z80 CPU plugin exists
- ✅ 6502 and 6809 CPU plugins exist as reference implementations
- ✅ CPU plugin architecture in place

**Goal:** Full Z80 instruction set support with Zilog syntax

## Requirements

### Functional Requirements

**FR-1: Z80 Instruction Set**
- ~150 opcodes (8080 base + Z80 extensions)
- Standard instructions: LD, ADD, SUB, AND, OR, XOR, CP, INC, DEC, etc.
- Z80-specific: BIT, SET, RES, RLC, RRC, RL, RR, SLA, SRA, SRL
- IX/IY index register support
- Block instructions: LDIR, LDDR, CPIR, CPDR, etc.

**FR-2: Addressing Modes**
- Register direct (A, B, C, D, E, H, L)
- Register indirect (HL), (BC), (DE)
- Indexed: (IX+d), (IY+d)
- Immediate: n, nn
- Extended: (nn)

**FR-3: Syntax Support**
- Zilog syntax (requires ZilogBaseSyntax base class)
- Case-insensitive mnemonics
- Hexadecimal: $HHLL or 0xHHLL
- Binary: %bbbbbbbb
- Decimal: default

**FR-4: Special Registers**
- 8-bit: A, B, C, D, E, H, L, I, R
- 16-bit: BC, DE, HL, SP, IX, IY, AF, AF'
- Flags: S, Z, H, P/V, N, C

### Non-Functional Requirements

**NFR-1: Code Quality**
- Follow existing CPU plugin patterns (6502, 6809)
- Unit tests for all instruction encodings
- Documentation for all public methods

**NFR-2: Performance**
- Instruction encoding in constant time
- No performance regression vs other CPUs

**NFR-3: Compatibility**
- Support common Z80 assembler syntax
- Compatible with Game Boy assembly code

## Acceptance Criteria

- [ ] **AC-1:** All ~150 Z80 opcodes implemented
- [ ] **AC-2:** IX/IY index registers work correctly
- [ ] **AC-3:** Bit manipulation instructions (BIT, SET, RES) functional
- [ ] **AC-4:** Block instructions (LDIR, CPIR, etc.) work
- [ ] **AC-5:** Unit tests for all instruction types
- [ ] **AC-6:** Can assemble Game Boy ROM header
- [ ] **AC-7:** No compiler warnings
- [ ] **AC-8:** Documentation complete

## Deliverables

1. **Source Code:**
   - `include/xasm++/cpu/cpu_z80.h`
   - `src/cpu/z80/cpu_z80.cpp`
   - `include/xasm++/cpu/opcodes_z80.h` (if needed)

2. **Tests:**
   - `tests/unit/test_cpu_z80.cpp`
   - Test coverage >85%

3. **Documentation:**
   - Doxygen comments in header
   - Implementation notes in plan

## Estimated Effort

**Total:** 15-25 hours (high complexity due to large instruction set)

**Breakdown:**
- Research Z80 instruction set: 2-3 hours
- Core instruction implementation: 10-15 hours
- IX/IY indexed addressing: 2-3 hours
- Bit operations: 2-3 hours
- Block instructions: 1-2 hours
- Testing: 3-5 hours

## Dependencies

- ✅ CPU plugin architecture exists
- ✅ ZilogBaseSyntax (may need creation)
- ✅ Expression evaluator
- ✅ Unit test framework (Catch2)

## References

- **Z80 CPU Manual:** https://www.zilog.com/docs/z80/um0080.pdf
- **6502 CPU Plugin:** Reference implementation
- **6809 CPU Plugin:** Reference implementation
- **Game Boy Z80:** http://gbdev.gg8.se/wiki/articles/CPU_Instruction_Set

---

**Status:** ✅ Ready for Planning
**Next Step:** Create implementation plan (10-plan.md)
