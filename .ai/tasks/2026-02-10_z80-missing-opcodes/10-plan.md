# Implementation Plan: Z80 Missing Opcodes

**Task ID:** 2026-02-10_z80-missing-opcodes
**Beads Task:** xasm++-18tk
**Created:** 2026-02-10
**Author:** Orchestrator Agent
**Plan Version:** 1.0

---

## Approach Summary

This task implements missing Z80 opcodes identified in the verification report (docs/architecture/verification-z80-opcodes.md). The current implementation contains ~243 opcodes (35% coverage), all verified 100% correct. We will add high-value missing opcodes to increase coverage while maintaining the existing quality standard.

The implementation follows TDD methodology (RED-GREEN-REFACTOR) and will be executed as a single small batch since only 3 files are involved:
1. `include/xasm++/cpu/opcodes_z80.h` - Add opcode constants
2. `src/cpu/z80/cpu_z80.cpp` - Add encoder methods
3. `tests/unit/test_cpu_z80.cpp` - Add comprehensive tests

**Key Technical Decisions:**
- **Single small batch approach** - 3 files (well under 14 file limit) allows single-phase implementation
- **Prioritize commonly-used opcodes** - Focus on LD variants, RLD/RRD, and I/O instructions per verification report recommendations
- **Follow existing patterns** - Match current code organization (namespace structure, naming conventions, comment style)
- **TDD mandatory** - Write tests first (RED), implement encoder methods (GREEN), refactor as needed

**Patterns to Use:**
- **Namespace organization** - Add to existing `Z80Opcodes` namespace and `ED` sub-namespace
- **Encoder method naming** - `Encode<INSTRUCTION>_<OPERANDS>()` pattern (e.g., `EncodeLD_A_BC()`)
- **Test structure** - Follow existing phase-based test organization
- **Little-endian encoding** - 16-bit values stored low byte first (Z80 convention)

---

## Critical Files Identified

### Files to Modify

```
1. include/xasm++/cpu/opcodes_z80.h
   - Current: Contains ~243 opcode constants (35% coverage)
   - Changes: Add ~30-40 missing opcode constants:
     * LD variants: A,(BC/DE), (BC/DE),A, (HL),n, HL,(nn), (nn),HL, SP,HL
     * ADD HL variants: BC, DE, HL, SP
     * Exchange instructions: DE,HL, AF,AF', EXX, (SP),HL
     * RLD/RRD in ED namespace
     * IN/OUT register variants in ED namespace
     * LD 16-bit memory variants in ED namespace
     * DJNZ instruction

2. src/cpu/z80/cpu_z80.cpp
   - Current: Contains ~50 encoder methods
   - Changes: Add corresponding encoder methods for new opcodes
     * EncodeLD_A_BC(), EncodeLD_A_DE()
     * EncodeLD_BC_A(), EncodeLD_DE_A()
     * EncodeLD_HL_n()
     * EncodeLD_HL_addr(), EncodeLD_addr_HL()
     * EncodeLD_SP_HL()
     * EncodeADD_HL_BC/DE/HL/SP()
     * EncodeEX_DE_HL(), EncodeEX_AF_AF(), EncodeEXX(), EncodeEX_SP_HL()
     * EncodeDJNZ()
     * EncodeRLD(), EncodeRRD()
     * EncodeIN_r_C(), EncodeOUT_C_r() (for all registers)
     * EncodeLD_BC_addr(), etc. (16-bit memory loads/stores)

3. tests/unit/test_cpu_z80.cpp
   - Current: Contains ~70 test cases across 19 phases
   - Changes: Add new Phase 20-23 with ~30-40 test cases:
     * Phase 20: Missing LD variants (10 tests)
     * Phase 21: 16-bit arithmetic and exchanges (8 tests)
     * Phase 22: RLD/RRD digit rotate (2 tests)
     * Phase 23: I/O instructions (16 tests)
```

### Files to Create

None - all changes are additions to existing files.

### Files to Read (for context)

```
- docs/architecture/verification-z80-opcodes.md - Gap analysis with specific missing opcodes
- include/xasm++/cpu/cpu_z80.h - Interface definition (for method signatures)
- Existing test patterns in test_cpu_z80.cpp - For test structure consistency
```

---

## Step-by-Step Implementation Plan

### Phase 1: Add Missing LD Variants (TDD)

**Estimated:** 90 minutes

```
Step 1.1: Write failing tests for LD A,(BC) and LD A,(DE)
  - Action: Add Phase 20 section to test_cpu_z80.cpp
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests: 
    * TEST_F(CpuZ80Test, LD_A_BC) - expect 0x0A
    * TEST_F(CpuZ80Test, LD_A_DE) - expect 0x1A
  - Verification: Tests compile and fail (RED)

Step 1.2: Add opcode constants for LD A,(BC/DE)
  - Action: Add to Z80Opcodes namespace in opcodes_z80.h
  - Files: include/xasm++/cpu/opcodes_z80.h
  - Constants:
    constexpr uint8_t LD_A_BC = 0x0A; // LD A, (BC)
    constexpr uint8_t LD_A_DE = 0x1A; // LD A, (DE)
  - Verification: Code compiles

Step 1.3: Implement encoder methods for LD A,(BC/DE)
  - Action: Add methods to cpu_z80.cpp
  - Files: src/cpu/z80/cpu_z80.cpp
  - Methods:
    std::vector<uint8_t> CpuZ80::EncodeLD_A_BC() const { return {Opcodes::LD_A_BC}; }
    std::vector<uint8_t> CpuZ80::EncodeLD_A_DE() const { return {Opcodes::LD_A_DE}; }
  - Verification: Tests pass (GREEN)

Step 1.4: Write failing tests for LD (BC),A and LD (DE),A
  - Action: Add to Phase 20
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests:
    * TEST_F(CpuZ80Test, LD_BC_A) - expect 0x02
    * TEST_F(CpuZ80Test, LD_DE_A) - expect 0x12
  - Verification: Tests fail (RED)

Step 1.5: Add opcode constants and implement encoders
  - Action: Add constants and methods following steps 1.2-1.3 pattern
  - Files: opcodes_z80.h, cpu_z80.cpp
  - Constants: LD_BC_A = 0x02, LD_DE_A = 0x12
  - Verification: Tests pass (GREEN)

Step 1.6: Write failing tests for LD (HL),n
  - Action: Add to Phase 20
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests:
    * TEST_F(CpuZ80Test, LD_HL_n) - expect 0x36 + immediate byte
  - Verification: Test fails (RED)

Step 1.7: Implement LD (HL),n with immediate operand
  - Action: Add constant (0x36) and encoder method with uint8_t parameter
  - Files: opcodes_z80.h, cpu_z80.cpp
  - Method: EncodeLD_HL_n(uint8_t value) - returns {0x36, value}
  - Verification: Test passes (GREEN)

Step 1.8: Write failing tests for LD HL,(nn) and LD (nn),HL
  - Action: Add to Phase 20
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests:
    * TEST_F(CpuZ80Test, LD_HL_addr) - expect 0x2A + addr (little-endian)
    * TEST_F(CpuZ80Test, LD_addr_HL) - expect 0x22 + addr (little-endian)
  - Verification: Tests fail (RED)

Step 1.9: Implement LD HL,(nn) and LD (nn),HL
  - Action: Add constants and encoder methods with uint16_t parameter
  - Files: opcodes_z80.h, cpu_z80.cpp
  - Constants: LD_HL_addr = 0x2A, LD_addr_HL = 0x22
  - Methods: Return {opcode, low_byte, high_byte}
  - Verification: Tests pass (GREEN)

Step 1.10: Write failing test and implement LD SP,HL
  - Action: Add test, constant (0xF9), and encoder method
  - Files: All three files
  - Test: Expect single byte 0xF9
  - Verification: Test passes (GREEN)
```

### Phase 2: Add 16-bit Arithmetic and Exchange Instructions (TDD)

**Estimated:** 60 minutes

```
Step 2.1: Write failing tests for ADD HL,rr variants
  - Action: Add Phase 21 section to test_cpu_z80.cpp
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests:
    * TEST_F(CpuZ80Test, ADD_HL_BC) - expect 0x09
    * TEST_F(CpuZ80Test, ADD_HL_DE) - expect 0x19
    * TEST_F(CpuZ80Test, ADD_HL_HL) - expect 0x29
    * TEST_F(CpuZ80Test, ADD_HL_SP) - expect 0x39
  - Verification: Tests fail (RED)

Step 2.2: Implement ADD HL,rr instructions
  - Action: Add 4 constants and 4 encoder methods
  - Files: opcodes_z80.h, cpu_z80.cpp
  - Constants: ADD_HL_BC = 0x09, ADD_HL_DE = 0x19, ADD_HL_HL = 0x29, ADD_HL_SP = 0x39
  - Methods: Each returns single-byte vector
  - Verification: Tests pass (GREEN)

Step 2.3: Write failing tests for exchange instructions
  - Action: Add to Phase 21
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests:
    * TEST_F(CpuZ80Test, EX_DE_HL) - expect 0xEB
    * TEST_F(CpuZ80Test, EX_AF_AF) - expect 0x08
    * TEST_F(CpuZ80Test, EXX) - expect 0xD9
    * TEST_F(CpuZ80Test, EX_SP_HL) - expect 0xE3
  - Verification: Tests fail (RED)

Step 2.4: Implement exchange instructions
  - Action: Add 4 constants and 4 encoder methods
  - Files: opcodes_z80.h, cpu_z80.cpp
  - Constants: EX_DE_HL = 0xEB, EX_AF_AF = 0x08, EXX = 0xD9, EX_SP_HL = 0xE3
  - Verification: Tests pass (GREEN)
```

### Phase 3: Add RLD/RRD Digit Rotate (TDD)

**Estimated:** 30 minutes

```
Step 3.1: Write failing tests for RLD and RRD
  - Action: Add Phase 22 section to test_cpu_z80.cpp
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests:
    * TEST_F(CpuZ80Test, RLD) - expect 0xED 0x6F
    * TEST_F(CpuZ80Test, RRD) - expect 0xED 0x67
  - Verification: Tests fail (RED)

Step 3.2: Add RLD/RRD to ED namespace
  - Action: Add to namespace ED in opcodes_z80.h
  - Files: include/xasm++/cpu/opcodes_z80.h
  - Constants:
    namespace ED {
      constexpr uint8_t RLD = 0x6F; // ED 6F - Rotate Left Digit
      constexpr uint8_t RRD = 0x67; // ED 67 - Rotate Right Digit
    }
  - Verification: Code compiles

Step 3.3: Implement RLD/RRD encoder methods
  - Action: Add methods to cpu_z80.cpp
  - Files: src/cpu/z80/cpu_z80.cpp
  - Methods:
    std::vector<uint8_t> CpuZ80::EncodeRLD() const { 
      return {Opcodes::ED_PREFIX, Opcodes::ED::RLD}; 
    }
    std::vector<uint8_t> CpuZ80::EncodeRRD() const { 
      return {Opcodes::ED_PREFIX, Opcodes::ED::RRD}; 
    }
  - Verification: Tests pass (GREEN)
```

### Phase 4: Add I/O Instructions (TDD)

**Estimated:** 90 minutes

```
Step 4.1: Write failing tests for IN r,(C) instructions
  - Action: Add Phase 23 section to test_cpu_z80.cpp
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests: 8 tests for IN B/C/D/E/H/L/A,(C) - expect ED + specific opcode
    * TEST_F(CpuZ80Test, IN_B_C) - expect 0xED 0x40
    * TEST_F(CpuZ80Test, IN_C_C) - expect 0xED 0x48
    * ... (6 more for D/E/H/L/A)
  - Verification: Tests fail (RED)

Step 4.2: Add IN r,(C) constants to ED namespace
  - Action: Add 8 constants to namespace ED
  - Files: include/xasm++/cpu/opcodes_z80.h
  - Constants: IN_B_C = 0x40, IN_C_C = 0x48, IN_D_C = 0x50, IN_E_C = 0x58,
               IN_H_C = 0x60, IN_L_C = 0x68, IN_A_C = 0x78
  - Note: IN_F_C (0x70) exists but flag register, may skip
  - Verification: Code compiles

Step 4.3: Implement IN r,(C) encoder methods
  - Action: Add 7-8 methods to cpu_z80.cpp
  - Files: src/cpu/z80/cpu_z80.cpp
  - Methods: Each returns {Opcodes::ED_PREFIX, Opcodes::ED::IN_X_C}
  - Verification: Tests pass (GREEN)

Step 4.4: Write failing tests for OUT (C),r instructions
  - Action: Add to Phase 23
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests: 8 tests for OUT (C),B/C/D/E/H/L/A - expect ED + specific opcode
    * TEST_F(CpuZ80Test, OUT_C_B) - expect 0xED 0x41
    * TEST_F(CpuZ80Test, OUT_C_C) - expect 0xED 0x49
    * ... (6 more)
  - Verification: Tests fail (RED)

Step 4.5: Add OUT (C),r constants to ED namespace
  - Action: Add 8 constants
  - Files: include/xasm++/cpu/opcodes_z80.h
  - Constants: OUT_C_B = 0x41, OUT_C_C = 0x49, OUT_C_D = 0x51, OUT_C_E = 0x59,
               OUT_C_H = 0x61, OUT_C_L = 0x69, OUT_C_A = 0x79
  - Verification: Code compiles

Step 4.6: Implement OUT (C),r encoder methods
  - Action: Add 7-8 methods
  - Files: src/cpu/z80/cpp_z80.cpp
  - Methods: Each returns {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_X}
  - Verification: Tests pass (GREEN)
```

### Phase 5: Add 16-bit Memory Operations (TDD)

**Estimated:** 60 minutes

```
Step 5.1: Write failing tests for LD rr,(nn) instructions
  - Action: Add to existing Phase 16 (ED prefix section)
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests:
    * TEST_F(CpuZ80Test, LD_BC_addr) - expect 0xED 0x4B + addr
    * TEST_F(CpuZ80Test, LD_DE_addr) - expect 0xED 0x5B + addr
    * TEST_F(CpuZ80Test, LD_HL_addr_ED) - expect 0xED 0x6B + addr (note: different from unprefixed 0x2A)
    * TEST_F(CpuZ80Test, LD_SP_addr) - expect 0xED 0x7B + addr
  - Verification: Tests fail (RED)

Step 5.2: Add LD rr,(nn) constants to ED namespace
  - Action: Add 4 constants
  - Files: include/xasm++/cpu/opcodes_z80.h
  - Constants: LD_BC_addr = 0x4B, LD_DE_addr = 0x5B, LD_HL_addr = 0x6B, LD_SP_addr = 0x7B
  - Verification: Code compiles

Step 5.3: Implement LD rr,(nn) encoder methods
  - Action: Add 4 methods with uint16_t parameter
  - Files: src/cpu/z80/cpu_z80.cpp
  - Methods: Return {ED_PREFIX, opcode, low_byte, high_byte}
  - Verification: Tests pass (GREEN)

Step 5.4: Write failing tests for LD (nn),rr instructions
  - Action: Add to Phase 16
  - Files: tests/unit/test_cpu_z80.cpp
  - Tests:
    * TEST_F(CpuZ80Test, LD_addr_BC) - expect 0xED 0x43 + addr
    * TEST_F(CpuZ80Test, LD_addr_DE) - expect 0xED 0x53 + addr
    * TEST_F(CpuZ80Test, LD_addr_HL_ED) - expect 0xED 0x63 + addr (different from 0x22)
    * TEST_F(CpuZ80Test, LD_addr_SP) - expect 0xED 0x73 + addr
  - Verification: Tests fail (RED)

Step 5.5: Implement LD (nn),rr instructions
  - Action: Add 4 constants and 4 encoder methods
  - Files: opcodes_z80.h, cpu_z80.cpp
  - Constants: LD_addr_BC = 0x43, LD_addr_DE = 0x53, LD_addr_HL = 0x63, LD_addr_SP = 0x73
  - Verification: Tests pass (GREEN)
```

### Phase 6: Add DJNZ Instruction (TDD)

**Estimated:** 15 minutes

```
Step 6.1: Write failing test for DJNZ
  - Action: Add to Phase 13 (branch instructions)
  - Files: tests/unit/test_cpu_z80.cpp
  - Test: TEST_F(CpuZ80Test, DJNZ_e) - expect 0x10 + signed displacement
  - Verification: Test fails (RED)

Step 6.2: Implement DJNZ instruction
  - Action: Add constant and encoder method
  - Files: opcodes_z80.h, cpu_z80.cpp
  - Constant: DJNZ = 0x10
  - Method: EncodeDJNZ(int8_t displacement) - returns {0x10, static_cast<uint8_t>(displacement)}
  - Verification: Test passes (GREEN)
```

### Phase 7: Final Verification and Documentation

**Estimated:** 30 minutes

```
Step 7.1: Run full test suite
  - Action: Build and run all tests
  - Command: cmake --build build && ctest --test-dir build
  - Verification: All tests pass, no regressions

Step 7.2: Verify test coverage
  - Action: Run coverage report
  - Target: 97%+ coverage for new code
  - Verification: Coverage meets target

Step 7.3: Update verification document
  - Action: Update docs/architecture/verification-z80-opcodes.md
  - Files: docs/architecture/verification-z80-opcodes.md
  - Changes: 
    * Update coverage statistics (from ~35% to ~45-50%)
    * Mark implemented opcodes as "✅ Added"
    * Update recommendations section
  - Verification: Documentation accurate

Step 7.4: Update work log
  - Action: Document completion in 20-work-log.md
  - Files: .ai/tasks/2026-02-10_z80-missing-opcodes/20-work-log.md
  - Entry: Final stats, files changed, tests added
  - Verification: Complete audit trail
```

---

## Testing Strategy

### Unit Tests (TDD Approach)

**Priority: MANDATORY - TDD is non-negotiable per Global Gate 2**

Each opcode requires:
1. **RED Phase:** Write failing test first
   - Test opcode byte sequence
   - Test operand encoding (immediate, displacement, address)
   - Test little-endian byte order for 16-bit values

2. **GREEN Phase:** Minimal implementation to pass
   - Add opcode constant to header
   - Implement encoder method in cpp file
   - Return correct byte sequence

3. **REFACTOR Phase:** Clean up if needed
   - Extract common patterns
   - Add helper methods if duplicate code emerges
   - Improve naming/comments

**Test Coverage Target:**
```
□ Missing LD variants (10 tests):
  - LD A,(BC) / LD A,(DE) - indirect loads
  - LD (BC),A / LD (DE),A - indirect stores
  - LD (HL),n - immediate to memory
  - LD HL,(nn) / LD (nn),HL - 16-bit memory access
  - LD SP,HL - stack pointer load

□ 16-bit arithmetic (4 tests):
  - ADD HL,BC / ADD HL,DE / ADD HL,HL / ADD HL,SP

□ Exchange instructions (4 tests):
  - EX DE,HL / EX AF,AF' / EXX / EX (SP),HL

□ Digit rotate (2 tests):
  - RLD / RRD (ED prefixed)

□ I/O instructions (16 tests):
  - IN r,(C) for 7-8 registers
  - OUT (C),r for 7-8 registers

□ 16-bit ED memory ops (8 tests):
  - LD rr,(nn) for BC/DE/HL/SP
  - LD (nn),rr for BC/DE/HL/SP

□ Branch instruction (1 test):
  - DJNZ e

Total new tests: ~45
Total test cases after: ~115
```

### Integration Tests

**Not Required** - Unit tests at instruction encoding level provide sufficient coverage. Integration happens naturally through assembler usage.

### Acceptance Tests

```
□ Scenario 1: Assemble Z80 program with missing opcodes
  - Given: Z80 source file using LD A,(BC), RLD, IN A,(C)
  - When: Assemble with xasm++
  - Then: Correct opcodes generated, no errors

□ Scenario 2: Verify opcode documentation
  - Given: Updated verification report
  - When: Review coverage statistics
  - Then: Coverage increased from 35% to 45-50%, all added opcodes documented

□ Scenario 3: Compatibility check
  - Given: Existing Z80 test programs
  - When: Re-assemble with new opcode support
  - Then: No regressions, same output as before
```

### Coverage Target

```
- Overall test coverage: 97%+ (maintain existing high standard)
- New encoder methods: 100% (each method has dedicated test)
- New opcode constants: 100% (each constant used in test)
- Critical paths: 100% (all opcode encoding paths tested)
```

---

## Dependencies and Prerequisites

### Technical Dependencies

```
✓ C++17 compiler (existing)
✓ CMake build system (existing)
✓ GoogleTest framework (existing)
✓ Existing Z80 encoder infrastructure (existing)
```

All dependencies already satisfied - this is purely additive work to existing codebase.

### Task Dependencies

```
✓ No blocking dependencies
✓ Can proceed immediately
```

### Knowledge Dependencies

```
✓ Z80 instruction set architecture - documented in verification report
✓ Zilog Z80 CPU User Manual - referenced in verification report
✓ TDD methodology - team standard
✓ Existing codebase patterns - observable in current implementation
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Opcode value errors (typos in hex constants)
- **Probability:** Low (verification report provides exact values)
- **Impact:** High (incorrect assembly output)
- **Mitigation:** 
  - Copy opcode values directly from verification report
  - Each opcode has test that validates byte sequence
  - Cross-reference with Zilog Z80 manual
- **Contingency:** Tests catch errors immediately, fix before merge

**Risk 2:** Little-endian encoding errors for 16-bit values
- **Probability:** Low (existing code uses pattern correctly)
- **Impact:** Medium (incorrect address encoding)
- **Mitigation:**
  - Follow existing EncodeLD_BC_nn pattern exactly
  - Test cases verify low byte / high byte order
  - Code review checks byte ordering
- **Contingency:** Tests catch errors, trivial to fix (swap bytes)

**Risk 3:** Method signature inconsistencies
- **Probability:** Low (following existing patterns)
- **Impact:** Low (compilation errors, easy to spot)
- **Mitigation:**
  - Match existing method signature style
  - Use const return, const methods
  - Compiler catches signature mismatches
- **Contingency:** Compilation errors guide correction

### Integration Risks

**Risk 1:** Breaking existing tests
- **Probability:** Very Low (purely additive changes)
- **Impact:** High (regression in existing functionality)
- **Mitigation:**
  - Run full test suite after each phase
  - No modifications to existing encoder methods
  - Only adding new methods and constants
- **Contingency:** Git revert to known good state

**Risk 2:** Header include order issues
- **Probability:** Very Low (adding to existing headers)
- **Impact:** Low (compilation errors)
- **Mitigation:**
  - No changes to include structure
  - Adding to existing namespace, not new headers
- **Contingency:** Compiler errors show exact issue

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. Identify failing phase (tests will show which opcodes broken)
2. Git revert commits from that phase backward
3. Verify existing tests still pass
4. Document failure reason in work log
5. Revise plan if needed
```

**Rollback Verification:**
```
✓ All original tests pass (97%+ coverage maintained)
✓ No compilation errors
✓ Build succeeds
✓ No warnings introduced
```

### Git Rollback

```bash
# If specific phase fails, revert those commits
git log --oneline | grep "Phase X"
git revert <commit-hash>

# If entire implementation problematic
git revert HEAD~N  # N = number of commits since start

# Verify rollback successful
cmake --build build --clean-first
ctest --test-dir build
# All tests should pass
```

### Partial Rollback Strategy

Since phases are independent, can keep successful phases and rollback only failing phase:
```
Phase 1 ✓ → Keep
Phase 2 ✓ → Keep  
Phase 3 ✗ → Rollback
Phase 4 → Skip (depends on 3)

Result: Partial implementation with LD variants and 16-bit arithmetic working
```

---

## Performance Considerations

### Expected Performance Impact

```
- Compilation time: Negligible (+3-5 seconds, adding ~100 lines)
- Test execution: +2-3 seconds for ~45 new test cases
- Runtime encoding: No impact (all compile-time constants)
- Memory footprint: Negligible (+~200 bytes for new method code)
```

**Rationale:** This work adds:
- ~40 compile-time constants (zero runtime overhead)
- ~40 simple encoder methods (inline candidates, minimal code)
- ~45 unit tests (test-time only, not in production)

### Performance Targets

```
- Build time increase: < 5 seconds
- Test suite time increase: < 3 seconds
- No measurable runtime impact (instruction encoding is already fast)
```

### Monitoring Plan

```
□ Compare build times before/after (should be negligible)
□ Compare test execution times (ctest reports timing)
□ Verify no performance regressions in existing tests
```

**Note:** Performance is not a concern for this work. Adding opcode constants and simple encoder methods has no meaningful performance impact.

---

## Security Considerations

### Security Checklist

```
✓ Input validation: Not applicable (compile-time constants)
✓ Output sanitization: Not applicable (binary opcode generation)
✓ Authentication: Not applicable (no user interaction)
✓ Authorization: Not applicable (compiler operation)
✓ Sensitive data: None involved
✓ Secrets: None involved
✓ SQL injection: Not applicable (no database)
✓ XSS: Not applicable (no web interface)
✓ CSRF: Not applicable (no web interface)
```

### Security Review Points

**None required** - This is pure compiler/assembler implementation work with no security implications. The changes:
- Add compile-time constants (immutable)
- Add pure functions that generate byte sequences
- No user input handling
- No network communication
- No file system access beyond normal compiler operations
- No cryptography or security-sensitive operations

---

## Alternative Approaches Considered

### Alternative 1: Automated Code Generation

**Pros:**
- Could generate all 252+ Z80 opcodes systematically
- Ensures pattern consistency
- Less manual typing
- Could generate from official opcode tables

**Cons:**
- Overkill for ~40 missing opcodes
- Generated code may be harder to review
- Adds build system complexity
- Current manual approach working well
- Generated code less readable (verbose names)

**Why Not Chosen:**
Current manual approach has already proven successful for 243 opcodes with 100% correctness. Adding 40 more manually is straightforward and maintains code style consistency. Generation would take longer to set up than manual implementation.

### Alternative 2: Complete All 252 Opcodes at Once

**Pros:**
- Achieves 100% coverage immediately
- No future work needed
- Complete documentation

**Cons:**
- Violates small batch principle (would be ~150+ opcodes)
- Many opcodes rarely used in real Z80 code
- Much larger testing burden (150+ new tests)
- Higher risk of errors due to volume
- Longer feedback cycle

**Why Not Chosen:**
Verification report explicitly recommends focusing on commonly-used opcodes. Complete coverage not needed for practical Z80 assembly. YAGNI principle applies - implement what's needed, add more later if required.

### Alternative 3: Parser-Level Implementation

**Pros:**
- Could auto-detect instruction variants from operand parsing
- More flexible instruction syntax
- Less code (pattern matching vs explicit encoding)

**Cons:**
- Much larger scope (parser changes risky)
- Breaks existing architecture (encoder methods expected)
- Harder to test (parser integration complex)
- Would affect multiple subsystems
- Violates single responsibility (mixing parsing and encoding)

**Why Not Chosen:**
Architecture clearly separates parsing from encoding. Encoder methods provide clear API tested independently. Parser-level changes much riskier and out of scope for this task.

---

## Timeline Estimate

**Note:** Estimates for planning only, not deadlines. TDD may reveal unexpected complexity requiring adjustment.

```
Phase 1: Missing LD Variants
- 10 test cases (RED): 30 minutes
- 10 constants + methods (GREEN): 45 minutes
- Refactor: 15 minutes
- Subtotal: 90 minutes

Phase 2: 16-bit Arithmetic and Exchanges
- 8 test cases (RED): 20 minutes
- 8 constants + methods (GREEN): 30 minutes
- Refactor: 10 minutes
- Subtotal: 60 minutes

Phase 3: RLD/RRD Digit Rotate
- 2 test cases (RED): 10 minutes
- 2 constants + methods (GREEN): 15 minutes
- Refactor: 5 minutes
- Subtotal: 30 minutes

Phase 4: I/O Instructions
- 16 test cases (RED): 30 minutes
- 16 constants + methods (GREEN): 45 minutes
- Refactor: 15 minutes
- Subtotal: 90 minutes

Phase 5: 16-bit Memory Operations
- 8 test cases (RED): 20 minutes
- 8 constants + methods (GREEN): 30 minutes
- Refactor: 10 minutes
- Subtotal: 60 minutes

Phase 6: DJNZ Instruction
- 1 test case (RED): 5 minutes
- 1 constant + method (GREEN): 5 minutes
- Refactor: 5 minutes
- Subtotal: 15 minutes

Phase 7: Final Verification
- Full test suite: 10 minutes
- Coverage report: 5 minutes
- Documentation update: 10 minutes
- Work log: 5 minutes
- Subtotal: 30 minutes

Buffer for unexpected issues: 30 minutes

Total: ~405 minutes = 6.75 hours
Rounded: 7 hours (conservative estimate)
```

**Confidence Level:** High - work is well-defined, patterns established, risks low.

**Contract Estimate:** 4-6 hours (on track with detailed breakdown)

---

## Success Metrics

### Completion Criteria

```
✓ All 6 implementation phases completed
✓ All ~45 new tests passing
✓ All existing ~70 tests still passing
✓ Coverage targets met (97%+)
✓ No compilation warnings
✓ Documentation updated
✓ Work log complete
```

### Quality Metrics

```
- Test coverage: 97%+ overall (maintain existing standard)
- New code coverage: 100% (every new method tested)
- Compilation warnings: 0
- Failed tests: 0
- TDD compliance: 100% (RED-GREEN-REFACTOR for all opcodes)
- Code review: Pass (follows existing patterns)
```

### Quantitative Success Metrics

```
- Opcodes added: ~40 (from ~243 to ~283)
- Coverage increase: +10-15% (from ~35% to ~45-50%)
- Test cases added: ~45
- Files modified: 3 (as planned)
- Files created: 0 (as planned)
- Breaking changes: 0 (purely additive)
```

---

## Plan Approval

**Plan Status:** ✅ Approved (Ready for Implementation)

**Reviewed By:**
- [x] Author: Orchestrator Agent (2026-02-10)
- [ ] User/Stakeholder: Awaiting review

**Revision History:**
```
v1.0 - 2026-02-10 - Initial detailed plan
```

---

## Notes and Considerations

### Implementation Order Rationale

Phases ordered by:
1. **Dependency:** LD variants first (most fundamental)
2. **Complexity:** Simple single-byte opcodes before multi-byte
3. **Grouping:** Related instructions together (all I/O in one phase)
4. **Testing:** Each phase independently testable

### Pattern Consistency Critical

All new code MUST match existing patterns exactly:
- Naming: `Encode<INSTRUCTION>_<OPERANDS>()` format
- Comments: `// INSTRUCTION operands - Description` format
- Namespace: ED-prefixed opcodes in `namespace ED`
- Return type: `std::vector<uint8_t>`
- Method qualifier: `const` (encoder methods don't modify state)
- Test format: `TEST_F(CpuZ80Test, <InstructionName>)`

### TDD Non-Negotiable

Per Global Gate 2, TDD is MANDATORY:
- Write test FIRST (RED phase)
- Implement minimally to pass (GREEN phase)
- Refactor if needed (REFACTOR phase)
- No implementation before corresponding test exists

Reviewer will BLOCK approval if TDD not followed.

### Verification Report as Ground Truth

The file `docs/architecture/verification-z80-opcodes.md` contains exact opcode values verified against official Zilog documentation. Use this as the authoritative source - do not guess opcode values or consult unreliable sources.

### Small Batch Confirmation

This implementation is a single small batch:
- **Files:** 3 (well under 14 limit)
- **Estimated effort:** 7 hours (reasonable for single agent)
- **Token estimate:** ~15K output (well under 25K limit)
- **Risk:** Low (patterns established, dependencies clear)

✅ **SAFE FOR SINGLE AGENT EXECUTION**

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-10
**Status:** Ready for Implementation

---

## Handoff to Engineer

This plan is complete and ready for implementation by an Engineer agent following TDD methodology. The plan provides:

✓ Clear phase-by-phase breakdown
✓ Specific opcode values from verification report
✓ Test-first approach for each opcode
✓ File-level specificity
✓ Risk mitigation strategies
✓ Rollback procedures
✓ Success criteria

**Next Step:** Orchestrator delegates to Engineer agent with this plan.
