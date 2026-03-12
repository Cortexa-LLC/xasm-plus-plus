# Implementation Plan

**Task ID:** 2026-02-13_a2osx-p0-directives
**Created:** 2026-02-13
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Implement 5 P0 SCMASM directives using phased TDD approach. Each directive follows: write test → implement handler → verify → commit pattern.

**Phase 1 (Simple):** .PS, .INB, .LIST (10 hours estimate)
**Phase 2 (Complex):** .DUMMY, .OP (12-17 hours estimate)

**Key Decisions:**
- Follow existing SCMASM handler patterns (HandleOrgDirective, HandleEquDirective, etc.)
- TDD with one directive at a time
- Commit after each directive completion
- Study A2oSX examples for each directive before implementing

---

## Implementation Order

### Phase 1: Simple Directives (Session 1)

**1. .PS (Pascal String) - 2-3 hours**
- **Complexity:** Low (similar to .AS/.AT/.AZ)
- **Test cases:** Empty string, normal string, max-length (255), over-length error
- **Implementation:** Emit length byte + string bytes

**2. .INB (Include Binary) - 3-5 hours**
- **Complexity:** Medium (new file I/O pattern)
- **Test cases:** Relative path, absolute path, missing file error, nested includes
- **Implementation:** Read binary file, insert bytes, track recursion depth

**3. .LIST (Listing Control) - 1-2 hours**
- **Complexity:** Trivial (stub)
- **Test cases:** .LIST ON, .LIST OFF, .LIST (no arg)
- **Implementation:** Parse argument, set flag (no output yet)

**CHECKPOINT:** Commit Phase 1, run full test suite, verify no regressions

---

### Phase 2: Complex Directives (Session 2)

**4. .DUMMY (Dummy Section) - 5-7 hours**
- **Complexity:** High (state management)
- **Test cases:** Basic section, nested sections, symbols in dummy, .ED end
- **Implementation:** Push/pop assembler state, symbol table in dummy mode

**5. .OP (CPU Operation Mode) - 8-10 hours**
- **Complexity:** Very High (CPU abstraction)
- **Test cases:** .OP 6502, .OP 65C02, .OP 65816, invalid opcode for CPU
- **Implementation:** CPU variant switching, opcode validation per CPU

**CHECKPOINT:** Commit Phase 2, run full test suite, integration tests

---

## Testing Strategy

### Per-Directive Tests (in test_scmasm_syntax.cpp)

**Pattern for each directive:**
```cpp
TEST_F(ScmasmSyntaxTest, DirectiveName_HappyPath)
TEST_F(ScmasmSyntaxTest, DirectiveName_EdgeCase1)
TEST_F(ScmasmSyntaxTest, DirectiveName_EdgeCase2)
TEST_F(ScmasmSyntaxTest, DirectiveName_ErrorCase)
```

### Integration Tests (test_a2osx_samples.cpp - NEW FILE)

Create integration test file with A2oSX samples:
```cpp
// Test .PS with real A2oSX usage
TEST(A2osXIntegration, PascalStrings) {
    // Assemble sample with .PS directives
    // Verify output matches expected
}

// Test .INB with includes
TEST(A2osXIntegration, BinaryIncludes) {
    // Assemble sample with .INB directives
    // Verify included bytes present
}
```

---

## Files to Modify/Create

### Modify:
1. `src/syntax/scmasm_directive_handlers.cpp` - Add 5 handler functions
2. `include/xasm++/syntax/scmasm_syntax.h` - Add 5 declarations
3. `src/syntax/scmasm_syntax.cpp` - Register 5 directives
4. `tests/unit/test_scmasm_syntax.cpp` - Add tests (~200 lines)

### Create:
5. `tests/integration/test_a2osx_samples.cpp` - Integration tests
6. `tests/integration/fixtures/a2osx_samples/` - Sample .S files

### Possibly Modify (for .OP):
7. `src/cpu/cpu_6502.cpp` - CPU variant support
8. `src/cpu/cpu_65c02.cpp` - CPU variant support
9. `include/xasm++/cpu/cpu.h` - CPU abstraction interface

---

## Success Metrics

**Phase 1 Complete:**
```
✓ .PS, .INB, .LIST implemented
✓ Unit tests passing for all 3
✓ No regressions
✓ Code committed
```

**Phase 2 Complete:**
```
✓ .DUMMY, .OP implemented
✓ Unit tests passing for all 5
✓ Integration tests passing
✓ A2oSX samples assemble
✓ Code coverage ≥ 90%
✓ All tests passing
✓ Ready for full A2oSX build validation
```

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-13
