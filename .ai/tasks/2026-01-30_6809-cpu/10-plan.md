# Implementation Plan

**Task ID:** 2026-01-30_6809-cpu
**Beads Task:** xasm++-7z9
**Created:** 2026-01-30
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Implement Motorola 6809 CPU plugin for xasm++ using Test-Driven Development (TDD). The 6809 is one of the most sophisticated 8-bit CPUs ever designed, featuring 13 addressing modes (including 16 indexed sub-modes), big-endian byte order, and dual stack pointers.

**Key Technical Decisions:**
- **Follow existing 6502 plugin pattern** - Use `CpuBase` interface, similar file structure, proven testing approach
- **Big-endian byte order** - Unlike 6502 (little-endian), 6809 stores 16-bit values MSB-first
- **Incremental TDD implementation** - Start with simple modes (inherent, immediate), progressively add complex indexed addressing
- **Phased approach** - 10 phases from inherent instructions through complex stack operations

**Patterns to Use:**
- **TDD RED-GREEN-REFACTOR** - Write failing tests first, make them pass, then refactor
- **CPU plugin interface** - Follows `Cpu6502` pattern with Execute_* methods
- **Opcode table encoding** - Constexpr arrays for page 1, page 2 ($10xx), page 3 ($11xx) opcodes
- **Indexed addressing parser** - Separate method to handle 16 complex indexed sub-modes

---

## Critical Files Identified

### Files to Create
```
1. include/xasm++/cpu/cpu_6809.h
   - Purpose: 6809 CPU plugin class interface
   - Rationale: New CPU family requires new plugin class

2. include/xasm++/cpu/opcodes_6809.h
   - Purpose: 6809 opcode tables (page 1, 2, 3)
   - Rationale: ~59 opcodes × modes need organized tables

3. src/cpu/m6809/cpu_6809.cpp
   - Purpose: 6809 instruction implementations
   - Rationale: Execute_* methods for all instructions

4. tests/unit/test_cpu6809.cpp
   - Purpose: Comprehensive 6809 CPU tests
   - Rationale: TDD requires tests first, ~100+ test cases
```

### Files to Modify
```
1. src/CMakeLists.txt
   - Current: Builds 6502 family CPU plugins
   - Changes: Add cpu_6809.cpp to build, create m6809 directory
```

### Files to Read (for context)
```
- docs/references/6809-ARCHITECTURE.md - Complete 6809 specification and instruction set
- include/xasm++/cpu/cpu_6502.h - Example CPU plugin interface pattern
- src/cpu/m6502/cpu_6502.cpp - Example implementation pattern
- tests/unit/test_cpu6502.cpp - Example test structure
```

---

## Step-by-Step Implementation Plan

**Engineer will follow TDD methodology: RED → GREEN → REFACTOR for each phase.**

### Phase 1: Foundation
```
Step 1.1: Create directory and stub files
  - Action: Create include/xasm++/cpu/cpu_6809.h with class stub
  - Action: Create src/cpu/m6809/cpu_6809.cpp with empty implementations
  - Action: Create tests/unit/test_cpu6809.cpp
  - Action: Update src/CMakeLists.txt to build new files
  - Verification: Project builds without errors

Step 1.2: Verify test infrastructure
  - Action: Add one trivial test (e.g., CPU constructor works)
  - Verification: ctest runs and passes (577 + 1 tests)
```

### Phase 2: Inherent Mode Instructions (TDD)
```
Step 2.1: Write failing tests for inherent instructions
  - Tests: NOP, CLRA, CLRB, RTS, etc. (~15 instructions)
  - Verification: Tests fail (methods don't exist yet)

Step 2.2: Implement inherent instructions
  - Action: Add Execute_NOP(), Execute_CLRA(), etc.
  - Verification: Tests pass

Step 2.3: Refactor
  - Action: Extract common patterns, add documentation
  - Verification: Tests still pass, zero warnings
```

### Phase 3: Immediate Addressing (TDD)
```
Step 3.1: Write failing tests for immediate mode
  - Tests: LDA #$42 (8-bit), LDD #$1234 (16-bit)
  - Tests: Explicit big-endian byte order tests
  - Verification: Tests fail

Step 3.2: Implement immediate addressing
  - Action: Add Execute_LDA_Immediate(), Execute_LDD_Immediate()
  - Action: Implement big-endian helpers (ToBigEndian, FromBigEndian)
  - Verification: Tests pass, big-endian verified

Step 3.3: Refactor
  - Verification: Tests pass, zero warnings
```

### Phase 4: Direct and Extended Addressing (TDD)
```
Step 4.1: Write failing tests
  - Tests: Direct page (<$80), Extended ($1234)
  - Tests: Direct page register tracking

Step 4.2: Implement addressing modes
  - Action: Add ResolveDirectPage(), ResolveExtended()
  - Action: Add direct_page_register_ member variable

Step 4.3: Refactor
```

### Phase 5-10: Progressive Implementation
```
Phase 5: Simple indexed addressing (zero offset, constant offsets)
Phase 6: Complex indexed addressing (accumulator, auto-inc/dec, PCR, indirect)
Phase 7: Branches (short 8-bit, long 16-bit, all conditionals)
Phase 8: Jumps and subroutines (JMP, JSR, BSR, LBSR, RTS)
Phase 9: Stack operations (PSHS/PULS/PSHU/PULU with register lists)
Phase 10: Special instructions (SWI, SWI2, SWI3, CWAI, SYNC, RTI, DAA, SEX, MUL)
```

**Each phase follows same pattern:**
1. Write failing tests (RED)
2. Implement to pass tests (GREEN)
3. Refactor and optimize (REFACTOR)
4. Verify zero warnings

---

## Testing Strategy

### Unit Tests
```
□ Inherent mode - Test each instruction
□ Immediate 8-bit - Test with various values
□ Immediate 16-bit - Test big-endian byte order explicitly
□ Direct page - Test offsets $00-$FF, verify DP register
□ Extended - Test full 16-bit addresses
□ Indexed (16 sub-modes) - Comprehensive tests for each sub-mode:
  - Zero offset (,X ,Y ,U ,S)
  - 5-bit offset (-16 to +15)
  - 8-bit offset (-128 to +127)
  - 16-bit offset (-32768 to +32767)
  - Accumulator offset (A,X; B,Y; D,U)
  - Auto-increment (,X+ ,X++)
  - Auto-decrement (,-X ,--X)
  - PC relative (n,PCR)
  - Indirect ([,X] [10,Y] [$1234])
□ Branches - Test all conditions, short and long
□ Stack operations - Test register list encoding
□ Special instructions - Test each one
```

### Coverage Target
```
- Overall: 90%+ for 6809 code
- Critical paths: 100% (big-endian, indexed addressing)
- New code: 90%+
```

---

## Dependencies and Prerequisites

### Technical Dependencies
```
✓ CPU plugin base class (existing)
✓ Test infrastructure (existing)
✓ Build system (minor CMakeLists.txt update needed)
✓ 6809 architecture documentation (complete in docs/references/)
```

### Task Dependencies
```
✓ No blocking dependencies - can proceed immediately
```

### Knowledge Dependencies
```
✓ 6809 architecture (documented in 6809-ARCHITECTURE.md)
✓ Big-endian byte order (different from 6502)
✓ Indexed addressing post-byte encoding (documented)
✓ Register list encoding for PSHS/PULS (documented)
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Complex indexed addressing (16 sub-modes)
- **Probability:** High
- **Impact:** High
- **Mitigation:** Implement incrementally with TDD, test each mode thoroughly, reference 6809-ARCHITECTURE.md
- **Contingency:** If too complex, split into multiple task packets

**Risk 2:** Big-endian byte order confusion
- **Probability:** Medium
- **Impact:** High
- **Mitigation:** Explicit unit tests for byte order, clear documentation, test FDB $1234 → $12 $34
- **Contingency:** Review all 16-bit operations if tests fail

**Risk 3:** Register list encoding (PSHS/PULS)
- **Probability:** Medium
- **Impact:** Medium
- **Mitigation:** Reference Motorola datasheet, test each register combination
- **Contingency:** Create helper lookup table if bit encoding proves difficult

**Risk 4:** Token limit exceeded
- **Probability:** Low (15K estimated tokens, well under 25K-32K limit)
- **Impact:** Medium
- **Mitigation:** Small batch size (5 files), incremental implementation
- **Contingency:** Split into multiple task packets if needed

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. git restore . (if changes not committed)
   OR
   git revert <commit-hash> (if changes committed)
2. Remove new files (cpu_6809.h, cpu_6809.cpp, opcodes_6809.h, test_cpu6809.cpp)
3. Revert CMakeLists.txt changes
4. Verify: ctest passes (577 tests)
```

**Rollback Verification:**
```
✓ Project builds without errors
✓ All 577 existing tests pass
✓ No 6809 files remain
✓ CMakeLists.txt restored
```

---

## Performance Considerations

### Expected Performance Impact
```
- Opcode lookup: O(1) or O(log n) using constexpr arrays
- Indexed addressing: O(1) post-byte decoding
- Memory: ~5KB additional code for 6809 plugin
```

### Performance Targets
```
- Opcode lookup: < 1μs
- Instruction encoding: < 10μs
- No impact on existing 6502 performance
```

---

## Success Metrics

### Completion Criteria
```
✓ All 59 fundamental 6809 opcodes implemented
✓ All 13 addressing modes working
✓ Big-endian byte order correctly handled
✓ Direct page register support
✓ All tests passing (577 existing + ~100 new 6809 tests)
✓ Test coverage ≥ 90% for 6809 code
✓ Zero compiler warnings
✓ Tester validation: APPROVED
✓ Reviewer validation: APPROVED
```

### Quality Metrics
```
- Test coverage: 90%+ for 6809 code
- Performance: Opcode lookup < 1μs
- Code quality: Zero warnings, follows 6502 pattern
- Security: No issues (CPU plugin is data processing only)
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [x] Orchestrator [2026-01-30]
- [ ] Engineer (will approve when starting implementation)

---

## Notes and Considerations

**Implementation Notes:**
- Engineer will use TDD methodology (mandatory for ai-pack)
- Each phase builds on previous (incremental approach)
- Reference 6809-ARCHITECTURE.md throughout implementation
- Follow 6502 plugin pattern closely (proven approach)
- Test big-endian byte order explicitly in multiple test cases
- Document indexed addressing post-byte encoding clearly

**Out of Scope:**
- EDTASM+ syntax support (separate Phase 2 task)
- Macro support (separate task)
- Conditional assembly (separate task)
- Binary output formats (separate task)

**Resources:**
- Architecture reference: `docs/references/6809-ARCHITECTURE.md`
- Example pattern: `src/cpu/m6502/cpu_6502.cpp`
- Test pattern: `tests/unit/test_cpu6502.cpp`

---

**Plan Version:** 1.0
**Last Updated:** 2026-01-30
**Status:** Ready for Engineer agent
