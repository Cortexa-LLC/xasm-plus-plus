# Implementation Plan

**Task ID:** 2026-01-21_complete-6502-instruction-set
**Created:** 2026-01-21
**Author:** Engineer Role
**Plan Version:** 1.0

---

## Approach Summary

This task completes the 6502 CPU plugin by implementing the remaining 26 instructions and 7 addressing modes using the same TDD approach proven successful in Phase 2.2. Work will be organized into 8 logical groups, each committed separately for tracking and rollback capability.

The implementation follows the established pattern from Phase 2.2: RED (write failing test) → GREEN (implement minimal code to pass) → REFACTOR (clean up) → COMMIT. This ensures correctness at each step and maintains the existing 173-test baseline.

**Key Technical Decisions:**
- Use standard 6502 opcode reference (6502.org) as authoritative source for all opcode values
- Extend existing AddressingMode enum with new modes (Accumulator, ZeroPageX, ZeroPageY, AbsoluteX, AbsoluteY, Indirect, IndexedIndirect, IndirectIndexed)
- Follow Phase 2.2 patterns for encoding methods (switch statement on addressing mode)
- Organize into 8 groups matching instruction categories (BIT, shifts, rotates, interrupts, flags, transfers, indexed modes, indirect modes)

**Patterns to Use:**
- **TDD RED-GREEN-REFACTOR** - All development, no exceptions
- **Group-and-commit** - Implement each group fully before moving to next
- **Switch-on-mode pattern** - Used in all encoding methods (established in Phase 2.2)
- **Little-endian encoding** - Low byte first, high byte second for multi-byte operands

---

## Critical Files Identified

### Files to Modify
```
1. include/xasm++/cpu/cpu_6502.h
   - Current: Declares 30 instruction encoding methods
   - Changes: Add 20 new method declarations (BIT, ASL, LSR, ROL, ROR, RTI, BRK, CLC, SEC, CLD, SED, CLI, SEI, CLV, TSX, TXS, TAX, TAY, TXA, TYA)
   - Changes: Extend AddressingMode enum with 7 new modes

2. src/cpu/m6502/cpu_6502.cpp
   - Current: Implements 30 instruction encoding methods (~500 lines)
   - Changes: Add 20 new encoding method implementations (~600 lines)
   - Changes: Extend existing methods (LDA, STA, etc.) with new addressing modes

3. src/core/assembler.cpp
   - Current: Dispatches 30 mnemonics to CPU encoding methods
   - Changes: Add 20 new mnemonic dispatch cases
   - Changes: Note in comments that dispatch table refactoring is future work

4. tests/unit/test_cpu6502.cpp
   - Current: 173 tests (Tests 1-67 from Phases 1 and 2.2, plus 106 original base tests)
   - Changes: Add ~85-90 new tests (Tests 68-~158)
   - Changes: Group tests by instruction category for clarity
```

### Files to Create
```
None - all modifications to existing files
```

### Files to Read (for context)
```
- docs/planning/project-plan.md - Phase 2.1 requirements
- src/cpu/m6502/cpu_6502.cpp - Existing Phase 2.2 patterns (ADC, LDA examples)
- tests/unit/test_cpu6502.cpp - Existing test patterns
- 6502 Opcode Reference: http://www.6502.org/tutorials/6502opcodes.html
```

---

## Step-by-Step Implementation Plan

### Group 1: BIT Instruction
```
Step 1.1: Implement BIT tests (RED)
  - Action: Add 2 tests - BIT ZeroPage, BIT Absolute
  - Files: tests/unit/test_cpu6502.cpp
  - Tests: Test 68 (BIT ZeroPage), Test 69 (BIT Absolute)
  - Verification: Tests fail (method not declared)

Step 1.2: Declare BIT method (RED continues)
  - Action: Add EncodeBIT declaration to Cpu6502 class
  - Files: include/xasm++/cpu/cpu_6502.h
  - Verification: Tests fail (method not implemented)

Step 1.3: Implement BIT encoding (GREEN)
  - Action: Implement EncodeBIT in cpu_6502.cpp
  - Files: src/cpu/m6502/cpu_6502.cpp
  - Opcodes: 0x24 (ZeroPage), 0x2C (Absolute)
  - Verification: Tests pass

Step 1.4: Add assembler dispatch (GREEN continues)
  - Action: Add "BIT" case to assembler dispatch chain
  - Files: src/core/assembler.cpp
  - Verification: Integration possible

Step 1.5: Build and test (GREEN verification)
  - Action: cmake --build build && build/tests/unit/test_cpu6502
  - Verification: All 175 tests pass (173 + 2 new)

Step 1.6: Commit Group 1
  - Action: git commit -m "feat(cpu): implement Group 1 - BIT instruction"
```

### Group 2: Shift Instructions (ASL, LSR)
```
Step 2.1: Add Accumulator addressing mode to enum
  - Action: Add AddressingMode::Accumulator to enum
  - Files: include/xasm++/cpu/cpu_6502.h
  - Verification: Compiles

Step 2.2: Implement ASL tests (RED)
  - Action: Add 5 tests - ASL (Accumulator, ZeroPage, ZeroPageX, Absolute, AbsoluteX)
  - Files: tests/unit/test_cpu6502.cpp
  - Tests: Tests 70-74
  - Verification: Tests fail

Step 2.3: Declare and implement ASL (GREEN)
  - Action: Add EncodeASL declaration and implementation
  - Files: include/xasm++/cpu/cpu_6502.h, src/cpu/m6502/cpu_6502.cpp
  - Opcodes: 0x0A (Accumulator), 0x06 (ZeroPage), 0x16 (ZeroPageX), 0x0E (Absolute), 0x1E (AbsoluteX)
  - Verification: ASL tests pass

Step 2.4: Implement LSR tests (RED)
  - Action: Add 5 tests - LSR (Accumulator, ZeroPage, ZeroPageX, Absolute, AbsoluteX)
  - Files: tests/unit/test_cpu6502.cpp
  - Tests: Tests 75-79
  - Verification: Tests fail

Step 2.5: Declare and implement LSR (GREEN)
  - Action: Add EncodeLSR declaration and implementation
  - Files: include/xasm++/cpu/cpu_6502.h, src/cpu/m6502/cpu_6502.cpp
  - Opcodes: 0x4A (Accumulator), 0x46 (ZeroPage), 0x56 (ZeroPageX), 0x4E (Absolute), 0x5E (AbsoluteX)
  - Verification: LSR tests pass

Step 2.6: Add assembler dispatch
  - Action: Add "ASL" and "LSR" cases
  - Files: src/core/assembler.cpp
  - Verification: All 183 tests pass (173 + 10 new)

Step 2.7: Commit Group 2
  - Action: git commit -m "feat(cpu): implement Group 2 - ASL/LSR shift instructions"
```

### Group 3: Rotate Instructions (ROL, ROR)
```
Step 3.1-3.6: Follow same pattern as Group 2
  - ROL: Tests 80-84, Opcodes: 0x2A, 0x26, 0x36, 0x2E, 0x3E
  - ROR: Tests 85-89, Opcodes: 0x6A, 0x66, 0x76, 0x6E, 0x7E
  - Total: 10 new tests (193 total)
  - Commit: "feat(cpu): implement Group 3 - ROL/ROR rotate instructions"
```

### Group 4: Interrupt Instructions (RTI, BRK)
```
Step 4.1-4.4: Implement RTI and BRK (Implied mode only)
  - RTI: Test 90, Opcode: 0x40
  - BRK: Test 91, Opcode: 0x00
  - Total: 2 new tests (195 total)
  - Commit: "feat(cpu): implement Group 4 - RTI/BRK interrupt instructions"
```

### Group 5: Flag Operations
```
Step 5.1-5.7: Implement 7 flag instructions (all Implied mode)
  - CLC: Test 92, Opcode: 0x18
  - SEC: Test 93, Opcode: 0x38
  - CLD: Test 94, Opcode: 0xD8
  - SED: Test 95, Opcode: 0xF8
  - CLI: Test 96, Opcode: 0x58
  - SEI: Test 97, Opcode: 0x78
  - CLV: Test 98, Opcode: 0xB8
  - Total: 7 new tests (202 total)
  - Commit: "feat(cpu): implement Group 5 - flag operation instructions"
```

### Group 6: Transfer Instructions
```
Step 6.1-6.6: Implement 6 transfer instructions (all Implied mode)
  - TSX: Test 99, Opcode: 0xBA
  - TXS: Test 100, Opcode: 0x9A
  - TAX: Test 101, Opcode: 0xAA
  - TAY: Test 102, Opcode: 0xA8
  - TXA: Test 103, Opcode: 0x8A
  - TYA: Test 104, Opcode: 0x98
  - Total: 6 new tests (208 total)
  - Commit: "feat(cpu): implement Group 6 - transfer instructions"
```

### Group 7: Indexed Addressing Modes for Existing Instructions
```
Step 7.1: Add ZeroPageX/Y, AbsoluteX/Y modes to enum (if not already present)
Step 7.2-7.10: Extend existing instruction implementations
  - LDA: Add ZeroPageX, AbsoluteX, AbsoluteY (Tests 105-107)
  - STA: Add ZeroPageX, AbsoluteX, AbsoluteY (Tests 108-110)
  - LDX: Add ZeroPageY, AbsoluteY (Tests 111-112)
  - STX: Add ZeroPageY (Test 113)
  - LDY: Add ZeroPageX, AbsoluteX (Tests 114-115)
  - STY: Add ZeroPageX (Test 116)
  - AND: Add ZeroPageX, AbsoluteX, AbsoluteY (Tests 117-119)
  - ORA: Add ZeroPageX, AbsoluteX, AbsoluteY (Tests 120-122)
  - EOR: Add ZeroPageX, AbsoluteX, AbsoluteY (Tests 123-125)
  - ADC: Add ZeroPageX, AbsoluteX, AbsoluteY (Tests 126-128)
  - SBC: Add ZeroPageX, AbsoluteX, AbsoluteY (Tests 129-131)
  - CMP: Add ZeroPageX, AbsoluteX, AbsoluteY (Tests 132-134)
  - INC: Add ZeroPageX, AbsoluteX (Tests 135-136)
  - DEC: Add ZeroPageX, AbsoluteX (Tests 137-138)
  - Total: ~34 new tests (242 total)
  - Commit: "feat(cpu): implement Group 7 - indexed addressing modes"
```

### Group 8: Indirect Addressing Modes
```
Step 8.1: Add Indirect, IndexedIndirect, IndirectIndexed to enum
Step 8.2: Extend JMP with Indirect mode
  - JMP Indirect: Test 139, Opcode: 0x6C
Step 8.3: Extend load/store/arithmetic with IndexedIndirect, IndirectIndexed
  - LDA: IndexedIndirect, IndirectIndexed (Tests 140-141)
  - STA: IndexedIndirect, IndirectIndexed (Tests 142-143)
  - CMP: IndexedIndirect, IndirectIndexed (Tests 144-145)
  - AND: IndexedIndirect, IndirectIndexed (Tests 146-147)
  - ORA: IndexedIndirect, IndirectIndexed (Tests 148-149)
  - EOR: IndexedIndirect, IndirectIndexed (Tests 150-151)
  - ADC: IndexedIndirect, IndirectIndexed (Tests 152-153)
  - SBC: IndexedIndirect, IndirectIndexed (Tests 154-155)
  - Total: ~17 new tests (259 total)
  - Commit: "feat(cpu): implement Group 8 - indirect addressing modes (COMPLETE)"
```

### Final Verification
```
Step 9.1: Run full test suite
  - Action: cmake --build build && build/tests/unit/test_cpu6502
  - Verification: All ~259 tests pass

Step 9.2: Verify opcode coverage
  - Action: Manual review against 6502 opcode table
  - Verification: All 56 legal opcodes implemented

Step 9.3: Push to GitHub
  - Action: git push origin main
  - Verification: CI passes
```

---

## Testing Strategy

### Unit Tests
```
□ Each instruction - Test all supported addressing modes
  - Verify correct opcode encoding
  - Verify correct operand encoding (byte count, order)
  - Verify little-endian for multi-byte operands

□ Addressing mode variations - Test boundary cases
  - Accumulator mode: No operand
  - ZeroPage modes: 1-byte operand (value ≤ 0xFF)
  - Absolute modes: 2-byte operand (little-endian)
  - Indexed modes: Base address + index encoding
  - Indirect modes: Pointer encoding

□ Negative cases - Verify unsupported modes return empty vector
  - Test instruction with wrong addressing mode
  - Verify graceful handling
```

### Integration Tests
```
□ Assembler dispatch
  - Verify all mnemonics route to correct encoding methods
  - Test with sample assembly programs using all instructions

□ Multi-pass assembly
  - Verify forward references work with all addressing modes
  - Test zero-page optimization (if applicable)
```

### Acceptance Tests
```
□ Complete 6502 instruction set
  - Given: All 56 legal opcodes should be encodable
  - When: Attempt to encode each instruction with valid modes
  - Then: Correct bytecode generated for all

□ Real-world program assembly
  - Given: Sample 6502 program using diverse instruction set
  - When: Assemble with xasm++
  - Then: Binary matches expected output
```

### Coverage Target
```
- Overall: 85%+
- New code (cpu_6502.cpp additions): 95%+
- Critical paths: 100%
```

---

## Dependencies and Prerequisites

### Technical Dependencies
```
☑ GoogleTest - Unit testing framework (available)
☑ CMake 3.20+ - Build system (available)
☑ C++17 compiler - GCC 9+, Clang 10+, or MSVC 2019+ (available)
```

### Task Dependencies
```
☑ Phase 2.2 complete - 173 tests passing (done)
☑ 6502 opcode reference - http://www.6502.org/tutorials/6502opcodes.html (available)
```

### Knowledge Dependencies
```
☑ Understanding of 6502 addressing modes (available)
☑ Familiarity with TDD RED-GREEN-REFACTOR (established)
☑ Knowledge of C++ implementation patterns (Phase 2.2 reference)
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Incorrect opcode encoding for addressing mode combinations
- **Probability:** Medium
- **Impact:** High (wrong binaries generated)
- **Mitigation:** Use authoritative 6502 opcode reference, verify each against table
- **Contingency:** Reference MOS 6502 Programming Manual for confirmation

**Risk 2:** Test suite execution time exceeds 3 seconds with ~259 tests
- **Probability:** Low
- **Impact:** Medium (slower development cycle)
- **Mitigation:** Monitor test execution time after each group, optimize if needed
- **Contingency:** Group-level test execution, parallel test execution

**Risk 3:** Assembler dispatch chain becomes unwieldy (50+ if-else cases)
- **Probability:** High
- **Impact:** Low (works correctly, just verbose)
- **Mitigation:** Document as acceptable technical debt for Phase 2
- **Contingency:** Refactor to dispatch table in future phase (post-Phase 2)

### Integration Risks

**Risk 1:** Addressing mode enum extension breaks existing code
- **Mitigation:** Run full test suite after enum modification, fix any breakage immediately

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. Identify last successful commit (Group N-1)
2. git reset --hard <last-good-commit>
3. Verify all tests pass: cmake --build build && build/tests/unit/test_cpu6502
4. Push force if already pushed: git push --force origin main (with user approval)
```

**Rollback Verification:**
```
✓ All 173 baseline tests pass (or appropriate baseline for rollback point)
✓ Build succeeds with no warnings
✓ Git history clean
```

### Git Rollback
```
# If current group fails, revert to previous commit
git reset --hard HEAD~1

# Verify baseline
cmake --build build && build/tests/unit/test_cpu6502

# If needed, force push (with approval)
git push --force origin main
```

---

## Performance Considerations

### Expected Performance Impact
```
- Test suite execution: ~85 new tests adds ~0.5-0.8 seconds (estimate)
- Encoding performance: Negligible (switch statements are fast)
- Binary size: +~2-3 KB for new methods
```

### Performance Targets
```
- Test suite: < 3 seconds total
- Single test: < 10ms average
- Binary size: < 2 MB executable
```

### Monitoring Plan
```
□ Measure test execution time after each group commit
□ If exceeds 3 seconds, investigate slow tests
□ Ensure no O(n²) algorithms in encoding methods
```

---

## Security Considerations

### Security Checklist
```
☑ Input validation: Operand values validated by addressing mode
☑ Buffer overflows: std::vector used (safe)
☑ Integer overflows: 16-bit values, no overflow risk
☑ Injection attacks: Not applicable (native code, no external input)
☑ Memory safety: Smart pointers and RAII throughout
```

### Security Review Points
```
- Verify no undefined behavior in opcode encoding
- Ensure no out-of-bounds array access
- Confirm little-endian byte order correct
```

---

## Alternative Approaches Considered

### Alternative 1: Implement all at once (no groups)
**Pros:**
- Fewer commits
- Faster initial implementation

**Cons:**
- Harder to debug if tests fail
- No rollback points
- Violates lean flow principles

**Why Not Chosen:**
Risk of token limit failure, harder to track progress, no incremental verification

### Alternative 2: Use dispatch table instead of if-else chain
**Pros:**
- More elegant code structure
- O(1) lookup vs O(n) if-else chain

**Cons:**
- Significant refactoring required
- Risk of breaking existing functionality
- Out of scope for Phase 2.3

**Why Not Chosen:**
Acceptable to defer to post-Phase 2, current approach proven and working

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Group 1 (BIT): 30 min
Group 2 (ASL, LSR): 1 hour
Group 3 (ROL, ROR): 1 hour
Group 4 (RTI, BRK): 30 min
Group 5 (Flag ops): 45 min
Group 6 (Transfers): 45 min
Group 7 (Indexed modes): 2 hours
Group 8 (Indirect modes): 1.5 hours

Testing/Verification: 30 min
Documentation: 30 min

Total: ~8-9 hours (1 full development session)
```

---

## Success Metrics

### Completion Criteria
```
✓ All 8 groups implemented
✓ All ~259 tests passing
✓ Coverage ≥ 85%
✓ Test suite < 3 seconds
✓ All 56 legal 6502 opcodes verified
✓ Pushed to GitHub, CI passing
```

### Quality Metrics
```
- Test coverage: 85%+ overall, 95%+ new code
- Test execution: < 3 seconds
- Code quality: Zero compiler warnings (-Wall -Wextra -Wpedantic)
- Correctness: 100% opcode accuracy vs 6502 reference
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [x] Author: Engineer Role [2026-01-21]
- [x] Orchestrator: Approved to proceed [2026-01-21]

**Revision History:**
```
v1.0 - 2026-01-21 - Initial plan created
```

---

## Notes and Considerations

**Opcode Reference Sources:**
- Primary: http://www.6502.org/tutorials/6502opcodes.html
- Secondary: MCS6500 Microcomputer Family Programming Manual
- Validation: Compare against vasm 6502 output for sample programs

**Test Numbering:**
- Continue from Test 68 (Phase 2.2 ended at Test 67)
- Final test number will be ~Test 155-158 depending on exact count

**Commit Messages:**
Follow established pattern:
```
feat(cpu): implement Group N - [description]
```

**Post-Implementation:**
- Tester validation (/ai-pack test)
- Reviewer validation (/ai-pack review)
- Update work log throughout

---

**Plan Version:** 1.0
**Last Updated:** 2026-01-21
