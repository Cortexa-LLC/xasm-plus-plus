# Task Contract

**Task ID:** 2026-01-21_complete-6502-instruction-set
**Beads Task:** xasm++-d9u
**Created:** 2026-01-21
**Requestor:** Project Plan Phase 2.1
**Assigned Role:** Orchestrator (default)
**Workflow:** Feature

---

## Task Description

Complete the 6502 CPU plugin by implementing all remaining instructions and addressing modes to achieve 100% coverage of the 56 legal 6502 opcodes as specified in Phase 2.1 of the project plan.

### Background and Context

Phase 2.2 successfully implemented 25 essential 6502 instructions, bringing the total from 5 to 30 implemented instructions. However, Phase 2.1 of the project plan requires **all 56 legal 6502 opcodes** and **all 13 addressing modes** to be implemented before Phase 2 can be considered complete.

The 6502 is the foundation CPU for this assembler, and complete instruction coverage is critical for:
- Assembling real-world 6502 programs (Apple II, C64, NES software)
- Validation of the CPU plugin architecture
- Foundation for 65C02 and 65816 extensions in later phases

### Current State

**Implemented (30/56 instructions):**
- Phase 1: LDA, STA, JMP, NOP, RTS (5 instructions)
- Phase 2.2 Groups 1-8: ADC, SBC, AND, ORA, EOR, LDX, LDY, STX, STY, CMP, CPX, CPY, BEQ, BNE, BCC, BCS, BMI, BPL, BVC, BVS, INX, INY, DEX, DEY, INC, DEC, PHA, PLA, PHP, PLP, JSR (25 instructions)

**Implemented Addressing Modes (6/13):**
- Implied
- Immediate
- ZeroPage
- Absolute
- Relative (branches)
- (Partial: Some instructions support additional modes not yet exposed)

### Desired State

**All 56 legal 6502 opcodes** implemented with full addressing mode support:

**Missing Instructions (26):**
1. **BIT** - Test bits (ZeroPage, Absolute)
2. **ASL** - Arithmetic shift left (Accumulator, ZeroPage, ZeroPageX, Absolute, AbsoluteX)
3. **LSR** - Logical shift right (Accumulator, ZeroPage, ZeroPageX, Absolute, AbsoluteX)
4. **ROL** - Rotate left (Accumulator, ZeroPage, ZeroPageX, Absolute, AbsoluteX)
5. **ROR** - Rotate right (Accumulator, ZeroPage, ZeroPageX, Absolute, AbsoluteX)
6. **RTI** - Return from interrupt (Implied)
7. **BRK** - Break (Implied)
8. **CLC** - Clear carry (Implied)
9. **SEC** - Set carry (Implied)
10. **CLD** - Clear decimal (Implied)
11. **SED** - Set decimal (Implied)
12. **CLI** - Clear interrupt disable (Implied)
13. **SEI** - Set interrupt disable (Implied)
14. **CLV** - Clear overflow (Implied)
15. **TSX** - Transfer SP to X (Implied)
16. **TXS** - Transfer X to SP (Implied)
17. **TAX** - Transfer A to X (Implied)
18. **TAY** - Transfer A to Y (Implied)
19. **TXA** - Transfer X to A (Implied)
20. **TYA** - Transfer Y to A (Implied)

Plus extend existing instructions with missing addressing modes:
21-26. LDA, STA, LDX, LDY, STX, STY with IndexedIndirect/IndirectIndexed modes

**Missing Addressing Modes (7):**
- Accumulator (ASL A, LSR A, ROL A, ROR A)
- ZeroPageX (most instructions)
- ZeroPageY (LDX, STX only)
- AbsoluteX (many instructions)
- AbsoluteY (LDA, STA, etc.)
- Indirect (JMP ($1234) only)
- IndexedIndirect (LDA ($80,X))
- IndirectIndexed (LDA ($80),Y)

---

## Success Criteria

```
✓ All 56 legal 6502 opcodes implemented
✓ All 13 addressing modes implemented
✓ All tests passing (173 existing + ~80 new = ~253 total)
✓ Code coverage ≥ 85%
✓ Integration tests validate complete instruction set
✓ Zero-page optimization working for applicable addressing modes
```

---

## Acceptance Criteria

### Functional Requirements
```
□ BIT instruction implemented (ZeroPage, Absolute)
□ Shift/rotate instructions: ASL, LSR, ROL, ROR (5 modes each)
□ Interrupt instructions: RTI, BRK (Implied)
□ Flag instructions: CLC, SEC, CLD, SED, CLI, SEI, CLV (Implied)
□ Transfer instructions: TSX, TXS, TAX, TAY, TXA, TYA (Implied)
□ Accumulator addressing mode for shifts/rotates
□ ZeroPageX addressing mode for applicable instructions
□ ZeroPageY addressing mode (LDX, STX)
□ AbsoluteX addressing mode for applicable instructions
□ AbsoluteY addressing mode (LDA, STA, AND, ORA, EOR, ADC, SBC, CMP)
□ Indirect addressing mode (JMP only)
□ IndexedIndirect addressing mode (LDA, STA, CMP, AND, ORA, EOR, ADC, SBC)
□ IndirectIndexed addressing mode (LDA, STA, CMP, AND, ORA, EOR, ADC, SBC)
□ All addressing mode combinations match 6502 specification
□ Assembler dispatch updated for all new instructions
```

### Quality Requirements
```
□ All tests passing (~253 tests)
□ Code coverage ≥ 85%
□ No compiler warnings (-Wall -Wextra -Wpedantic)
□ C++ implementation follows 2-space indentation
□ Declarations in headers, implementations in .cpp
□ TDD RED-GREEN-REFACTOR cycle followed
□ Code review approved (Tester + Reviewer)
□ Work log maintained
```

### Non-Functional Requirements
```
□ Test suite runs in < 3 seconds
□ Correct 6502 opcodes from standard reference
□ Little-endian encoding for multi-byte operands
□ Error messages clear for unsupported mode combinations
```

---

## Constraints and Dependencies

### Constraints
```
□ Must implement only legal 6502 opcodes (no undocumented)
□ Must match standard 6502 opcode table exactly
□ Must maintain compatibility with existing 173 tests
□ Must follow C++ project standards (2-space indent, .cpp implementations)
```

### Dependencies
```
□ Phase 2.2 complete (✅ Done: 173 tests passing)
□ Google Test framework (✅ Available)
□ CMake build system (✅ Working)
□ 6502 opcode reference documentation
```

### Out of Scope
```
✗ Undocumented/illegal 6502 opcodes
✗ 65C02-specific instructions (Phase 3)
✗ Zero-page optimization refinement (acceptable if basic works)
✗ Performance optimization
✗ Macro support
✗ Advanced syntax features
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: 4 files
  - `tests/unit/test_cpu6502.cpp` - Add ~80 tests
  - `include/xasm++/cpu/cpu_6502.h` - Add 20 method declarations
  - `src/cpu/m6502/cpu_6502.cpp` - Add ~600 lines of implementations
  - `src/core/assembler.cpp` - Add 20 dispatch cases
- Lines of code estimate: ~700 lines
- New concepts/patterns: No (following established TDD pattern from Phase 2.2)
- Integration complexity: Low (same pattern as Phase 2.2)
- Risk level: Low (well-understood problem, clear specification)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 4 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 4 files → ✅ IDEAL
```

**Decision:** Proceed as single task packet (optimal batch size).

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
4 × 3,000 tokens = 12,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 12,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (Engineer role, direct implementation)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [x] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

**Rationale:**
- 4 files: Well within ideal range
- 12K tokens: Safe, plenty of headroom
- 1 agent: Single Engineer implementation
- High cohesion: All changes complete 6502 instruction set
- Clear specification: 6502 opcode table is definitive reference

---

## Resources and References

### Relevant Files
```
- tests/unit/test_cpu6502.cpp - Add ~80 new tests (Tests 68-~148)
- include/xasm++/cpu/cpu_6502.h - Add 20 method declarations
- src/cpu/m6502/cpu_6502.cpp - Implement 26 encoding methods
- src/core/assembler.cpp - Add 20 mnemonic dispatch cases
```

### Documentation
```
- docs/planning/project-plan.md - Phase 2.1 requirements
- 6502 Opcode Reference: http://www.6502.org/tutorials/6502opcodes.html
- MCS6500 Microcomputer Family Programming Manual
- Phase 2.2 work log (.ai/tasks/2026-01-21_essential-6502-instructions/) - Implementation pattern reference
```

### Examples
```
- src/cpu/m6502/cpu_6502.cpp - Existing Phase 2.2 implementations (ADC, LDA patterns)
- tests/unit/test_cpu6502.cpp - Existing Phase 2.2 test patterns (Tests 13-67)
```

---

## Assumptions

```
1. Standard 6502 opcode table is definitive reference
2. TDD RED-GREEN-REFACTOR approach will be followed
3. Implementation will follow Phase 2.2 established patterns
4. Zero-page optimization only needs basic implementation
5. Test suite < 3 seconds is achievable with ~253 tests
6. All addressing mode combinations per instruction are specified in 6502 docs
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Addressing mode combinations correctness
   - Probability: Medium
   - Impact: High (incorrect opcodes = wrong binaries)
   - Mitigation: Use authoritative 6502 opcode reference, validate each combination

2. Test count explosion (80+ new tests)
   - Probability: Low
   - Impact: Medium (test suite slowdown)
   - Mitigation: Group related tests, monitor test execution time

3. Dispatch chain growing very long in assembler.cpp
   - Probability: High
   - Impact: Low (works fine, just verbose)
   - Mitigation: Document as acceptable technical debt, consider refactoring post-Phase 2
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: Project Plan Phase 2.1 [2026-01-21]
- [x] Agent: Orchestrator [2026-01-21]

**Changes to Contract:**
[No changes yet]

---

## Notes

**Implementation Strategy:**

Organize into logical groups similar to Phase 2.2:
- **Group 1**: BIT instruction (2 tests)
- **Group 2**: Shift instructions - ASL, LSR (10 tests)
- **Group 3**: Rotate instructions - ROL, ROR (10 tests)
- **Group 4**: Interrupt - RTI, BRK (2 tests)
- **Group 5**: Flag operations - CLC, SEC, CLD, SED, CLI, SEI, CLV (7 tests)
- **Group 6**: Transfer instructions - TSX, TXS, TAX, TAY, TXA, TYA (6 tests)
- **Group 7**: Indexed addressing modes for existing instructions (40+ tests)
- **Group 8**: Indirect addressing modes - Indirect, IndexedIndirect, IndirectIndexed (10+ tests)

**Total**: ~8 groups, ~85-90 new tests, bringing total from 173 to ~260 tests.

**Commit Strategy**: Commit each group separately for tracking and rollback capability.

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-21
