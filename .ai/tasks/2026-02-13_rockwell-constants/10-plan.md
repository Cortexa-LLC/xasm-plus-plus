# Implementation Plan

**Task ID:** 2026-02-13_rockwell-constants
**Created:** 2026-02-13
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

This is a pure refactoring task that replaces 64 magic values (32 strings + 32 hex literals) with named constants. The approach follows the existing pattern established for M6502Mnemonics and M6502Opcodes, creating two new namespaces: RockwellMnemonics and RockwellOpcodes.

The implementation is low-risk because:
1. Only 2 files modified (opcodes_6502.h, cpu_6502.cpp)
2. No behavior changes - opcodes remain identical
3. Comprehensive test coverage validates equivalence (1564 tests)
4. Following established codebase patterns

**Key Technical Decisions:**
- **Use constexpr**: Ensures compile-time constants with zero runtime overhead
- **Separate namespaces**: RockwellMnemonics and RockwellOpcodes mirror existing pattern
- **No conditional compilation**: Constants always defined (matching 65C02/65816 pattern)

**Patterns to Use:**
- **Namespace pattern**: Follow M6502Mnemonics/M6502Opcodes structure in opcodes_6502.h
- **Constexpr pattern**: Use `constexpr const char*` for mnemonics, `constexpr uint8_t` for opcodes
- **Comment pattern**: Document Rockwell extension sections clearly

---

## Critical Files Identified

### Files to Modify

```
1. include/xasm++/cpu/opcodes_6502.h
   - Current: Defines M6502Mnemonics, M6502Opcodes, M65816Mnemonics namespaces
   - Changes: Add RockwellMnemonics namespace (32 constants), RockwellOpcodes namespace (32 constants)
   - Location: Add after existing namespace definitions

2. src/cpu/cpu_6502.cpp
   - Current: Uses magic strings and hex literals for Rockwell instructions
   - Changes:
     - Lines 2467-2470: Replace 32 magic strings with RockwellMnemonics:: references in HasOpcode()
     - Lines 1254-1500+: Replace 32 magic hex literals with RockwellOpcodes:: references in 32 Encode*() functions
```

### Files to Create

None - all changes are to existing files.

### Files to Read (for context)

```
- include/xasm++/cpu/opcodes_6502.h - Understand existing constant patterns
- src/cpu/cpu_6502.cpp - Identify all Rockwell magic value locations
```

---

## Step-by-Step Implementation Plan

### Phase 1: Define Constants (TDD - Test Infrastructure)

```
Step 1.1: Add RockwellMnemonics namespace to opcodes_6502.h
  - Action: Define 32 mnemonic constants (RMB0-7, SMB0-7, BBR0-7, BBS0-7)
  - Files: include/xasm++/cpu/opcodes_6502.h
  - Pattern:
    namespace RockwellMnemonics {
      constexpr const char* RMB0 = "RMB0";
      constexpr const char* RMB1 = "RMB1";
      // ... 30 more
      constexpr const char* BBS7 = "BBS7";
    }
  - Verification: Compiles successfully

Step 1.2: Add RockwellOpcodes namespace to opcodes_6502.h
  - Action: Define 32 opcode constants with correct hex values
  - Files: include/xasm++/cpu/opcodes_6502.h
  - Pattern:
    namespace RockwellOpcodes {
      constexpr uint8_t RMB0 = 0x07;
      constexpr uint8_t RMB1 = 0x17;
      constexpr uint8_t RMB2 = 0x27;
      // ... 29 more
      constexpr uint8_t BBS7 = 0xFF;
    }
  - Verification: Compiles successfully, values match existing magic literals

Step 1.3: Verify constant values match existing opcodes
  - Action: Cross-check each constant against current magic literal in Encode*() functions
  - Files: Compare opcodes_6502.h constants to cpu_6502.cpp literals
  - Verification: All 32 values match exactly
```

### Phase 2: Replace Magic Values (TDD - RED → GREEN)

```
Step 2.1: Replace mnemonic strings in HasOpcode()
  - Action: Replace 32 magic strings with RockwellMnemonics:: references
  - Files: src/cpu/cpu_6502.cpp lines 2467-2470
  - Before:
    "RMB0", "RMB1", "RMB2", ..., "BBS7",
  - After:
    RockwellMnemonics::RMB0, RockwellMnemonics::RMB1, ..., RockwellMnemonics::BBS7,
  - Tests: Run full test suite (1564 tests)
  - Verification: All tests pass, zero warnings

Step 2.2: Replace opcode literals in EncodeRMB* functions (8 functions)
  - Action: Replace magic hex literals with RockwellOpcodes:: references
  - Files: src/cpu/cpu_6502.cpp lines ~1254-1350
  - Example:
    Before: return {0x07, operand};
    After:  return {RockwellOpcodes::RMB0, operand};
  - Functions: EncodeRMB0 through EncodeRMB7
  - Tests: Run test suite
  - Verification: All tests pass

Step 2.3: Replace opcode literals in EncodeSMB* functions (8 functions)
  - Action: Replace magic hex literals with RockwellOpcodes:: references
  - Files: src/cpu/cpu_6502.cpp lines ~1351-1450
  - Functions: EncodeSMB0 through EncodeSMB7
  - Tests: Run test suite
  - Verification: All tests pass

Step 2.4: Replace opcode literals in EncodeBBR* functions (8 functions)
  - Action: Replace magic hex literals with RockwellOpcodes:: references
  - Files: src/cpu/cpu_6502.cpp lines ~1451-1550
  - Functions: EncodeBBR0 through EncodeBBR7
  - Tests: Run test suite
  - Verification: All tests pass

Step 2.5: Replace opcode literals in EncodeBBS* functions (8 functions)
  - Action: Replace magic hex literals with RockwellOpcodes:: references
  - Files: src/cpu/cpu_6502.cpp lines ~1551-1650
  - Functions: EncodeBBS0 through EncodeBBS7
  - Tests: Run test suite
  - Verification: All tests pass
```

### Phase 3: Verification and Cleanup

```
Step 3.1: Verify no Rockwell magic values remain
  - Action: Grep search for any remaining Rockwell magic values
  - Commands:
    grep -n '"RMB[0-7]"' src/cpu/cpu_6502.cpp
    grep -n '"SMB[0-7]"' src/cpu/cpu_6502.cpp
    grep -n '"BBR[0-7]"' src/cpu/cpu_6502.cpp
    grep -n '"BBS[0-7]"' src/cpu/cpu_6502.cpp
  - Verification: All grep searches return no results (only in comments/strings if any)

Step 3.2: Run full test suite
  - Action: Build and run all 1564 tests
  - Command: cmake --build build && ctest --test-dir build --output-on-failure
  - Verification: 100% tests passed, 0 tests failed

Step 3.3: Verify zero compiler warnings
  - Action: Clean build with warnings enabled
  - Command: cmake --build build --clean-first
  - Verification: Zero warnings during compilation

Step 3.4: Format code
  - Action: Run clang-format on modified files
  - Command: clang-format -i include/xasm++/cpu/opcodes_6502.h src/cpu/cpu_6502.cpp
  - Verification: Code formatted consistently

Step 3.5: Git commit
  - Action: Commit with descriptive message
  - Command: git add include/xasm++/cpu/opcodes_6502.h src/cpu/cpu_6502.cpp
  - Message: "refactor: Replace Rockwell magic values with named constants"
  - Verification: Clean commit, no unintended changes
```

---

## Testing Strategy

### Unit Tests

No new unit tests required - this is pure refactoring with existing coverage.

**Existing tests validate:**
```
✓ All Rockwell instructions assemble correctly
✓ Opcodes match expected values
✓ Addressing modes work properly
✓ Edge cases handled correctly
```

### Integration Tests

**Existing integration tests validate:**
```
✓ Full assembly programs using Rockwell instructions
✓ Interaction with standard 65C02 instructions
✓ Binary output correctness
```

### Acceptance Tests

**Test suite coverage:**
```
✓ 1564 existing tests must all pass
✓ Zero regression in behavior
✓ Binary-identical output for Rockwell instructions
```

### Coverage Target

```
- Overall: Maintain existing 80%+ coverage
- Rockwell code: Already covered by existing tests
- New code: N/A (no new logic, only constant definitions)
```

---

## Dependencies and Prerequisites

### Technical Dependencies

```
□ C++17 compiler (constexpr support) - Already available
□ CMake build system - Already configured
□ Test framework - Already in place
```

### Task Dependencies

```
□ None - self-contained refactoring
```

### Knowledge Dependencies

```
□ Understanding of C++ constexpr
□ Familiarity with 6502 opcodes (documented in contract)
□ Knowledge of existing M6502Mnemonics/Opcodes pattern
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Typo in constant definition causes wrong opcode
- **Probability:** Low
- **Impact:** High
- **Mitigation:**
  - Cross-check each constant against existing literal (Step 1.3)
  - Full test suite validates behavior (1564 tests)
  - Binary output comparison
- **Contingency:** Revert commit, fix constant, retest

**Risk 2:** Missing a magic value replacement
- **Probability:** Low
- **Impact:** Medium
- **Mitigation:**
  - Grep search for remaining magic values (Step 3.1)
  - Test suite would catch inconsistent behavior
- **Contingency:** Add missed replacement, retest

**Risk 3:** Breaking existing assembly code
- **Probability:** Very Low
- **Impact:** Critical
- **Mitigation:**
  - Pure refactoring - opcodes unchanged
  - No API changes - internal implementation only
  - Test suite validates equivalence
- **Contingency:** Revert commit immediately

### Integration Risks

**Risk 1:** Compiler doesn't optimize constexpr properly
- **Mitigation:** constexpr is standard C++17, well-supported, existing code uses it
- **Contingency:** Verify assembly output, compare binary size (should be identical)

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. git revert <commit-hash>
2. Rebuild: cmake --build build --clean-first
3. Retest: ctest --test-dir build --output-on-failure
```

**Rollback Verification:**
```
✓ All 1564 tests pass
✓ Magic values back in code (confirmed by grep)
✓ Zero warnings during build
```

### Git Rollback

```
# If committed
git revert <commit-hash>

# If not committed
git restore include/xasm++/cpu/opcodes_6502.h src/cpu/cpu_6502.cpp

# Verify
cmake --build build && ctest --test-dir build
```

---

## Performance Considerations

### Expected Performance Impact

```
- Compilation time: None (constexpr evaluated at compile time)
- Runtime performance: None (identical binary output)
- Binary size: None (constants inline identically to literals)
```

### Performance Targets

```
- Build time: No change
- Test execution time: No change
- Binary size: Identical to current
```

### Monitoring Plan

```
□ Compare binary size before/after (should be identical)
□ Compare test execution time (should be identical)
□ Visual inspection of compiler-generated assembly (optional)
```

---

## Security Considerations

### Security Checklist

Not applicable - pure refactoring with no security-relevant changes.

```
□ No input validation needed (compile-time constants)
□ No output sanitization needed (internal opcodes)
□ No authentication/authorization involved
□ No sensitive data
□ No SQL/XSS/CSRF concerns
```

### Security Review Points

None - refactoring only, no security surface changes.

---

## Alternative Approaches Considered

### Alternative 1: Keep Magic Values

**Pros:**
- No work required
- Current code functions correctly

**Cons:**
- Inconsistent with 65C02/65816 patterns
- Magic values violate clean code principles
- No compile-time validation
- Poor maintainability

**Why Not Chosen:**
Inconsistency with existing patterns creates technical debt. Clean code principles mandate named constants.

### Alternative 2: Use Macros Instead of constexpr

**Pros:**
- Slightly more explicit preprocessor expansion

**Cons:**
- No type safety
- Not C++ idiomatic
- Inconsistent with existing M6502Mnemonics/Opcodes pattern

**Why Not Chosen:**
constexpr is superior: type-safe, debuggable, C++ standard practice. Existing codebase uses constexpr.

### Alternative 3: Single Namespace for Both Mnemonics and Opcodes

**Pros:**
- One namespace instead of two

**Cons:**
- Name collision risk (RMB0 mnemonic vs RMB0 opcode)
- Inconsistent with existing two-namespace pattern

**Why Not Chosen:**
Following established codebase pattern is more important than minor namespace reduction.

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Phase 1: Define Constants - 1 hour
- Step 1.1: RockwellMnemonics namespace - 20 min
- Step 1.2: RockwellOpcodes namespace - 20 min
- Step 1.3: Verify values - 20 min

Phase 2: Replace Magic Values - 1.5 hours
- Step 2.1: Replace mnemonic strings - 15 min
- Step 2.2: Replace RMB opcodes - 20 min
- Step 2.3: Replace SMB opcodes - 20 min
- Step 2.4: Replace BBR opcodes - 20 min
- Step 2.5: Replace BBS opcodes - 15 min

Phase 3: Verification - 1 hour
- Step 3.1: Grep verification - 10 min
- Step 3.2: Full test suite - 30 min
- Step 3.3: Zero warnings - 10 min
- Step 3.4: Format code - 5 min
- Step 3.5: Git commit - 5 min

Total: ~3.5 hours
```

---

## Success Metrics

### Completion Criteria

```
✓ All 64 magic values replaced with named constants
✓ All 1564 tests passing
✓ Zero compiler warnings
✓ Code formatted consistently
✓ No Rockwell magic values remain (grep confirms)
✓ Committed to repository
```

### Quality Metrics

```
- Test coverage: Maintain 80%+ (existing coverage)
- Performance: Identical to current
- Code quality: Consistent with existing patterns
- Security: N/A (no security surface changes)
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [X] Author: Orchestrator (2026-02-13)
- [ ] Engineer: (pending - will approve before execution)

**Revision History:**
```
v1.0 - 2026-02-13 - Initial plan
```

---

## Notes and Considerations

**Implementation Notes:**
- This is a straightforward refactoring with clear success criteria
- Existing test coverage provides excellent safety net
- Following established patterns ensures consistency
- Low risk due to small scope (2 files, pure refactoring)

**Execution Strategy:**
- Single engineer agent (no parallelization needed)
- Sequential phases (constants → replacements → verification)
- TDD approach: define constants (test infrastructure), replace values (RED→GREEN), verify (test)

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-13
