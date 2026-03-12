# Task Contract

**Task ID:** 2026-01-30_6809-cpu
**Beads Task:** xasm++-7z9
**Created:** 2026-01-30
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement Motorola 6809 CPU plugin for xasm++ to enable assembly of TRS-80 Color Computer (CoCo) code.

### Background and Context

The xasm++ cross-assembler currently supports the 6502 family (6502, 65C02, 65816). The Motorola 6809 is a sophisticated 8-bit CPU used in the TRS-80 Color Computer, Dragon 32/64, and Vectrex arcade system. It features advanced addressing modes (13 modes including sophisticated indexed addressing) and 16-bit operations, making it one of the most capable 8-bit CPUs ever designed.

The 6809 requires a new CPU plugin architecture due to:
- **Big-endian byte order** (MSB first, unlike 6502's little-endian)
- **16 indexed addressing sub-modes** (most complex of any 8-bit CPU)
- **Dual stack pointers** (S for system, U for user)
- **Different syntax requirements** (EDTASM+ uses semicolon comments, no dot-prefixed directives)

### Current State

- xasm++ supports 6502/65C02/65816 CPU family
- CPU plugin interface defined in `include/xasm++/cpu/`
- Test infrastructure in place (`tests/unit/test_cpu*.cpp`)
- 6809 architecture documented in `docs/references/6809-ARCHITECTURE.md`

### Desired State

- Complete 6809 instruction set support (~59 fundamental opcodes)
- All 13 addressing modes implemented, especially the complex indexed modes
- Big-endian byte order handling
- Direct page register tracking
- Test coverage for all instructions and addressing modes
- Ready for EDTASM+ syntax integration (Phase 2 task)

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ All 59 fundamental 6809 opcodes implemented
✓ All 13 addressing modes working (inherent, immediate, direct, extended, indexed variants, relative)
✓ Big-endian byte order correctly handled for 16-bit values
✓ Direct page register support (SETDP directive)
✓ All tests passing (existing + new 6809 tests)
✓ Test coverage ≥ 90% for 6809 code
✓ Zero compiler warnings
✓ Code review approved
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements - Core Instructions
```
□ Data movement (LDA, LDB, LDD, LDX, LDY, LDS, LDU, STA, STB, STD, STX, STY, STS, STU)
□ Transfers and exchanges (TFR, EXG, SEX)
□ 8-bit arithmetic (ADDA, ADCA, SUBA, SBCA, INCA, DECA, NEGA, and B variants)
□ 16-bit arithmetic (ADDD, SUBD, LEAX, LEAY, LEAS, LEAU, ABX, MUL)
□ Logical operations (ANDA, ORA, EORA, COMA, ANDCC, ORCC, and B variants)
□ Shift and rotate (ASL, ASR, LSL, LSR, ROL, ROR - A, B, and memory variants)
□ Test and compare (TSTA, TSTB, TST, CMPA, CMPB, CMPD, CMPX, CMPY, CMPS, CMPU, BITA, BITB)
□ Branch instructions (BRA, BRN, BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC, BHI, BLS, BGE, BLT, BGT, BLE)
□ Long branches (LBRA, LBRN, LBEQ, LBNE, etc.)
□ Jump and subroutine (JMP, JSR, BSR, LBSR, RTS)
□ Stack operations (PSHS, PULS, PSHU, PULU with register lists)
□ Interrupt handling (SWI, SWI2, SWI3, CWAI, SYNC, RTI)
□ Miscellaneous (NOP, DAA)
```

### Functional Requirements - Addressing Modes
```
□ Inherent/Implied (NOP, CLRA, RTS, etc.)
□ Immediate 8-bit (#$42)
□ Immediate 16-bit (#$1234)
□ Direct page (<$80)
□ Extended/Absolute ($1234)
□ Indexed - Zero offset (,X, ,Y, ,U, ,S)
□ Indexed - 5-bit constant offset (-16 to +15)
□ Indexed - 8-bit constant offset (-128 to +127)
□ Indexed - 16-bit constant offset (-32768 to +32767)
□ Indexed - Accumulator offset (A,X; B,Y; D,U)
□ Indexed - Auto-increment (,X+, ,X++)
□ Indexed - Auto-decrement (,-X, ,--X)
□ Indexed - PC relative (n,PCR)
□ Indexed - Indirect ([,X], [10,Y], [$1234])
□ Relative - 8-bit offset (short branches)
□ Relative - 16-bit offset (long branches)
```

### Functional Requirements - Data Handling
```
□ Big-endian byte order for 16-bit values (FDB $1234 → $12 $34, not $34 $12)
□ Direct page register tracking (default $00, configurable)
□ Register list encoding for PSHS/PULS/PSHU/PULU
□ Opcode generation with correct byte counts and cycle times
```

### Quality Requirements
```
□ All tests passing (existing 577 + new 6809 tests)
□ Test coverage ≥ 90% for 6809 CPU code
□ Zero compiler warnings (-Wall -Wextra -Werror)
□ TDD process followed (RED-GREEN-REFACTOR)
□ Code review approved by Reviewer agent
□ Documentation complete (inline comments, opcode tables)
```

### Non-Functional Requirements
```
□ Performance: Opcode lookup O(1) or O(log n)
□ Memory: Reasonable opcode table size
□ Architecture: Follows existing CPU plugin pattern (see cpu_6502.cpp)
□ Maintainability: Clear separation of addressing mode logic
□ Testability: Each instruction and mode independently testable
```

---

## Constraints and Dependencies

### Constraints
```
□ Must follow existing CPU plugin interface (CpuBase class pattern)
□ Must use big-endian byte order (different from 6502)
□ Complex indexed addressing requires careful parsing (16 sub-modes)
□ Register list encoding is bit-mapped (PSHS A,B,X = specific byte pattern)
□ C++20 codebase (use modern C++ features)
```

### Dependencies
```
□ Existing CPU plugin infrastructure (include/xasm++/cpu/)
□ Test infrastructure (tests/unit/test_cpu6502.cpp pattern)
□ Build system (CMakeLists.txt updates)
□ 6809 architecture documentation (docs/references/6809-ARCHITECTURE.md)
□ No dependencies on other tasks (can proceed immediately)
```

### Out of Scope
```
✗ EDTASM+ syntax implementation (separate Phase 2 task)
✗ Macro support (Merlin macros task)
✗ Conditional assembly (separate task)
✗ Binary output formats (.BIN, .CCC, .ROM) - separate task
✗ CoCo-specific I/O addresses and hardware support
✗ Assembler directives (ORG, EQU, FCB, FDB, etc. - syntax layer)
```

---

## Estimated Complexity

**Complexity:** Large

**Rationale:**
- Number of files affected: **3-5 files**
  - `include/xasm++/cpu/cpu_6809.h` (new)
  - `include/xasm++/cpu/opcodes_6809.h` (new)
  - `src/cpu/m6809/cpu_6809.cpp` (new)
  - `tests/unit/test_cpu6809.cpp` (new)
  - `src/CMakeLists.txt` (minor update)
- Lines of code estimate: **~2000-3000 LOC**
  - ~59 opcodes × ~20 LOC each = ~1180 LOC
  - 13 addressing modes × ~50 LOC each = ~650 LOC
  - Tests: ~1000 LOC
  - Headers and infrastructure: ~200 LOC
- New concepts/patterns: **Yes** (big-endian, complex indexed addressing, dual stacks)
- Integration complexity: **Medium** (follows existing CPU plugin pattern)
- Risk level: **Medium** (complex addressing modes, new byte order)

---

## Lean Flow Analysis (MANDATORY)

**Purpose:** Prevent token limit failures and verification chaos (see `principles/LEAN-FLOW.md`)

### Batch Size Assessment

**Estimated Files:** 5 files (3 implementation + 1 test + 1 CMake)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 5 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
5 × 3,000 tokens = 15,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 15,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (Engineer)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [x] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

**Rationale:**
- 5 files well within ideal batch size (1-5 files)
- Estimated 15K tokens well under 25K-32K limit
- Single agent execution (no WIP concerns)
- High cohesion: all files part of single CPU plugin
- Single logical unit: complete 6809 CPU implementation

---

## Resources and References

### Relevant Files
```
- docs/references/6809-ARCHITECTURE.md - Complete 6809 architecture and instruction set
- include/xasm++/cpu/cpu_6502.h - Example CPU plugin interface
- src/cpu/m6502/cpu_6502.cpp - Example CPU implementation pattern
- tests/unit/test_cpu6502.cpp - Example test pattern
- src/CMakeLists.txt - Build system (needs minor update for 6809 files)
```

### Documentation
```
- 6809-ARCHITECTURE.md - Complete instruction set reference
- Lance A. Leventhal "6809 Assembly Language Programming" (if available)
- Motorola MC6809 datasheet (if available)
- EDTASM+ manual (for future syntax work)
```

### Examples
```
- cpu_6502.cpp - Similar CPU plugin implementation
- test_cpu6502.cpp - Test pattern to follow
```

---

## Assumptions

```
1. Engineer will follow TDD (RED-GREEN-REFACTOR) process
2. Existing CPU plugin interface is suitable for 6809 (may need minor extensions)
3. Test infrastructure supports new CPU plugin without major changes
4. Build system can accommodate new CPU files with minor CMakeLists.txt updates
5. EDTASM+ syntax support is a separate Phase 2 task (only CPU plugin in this task)
6. No need for position-independent code (PCR) optimization in Phase 1 (just make it work)
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Complex indexed addressing modes (16 sub-modes)
   - Probability: High
   - Impact: High
   - Mitigation: Study existing implementations (sourcerer), implement incrementally with TDD, thorough testing

2. Big-endian byte order confusion (different from 6502)
   - Probability: Medium
   - Impact: High
   - Mitigation: Explicit tests for byte order, clear documentation, review existing big-endian code

3. Register list encoding complexity (PSHS/PULS)
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Reference Motorola datasheet, test each combination

4. Token limit exceeded during implementation
   - Probability: Low (15K estimated tokens)
   - Impact: Medium
   - Mitigation: Small batch size (5 files), incremental implementation

5. Opcode table size (59 opcodes × addressing modes)
   - Probability: Low
   - Impact: Low
   - Mitigation: Use efficient encoding, follow 6502 pattern
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: Bryan Woodruff [2026-01-30]
- [ ] Agent: Engineer [Date]

**Changes to Contract:**
[Document any contract changes here with date and rationale]

---

## Notes

**Implementation Strategy:**

**Phase 1: Core Instructions (Simplified Addressing)**
- Start with inherent/implied instructions (NOP, CLRA, RTS, etc.)
- Add immediate mode instructions (LDA #$42, LDD #$1234, etc.)
- Verify big-endian byte order working correctly

**Phase 2: Basic Addressing Modes**
- Implement direct page addressing (<$80)
- Implement extended addressing ($1234)
- Verify all data movement instructions work with these modes

**Phase 3: Complex Indexed Addressing**
- Zero offset indexed (,X, ,Y, ,U, ,S)
- Constant offset indexed (5-bit, 8-bit, 16-bit)
- Accumulator offset indexed (A,X; B,Y; D,U)
- Auto-increment/decrement (,X+, ,X++, ,-X, ,--X)
- PC relative (n,PCR)
- Indirect modes ([,X], [10,Y], [$1234])

**Phase 4: Branches and Jumps**
- Short branches (8-bit relative)
- Long branches (16-bit relative)
- Jumps and subroutines (JMP, JSR, BSR, LBSR, RTS)

**Phase 5: Stack and Special Instructions**
- PSHS/PULS/PSHU/PULU with register lists
- Interrupt handling (SWI, SWI2, SWI3, CWAI, SYNC, RTI)
- Miscellaneous (DAA, SEX, MUL)

**Testing Strategy:**
- Follow 6502 test pattern: one test per instruction/mode combination
- Test big-endian byte order explicitly
- Test indexed addressing thoroughly (most complex feature)
- Test register list encoding for stack operations
- Verify all branch offsets calculate correctly

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-30
