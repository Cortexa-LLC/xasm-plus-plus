# Implementation Plan: Fix FLEX ASM Phase 2 Macro Test Failures

**Task ID:** 2026-02-09_fix-flex-macro-tests
**Created:** 2026-02-09
**Author:** Orchestrator
**Plan Version:** 1.0
**Workflow:** Bugfix (`.ai-pack/workflows/bugfix.md`)

---

## Approach Summary

This is a test-driven bugfix. The root cause has been identified (local label generation in macro expansion), but we need to verify the exact failure mode before implementing the fix. The approach is:

1. **Debug failing tests** to understand exact failure mode (expected vs actual)
2. **Fix local label generation** in macro_processor.cpp
3. **Verify all 18 tests pass** with zero regression

The fix is localized to the macro expansion logic. The current implementation likely has a simple counter that doesn't properly distinguish between:
- Different macro expansions (need unique expansion ID per call)
- Different local labels within one macro (need unique index per label)

**Key Technical Decisions:**
- Use TDD approach: Understand test failures → Implement fix → Verify green
- Fix will be in macro_processor.cpp (implementation file only)
- No changes to macro_processor.h interface (internal implementation detail)
- Preserve all existing test behavior (zero regression)

**Patterns to Use:**
- Debug-first approach for bugfixes (understand failure mode before fixing)
- Unique ID generation: expansion_id + label_index within expansion
- TDD cycle: RED (understand) → GREEN (fix) → REFACTOR (clean up)

---

## Critical Files Identified

### Files to Modify

```
1. src/syntax/flex/macro_processor.cpp
   - Current: Local label generation uses simple counter
   - Changes: Fix to generate unique names per expansion AND per label
   - Estimated lines: ~30-50 lines

2. tests/unit/test_flex_macros.cpp (potentially for debugging)
   - Current: 16/18 tests passing, 2 failing
   - Changes: May add debug output to understand failures (temporary)
   - No permanent changes expected
```

### Files to Read (for context)

```
- include/xasm++/syntax/macro_processor.h - Interface and data structures
- src/syntax/flex/macro_processor.cpp - Current implementation
- tests/unit/test_flex_macros.cpp - All test cases (especially failing ones)
```

---

## Step-by-Step Implementation Plan

### Phase 1: Investigation (Understand Failure Mode)

```
Step 1.1: Run failing tests with verbose output
  - Action: Run test_flex_macros with --gtest_filter for 2 failing tests
  - Files: tests/unit/test_flex_macros.cpp
  - Tests: MacroWithMultipleLocalLabels, CompleteProgram
  - Verification: Capture exact error messages (expected vs actual)

Step 1.2: Analyze local label generation logic
  - Action: Read macro_processor.cpp implementation
  - Files: src/syntax/flex/macro_processor.cpp
  - Focus: Find where local labels (?) are processed
  - Verification: Understand current algorithm

Step 1.3: Identify root cause
  - Action: Compare expected vs actual from Step 1.1 with algorithm from 1.2
  - Verification: Confirm hypothesis about local label generation
```

### Phase 2: Implementation (Fix Local Label Generation)

```
Step 2.1: Implement expansion counter
  - Action: Add per-expansion counter to track unique macro invocations
  - Files: src/syntax/flex/macro_processor.cpp
  - Implementation: Likely in ExpandMacro() or similar method
  - Verification: Counter increments per macro expansion

Step 2.2: Implement label index within expansion
  - Action: Track local label index within each macro expansion
  - Files: src/syntax/flex/macro_processor.cpp
  - Implementation: Reset per expansion, increment per local label
  - Verification: Each label gets unique index

Step 2.3: Generate unique local label names
  - Action: Combine expansion_id + label_index to generate unique names
  - Files: src/syntax/flex/macro_processor.cpp
  - Format: ?label_name.expansion_id.label_index
  - Example: ?start → ?start.1.1, ?start.1.2 (first expansion, labels 1 and 2)
  - Verification: Generated names are unique
```

### Phase 3: Testing (Verify Fix)

```
Step 3.1: Run failing tests
  - Action: Build and run test_flex_macros
  - Tests: MacroWithMultipleLocalLabels, CompleteProgram
  - Verification: Both tests pass

Step 3.2: Run all FLEX macro tests
  - Action: Build and run full test suite
  - Tests: All 18 tests in test_flex_macros.cpp
  - Verification: 18/18 passing (100%), zero regression

Step 3.3: Build with warnings enabled
  - Action: cmake --build build 2>&1 | grep -i warning
  - Verification: Zero compiler warnings
```

### Phase 4: Documentation

```
Step 4.1: Update work log
  - Action: Document failure mode, fix approach, results
  - Files: .ai/tasks/2026-02-09_fix-flex-macro-tests/20-work-log.md
  - Verification: Work log complete

Step 4.2: Add code comments
  - Action: Document local label generation algorithm in code
  - Files: src/syntax/flex/macro_processor.cpp
  - Verification: Algorithm is clear to future maintainers
```

---

## Testing Strategy

### Unit Tests (All in test_flex_macros.cpp)

```
□ Failing Tests (must fix):
  - MacroWithMultipleLocalLabels - Multiple ? labels in one macro
  - CompleteProgram - Complex program with macros and local labels

□ Passing Tests (must not regress):
  - SimpleMacro - Basic macro definition and expansion
  - MacroWithParameters - Macro with \1, \2 parameters
  - MacroWithSingleLocalLabel - Single ? label
  - NestedMacros - Macro calls within macros
  - ... (10 more tests)
```

### Acceptance Tests

```
□ Test Scenario 1: Multiple local labels in one macro
  - Given: Macro with ?start and ?end labels
  - When: Macro expanded twice
  - Then: ?start.1.1, ?end.1.2 (first), ?start.2.1, ?end.2.2 (second)

□ Test Scenario 2: Multiple macro expansions
  - Given: Macro with local labels
  - When: Macro called N times
  - Then: Each expansion gets unique suffix (.1, .2, .3, ..., .N)
```

### Coverage Target

```
- Overall: Maintain existing coverage (no decrease)
- Critical paths: 100% (local label generation)
- New code: N/A (fixing existing code)
```

---

## Dependencies and Prerequisites

### Technical Dependencies

```
✓ Google Test framework (already available)
✓ CMake build system (already configured)
✓ FLEX Phase 2 implementation (exists, needs fixing)
```

### Task Dependencies

```
✓ No blockers (task is ready)
```

### Knowledge Dependencies

```
✓ FLEX ASM09 macro syntax (documented in reference manual)
✓ Local label conventions (? prefix)
✓ Macro expansion algorithm (implemented in macro_processor.cpp)
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Fix breaks existing tests
- **Probability:** Low
- **Impact:** Medium
- **Mitigation:** Run full test suite after every change
- **Contingency:** Git restore if tests fail, re-analyze root cause

**Risk 2:** Root cause is more complex than identified
- **Probability:** Low
- **Impact:** Medium
- **Mitigation:** Debug tests first (Phase 1) before implementing fix
- **Contingency:** If fix doesn't work, deeper investigation required

**Risk 3:** Multiple macro expansions in test cases reveal edge cases
- **Probability:** Medium
- **Impact:** Low
- **Mitigation:** Test all 18 tests, including passing ones
- **Contingency:** Add more test cases if edge cases discovered

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. git restore src/syntax/flex/macro_processor.cpp
2. git restore tests/unit/test_flex_macros.cpp (if modified)
3. Rebuild: cmake --build build
4. Verify: ./build/tests/unit/test_flex_macros (should return to 16/18)
```

**Rollback Verification:**
```
✓ 16/18 tests passing (original state)
✓ Build succeeds
✓ No uncommitted changes
```

### Git Rollback

```
# If changes committed (shouldn't be until complete)
git revert <commit-hash>

# If not committed
git restore src/syntax/flex/macro_processor.cpp

# Verify
./build/tests/unit/test_flex_macros --gtest_filter="*"
```

---

## Performance Considerations

### Expected Performance Impact

```
- Local label generation: Minimal impact (string concatenation)
- Macro expansion: No change (same algorithm, different label names)
```

### Performance Targets

```
- No performance degradation
- Test execution time: Same as before
```

---

## Security Considerations

### Security Checklist

```
✓ Input validation implemented (already exists in macro system)
✓ No external input (assembler source code only)
✓ No network operations
✓ No filesystem operations outside project
✓ No security implications
```

---

## Alternative Approaches Considered

### Alternative 1: Global counter only

**Pros:**
- Simple implementation (single counter)
- Easy to understand

**Cons:**
- Doesn't distinguish multiple labels in one macro
- Would fail MacroWithMultipleLocalLabels test
- Labels like ?start and ?end would get same suffix

**Why Not Chosen:**
Multiple local labels in one macro need unique suffixes. Global counter alone is insufficient.

### Alternative 2: UUID/random suffix

**Pros:**
- Guaranteed uniqueness
- No state tracking needed

**Cons:**
- Non-deterministic (harder to debug)
- Verbose output (long label names)
- Overkill for the problem

**Why Not Chosen:**
Simple sequential counters are sufficient and more readable. UUIDs are unnecessary complexity.

### Alternative 3: Hash of label name

**Pros:**
- Deterministic based on label name
- No state tracking

**Cons:**
- Hash collisions possible
- Multiple expansions of same label would collide
- Doesn't solve the problem

**Why Not Chosen:**
Need to distinguish multiple expansions of same label. Hash doesn't provide this.

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Phase 1 (Investigation): 1-2 hours
- Step 1.1: Run failing tests (30 min)
- Step 1.2: Analyze implementation (30 min)
- Step 1.3: Confirm root cause (30 min)

Phase 2 (Implementation): 1-2 hours
- Step 2.1: Add expansion counter (30 min)
- Step 2.2: Add label index (30 min)
- Step 2.3: Generate unique names (30 min)

Phase 3 (Testing): 30 min
- Step 3.1: Run failing tests (10 min)
- Step 3.2: Run all tests (10 min)
- Step 3.3: Check warnings (10 min)

Phase 4 (Documentation): 30 min
- Step 4.1: Update work log (15 min)
- Step 4.2: Add code comments (15 min)

Total: 3-5 hours
```

---

## Success Metrics

### Completion Criteria

```
✓ All steps completed
✓ All 18 tests passing (100%)
✓ Zero compiler warnings
✓ Work log updated
✓ Code documented
✓ Ready for commit
```

### Quality Metrics

```
- Test coverage: 18/18 (100%)
- Performance: No degradation
- Code quality: Follows existing patterns
- Security: No issues
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [x] Author: Orchestrator 2026-02-09
- [ ] Agent: Engineer [Pending]

**Revision History:**
```
v1.0 - 2026-02-09 - Initial plan
```

---

## Notes and Considerations

**Expected Fix Location:**

```cpp
// In macro_processor.cpp, likely in ExpandMacro() or ProcessLocalLabel()

// Current (broken):
std::string ProcessLocalLabel(const std::string& label) {
    static int counter = 0;  // Global counter, shared across all labels
    counter++;
    return label + "." + std::to_string(counter);
}

// Fixed:
class MacroProcessor {
private:
    int expansion_counter_ = 0;  // Per macro expansion

    std::string ExpandMacro(const MacroDefinition& macro) {
        expansion_counter_++;  // Increment per expansion
        int label_index = 0;    // Reset per expansion

        // Process macro body
        for (each local label in body) {
            label_index++;  // Increment per label
            std::string unique_name = label + "." +
                                     std::to_string(expansion_counter_) + "." +
                                     std::to_string(label_index);
        }
    }
};
```

**Key Insight:**

Need TWO counters:
1. Expansion counter (increments per macro call)
2. Label index (resets per expansion, increments per local label)

Combined: `?label.{expansion}.{index}`

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-09
