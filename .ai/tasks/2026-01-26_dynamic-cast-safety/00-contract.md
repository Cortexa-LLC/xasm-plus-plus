# Task Contract

**Task ID:** 2026-01-26_dynamic-cast-safety
**Created:** 2026-01-26
**Beads Task:** xasm++-d60
**Parent Task:** 2026-01-26_code-quality-refactoring (Task A2)
**Issue:** [C2] Critical - Unchecked Dynamic Casts
**Workflow:** Refactor (Micro-Task)

---

## Task Description

Add null checks after `dynamic_pointer_cast` calls in `assembler.cpp` to prevent null pointer dereference crashes.

### Background

Code review identified multiple `dynamic_pointer_cast` calls without null checks in `assembler.cpp` around lines 180-220. This is a **critical security and stability issue** that can cause crashes.

### Current State

- `dynamic_pointer_cast` calls exist without null validation
- Potential for null pointer dereference
- No error messages on cast failure

### Desired State

- All dynamic casts have null checks
- Descriptive error messages on cast failure
- Tests verify cast failure handling

---

## Scope (NARROW - Avoid Token Limits)

**Files to Modify:**
- `src/core/assembler.cpp` - Add null checks (lines ~180-220 only)
- `tests/unit/test_assembler.cpp` - Add test cases

**Files to Read (Context Only):**
- `include/xasm++/core/assembler.h` - Understand class structure
- That's it. DO NOT read other files.

**Out of Scope:**
- Other files
- Other issues
- Refactoring beyond null checks

---

## Success Criteria

```
✓ 3-5 null checks added after dynamic_pointer_cast calls
✓ Descriptive error messages for each cast failure
✓ 5-10 test cases for cast failure scenarios
✓ All 444 tests still passing (zero regressions)
✓ Zero new compiler warnings
✓ Changes follow TDD (test first)
```

---

## Specific Changes Required

### Step 1: Identify Dynamic Casts

Search `assembler.cpp` for `dynamic_pointer_cast` calls around lines 180-220.

### Step 2: Add Null Checks

For each cast, add pattern:
```cpp
auto casted = std::dynamic_pointer_cast<Type>(expr);
if (!casted) {
    throw std::runtime_error("Failed to cast to Type at [context]");
}
// Use casted safely
```

### Step 3: Write Tests

Add tests to `test_assembler.cpp`:
```cpp
TEST_F(AssemblerTest, DynamicCast_InvalidType_ThrowsError) {
    // Setup: Create scenario where cast will fail
    // Execute: Attempt operation that triggers cast
    // Assert: Verify exception thrown with clear message
}
```

### Step 4: Verify

```bash
cmake --build build
cmake --build build --target test  # Must show 444/444 passing
```

---

## Acceptance Criteria

- [ ] Located all dynamic_pointer_cast calls in scope (lines 180-220)
- [ ] Added null check after each cast
- [ ] Each null check has descriptive error message with context
- [ ] Added 5-10 test cases for cast failures
- [ ] All 444 tests passing
- [ ] Zero new compiler warnings
- [ ] Changes committed with clear message

---

## Estimated Effort

- **Time:** 1 hour
- **Agent Turns:** <15 turns (well under token limit)
- **Lines Changed:** 15-30 lines

---

## Dependencies

**Required:**
- Task A1 (ParseHex) complete ✅

**Blocks:**
- Task B1 (God Method refactoring)

---

## Notes

**CRITICAL - Keep Narrow Scope:**
- ONLY modify assembler.cpp lines 180-220
- ONLY read assembler.h for context
- DO NOT explore entire codebase
- DO NOT read unrelated files
- Focus = avoid token limits

**TDD Process:**
1. Write failing test for cast failure
2. Add null check
3. Test passes
4. Repeat for each cast

**Success Metric:**
- All casts have null checks
- Zero crashes on invalid casts
- Clear error messages for debugging
