# Implementation Plan

**Task ID:** 2026-01-31_bug-ds-program-counter
**Created:** 2026-01-31
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

The fix involves modifying the HandleDS function to distinguish between `*` as:
1. **Program counter symbol** - When used alone or in expressions (`*`, `$900-*`, `*+10`)
2. **Multiplication operator** - When between two numeric operands (`10*20`)

**Strategy:**
1. Before checking for multiplication, substitute `*` with current_address_ value
2. Convert current_address_ to hex string format (`$XXXX`)
3. Then parse the resulting expression normally
4. This handles subtraction, addition automatically

**Key Technical Decisions:**
- Use string substitution approach (simple, clear, maintainable)
- Replace `*` with `$` + hex(current_address_) before expression parsing
- Preserves existing multiplication logic for true multiplication cases

**Patterns to Use:**
- TDD approach: Write failing test first, then implement fix
- String manipulation: Replace-then-parse pattern (similar to hex parsing fix)
- Defensive programming: Validate assumptions with tests

---

## Critical Files Identified

### Files to Modify
```
1. src/syntax/merlin/merlin_syntax.cpp
   - Current: HandleDS function treats * as multiplication only
   - Changes: Add * substitution before multiplication check (lines 463-539)

2. tests/unit/test_merlin_syntax.cpp
   - Current: No tests for * program counter in DS directive
   - Changes: Add test cases for ds $900-*, ds *+10, ds 10*20
```

### Files to Read (for context)
```
- include/xasm++/syntax/merlin_syntax.h - Verify current_address_ member
- tests/e2e/apple2/prince_of_persia/BOOT.S - Real-world test case (line 89)
```

---

## Step-by-Step Implementation Plan

### Phase 1: Test First (TDD RED)
```
Step 1.1: Add failing test for program counter subtraction
  - Action: Add test case "ds $900-*" with known current_address
  - Files: tests/unit/test_merlin_syntax.cpp
  - Tests: TEST(MerlinSyntax, DS_ProgramCounterSubtraction)
  - Verification: Test fails with "Multiplication requires operands" error

Step 1.2: Add test for program counter addition
  - Action: Add test case "ds *+10"
  - Files: tests/unit/test_merlin_syntax.cpp
  - Tests: TEST(MerlinSyntax, DS_ProgramCounterAddition)
  - Verification: Test fails

Step 1.3: Add test for true multiplication (regression check)
  - Action: Add test case "ds 10*20"
  - Files: tests/unit/test_merlin_syntax.cpp
  - Tests: TEST(MerlinSyntax, DS_Multiplication)
  - Verification: Test should PASS (validates existing functionality)
```

### Phase 2: Implement Fix (TDD GREEN)
```
Step 2.1: Add helper function to substitute * with current address
  - Action: Create SubstituteProgramCounter(operand, current_address_)
  - Files: src/syntax/merlin/merlin_syntax.cpp
  - Logic:
    1. Check if operand contains '*'
    2. If yes, convert current_address_ to hex string ($XXXX format)
    3. Replace all '*' with hex string
    4. Return substituted string
  - Verification: Manually test with debug output

Step 2.2: Call substitution before expression parsing
  - Action: In HandleDS, call substitution early
  - Files: src/syntax/merlin/merlin_syntax.cpp (line ~467)
  - Code location: After "std::string op = Trim(operand);"
  - Verification: Build succeeds

Step 2.3: Run tests
  - Action: cmake --build build && ctest
  - Expected: All 3 new tests pass
  - Verification: "Passed: 3/3"
```

### Phase 3: Integration Testing
```
Step 3.1: Test with real POP files
  - Action: Assemble BOOT.S, FRAMEDEF.S, TABLES.S
  - Command: ./build/src/xasm++ --cpu 6502 --syntax merlin -o /tmp/test.bin <file>
  - Expected: All 3 assemble without error
  - Verification: No "Multiplication requires operands" error

Step 3.2: Binary compatibility check
  - Action: Compare with vasm output
  - Files: BOOT.S (if vasm reference available)
  - Verification: Byte-for-byte match
```

### Phase 4: Refactor (TDD REFACTOR)
```
Step 4.1: Clean up code
  - Action: Add comments, improve naming
  - Files: src/syntax/merlin/merlin_syntax.cpp
  - Changes: Document * substitution logic
  - Verification: No behavior change, tests still pass
```

---

## Testing Strategy

### Unit Tests
```
□ DS directive with program counter subtraction:
  - Happy path: ds $900-* with current_address=$800 → 256 bytes
  - Edge case: ds *+10 with current_address=$100 → 266 bytes
  - Edge case: ds * with current_address=$200 → 512 bytes

□ DS directive with multiplication (regression):
  - Happy path: ds 10*20 → 200 bytes
  - Edge case: ds 5*8 → 40 bytes
```

### Integration Tests
```
□ Real POP files assemble successfully:
  - BOOT.S line 89: ds $900-*
  - FRAMEDEF.S line 275: ds $900-*
  - TABLES.S line 51: ds $900-*
```

### Coverage Target
```
- Overall: Maintain existing coverage
- New code: 100% (all branches tested)
- Critical paths: 100% (DS directive parsing)
```

---

## Implementation Details

### Algorithm: Substitute Program Counter

```cpp
std::string SubstituteProgramCounter(const std::string& operand, uint32_t current_addr) {
    // Find * in operand
    if (operand.find('*') == std::string::npos) {
        return operand;  // No substitution needed
    }

    // Convert current address to hex string ($XXXX format)
    std::ostringstream oss;
    oss << "$" << std::hex << current_addr;
    std::string addr_str = oss.str();

    // Replace all * with address
    std::string result = operand;
    size_t pos = 0;
    while ((pos = result.find('*', pos)) != std::string::npos) {
        result.replace(pos, 1, addr_str);
        pos += addr_str.length();
    }

    return result;
}
```

### Integration into HandleDS:

**Before (line 463-475):**
```cpp
void MerlinSyntaxParser::HandleDS(const std::string& operand, Section& section,
                                   ConcreteSymbolTable& symbols) {
    std::string op = Trim(operand);
    uint32_t count = 0;

    if (op.empty()) {
        count = 0;
    }
    else if (op.find('*') != std::string::npos) {
        // Current code: treats * as multiplication
```

**After:**
```cpp
void MerlinSyntaxParser::HandleDS(const std::string& operand, Section& section,
                                   ConcreteSymbolTable& symbols) {
    std::string op = Trim(operand);

    // Substitute * (program counter) with current address before parsing
    op = SubstituteProgramCounter(op, current_address_);

    uint32_t count = 0;

    if (op.empty()) {
        count = 0;
    }
    else if (op.find('*') != std::string::npos) {
        // Now * can only be multiplication (program counter already substituted)
```

---

## Dependencies and Prerequisites

### Technical Dependencies
```
□ current_address_ member variable - ✅ EXISTS
□ ParseNumber() function - ✅ EXISTS
□ std::ostringstream for hex formatting - ✅ STANDARD LIBRARY
```

### Task Dependencies
```
□ None - can proceed immediately
```

### Knowledge Dependencies
```
□ Understanding of Merlin assembler syntax (documented in contract)
□ Familiarity with HandleDS function (see lines 463-539)
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Breaking multiplication functionality
- **Probability:** Low (mitigation in place)
- **Impact:** High
- **Mitigation:** Test multiplication FIRST (TDD), ensure it passes before fixing
- **Contingency:** Revert changes if multiplication breaks

**Risk 2:** Incorrect address substitution
- **Probability:** Low
- **Impact:** Medium
- **Mitigation:** Unit tests verify exact byte counts
- **Contingency:** Add debug output to verify substitution

**Risk 3:** Edge cases with nested expressions
- **Probability:** Medium
- **Impact:** Low
- **Mitigation:** Out of scope (contract specifies simple expressions only)
- **Contingency:** Document limitation if discovered

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. git restore src/syntax/merlin/merlin_syntax.cpp
2. git restore tests/unit/test_merlin_syntax.cpp
3. cmake --build build && ctest
```

**Rollback Verification:**
```
✓ All tests pass (back to baseline)
✓ Build succeeds
```

---

## Alternative Approaches Considered

### Alternative 1: Modify multiplication check logic
**Approach:** Add special cases to detect when * is program counter
**Pros:**
- Minimal code changes
- No string manipulation

**Cons:**
- Complex logic (check left/right context of *)
- Error-prone (many edge cases)
- Harder to maintain

**Why Not Chosen:**
String substitution is simpler and more maintainable

### Alternative 2: Full expression parser with AST
**Approach:** Build abstract syntax tree for expressions
**Pros:**
- Handles all cases cleanly
- Extensible for future operators

**Cons:**
- Over-engineering for this problem
- Much larger scope
- Breaks "small batch" principle

**Why Not Chosen:**
Out of scope, violates simplicity principle

---

## Success Metrics

### Completion Criteria
```
✓ All new tests passing (3/3)
✓ All existing tests passing
✓ BOOT.S assembles
✓ FRAMEDEF.S assembles
✓ TABLES.S assembles
✓ Code reviewed (follows patterns)
```

### Quality Metrics
```
- Test coverage: 100% of new code
- All 3 POP files assemble without error
- Binary compatibility maintained (if vasm reference available)
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [X] Author: Orchestrator (2026-01-31)
- [ ] Engineer: [Will approve when assigned]

---

## Notes and Considerations

**Key insight:** By substituting * early, we avoid complex logic and leverage existing expression parsing.

**Test strategy:** TDD approach ensures we don't break multiplication while fixing program counter.

**Scope limitation:** Only handles simple expressions (no parentheses, no complex nesting). This is acceptable for Merlin DS directive usage.

---

**Plan Version:** 1.0
**Last Updated:** 2026-01-31
