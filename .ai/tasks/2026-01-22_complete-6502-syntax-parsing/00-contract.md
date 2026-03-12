# Task Contract

**Task ID:** 2026-01-22_complete-6502-syntax-parsing
**Created:** 2026-01-22
**Stakeholders:** Project Owner, AI Orchestrator

---

## Problem Statement

The 6502 CPU plugin can encode all 56 legal opcodes with all 13 addressing modes, but the assembler's syntax parser (`DetermineAddressingMode()`) only recognizes 4 addressing mode syntaxes:
- Implied (no operand)
- Immediate (`#$42`)
- ZeroPage/Absolute (`$80` or `$1234`)
- Absolute via label (`target`)

**This means 8 addressing mode syntaxes cannot be parsed:**
- Accumulator mode: `ASL A`
- ZeroPageX: `LDA $80,X`
- ZeroPageY: `LDX $80,Y`
- AbsoluteX: `LDA $1234,X`
- AbsoluteY: `LDA $1234,Y`
- Indirect: `JMP ($1234)`
- IndexedIndirect: `LDA ($80,X)`
- IndirectIndexed: `LDA ($80),Y`

Real assembly source code using these syntaxes cannot currently be assembled, even though the CPU plugin can encode them when called directly (as the unit tests do).

---

## Requirements

### Functional Requirements

#### FR1: Accumulator Addressing Mode
- **Requirement:** Parse accumulator mode syntax: `<INSTR> A`
- **Example:** `ASL A`, `LSR A`, `ROL A`, `ROR A`
- **Behavior:** Recognize single `A` operand as `AddressingMode::Accumulator`
- **Applies to:** ASL, LSR, ROL, ROR instructions

#### FR2: Indexed Addressing Modes (,X and ,Y)
- **Requirement:** Parse `,X` and `,Y` index suffixes
- **Examples:**
  - ZeroPageX: `LDA $80,X`, `STA $FF,X`
  - ZeroPageY: `LDX $80,Y`, `STX $FF,Y`
  - AbsoluteX: `LDA $1234,X`, `STA $ABCD,X`
  - AbsoluteY: `LDA $1234,Y`, `STA $ABCD,Y`
- **Behavior:**
  - Detect `,X` or `,Y` suffix after address
  - Distinguish ZeroPage vs Absolute based on address value (≤$FF = ZeroPage)
  - Return appropriate `AddressingMode::ZeroPageX/Y` or `AbsoluteX/Y`
- **Validation:** Whitespace around comma should be tolerated: `$80,X` and `$80, X` both valid

#### FR3: Indirect Addressing Mode
- **Requirement:** Parse indirect syntax with parentheses: `(<ADDR>)`
- **Example:** `JMP ($1234)`
- **Behavior:** Recognize parentheses around address, return `AddressingMode::Indirect`
- **Applies to:** JMP instruction only (6502 restriction)

#### FR4: IndexedIndirect Addressing Mode (Pre-indexed)
- **Requirement:** Parse pre-indexed indirect syntax: `(<ADDR>,X)`
- **Example:** `LDA ($80,X)`, `STA ($40,X)`
- **Behavior:** Recognize parentheses with `,X` inside, return `AddressingMode::IndirectX`
- **Applies to:** LDA, STA, CMP, AND, ORA, EOR, ADC, SBC
- **Validation:** Zero page addresses only ($00-$FF)

#### FR5: IndirectIndexed Addressing Mode (Post-indexed)
- **Requirement:** Parse post-indexed indirect syntax: `(<ADDR>),Y`
- **Example:** `LDA ($80),Y`, `STA ($40),Y`
- **Behavior:** Recognize parentheses with `,Y` outside, return `AddressingMode::IndirectY`
- **Applies to:** LDA, STA, CMP, AND, ORA, EOR, ADC, SBC
- **Validation:** Zero page addresses only ($00-$FF)

#### FR6: Label Support for Indexed Modes
- **Requirement:** Support label references with index suffixes
- **Examples:**
  - `LDA table,X` → AbsoluteX with label "table"
  - `STA buffer,Y` → AbsoluteY with label "buffer"
- **Behavior:** Resolve label address, apply indexing

#### FR7: Whitespace Tolerance
- **Requirement:** Parse syntax with varying whitespace
- **Examples (all should be equivalent):**
  - `LDA $80,X` ≡ `LDA $80, X` ≡ `LDA $80 , X`
  - `LDA ($80,X)` ≡ `LDA ( $80 , X )`
  - `LDA ($80),Y` ≡ `LDA ( $80 ) , Y`

---

### Quality Requirements

#### QR1: Test Coverage
- **Target:** ≥85% overall coverage, 95%+ for syntax parsing logic
- **Tests required:**
  - Unit tests: Each addressing mode syntax variant (≥30 tests)
  - Integration tests: Complete assembly programs using all syntaxes (≥10 tests)
  - Error cases: Invalid syntax detection (≥10 tests)

#### QR2: Test-Driven Development
- **Process:** Follow RED-GREEN-REFACTOR cycle
- **Evidence:** Git commits show tests before implementation
- **Validation:** Tester role validates TDD compliance

#### QR3: Code Quality
- **Standards:** C++ 2-space indentation, declarations in headers, implementations in .cpp
- **Warnings:** Zero compiler warnings
- **Review:** Code review approved by Reviewer role

#### QR4: Error Handling
- **Invalid syntax:** Clear error messages for malformed operands
- **Examples:**
  - `LDA ($80,Y)` → Error: IndexedIndirect requires ,X not ,Y
  - `LDA ($1234,X)` → Error: IndexedIndirect requires zero page address
  - `STA A` → Error: STA does not support Accumulator mode

#### QR5: Documentation
- **Inline comments:** Explain parsing logic
- **Examples:** Document supported syntax variants
- **Work log:** Track progress via git commits

---

### Non-Functional Requirements

#### NFR1: Performance
- **Target:** Parsing overhead <1ms per instruction (not measurable yet, just "fast")
- **No performance regression from Phase 2.3

#### NFR2: Backward Compatibility
- **Existing syntax:** All Phase 2.3 syntax must still work
- **Existing tests:** All 155 existing tests must continue passing

#### NFR3: Maintainability
- **Clear code:** Self-documenting parsing logic
- **Extensible:** Easy to add future addressing modes (e.g., for other CPUs)

---

## Success Criteria

### Must Have (Blocking)

1. ✅ All 8 missing addressing mode syntaxes parseable:
   - Accumulator: `ASL A`
   - ZeroPageX/Y: `LDA $80,X`, `LDX $80,Y`
   - AbsoluteX/Y: `LDA $1234,X`, `LDA $1234,Y`
   - Indirect: `JMP ($1234)`
   - IndexedIndirect: `LDA ($80,X)`
   - IndirectIndexed: `LDA ($80),Y`

2. ✅ All 155 existing CPU unit tests still pass

3. ✅ At least 30 new unit tests for syntax parsing

4. ✅ At least 10 integration tests assembling real programs with all syntaxes

5. ✅ Error handling for invalid syntax with clear messages

6. ✅ Zero compiler warnings

7. ✅ TDD process followed (RED-GREEN-REFACTOR)

8. ✅ Tester validates tests and coverage

9. ✅ Reviewer validates code quality

10. ✅ Work log maintained (git commits)

### Should Have (Important)

11. ✅ Label support for indexed modes (`table,X`)

12. ✅ Whitespace tolerance in parsing

13. ✅ Performance: No measurable slowdown

### Nice to Have (Optional)

14. ⚪ Parser refactoring: Extract to separate class (future)

15. ⚪ Parser unit tests: Test `DetermineAddressingMode()` directly (future)

---

## Out of Scope

### Explicitly NOT Included

- **Other CPU architectures** - Z80, 68000, etc. (future phases)
- **Macro support** - `.macro`, `.endmacro` (future)
- **Advanced directives** - `.include`, `.if`, `.ifdef` (future)
- **Binary output** - Generate `.prg` or `.bin` files (future)
- **Illegal opcodes** - Undocumented 6502 instructions (future)
- **Parser refactoring** - Extracting parsing to separate class (could be done but not required)

---

## Dependencies

### Technical Dependencies
- ✅ Phase 2.3 complete (all 56 opcodes implemented)
- ✅ CPU plugin with all 13 addressing modes
- ✅ Assembler infrastructure (Section, Atom, Assemble() method)
- ✅ Test infrastructure (Google Test)

### External Dependencies
- None

---

## Risks and Mitigation

### Risk 1: Parsing Complexity
- **Risk:** Complex parsing logic for indirect modes with parentheses and commas
- **Likelihood:** Medium
- **Impact:** High (could introduce bugs)
- **Mitigation:**
  - Use TDD to catch bugs early
  - Add comprehensive error handling tests
  - Incremental implementation (one addressing mode at a time)

### Risk 2: Backward Compatibility
- **Risk:** New parsing breaks existing Phase 2.3 syntax
- **Likelihood:** Low
- **Impact:** High (regression)
- **Mitigation:**
  - Run all 155 existing tests after each change
  - Add tests for existing syntax variants explicitly

### Risk 3: Whitespace Handling
- **Risk:** Inconsistent whitespace handling causes edge case bugs
- **Likelihood:** Medium
- **Impact:** Medium (user frustration)
- **Mitigation:**
  - Trim whitespace aggressively
  - Add tests with various whitespace patterns
  - Use standard string manipulation (no regex complexity)

### Risk 4: Error Messages Unclear
- **Risk:** Users don't understand why syntax is invalid
- **Likelihood:** Medium
- **Impact:** Medium (poor UX)
- **Mitigation:**
  - Provide specific error messages per failure case
  - Include examples of correct syntax in errors
  - Test error messages explicitly

---

## Acceptance Criteria Summary

**Checklist for Task Completion:**

### Functional (8 items)
- [ ] Accumulator mode parsed: `ASL A`
- [ ] ZeroPageX/Y parsed: `LDA $80,X`, `LDX $80,Y`
- [ ] AbsoluteX/Y parsed: `LDA $1234,X`, `LDA $1234,Y`
- [ ] Indirect parsed: `JMP ($1234)`
- [ ] IndexedIndirect parsed: `LDA ($80,X)`
- [ ] IndirectIndexed parsed: `LDA ($80),Y`
- [ ] Label support for indexed modes: `LDA table,X`
- [ ] Whitespace tolerance: `LDA $80, X` ≡ `LDA $80,X`

### Quality (12 items)
- [ ] All 155 existing tests still pass
- [ ] At least 30 new unit tests for syntax parsing
- [ ] At least 10 integration tests with real assembly
- [ ] Error handling with clear messages
- [ ] Zero compiler warnings
- [ ] Test coverage ≥85% overall, 95%+ parsing
- [ ] TDD RED-GREEN-REFACTOR followed
- [ ] Tester approved (≥80/100 score)
- [ ] Reviewer approved (no blocking/major issues)
- [ ] Work log maintained (git commits)
- [ ] C++ standards: 2-space indent, header/impl separation
- [ ] No performance regression

### Total: 20 acceptance criteria (all must pass)

---

## Timeline Estimate

**Estimated Effort:** 4-6 hours
**Complexity:** Medium (parsing logic, but well-defined)

**Breakdown:**
- Setup: 15 min
- Accumulator mode: 30 min (simplest)
- Indexed modes (,X ,Y): 1 hour
- Indirect modes (parentheses): 2 hours (most complex)
- Error handling: 1 hour
- Integration tests: 1 hour
- Review and refinement: 30 min

---

## Sign-Off

**Contract Author:** Orchestrator Role (ai-pack framework)
**Date:** 2026-01-22

**Stakeholder Approval:**
- [ ] Project Owner
- [ ] Technical Lead (if applicable)
- [x] Orchestrator (this contract is ready for planning)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-22
