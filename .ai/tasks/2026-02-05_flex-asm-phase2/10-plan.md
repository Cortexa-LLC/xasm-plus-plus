# Implementation Plan: FLEX ASM Phase 2 - Macro Processor

**Task:** xasm++-684
**Date:** 2026-02-05
**Workflow:** Feature (Macro Capability)

---

## Approach Summary

This implementation will add macro definition and expansion capabilities to FLEX ASM by creating a separate `MacroProcessor` class and integrating it with the existing `FlexAsmSyntax` parser. The approach follows a preprocessor pattern where macros are expanded during parsing (inline expansion) rather than as a separate pass.

The macro processor will handle parameter substitution, local label uniquification, and nested macro calls. It will be implemented as a standalone, well-tested component first, then integrated into the FLEX ASM syntax parser.

**Key Technical Decisions:**
- **Inline expansion:** Expand macros during parsing rather than as separate pass (simpler, matches FLEX behavior)
- **Local label strategy:** Prefix with unique counter `.LOOP` → `.LOOP_001` (simple, effective)
- **Parameter limit:** 8 parameters (FLEX spec)
- **Storage:** Unordered map for O(1) macro lookup

**Patterns to Use:**
- **Separation of Concerns:** MacroProcessor class is independent of syntax parser
- **Template Method:** FlexAsmSyntax extends MotorolaBaseSyntax with macro support
- **State Machine:** ParserMode enum for tracking macro definition vs normal parsing

---

## Critical Files Identified

### Files to Create

```
1. include/xasm++/syntax/macro_processor.h
   - Purpose: Header for MacroProcessor class
   - Rationale: Separate component for macro management

2. src/syntax/flex/macro_processor.cpp
   - Purpose: Implementation of macro processor
   - Rationale: Parameter substitution, label uniquification, expansion logic

3. tests/unit/test_macro_processor.cpp
   - Purpose: Unit tests for MacroProcessor
   - Rationale: Test macro functionality in isolation

4. tests/unit/test_flex_macros.cpp
   - Purpose: Integration tests for MACRO/ENDM directives
   - Rationale: Test macro integration with FlexAsmSyntax

5. tests/integration/flex/macro_simple.asm
6. tests/integration/flex/macro_params.asm
7. tests/integration/flex/macro_local.asm
   - Purpose: Real FLEX programs using macros
   - Rationale: End-to-end validation
```

### Files to Modify

```
1. include/xasm++/syntax/flex_syntax.h
   - Current: FlexAsmSyntax class without macro support
   - Changes: Add MacroProcessor member, ParserMode enum, macro handlers

2. src/syntax/motorola/flex_syntax.cpp
   - Current: Phase 1 directives, TODO stubs for macros (lines 368, 375, 380)
   - Changes: Implement MACRO/ENDM handlers, integrate macro expansion

3. src/CMakeLists.txt
   - Current: Builds flex_syntax.cpp
   - Changes: Add macro_processor.cpp to build

4. tests/unit/CMakeLists.txt
   - Current: Builds existing unit tests
   - Changes: Add test_macro_processor.cpp and test_flex_macros.cpp
```

### Files to Read (for context)

```
- src/syntax/motorola_base_syntax.cpp - Base class parsing patterns
- include/xasm++/syntax/flex_syntax.h - Current FlexAsmSyntax API
- tests/unit/test_flex_syntax.cpp - Existing test patterns
- .ai/tasks/2026-02-04_flex-asm-syntax/10-plan.md - Original Phase 2 plan
```

---

## Step-by-Step Implementation Plan

### Phase 1: MacroProcessor Class Implementation

**Goal:** Create standalone, fully-tested macro processor

**Step 1.1: Create MacroProcessor header**
- Action: Define MacroProcessor class with public API
- Files: `include/xasm++/syntax/macro_processor.h`
- Tests: None yet
- Verification: Compiles without errors
- Time: 1 hour

**Step 1.2: Implement core methods**
- Action: Implement DefineMacro, IsMacro, GetMacro, Clear
- Files: `src/syntax/flex/macro_processor.cpp`
- Tests: None yet
- Verification: Compiles, links
- Time: 1 hour

**Step 1.3: Implement parameter substitution**
- Action: Implement SubstituteParameters with word-boundary checking
- Files: `src/syntax/flex/macro_processor.cpp`
- Tests: Unit tests for substitution edge cases
- Verification: Tests pass
- Time: 2-3 hours

**Step 1.4: Implement local label uniquification**
- Action: Implement MakeLocalLabelUnique and ExpandMacro
- Files: `src/syntax/flex/macro_processor.cpp`
- Tests: Unit tests for label uniqueness
- Verification: Each expansion gets unique labels
- Time: 2-3 hours

**Step 1.5: Comprehensive unit testing**
- Action: Create test_macro_processor.cpp with 15+ test cases
- Files: `tests/unit/test_macro_processor.cpp`
- Tests: All unit tests
- Verification: >80% coverage, all tests pass
- Time: 2-3 hours

**Phase 1 Total:** 8-10 hours

---

### Phase 2: Integration with FlexAsmSyntax

**Goal:** Integrate macro processor into FLEX ASM parser

**Step 2.1: Add ParserMode to FlexAsmSyntax**
- Action: Add enum and mode tracking to header
- Files: `include/xasm++/syntax/flex_syntax.h`
- Tests: None yet
- Verification: Compiles
- Time: 1 hour

**Step 2.2: Implement MACRO directive handler**
- Action: Implement HandleMACRO, enter macro definition mode
- Files: `src/syntax/motorola/flex_syntax.cpp`
- Tests: Unit test for MACRO parsing
- Verification: Macro definition mode entered correctly
- Time: 2 hours

**Step 2.3: Implement ENDM directive handler**
- Action: Implement HandleENDM, CollectMacroLine, store macro
- Files: `src/syntax/motorola/flex_syntax.cpp`
- Tests: Unit test for ENDM parsing
- Verification: Macro stored in processor
- Time: 1 hour

**Step 2.4: Integrate macro expansion into ParseLine**
- Action: Check for macro invocations, expand, parse recursively
- Files: `src/syntax/motorola/flex_syntax.cpp`
- Tests: Unit tests for expansion integration
- Verification: Macros expand and assemble correctly
- Time: 3-4 hours

**Phase 2 Total:** 7-8 hours

---

### Phase 3: Integration Testing

**Goal:** Validate with real FLEX programs

**Step 3.1: Create integration test files**
- Action: Write macro_simple.asm, macro_params.asm, macro_local.asm
- Files: `tests/integration/flex/*.asm`
- Tests: Integration tests
- Verification: Programs assemble correctly
- Time: 2-3 hours

**Step 3.2: Integration unit tests**
- Action: Create test_flex_macros.cpp with integration scenarios
- Files: `tests/unit/test_flex_macros.cpp`
- Tests: Directive parsing, expansion, nested calls
- Verification: All tests pass
- Time: 2-3 hours

**Step 3.3: Regression testing**
- Action: Run Phase 1 tests, verify no regressions
- Files: `tests/unit/test_flex_syntax.cpp`
- Tests: All Phase 1 tests
- Verification: 13 Phase 1 tests still pass
- Time: 1 hour

**Phase 3 Total:** 5-7 hours

---

## Testing Strategy

### Unit Tests

**MacroProcessor (test_macro_processor.cpp):**
```
□ Define simple macro (no parameters)
  - Happy path: Store and retrieve macro
  - Edge case: Empty body
  - Error case: None (all valid)

□ Define macro with parameters
  - Happy path: 1-8 parameters
  - Edge case: 0 parameters
  - Error case: >8 parameters (should warn/error)

□ Expand macro without parameters
  - Happy path: Simple expansion
  - Edge case: Multiple invocations
  - Error case: Undefined macro

□ Parameter substitution
  - Happy path: All parameters substituted
  - Edge case: Missing arguments (empty string)
  - Edge case: Parameter names as substrings
  - Edge case: Parameters in comments/strings
  - Error case: None (graceful degradation)

□ Local label uniqueness
  - Happy path: Labels unique per invocation
  - Edge case: Multiple local labels in one macro
  - Edge case: Nested macros with local labels
  - Error case: None

□ Nested macro calls
  - Happy path: Macro calling another macro
  - Edge case: Chain of 3+ macros
  - Error case: Recursion (if detected)
```

**FlexAsmSyntax Integration (test_flex_macros.cpp):**
```
□ MACRO directive parsing
  - Happy path: Name and parameters extracted
  - Edge case: No parameters
  - Error case: Nested MACRO

□ ENDM directive parsing
  - Happy path: Exits macro definition mode
  - Error case: ENDM without MACRO

□ Macro expansion during assembly
  - Happy path: Invocation expands correctly
  - Edge case: Multiple invocations
  - Error case: Wrong parameter count

□ Integration with Phase 1 directives
  - Happy path: Macro with ORG, FCB, etc.
  - Edge case: Macro generating labels
```

### Integration Tests

```
□ macro_simple.asm - Simple macro without parameters
  - Setup: Define DELAY macro
  - Execute: Invoke DELAY 3 times
  - Verify: 9 bytes (3 x LDA + NOP)

□ macro_params.asm - Macro with parameters
  - Setup: Define MOVB macro with SRC, DEST
  - Execute: Invoke MOVB with different arguments
  - Verify: Correct LDA/STA with substituted addresses

□ macro_local.asm - Macro with local labels
  - Setup: Define COPY macro with .LOOP label
  - Execute: Invoke COPY twice
  - Verify: Branch targets are unique per invocation
```

### Coverage Target

```
- Overall: 85%+
- MacroProcessor: 90%+ (critical component)
- New code in FlexAsmSyntax: 85%+
```

---

## Dependencies and Prerequisites

### Technical Dependencies

```
□ C++17 compiler - Required for xasm++
□ Catch2 - Unit test framework (existing)
□ CMake 3.15+ - Build system (existing)
```

### Task Dependencies

```
✅ Phase 1 complete (xasm++-k5s) - Basic directives working
□ MotorolaBaseSyntax - Provides parsing foundation (existing)
□ Expression evaluator - For parameter expressions (existing)
```

### Knowledge Dependencies

```
□ Understanding of FLEX ASM macro syntax
□ Familiarity with xasm++ parsing architecture
□ Knowledge of C++ string manipulation
□ Unit testing with Catch2
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1: Parameter substitution edge cases**
- **Probability:** Medium
- **Impact:** High (core feature)
- **Mitigation:** Comprehensive unit tests, word-boundary checking
- **Contingency:** Start with simple substring replacement, iterate

**Risk 2: Local label uniqueness conflicts**
- **Probability:** Low
- **Impact:** Medium (important but solvable)
- **Mitigation:** Simple counter-based approach, extensive tests
- **Contingency:** Fall back to requiring explicit labels

**Risk 3: Performance degradation**
- **Probability:** Low
- **Impact:** Medium
- **Mitigation:** Profile after implementation, optimize if needed
- **Contingency:** Add macro depth limits, cache expansions

### Integration Risks

**Risk 1: Breaking Phase 1 functionality**
- **Mitigation:** Run Phase 1 tests after each change, maintain backward compatibility

**Risk 2: Recursive macro expansion**
- **Mitigation:** Add recursion detection or depth limit

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. Revert commits for Phase 2
2. Verify Phase 1 tests still pass
3. Document issues encountered
4. Plan alternative approach
```

**Git Rollback:**
```bash
# If committed
git revert <commit-range-for-phase2>

# Verify Phase 1 still works
cmake --build build
ctest --test-dir build -R test_flex_syntax
```

---

## Performance Considerations

### Expected Performance Impact

```
- Macro definition: Minimal (one-time storage)
- Macro expansion: O(n) where n = macro body lines
- Parameter substitution: O(m*p) where m = line length, p = parameter count
```

### Performance Targets

```
- Assembly time: Within 10% of Phase 1 (without macros)
- Macro expansion: <1ms per invocation
- Memory overhead: <1MB for macro storage
```

### Monitoring Plan

```
□ Profile assembly time for large programs
□ Measure macro expansion time
□ Track memory usage during heavy macro use
```

---

## Timeline Estimate

**Note:** Estimates are for planning, not deadlines.

```
Phase 1: MacroProcessor Implementation
- Step 1.1: 1 hour
- Step 1.2: 1 hour
- Step 1.3: 2-3 hours
- Step 1.4: 2-3 hours
- Step 1.5: 2-3 hours
Subtotal: 8-10 hours

Phase 2: Integration
- Step 2.1: 1 hour
- Step 2.2: 2 hours
- Step 2.3: 1 hour
- Step 2.4: 3-4 hours
Subtotal: 7-8 hours

Phase 3: Testing
- Step 3.1: 2-3 hours
- Step 3.2: 2-3 hours
- Step 3.3: 1 hour
Subtotal: 5-7 hours

Total: 20-25 hours (within 15-20 hour estimate + contingency)
```

---

## Success Metrics

### Completion Criteria

```
✓ MacroProcessor class implemented and tested
✓ MACRO/ENDM directives parse correctly
✓ Macro expansion works with parameter substitution
✓ Local labels are unique per invocation
✓ Nested macro calls work
✓ All unit tests pass (30+ tests)
✓ Integration tests pass
✓ Phase 1 tests still pass (no regression)
✓ Code reviewed and approved
```

### Quality Metrics

```
- Test coverage: 85%+ overall, 90%+ for MacroProcessor
- Performance: No more than 10% slower than Phase 1
- Code quality: No compiler warnings, passes review
- Documentation: API docs complete
```

---

## Plan Approval

**Plan Status:** Draft - Ready for Review

**Reviewed By:**
- [ ] Author: Claude (Orchestrator) - 2026-02-05
- [ ] User: To be reviewed

---

## Notes and Considerations

- Implementation will be done incrementally (MacroProcessor first, then integration)
- Comprehensive unit testing at each step ensures solid foundation
- Integration testing with real FLEX programs validates end-to-end functionality
- Plan allows for iteration and adjustment based on checkpoints

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-05
