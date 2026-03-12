# Implementation Plan: Fix A2oSX Critical Bugs

**Task ID:** 2026-02-13_a2osx-critical-bugs
**Beads Task:** xasm++-zmna
**Created:** 2026-02-13
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Fix 3 critical bugs in priority order (BUG-001 P0, then BUG-002/003 P1) using Test-Driven Development with existing test cases from A2oSX validation suite.

**Execution Strategy: Sequential** (dependencies between bugs, token budget risk)

1. **Phase 1 (P0 CRITICAL)**: Fix BUG-001 symbol resolution - blocks everything
2. **Phase 2 (P1 HIGH)**: Fix BUG-002 .BS directive - affects 280 usages
3. **Phase 3 (P1 HIGH)**: Fix BUG-003 expression operators - affects 550 expressions

**Key Technical Decisions:**
- TDD approach: Use existing test_*.S files as validation
- Incremental: Verify each bug fix before proceeding
- Token-aware: Monitor usage, decompose if needed

**Patterns to Use:**
- Symbol table lookup pattern (existing infrastructure)
- Expression operator pattern (extend existing parser)
- Directive handler pattern (fix HandleBs)

---

## Critical Files Identified

### Files to Modify

**Phase 1 - BUG-001 (Symbol Resolution):**
```
1. src/syntax/scmasm_syntax.cpp
   - Current: Operand parsing fails for non-immediate symbols
   - Changes: Fix symbol resolution in all addressing modes

2. src/symbol_table.cpp (if needed)
   - Current: Symbol table exists but may need lookup fixes
   - Changes: Ensure proper symbol resolution API

3. tests/unit/test_scmasm_syntax.cpp
   - Current: May lack symbol resolution tests
   - Changes: Add comprehensive symbol tests
```

**Phase 2 - BUG-002 (.BS Directive):**
```
1. src/syntax/scmasm_directive_handlers.cpp
   - Current: HandleBs treats argument as binary
   - Changes: Parse as decimal/hex byte count, emit N zeros

2. tests/unit/test_scmasm_syntax.cpp
   - Current: May lack .BS tests
   - Changes: Add .BS tests with various sizes
```

**Phase 3 - BUG-003 (Expression Operators):**
```
1. src/expression_parser.cpp
   - Current: Missing <, >, +, - operators
   - Changes: Add operator parsing and evaluation

2. include/xasm++/expression_parser.h
   - Current: May need operator enum updates
   - Changes: Add operator definitions

3. tests/unit/test_expression_parser.cpp
   - Current: Tests for existing operators
   - Changes: Add tests for new operators
```

### Files to Read (for context)
```
- tests/integration/a2osx/BUG-*.md - Detailed bug analysis
- tests/integration/a2osx/test_*.S - Test cases
- src/syntax/merlin_syntax.cpp - Reference implementation
```

---

## Step-by-Step Implementation Plan

### Phase 1: Fix BUG-001 (Symbol Resolution) - P0 CRITICAL

**Goal:** Symbols work in all addressing modes, not just immediate

**Step 1.1: Understand Current Implementation**
  - Action: Read symbol table and operand parsing code
  - Files: src/symbol_table.cpp, src/syntax/scmasm_syntax.cpp
  - Look for: How symbols are resolved, why immediate works but others fail
  - Verification: Understand root cause

**Step 1.2: Create Minimal Reproduction Test**
  - Action: Create unit test that fails with current code
  - File: tests/unit/test_scmasm_syntax.cpp
  - Test: Symbol defined with .EQ used in various addressing modes
  - Verification: Test fails (RED phase)

**Step 1.3: Fix Symbol Resolution**
  - Action: Modify operand parser to resolve symbols
  - File: src/syntax/scmasm_syntax.cpp
  - Changes: 
    - When parsing operand (non-immediate), check symbol table
    - If symbol found, use its value for address
    - Handle zero page vs absolute addressing based on value
  - Verification: Unit test passes (GREEN phase)

**Step 1.4: Run Integration Test**
  - Action: Test with tests/integration/a2osx/test_symbols.S
  - Command: `./build/src/xasm++ --syntax scmasm --cpu 65c02 test_symbols.S -o test.bin`
  - Expected: Assembles successfully, no errors
  - Verification: Integration test passes

**Step 1.5: Run Full Test Suite**
  - Action: Ensure no regressions
  - Command: `cd build && ctest --output-on-failure`
  - Expected: All 1564+ tests passing
  - Verification: Zero regressions (REFACTOR phase complete)

---

### Phase 2: Fix BUG-002 (.BS Directive) - P1 HIGH

**Goal:** .BS N reserves N bytes, not binary parsing

**Step 2.1: Locate HandleBs Function**
  - Action: Find and read current .BS implementation
  - File: src/syntax/scmasm_directive_handlers.cpp
  - Look for: Why it treats argument as binary
  - Verification: Understand current logic

**Step 2.2: Create Test for .BS**
  - Action: Add unit test for .BS directive
  - File: tests/unit/test_scmasm_syntax.cpp
  - Tests:
    - .BS 256 (decimal)
    - .BS $100 (hex)
    - .BS 0 (edge case)
  - Verification: Tests fail (RED phase)

**Step 2.3: Fix HandleBs Implementation**
  - Action: Rewrite HandleBs to reserve bytes
  - File: src/syntax/scmasm_directive_handlers.cpp
  - Changes:
    - Parse operand as numeric value (decimal or hex)
    - Emit N zero bytes (or use space reservation atom)
    - Remove binary digit validation
  - Verification: Unit tests pass (GREEN phase)

**Step 2.4: Run Integration Test**
  - Action: Test with tests/integration/a2osx/test_bs.S
  - Command: `./build/src/xasm++ --syntax scmasm --cpu 65c02 test_bs.S -o test.bin`
  - Expected: Assembles successfully, reserves correct space
  - Verification: Integration test passes

**Step 2.5: Run Full Test Suite**
  - Action: Ensure no regressions
  - Expected: All tests passing
  - Verification: Zero regressions

---

### Phase 3: Fix BUG-003 (Expression Operators) - P1 HIGH

**Goal:** Add <, >, +, - operators for expression evaluation

**Step 3.1: Understand Expression Parser**
  - Action: Read expression parser implementation
  - Files: src/expression_parser.cpp, include/xasm++/expression_parser.h
  - Look for: How operators are defined and evaluated
  - Verification: Understand operator infrastructure

**Step 3.2: Create Tests for New Operators**
  - Action: Add unit tests for <, >, +, - operators
  - File: tests/unit/test_expression_parser.cpp
  - Tests:
    - <ADDR extracts low byte
    - >ADDR extracts high byte
    - ADDR+10 performs addition
    - ADDR-10 performs subtraction
  - Verification: Tests fail (RED phase)

**Step 3.3: Add Operator Definitions**
  - Action: Add operator enums/tokens
  - File: include/xasm++/expression_parser.h
  - Changes: Add LO_BYTE, HI_BYTE, PLUS, MINUS operators
  - Verification: Compiles cleanly

**Step 3.4: Implement Operator Parsing**
  - Action: Extend parser to recognize new operators
  - File: src/expression_parser.cpp
  - Changes:
    - Recognize < and > as prefix operators
    - Recognize + and - as infix operators
    - Add precedence rules
  - Verification: Parser recognizes operators

**Step 3.5: Implement Operator Evaluation**
  - Action: Add evaluation logic for new operators
  - File: src/expression_parser.cpp
  - Changes:
    - <VALUE: return VALUE & 0xFF
    - >VALUE: return (VALUE >> 8) & 0xFF
    - A + B: return A + B
    - A - B: return A - B
  - Verification: Unit tests pass (GREEN phase)

**Step 3.6: Run Integration Tests**
  - Action: Test with tests/integration/a2osx/test_expr_*.S
  - Command: `./build/src/xasm++ --syntax scmasm --cpu 65c02 test_expr_lo.S -o test.bin`
  - Expected: All expression tests assemble successfully
  - Verification: Integration tests pass

**Step 3.7: Run Full Test Suite**
  - Action: Ensure no regressions
  - Expected: All tests passing
  - Verification: Zero regressions

---

## Testing Strategy

### Unit Tests
```
□ Symbol Resolution - Test cases:
  - Symbol in zero page addressing
  - Symbol in absolute addressing
  - Symbol in indexed addressing (,X and ,Y)
  - Symbol in indirect addressing
  - Symbol not found (error case)

□ .BS Directive - Test cases:
  - .BS with decimal number
  - .BS with hex number
  - .BS 0 (edge case)
  - .BS with large number (1024)

□ Expression Operators - Test cases:
  - < operator on 16-bit address
  - > operator on 16-bit address
  - + operator with offset
  - - operator with subtraction
  - Combined expressions (<ADDR+10)
```

### Integration Tests
```
□ test_symbols.S - Symbol resolution in real assembly
□ test_bs.S - .BS directive with various sizes
□ test_expr_lo.S - Low byte extraction
□ test_expr_hi.S - High byte extraction
□ test_expr_offset.S - Addition/subtraction
```

### Regression Tests
```
□ All 1564+ existing tests must pass
□ Other SCMASM features unaffected
□ Other syntax parsers (Merlin, etc.) unaffected
```

---

## Token Budget Monitoring

**Estimated: 30,000 tokens (HIGH RISK)**

**Checkpoints:**
- After Phase 1: If >15K tokens used, consider decomposing
- After Phase 2: If >25K tokens used, MUST decompose Phase 3
- Monitor agent output size continuously

**Decomposition Plan (if needed):**
- **Subtask 1**: BUG-001 only (current task packet)
- **Subtask 2**: BUG-002 + BUG-003 (new task packet)

---

## Verification Checklist

### Phase 1 (BUG-001)
- [ ] Root cause identified
- [ ] Unit tests added
- [ ] Symbol resolution fixed
- [ ] test_symbols.S passes
- [ ] All existing tests passing

### Phase 2 (BUG-002)
- [ ] HandleBs implementation understood
- [ ] Unit tests added
- [ ] .BS directive fixed
- [ ] test_bs.S passes
- [ ] All existing tests passing

### Phase 3 (BUG-003)
- [ ] Expression parser understood
- [ ] Unit tests added
- [ ] Operators implemented (<, >, +, -)
- [ ] test_expr_*.S all pass
- [ ] All existing tests passing

### Final Verification
- [ ] All 3 bugs fixed
- [ ] A2oSX validation suite updated
- [ ] All 1564+ tests passing
- [ ] Zero regressions
- [ ] Zero new warnings
- [ ] Documentation updated

---

## Success Criteria

- BUG-001 fixed: Symbols work in all addressing modes
- BUG-002 fixed: .BS reserves N bytes correctly
- BUG-003 fixed: Expression operators <, >, +, - functional
- All A2oSX validation tests passing (11/11)
- All existing tests passing (1564+)

---

**Plan approved by:** Orchestrator
**Ready for execution by:** Engineer
**Estimated effort:** 8-12 hours (2-4 hours per bug)
**Token budget:** 30K (HIGH RISK - monitor and decompose if needed)
