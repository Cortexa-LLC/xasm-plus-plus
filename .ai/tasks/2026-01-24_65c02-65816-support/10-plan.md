# Implementation Plan

**Task ID:** 2026-01-24_65c02-65816-support
**Created:** 2026-01-24
**Engineer:** Claude Sonnet 4.5 (ai-pack Engineer role)
**Workflow:** Feature (TDD RED-GREEN-REFACTOR)

---

## Overview

Extend xasm++ 6502 CPU plugin to support 65C02 and 65816 processors with switchable CPU modes.

**Approach:**
- **Incremental TDD** - Test-first, small commits
- **Backward compatibility first** - Verify 197 existing tests pass after each group
- **65C02 before 65816** - Build on simpler variant first
- **Quality over speed** - Comprehensive testing, clean code

---

## Implementation Strategy

### Phase 1: 65C02 Support (~30-40 tests)

Extend Cpu6502 with 65C02 opcodes and enhanced addressing modes.

**Groups:**
1. CPU Mode Infrastructure (foundation)
2. New 65C02 Opcodes - Stack (PHX, PLX, PHY, PLY)
3. New 65C02 Opcodes - Store Zero (STZ)
4. New 65C02 Opcodes - Bit Test (TRB, TSB)
5. New 65C02 Opcodes - Branch Always (BRA)
6. Enhanced Addressing Modes (indirect, BIT variants)
7. Rockwell Extensions (optional: BBR, BBS, RMB, SMB)

### Phase 2: 65816 Support (~50-70 tests)

Add 65816 long addressing, MX directive, and new opcodes.

**Groups:**
8. MX Directive Infrastructure (register width tracking)
9. Long Addressing Modes (24-bit)
10. Stack Relative Addressing
11. Bank Operations (PHB, PLB, PHK, PHD, PLD)
12. Transfer Operations (TCD, TDC, TCS, TSC)
13. Long Jumps (JML, JSL, RTL)
14. Misc 65816 Opcodes (PEA, PEI, PER, MVN, MVP, XBA, XCE, COP, WDM)

---

## Detailed Group Breakdown

### Group 1: CPU Mode Infrastructure

**Goal:** Add CPU mode switching without breaking existing functionality.

**TDD Steps:**
1. **RED:** Test SetCpuMode() API
2. **GREEN:** Add CpuMode enum and SetCpuMode() method to Cpu6502
3. **REFACTOR:** Clean up, ensure default mode = 6502
4. **VERIFY:** All 197 existing tests still pass

**API Design:**
```cpp
enum class CpuMode {
  Cpu6502,       // Original 6502 (default)
  Cpu65C02,      // 65C02 with enhanced opcodes
  Cpu65C02Rock,  // 65C02 with Rockwell extensions
  Cpu65816       // 65816 with 16-bit support
};

class Cpu6502 {
public:
  void SetCpuMode(CpuMode mode);
  CpuMode GetCpuMode() const;

private:
  CpuMode cpu_mode_ = CpuMode::Cpu6502;  // Default
};
```

**Tests:**
- Test default mode is 6502
- Test mode switching
- Test existing opcodes work in all modes
- Test mode validation

**Estimate:** 5 tests, 1 commit

---

### Group 2: 65C02 Stack Operations (PHX, PLX, PHY, PLY)

**Goal:** Add push/pull for X and Y registers.

**TDD Steps:**
1. **RED:** Test PHX (opcode $DA)
2. **GREEN:** Implement PHX encoding
3. **RED:** Test PLX (opcode $FA)
4. **GREEN:** Implement PLX encoding
5. **RED:** Test PHY (opcode $5A)
6. **GREEN:** Implement PHY encoding
7. **RED:** Test PLY (opcode $7A)
8. **GREEN:** Implement PLY encoding
9. **REFACTOR:** Clean up, add comments
10. **VERIFY:** All tests pass (197 + 4 new = 201)

**Opcodes:**
- `PHX` - $DA (implied)
- `PLX` - $FA (implied)
- `PHY` - $5A (implied)
- `PLY` - $7A (implied)

**Tests:**
- Test PHX encodes to $DA
- Test PLX encodes to $FA
- Test PHY encodes to $5A
- Test PLY encodes to $7A
- Test mode validation (error in 6502 mode)

**Estimate:** 5 tests, 1 commit

---

### Group 3: 65C02 Store Zero (STZ)

**Goal:** Add STZ instruction with multiple addressing modes.

**TDD Steps:**
1. **RED:** Test STZ ZeroPage (opcode $64)
2. **GREEN:** Implement STZ ZeroPage
3. **RED:** Test STZ ZeroPageX (opcode $74)
4. **GREEN:** Implement STZ ZeroPageX
5. **RED:** Test STZ Absolute (opcode $9C)
6. **GREEN:** Implement STZ Absolute
7. **RED:** Test STZ AbsoluteX (opcode $9E)
8. **GREEN:** Implement STZ AbsoluteX
9. **REFACTOR:** Clean up
10. **VERIFY:** All tests pass (201 + 4 new = 205)

**Opcodes:**
- `STZ $80` - $64 (ZeroPage)
- `STZ $80,X` - $74 (ZeroPageX)
- `STZ $1234` - $9C (Absolute)
- `STZ $1234,X` - $9E (AbsoluteX)

**Tests:**
- Test STZ with each addressing mode
- Test mode validation

**Estimate:** 5 tests, 1 commit

---

### Group 4: 65C02 Bit Test Operations (TRB, TSB)

**Goal:** Add test and reset/set bits instructions.

**TDD Steps:**
1. **RED:** Test TRB ZeroPage (opcode $14)
2. **GREEN:** Implement TRB ZeroPage
3. **RED:** Test TRB Absolute (opcode $1C)
4. **GREEN:** Implement TRB Absolute
5. **RED:** Test TSB ZeroPage (opcode $04)
6. **GREEN:** Implement TSB ZeroPage
7. **RED:** Test TSB Absolute (opcode $0C)
8. **GREEN:** Implement TSB Absolute
9. **REFACTOR:** Clean up
10. **VERIFY:** All tests pass (205 + 4 new = 209)

**Opcodes:**
- `TRB $80` - $14 (ZeroPage)
- `TRB $1234` - $1C (Absolute)
- `TSB $80` - $04 (ZeroPage)
- `TSB $1234` - $0C (Absolute)

**Tests:**
- Test TRB/TSB with ZeroPage and Absolute
- Test mode validation

**Estimate:** 5 tests, 1 commit

---

### Group 5: 65C02 Branch Always (BRA)

**Goal:** Add unconditional branch instruction.

**TDD Steps:**
1. **RED:** Test BRA relative (opcode $80)
2. **GREEN:** Implement BRA encoding with relative addressing
3. **RED:** Test BRA with forward reference
4. **GREEN:** Ensure multi-pass works
5. **REFACTOR:** Clean up
6. **VERIFY:** All tests pass (209 + 2 new = 211)

**Opcodes:**
- `BRA label` - $80 (Relative)

**Tests:**
- Test BRA with backward branch
- Test BRA with forward branch
- Test mode validation

**Estimate:** 3 tests, 1 commit

---

### Group 6: 65C02 Enhanced Addressing Modes

**Goal:** Add 65C02-specific addressing mode variants.

**TDD Steps:**
1. **RED:** Test LDA ($80) - Indirect (no ,Y) - opcode $B2
2. **GREEN:** Implement indirect mode (65C02 only)
3. **RED:** Test JMP ($1234,X) - Indexed Indirect JMP - opcode $7C
4. **GREEN:** Implement indexed indirect JMP
5. **RED:** Test BIT #$80 - Immediate BIT - opcode $89
6. **GREEN:** Implement immediate BIT
7. **RED:** Test BIT $80,X - ZeroPageX BIT - opcode $34
8. **GREEN:** Implement ZeroPageX BIT
9. **RED:** Test BIT $1234,X - AbsoluteX BIT - opcode $3C
10. **GREEN:** Implement AbsoluteX BIT
11. **REFACTOR:** Clean up addressing mode detection
12. **VERIFY:** All tests pass (211 + 5 new = 216)

**New Addressing Modes:**
- Indirect (no ,Y): `LDA ($80)` - $B2
- Indexed Indirect JMP: `JMP ($1234,X)` - $7C
- Immediate BIT: `BIT #$80` - $89
- ZeroPageX BIT: `BIT $80,X` - $34
- AbsoluteX BIT: `BIT $1234,X` - $3C

**Tests:**
- Test each new addressing mode variant
- Test mode validation (65C02 required)

**Estimate:** 6 tests, 1 commit

---

### Group 7: Rockwell Extensions (Optional - Stretch Goal)

**Goal:** Add Rockwell/WDC bit manipulation instructions.

**TDD Steps:**
1. **RED:** Test BBR0 $80,label - opcode $0F
2. **GREEN:** Implement BBR0-7
3. **RED:** Test BBS0 $80,label - opcode $8F
4. **GREEN:** Implement BBS0-7
5. **RED:** Test RMB0 $80 - opcode $07
6. **GREEN:** Implement RMB0-7
7. **RED:** Test SMB0 $80 - opcode $87
8. **GREEN:** Implement SMB0-7
9. **REFACTOR:** Clean up
10. **VERIFY:** All tests pass (216 + 8 new = 224)

**Opcodes (8 variants each):**
- `BBR0-7 $80,label` - $0F, $1F, $2F, $3F, $4F, $5F, $6F, $7F
- `BBS0-7 $80,label` - $8F, $9F, $AF, $BF, $CF, $DF, $EF, $FF
- `RMB0-7 $80` - $07, $17, $27, $37, $47, $57, $67, $77
- `SMB0-7 $80` - $87, $97, $A7, $B7, $C7, $D7, $E7, $F7

**Tests:**
- Test one variant of each instruction
- Test mode validation (65c02-rockwell mode required)

**Estimate:** 8 tests, 1 commit (IF implemented)

---

### Group 8: MX Directive Infrastructure (65816)

**Goal:** Add register width tracking for 65816.

**TDD Steps:**
1. **RED:** Test MX directive parsing
2. **GREEN:** Implement MxState class to track m and x flags
3. **RED:** Test immediate operand size changes based on MX
4. **GREEN:** Implement context-sensitive immediate size
5. **REFACTOR:** Clean up
6. **VERIFY:** All tests pass (224 + 5 new = 229)

**API Design:**
```cpp
struct MxState {
  bool m_flag;  // 0 = 16-bit A, 1 = 8-bit A
  bool x_flag;  // 0 = 16-bit X/Y, 1 = 8-bit X/Y

  uint8_t GetAccumulatorSize() const { return m_flag ? 1 : 2; }
  uint8_t GetIndexSize() const { return x_flag ? 1 : 2; }
};
```

**Tests:**
- Test MX %00 (all 16-bit)
- Test MX %01 (A 16-bit, X/Y 8-bit)
- Test MX %10 (A 8-bit, X/Y 16-bit)
- Test MX %11 (all 8-bit)
- Test LDA #$1234 size varies with MX state

**Estimate:** 6 tests, 1 commit

---

### Group 9: 65816 Long Addressing Modes

**Goal:** Add 24-bit addressing modes.

**TDD Steps:**
1. **RED:** Test LDA $123456 - Absolute Long - opcode $AF
2. **GREEN:** Implement absolute long addressing
3. **RED:** Test LDA [$80] - Indirect Long - opcode $A7
4. **GREEN:** Implement indirect long
5. **RED:** Test LDA [$80],Y - Indirect Long Indexed - opcode $B7
6. **GREEN:** Implement indirect long indexed
7. **REFACTOR:** Clean up
8. **VERIFY:** All tests pass (229 + 3 new = 232)

**Opcodes:**
- `LDA $123456` - $AF (Absolute Long)
- `LDA [$80]` - $A7 (Indirect Long)
- `LDA [$80],Y` - $B7 (Indirect Long Indexed)

**Tests:**
- Test each long addressing mode
- Test mode validation (65816 required)

**Estimate:** 4 tests, 1 commit

---

### Group 10: 65816 Stack Relative Addressing

**Goal:** Add stack-relative addressing modes.

**TDD Steps:**
1. **RED:** Test LDA $03,S - Stack Relative - opcode $A3
2. **GREEN:** Implement stack relative addressing
3. **RED:** Test LDA ($03,S),Y - SR Indirect Indexed - opcode $B3
4. **GREEN:** Implement SR indirect indexed
5. **REFACTOR:** Clean up
6. **VERIFY:** All tests pass (232 + 2 new = 234)

**Opcodes:**
- `LDA $03,S` - $A3 (Stack Relative)
- `LDA ($03,S),Y` - $B3 (Stack Relative Indirect Indexed)

**Tests:**
- Test stack relative
- Test stack relative indirect indexed
- Test mode validation

**Estimate:** 3 tests, 1 commit

---

### Group 11: 65816 Bank Operations

**Goal:** Add data bank and direct page operations.

**TDD Steps:**
1. **RED:** Test PHB, PLB (opcodes $8B, $AB)
2. **GREEN:** Implement PHB, PLB
3. **RED:** Test PHK (opcode $4B)
4. **GREEN:** Implement PHK
5. **RED:** Test PHD, PLD (opcodes $0B, $2B)
6. **GREEN:** Implement PHD, PLD
7. **REFACTOR:** Clean up
8. **VERIFY:** All tests pass (234 + 5 new = 239)

**Opcodes:**
- `PHB` - $8B (implied)
- `PLB` - $AB (implied)
- `PHK` - $4B (implied)
- `PHD` - $0B (implied)
- `PLD` - $2B (implied)

**Tests:**
- Test each bank operation
- Test mode validation

**Estimate:** 6 tests, 1 commit

---

### Group 12: 65816 Transfer Operations

**Goal:** Add 65816-specific transfer instructions.

**TDD Steps:**
1. **RED:** Test TCD (opcode $5B)
2. **GREEN:** Implement TCD
3. **RED:** Test TDC (opcode $7B)
4. **GREEN:** Implement TDC
5. **RED:** Test TCS (opcode $1B)
6. **GREEN:** Implement TCS
7. **RED:** Test TSC (opcode $3B)
8. **GREEN:** Implement TSC
9. **REFACTOR:** Clean up
10. **VERIFY:** All tests pass (239 + 4 new = 243)

**Opcodes:**
- `TCD` - $5B (implied) - Transfer C to Direct Page
- `TDC` - $7B (implied) - Transfer Direct Page to C
- `TCS` - $1B (implied) - Transfer C to Stack
- `TSC` - $3B (implied) - Transfer Stack to C

**Tests:**
- Test each transfer operation
- Test mode validation

**Estimate:** 5 tests, 1 commit

---

### Group 13: 65816 Long Jumps

**Goal:** Add 24-bit jump/call instructions.

**TDD Steps:**
1. **RED:** Test JML $123456 (opcode $5C)
2. **GREEN:** Implement JML absolute long
3. **RED:** Test JML [$1234] (opcode $DC)
4. **GREEN:** Implement JML indirect long
5. **RED:** Test JSL $123456 (opcode $22)
6. **GREEN:** Implement JSL
7. **RED:** Test RTL (opcode $6B)
8. **GREEN:** Implement RTL
9. **REFACTOR:** Clean up
10. **VERIFY:** All tests pass (243 + 4 new = 247)

**Opcodes:**
- `JML $123456` - $5C (Absolute Long)
- `JML [$1234]` - $DC (Indirect Long)
- `JSL $123456` - $22 (Absolute Long)
- `RTL` - $6B (implied)

**Tests:**
- Test each long jump variant
- Test mode validation

**Estimate:** 5 tests, 1 commit

---

### Group 14: 65816 Miscellaneous Opcodes

**Goal:** Add remaining 65816 instructions.

**TDD Steps:**
1. **RED:** Test PEA #$1234 (opcode $F4)
2. **GREEN:** Implement PEA
3. **RED:** Test PEI ($80) (opcode $D4)
4. **GREEN:** Implement PEI
5. **RED:** Test PER label (opcode $62)
6. **GREEN:** Implement PER
7. **RED:** Test MVN srcbank,dstbank (opcode $54)
8. **GREEN:** Implement MVN
9. **RED:** Test MVP srcbank,dstbank (opcode $44)
10. **GREEN:** Implement MVP
11. **RED:** Test XBA (opcode $EB)
12. **GREEN:** Implement XBA
13. **RED:** Test XCE (opcode $FB)
14. **GREEN:** Implement XCE
15. **RED:** Test COP #$00 (opcode $02)
16. **GREEN:** Implement COP
17. **RED:** Test WDM #$00 (opcode $42)
18. **GREEN:** Implement WDM
19. **REFACTOR:** Clean up
20. **VERIFY:** All tests pass (247 + 9 new = 256)

**Opcodes:**
- `PEA #$1234` - $F4 (immediate)
- `PEI ($80)` - $D4 (indirect)
- `PER label` - $62 (relative)
- `MVN srcbank,dstbank` - $54 (block move)
- `MVP srcbank,dstbank` - $44 (block move)
- `XBA` - $EB (implied) - Exchange B and A
- `XCE` - $FB (implied) - Exchange Carry and Emulation
- `COP #$00` - $02 (immediate)
- `WDM #$00` - $42 (immediate)

**Tests:**
- Test each misc opcode
- Test mode validation

**Estimate:** 10 tests, 1 commit

---

## Test Strategy

### Test Count Targets

**65C02:**
- Group 1 (Infrastructure): 5 tests
- Group 2 (Stack): 5 tests
- Group 3 (STZ): 5 tests
- Group 4 (TRB/TSB): 5 tests
- Group 5 (BRA): 3 tests
- Group 6 (Enhanced modes): 6 tests
- Group 7 (Rockwell - optional): 8 tests
- **Subtotal: 29-37 tests**

**65816:**
- Group 8 (MX): 6 tests
- Group 9 (Long addressing): 4 tests
- Group 10 (Stack relative): 3 tests
- Group 11 (Bank ops): 6 tests
- Group 12 (Transfers): 5 tests
- Group 13 (Long jumps): 5 tests
- Group 14 (Misc): 10 tests
- **Subtotal: 39 tests**

**Total New Tests: 68-76 tests**
**Total Tests After Completion: 265-273 tests**

### Test Types

**Unit Tests (~90%):**
- Test each opcode encodes to correct bytes
- Test addressing mode detection
- Test CPU mode validation
- Test MX state tracking (65816)

**Integration Tests (~10%):**
- Test realistic 65C02 programs (bit manipulation, stack operations)
- Test realistic 65816 programs (long addressing, bank switching)
- Test mixed-mode programs

### Backward Compatibility Verification

**After EVERY group:**
```bash
# Run all existing 6502 tests
tests/unit/test_cpu6502    # 155 tests must pass
tests/unit/test_assembler  # 42 tests must pass
```

If any existing test fails, **STOP** and fix immediately before proceeding.

---

## File Structure

### Files to Modify

**src/plugins/cpu6502.h:**
- Add CpuMode enum
- Add SetCpuMode() / GetCpuMode() methods
- Add MxState struct (65816)
- Add cpu_mode_ member variable
- Add mx_state_ member variable (65816)

**src/plugins/cpu6502.cpp:**
- Implement CPU mode switching
- Add 65C02 opcode encoding methods
- Add 65816 opcode encoding methods
- Add MX state management (65816)
- Add long addressing mode support (65816)

**tests/unit/test_cpu6502.cpp:**
- Add 65C02 opcode tests
- Add 65816 opcode tests
- Add CPU mode validation tests
- Add MX state tests (65816)

**No new files created** - extend existing classes.

---

## Risks and Mitigation

### Risk 1: Backward Compatibility Break

**Mitigation:**
- Run all 197 tests after EVERY group
- Use default mode = 6502 (no behavior change if not specified)
- Atomic commits allow easy rollback

### Risk 2: MX State Complexity (65816)

**Mitigation:**
- Implement MX infrastructure first (Group 8)
- Test simple cases before complex
- Reference Merlin 32 implementation
- Start with explicit MX states, add default later

### Risk 3: Test Count Explosion

**Mitigation:**
- Focus on representative cases, not exhaustive
- Group similar tests
- Use parameterized tests where appropriate

### Risk 4: Opcode Table Conflicts

**Mitigation:**
- Reference WDC datasheets for exact opcodes
- Cross-check with docs/references/6502-SYNTAX-COMPARISON.md
- Test mode validation prevents using wrong opcodes in wrong modes

---

## Commit Strategy

**One commit per group:**
- Atomic, revertible commits
- Clear commit messages referencing group number
- Includes tests + implementation
- Verified: all tests pass before commit

**Commit Message Format:**
```
feat(cpu): add 65C02 <feature> support (Group N)

TDD Implementation:
- Added N tests for <feature>
- Implemented <opcode1>, <opcode2>, etc.
- All 197+N tests passing

References: docs/references/6502-SYNTAX-COMPARISON.md (65C02 section)
```

---

## References

### Documentation

**Primary:**
- `docs/references/6502-SYNTAX-COMPARISON.md` - 65C02 and 65816 sections
- `docs/references/Pages from 6502 Assembly Language Programming (Lance Levanthal).pdf`

**Online:**
- [WDC 65C02 Datasheet](https://www.westerndesigncenter.com/wdc/documentation/w65c02s.pdf)
- [WDC 65C816 Datasheet](https://www.westerndesigncenter.com/wdc/documentation/w65c816s.pdf)
- [65816 Programming Manual](http://www.defence-force.org/computing/oric/coding/annexe_2/65816.pdf)

### Existing Code

**Phase 2.4 Reference:**
- `.ai/tasks/2026-01-22_complete-6502-syntax-parsing/` - TDD approach, test structure
- `src/plugins/cpu6502.cpp` - Existing opcode structure
- `tests/unit/test_cpu6502.cpp` - Test patterns

---

## Approval

**Plan Reviewed By:**
- [ ] Engineer: Claude Sonnet 4.5 - 2026-01-24

**Ready to Begin:** [X]

---

**Version:** 1.0
**Last Updated:** 2026-01-24
**Estimated Duration:** 14 groups, ~75 tests, 14 commits

