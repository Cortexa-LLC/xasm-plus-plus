# Task Contract

**Task ID:** 2026-01-24_65c02-65816-support
**Created:** 2026-01-24
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Extend the xasm++ 6502 CPU plugin to support the 65C02 and 65816 processors, enabling switchable CPU modes required for Merlin and SCMASM syntax support.

### Background and Context

**Why:** Merlin and SCMASM assemblers require support for the complete 6502 family (6502, 65C02, 65816) with switchable CPU modes. The Apple IIgs uses the 65816, and many Apple II programs use 65C02-specific opcodes.

**Problem:** Current implementation only supports the original 6502. Users cannot assemble code using 65C02 or 65816 instructions.

**Context:** This is part of completing the 6502 CPU family before moving to CLI implementation (Phase 3). It enables the assembler to support real-world Apple II and Apple IIgs assembly code.

### Current State

**6502 Support (Phase 2.4 - COMPLETE):**
- ✅ All 56 legal 6502 opcodes
- ✅ All 13 addressing modes
- ✅ 197/197 tests passing
- ✅ Production quality

**What's Missing:**
- ❌ 65C02 opcodes and addressing modes
- ❌ 65816 opcodes and addressing modes
- ❌ CPU mode switching
- ❌ MX directive for 65816 register width tracking

### Desired State

**After Completion:**
- ✅ Switchable CPU modes: `6502`, `65c02`, `65c02-rockwell`, `65816`
- ✅ All 65C02 opcodes (BRA, PHX/PLX, PHY/PLY, STZ, TRB, TSB, etc.)
- ✅ All 65C02 addressing mode enhancements
- ✅ All 65816 opcodes (PHB/PLB, MVN/MVP, JML/JSL, etc.)
- ✅ 65816 long addressing modes (24-bit)
- ✅ MX directive for register width control
- ✅ Comprehensive test coverage
- ✅ Backward compatibility maintained (all 197 existing tests still pass)

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ All existing 6502 tests pass (197/197) - backward compatibility
✓ 65C02 mode: All new opcodes work with tests
✓ 65816 mode: All new opcodes work with tests
✓ CPU mode switching works correctly
✓ MX directive tracks register width accurately
✓ Code coverage ≥ 85% overall, 95%+ for new code
✓ Zero compiler warnings
✓ Tester validation: APPROVED
✓ Reviewer validation: APPROVED
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements - 65C02 Support

**Must Have:**
```
□ FR1: New 65C02 opcodes
  - BRA (Branch Always) - relative addressing
  - PHX, PLX (Push/Pull X register)
  - PHY, PLY (Push/Pull Y register)
  - STZ (Store Zero) - ZeroPage, ZeroPageX, Absolute, AbsoluteX
  - TRB, TSB (Test and Reset/Set Bits) - ZeroPage, Absolute

□ FR2: Enhanced addressing modes (65C02)
  - LDA ($80) - Indirect (no ,Y) - NEW for 65C02
  - JMP ($1234,X) - Indexed Indirect JMP - NEW for 65C02
  - BIT #$80 - Immediate BIT - NEW for 65C02
  - BIT $80,X - ZeroPageX BIT - NEW for 65C02
  - BIT $1234,X - AbsoluteX BIT - NEW for 65C02

□ FR3: Rockwell/WDC extensions (optional, mode flag: 65c02-rockwell)
  - BBR0-7 $80,label - Branch on Bit Reset
  - BBS0-7 $80,label - Branch on Bit Set
  - RMB0-7 $80 - Reset Memory Bit
  - SMB0-7 $80 - Set Memory Bit

□ FR4: CPU mode selection
  - API: SetCpuMode("6502") or SetCpuMode("65c02") or SetCpuMode("65c02-rockwell")
  - Validate opcodes against current CPU mode
  - Warn/error on 65C02-specific opcodes when in 6502 mode
```

### Functional Requirements - 65816 Support

**Must Have:**
```
□ FR5: Register width control (MX directive)
  - MX %00 - A and X/Y are 16-bit
  - MX %01 - A is 16-bit, X/Y are 8-bit
  - MX %10 - A is 8-bit, X/Y are 16-bit
  - MX %11 - A and X/Y are 8-bit (6502 mode)
  - Track m and x flags during assembly
  - Affect immediate operand size (context-sensitive)

□ FR6: Long addressing modes (24-bit)
  - LDA $123456 - Absolute Long
  - LDA [$80] - Indirect Long
  - LDA [$80],Y - Indirect Long Indexed

□ FR7: Stack relative addressing
  - LDA $03,S - Stack Relative
  - LDA ($03,S),Y - Stack Relative Indirect Indexed

□ FR8: New 65816 opcodes (Data Bank)
  - PHB, PLB - Push/Pull Data Bank
  - PHK - Push Program Bank
  - PHD, PLD - Push/Pull Direct Page

□ FR9: New 65816 opcodes (Transfers)
  - TCD - Transfer C (16-bit accumulator) to Direct Page
  - TDC - Transfer Direct Page to C
  - TCS - Transfer C to Stack Pointer
  - TSC - Transfer Stack Pointer to C

□ FR10: New 65816 opcodes (Jumps)
  - JML $123456 - Jump Long
  - JSL $123456 - Jump to Subroutine Long
  - RTL - Return from Subroutine Long

□ FR11: New 65816 opcodes (Misc)
  - PEA #$1234 - Push Effective Address
  - PEI ($80) - Push Effective Indirect Address
  - PER label - Push Effective PC Relative Address
  - MVN srcbank,dstbank - Block Move Negative
  - MVP srcbank,dstbank - Block Move Positive
  - COP #$00 - Coprocessor
  - WDM #$00 - Reserved (WDC)
  - XBA - Exchange B and A accumulators
  - XCE - Exchange Carry and Emulation bits

□ FR12: CPU mode selection for 65816
  - API: SetCpuMode("65816")
  - Enable all 65816 opcodes and addressing modes
  - Require MX directive to set register widths
```

### Functional Requirements - Common

**Must Have:**
```
□ FR13: Backward compatibility
  - All 197 existing 6502 tests must pass
  - No breaking changes to existing API
  - Default mode: 6502 (if not specified)

□ FR14: Mode validation
  - Error on 65C02 opcodes in 6502 mode
  - Error on 65816 opcodes in 6502/65C02 mode
  - Clear error messages indicating required CPU mode
```

### Quality Requirements

**Must Have:**
```
□ QR1: Test coverage
  - Target: ≥85% overall, 95%+ new code
  - 65C02: ≥30 new tests (unit + integration)
  - 65816: ≥50 new tests (unit + integration)
  - All new opcodes tested
  - All new addressing modes tested
  - CPU mode switching tested
  - MX directive tested (65816)

□ QR2: Test-Driven Development (TDD)
  - Follow RED-GREEN-REFACTOR cycle
  - Write tests before implementation
  - Atomic commits per feature group
  - Evidence in git history

□ QR3: Code Quality
  - Zero compiler warnings
  - C++ best practices (2-space indentation, implementations in .cpp)
  - Clean separation of concerns
  - No code duplication
  - Reviewer approval required

□ QR4: Error Handling
  - Invalid CPU mode errors
  - Opcode not available in mode errors
  - Clear error messages
  - Graceful failure

□ QR5: Documentation
  - Inline comments for complex logic
  - Examples in comments
  - Work log complete
  - Reference to docs/references/ materials
```

### Non-Functional Requirements

**Must Have:**
```
□ NFR1: Performance
  - No measurable regression vs Phase 2.4
  - Test execution time: <500ms for all tests

□ NFR2: Backward Compatibility
  - All existing 6502 tests pass (197/197)
  - No breaking API changes
  - Default behavior unchanged (6502 mode)

□ NFR3: Maintainability
  - Clear code organization
  - Easy to extend for future CPU variants
  - Well-documented mode switching logic
```

### Should Have (Stretch Goals)

```
□ SH1: Rockwell/WDC BBR/BBS/RMB/SMB instructions (65c02-rockwell mode)
□ SH2: Force addressing mode syntax (< for direct page, > for absolute)
□ SH3: Comprehensive MX tracking examples in tests
```

### Nice to Have (Future)

```
□ NH1: Undocumented 65C02 opcodes
□ NH2: 65816 emulation mode vs native mode tracking
□ NH3: Bank byte notation (LABEL:BANK)
```

---

## Constraints and Dependencies

### Constraints

```
□ Must maintain backward compatibility with Phase 2.4 (6502)
□ Must use existing CPU plugin architecture
□ Must follow TDD methodology (framework requirement)
□ Must achieve Tester + Reviewer approval (gates)
□ Timeline: No specific deadline, but complete before Phase 3 (CLI)
```

### Dependencies

```
□ Phase 2.4 Complete (DONE) - 6502 implementation and tests
□ Reference documentation (DONE) - docs/references/6502-SYNTAX-COMPARISON.md
□ CPU plugin interface (EXISTS) - src/plugins/cpu_plugin.h
□ Test infrastructure (EXISTS) - Google Test framework
□ Build system (EXISTS) - CMake configuration
```

### Out of Scope

```
✗ CLI implementation (Phase 3)
✗ Source file parsing (Phase 3)
✗ Binary output formats (Phase 3)
✗ Syntax mode switching (Merlin vs SCMASM) - Phase 3+
✗ Directive parsing (.EQ, EQU, .OR, ORG) - Phase 3+
✗ Macro support - Phase 4+
✗ Other CPU architectures (6809, Z80) - Phase 5+
✗ 65802 (rare variant) - Future consideration
```

---

## Risks and Mitigation

### Technical Risks

**Risk 1: MX directive state tracking complexity (65816)**
- **Impact:** High - Affects operand size for immediate instructions
- **Likelihood:** Medium
- **Mitigation:**
  - Implement incremental tests for MX state transitions
  - Start with simple cases (all 8-bit, all 16-bit)
  - Add edge cases incrementally
  - Reference Merlin 32 implementation for guidance

**Risk 2: Backward compatibility breakage**
- **Impact:** Critical - Would fail Phase 2.4 tests
- **Likelihood:** Low (with proper testing)
- **Mitigation:**
  - Run all 197 existing tests after each change
  - Use default mode = 6502 (no behavior change if mode not set)
  - Atomic commits allow easy rollback

**Risk 3: Test count explosion**
- **Impact:** Medium - Could slow down development
- **Likelihood:** High - 65816 has many addressing mode variants
- **Mitigation:**
  - Focus on quality over quantity
  - Test representative cases, not exhaustive permutations
  - Group similar tests for efficiency

### Process Risks

**Risk 4: Scope creep**
- **Impact:** Medium - Could delay completion
- **Likelihood:** Medium
- **Mitigation:**
  - Stick to Must Have requirements
  - Defer Should Have to future phases
  - Mark Nice to Have explicitly out of scope

---

## Stakeholders

**Primary:**
- Bryan Woodruff (requestor, developer)

**Roles:**
- Engineer (implementation)
- Tester (validation)
- Reviewer (code quality)
- Orchestrator (oversight)

---

## References

### Reference Documentation

**Primary:**
- `docs/references/6502-SYNTAX-COMPARISON.md` - 65C02 and 65816 sections
- `docs/references/Pages from 6502 Assembly Language Programming (Lance Levanthal).pdf`

**Online:**
- [WDC 65C02 Datasheet](https://www.westerndesigncenter.com/wdc/documentation/w65c02s.pdf)
- [WDC 65C816 Datasheet](https://www.westerndesigncenter.com/wdc/documentation/w65c816s.pdf)
- [65816 Programming Manual](http://www.defence-force.org/computing/oric/coding/annexe_2/65816.pdf)

### Related Tasks

**Completed:**
- Phase 2.4: Complete 6502 Syntax Parsing (`.ai/tasks/2026-01-22_complete-6502-syntax-parsing/`)

**Future:**
- Phase 3: CLI and File I/O
- Phase 4: Directives and Syntax Modes (Merlin, SCMASM)

---

## Approval

**Contract Reviewed By:**
- [ ] Engineer: [Name] - [Date]
- [ ] Tester: [Name] - [Date]
- [ ] Reviewer: [Name] - [Date]

**Contract Approved:** [ ]

**Ready to Proceed:** [ ]

---

**Version:** 1.0
**Last Updated:** 2026-01-24

