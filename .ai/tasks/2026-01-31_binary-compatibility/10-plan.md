# Implementation Plan

**Task ID:** 2026-01-31_binary-compatibility
**Created:** 2026-01-31
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Fix three specific binary compatibility issues to achieve 100% byte-for-byte match with vasm for Prince of Persia binaries. Use Test-Driven Development (TDD) with dedicated test files comparing xasm++ output to known vasm reference binaries.

The approach isolates each issue into a separate phase with TDD cycle (RED-GREEN-REFACTOR):
1. **Equate-only files** - Detect files with no code/data output, emit 0 bytes
2. **ASC high-bit mode** - Add Apple II text mode (0x80-0xFF range)
3. **Padding removal** - Start output at ORG address, not $0000

Each fix maintains backward compatibility via command-line flags or Merlin syntax compatibility.

**Key Technical Decisions:**
- Use existing POP test files (tests/e2e/apple2/prince_of_persia/) with vasm reference binaries
- Add binary comparison tests for each issue
- Implement fixes in Merlin syntax handler (src/syntax/merlin/)
- Add command-line flags for Apple II compatibility modes if needed
- Preserve 729/729 existing test pass rate

**Patterns to Use:**
- TDD cycle - Write failing test, implement fix, verify all tests pass
- Feature flags - Optional modes for Apple II compatibility
- Binary comparison - hexdump -C or byte-by-byte comparison

---

## Critical Files Identified

### Files to Modify

```
1. src/syntax/merlin/merlin_syntax.cpp
   - Current: ASC directive outputs plain ASCII
   - Changes: Add high-bit mode (Apple II format), detect equate-only files

2. include/xasm++/syntax/merlin_syntax.h
   - Current: MerlinSyntax class declaration
   - Changes: Add state tracking for equate-only detection

3. src/cpu/m6502/cpu_6502.cpp (possibly)
   - Current: Code generation and binary output
   - Changes: Remove leading zero padding (start at ORG)

4. include/xasm++/cpu/cpu_6502.h (possibly)
   - Current: CPU6502 class declaration
   - Changes: Add methods for output range control

5. include/xasm++/cli/command_line_options.h
   - Current: Command-line flag definitions
   - Changes: Add --apple2-compat or similar flags if needed

6. src/cli_parser.cpp
   - Current: Command-line parsing
   - Changes: Parse new compatibility flags
```

### Files to Create

```
1. tests/unit/test_binary_compat.cpp (or similar)
   - Purpose: Binary compatibility regression tests
   - Rationale: Prevent future compatibility breaks

2. tests/e2e/apple2/prince_of_persia/reference/*.bin (if not exists)
   - Purpose: vasm reference binaries for comparison
   - Rationale: Ground truth for byte-for-byte comparison
```

### Files to Read (for context)

```
- tests/e2e/apple2/prince_of_persia/test_pop.py - Test harness structure
- src/main.cpp - Entry point and binary output logic
- tests/unit/test_merlin_syntax.cpp - Existing Merlin tests
- tests/unit/test_cpu6502.cpp - Existing CPU tests
```

---

## Step-by-Step Implementation Plan

### Phase 1: Equate-Only File Detection (0-byte output)

```
Step 1.1: Create failing test for equate-only files
  - Action: Add test case in tests/unit/ that assembles EQ.S, GAMEEQ.S, HRPARAMS.S
  - Files: tests/unit/test_binary_compat.cpp (new)
  - Tests: Verify xasm++ outputs 0 bytes (like vasm)
  - Verification: Test fails (currently outputs 7592, 3401, 2500 bytes)

Step 1.2: Implement equate-only detection
  - Action: Track whether any code/data bytes were emitted during assembly
  - Files: src/syntax/merlin/merlin_syntax.cpp, include/xasm++/syntax/merlin_syntax.h
  - Tests: Existing unit tests + new binary compat test
  - Verification: EQ.S, GAMEEQ.S, HRPARAMS.S produce 0-byte .bin files

Step 1.3: Verify all tests still pass
  - Action: Run full test suite (ctest)
  - Files: N/A
  - Tests: All 729 tests must pass
  - Verification: No regressions introduced
```

### Phase 2: ASC High-Bit Mode (Apple II text)

```
Step 2.1: Create failing test for ASC high-bit
  - Action: Add test comparing VERSION.S output to vasm reference
  - Files: tests/unit/test_binary_compat.cpp
  - Tests: Verify ASC "text" outputs 0xD0... not 0x50...
  - Verification: Test fails (currently outputs plain ASCII)

Step 2.2: Implement ASC high-bit mode
  - Action: Modify ASC directive handler to set bit 7 (0x80) for Merlin syntax
  - Files: src/syntax/merlin/merlin_syntax.cpp
  - Tests: New binary compat test + existing Merlin tests
  - Verification: ASC "Prince of Persia" outputs 0xD0F2E9EE... (high-bit ASCII)

Step 2.3: Verify backward compatibility
  - Action: Ensure non-Merlin syntaxes still use plain ASCII
  - Files: tests/unit/test_merlin_syntax.cpp
  - Tests: Add explicit test for Merlin ASC high-bit behavior
  - Verification: All 729 tests pass, VERSION.S matches vasm
```

### Phase 3: Remove Leading Zero Padding

```
Step 3.1: Create failing test for padding removal
  - Action: Add test comparing SEQTABLE.S output to vasm reference
  - Files: tests/unit/test_binary_compat.cpp
  - Tests: Verify output starts at ORG address, no leading zeros
  - Verification: Test fails (currently has leading zeros)

Step 3.2: Implement output range control
  - Action: Modify binary output to start at lowest ORG address, not $0000
  - Files: src/cpu/m6502/cpu_6502.cpp, include/xasm++/cpu/cpu_6502.h
  - Tests: New binary compat test + existing CPU tests
  - Verification: SEQTABLE.S matches vasm byte-for-byte

Step 3.3: Verify all POP binaries match
  - Action: Run binary comparison on all 10 POP files
  - Files: tests/e2e/apple2/prince_of_persia/
  - Tests: All 10 binaries match vasm exactly
  - Verification: 100% binary compatibility achieved (10/10 files)
```

---

## Testing Strategy

### Unit Tests

```
□ Binary compatibility test suite - Test cases:
  - Equate-only files (EQ.S, GAMEEQ.S, HRPARAMS.S) → 0 bytes
  - ASC high-bit (VERSION.S) → 0xD0... not 0x50...
  - No leading padding (SEQTABLE.S) → starts at ORG
  - Files with data (BGDATA.S, HRTABLES.S) → still match

□ Merlin syntax tests - Test cases:
  - ASC high-bit behavior explicit test
  - Backward compatibility for non-Merlin syntaxes
  - Edge cases (empty strings, special characters)

□ CPU 6502 tests - Test cases:
  - Output range control
  - Multiple ORG directives
  - Gaps in address space
```

### Integration Tests

```
□ Prince of Persia full build
  - Setup: tests/e2e/apple2/prince_of_persia/
  - Execute: Assemble all 10 files with xasm++ --syntax merlin --cpu 6502
  - Verify: All binaries match vasm byte-for-byte (md5sum or sha256sum)

□ POP Makefile integration
  - Setup: ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/
  - Execute: make clean && make ASM=xasm build-all
  - Verify: All binaries match vasm reference builds
```

### Acceptance Tests

```
□ Binary compatibility verification
  - Given: xasm++ and vasm both installed
  - When: Assemble all 10 POP files with both assemblers
  - Then: All 10 outputs match byte-for-byte (10/10 = 100%)

□ Test suite regression check
  - Given: All changes implemented
  - When: Run ctest
  - Then: 729/729 tests pass (100% pass rate maintained)
```

### Coverage Target

```
- Overall: 85%+ (existing target)
- Critical paths: 100% (binary output, ASC directive, equate detection)
- New code: 90%+ (binary compat logic)
```

---

## Dependencies and Prerequisites

### Technical Dependencies

```
□ vasm6502_merlin - Reference assembler for comparison
□ CMake 3.10+ - Build system
□ Catch2 - Unit testing framework (already integrated)
```

### Task Dependencies

```
□ None - This is foundational work
```

### Knowledge Dependencies

```
□ Understanding of Apple II text encoding (high-bit ASCII)
□ Familiarity with vasm behavior for equate-only files
□ Knowledge of Merlin assembler conventions
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Breaking existing tests with ASC high-bit change
- **Probability:** Medium
- **Impact:** High (blocks task completion)
- **Mitigation:** Make high-bit mode Merlin-specific, preserve other syntaxes
- **Contingency:** Add command-line flag (--apple2-text) for opt-in

**Risk 2:** Performance regression from equate detection
- **Probability:** Low
- **Impact:** Low (assembler performance not critical)
- **Mitigation:** Use simple flag check, no complex analysis
- **Contingency:** Profile and optimize if needed

**Risk 3:** Edge cases in padding removal
- **Probability:** Medium
- **Impact:** Medium (some files may have unexpected output)
- **Mitigation:** Test with all 10 POP files, add edge case tests
- **Contingency:** Revert to optional flag-based padding control

### Integration Risks

**Risk 1:** Merlin syntax edge cases not covered by tests
- **Mitigation:** Add comprehensive Merlin-specific test cases

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. git restore src/ include/ tests/ (revert all changes)
2. cmake --build build && ctest (verify 729/729 pass)
3. Document blocking issues in work log
```

**Rollback Verification:**
```
✓ All 729 tests pass (pre-change state)
✓ No binary compatibility improvements (2/10 files match)
✓ System stable for other work
```

### Git Rollback

```
# If changes committed
git revert <commit-hash>

# If not committed
git restore .

# Verify
cmake --build build && ctest
```

---

## Performance Considerations

### Expected Performance Impact

```
- Equate detection: Negligible (single flag check)
- ASC high-bit: None (single OR operation per character)
- Padding removal: Negligible (address range calculation)
```

### Performance Targets

```
- Assemble time: <100ms per POP file (current baseline)
- Full POP build: <3 seconds (10 files)
```

### Monitoring Plan

```
□ Measure with `time` command before/after changes
□ Run existing benchmark tests if available
□ Compare xasm++ vs vasm assembly speed
```

---

## Security Considerations

### Security Checklist

```
✓ Input validation implemented (existing assembler validation)
✓ No authentication/authorization needed (local tool)
✓ No sensitive data (assembling public code)
✓ No network operations
✓ File I/O properly validated (existing code)
```

### Security Review Points

```
- Ensure binary output doesn't have buffer overflows
- Validate address range calculations don't wrap
```

---

## Alternative Approaches Considered

### Alternative 1: Command-line flags for all behaviors

**Pros:**
- Maximum backward compatibility
- User control over output format

**Cons:**
- Requires users to know about flags
- Breaks "vasm compatibility by default" for Merlin syntax
- More complex user experience

**Why Not Chosen:**
Merlin syntax should match vasm behavior by default. Flags add complexity without benefit for primary use case (POP disk imaging).

### Alternative 2: Separate "apple2" CPU variant

**Pros:**
- Clear separation of Apple II-specific behavior
- Easy to extend with other platform differences

**Cons:**
- 6502 is 6502 - CPU doesn't change
- Conflates CPU architecture with text encoding conventions
- Adds maintenance burden

**Why Not Chosen:**
Text encoding is a syntax concern (Merlin), not CPU architecture. Keep CPU plugins focused on instruction sets.

### Alternative 3: Post-processing script for binary conversion

**Pros:**
- No changes to xasm++ core
- Easy to prototype

**Cons:**
- Extra build step for users
- Doesn't fix root cause (assembler compatibility)
- Complicated build system integration

**Why Not Chosen:**
The assembler should produce correct output directly. Post-processing is a workaround, not a solution.

---

## Success Metrics

### Completion Criteria

```
✓ All 3 phases completed (equate, ASC, padding)
✓ All 10 POP binaries match vasm byte-for-byte (100%)
✓ All 729 existing tests still pass
✓ Binary compatibility tests added (prevent regression)
✓ Work log updated with final verification
```

### Quality Metrics

```
- Test coverage: 85%+ overall, 95%+ for new code
- Binary compatibility: 10/10 files (100%)
- Test pass rate: 729/729 (100%)
- No linting errors
```

---

## Plan Approval

**Plan Status:** Approved (proceeding to implementation)

**Reviewed By:**
- [x] Author: Orchestrator (2026-01-31)
- [ ] User/Stakeholder: (pending)

**Revision History:**
```
v1.0 - 2026-01-31 - Initial plan
```

---

## Notes and Considerations

- POP disk imaging is blocked until this task completes (P2 priority)
- Binary compatibility is non-negotiable: "File sizes and contents must match" (user requirement)
- Previous issues occurred due to "mapping assumptions" - must be exact, not approximate
- BGDATA.bin and HRTABLES.bin already match (2/10) - preserve these while fixing others
- Merlin syntax should default to vasm-compatible behavior without flags

---

**Plan Version:** 1.0
**Last Updated:** 2026-01-31
