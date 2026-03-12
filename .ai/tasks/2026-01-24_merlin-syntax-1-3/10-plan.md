# Implementation Plan

**Task ID:** 2026-01-24_merlin-syntax-1-3
**Created:** 2026-01-24
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

This plan implements Merlin Phases 1-3 (Foundation, Local Labels, DUM Blocks) using incremental TDD (RED-GREEN-REFACTOR). We'll create a MerlinSyntax class following the SimpleSyntax pattern, building functionality in small, testable increments.

Phase 1 establishes the foundation: comment parsing, basic line parsing, global labels, and core directives (ORG, EQU, DB, DW, HEX). Phase 2 adds local label scoping (:label), the most-used feature in Prince of Persia (2,077 instances). Phase 3 implements DUM/DEND blocks and ]variable labels (730 instances in PoP).

Each phase follows strict TDD: write failing test (RED), implement minimal code to pass (GREEN), refactor for quality (REFACTOR). All existing tests must continue passing after each commit.

**Key Technical Decisions:**
- **Follow SimpleSyntax pattern** - Use existing parser structure for consistency
- **Leverage existing Symbol table** - No modifications to core classes needed
- **Parse line-by-line** - Match vasm-ext architecture for compatibility
- **Three-tier label system** - Global (std::string), Local (:label map per scope), Variable (]label map)
- **Incremental commits** - Each phase is one commit after all tests pass

**Patterns to Use:**
- **Parser pattern** - ParseLine() delegates to handlers based on directive type
- **Strategy pattern** - Separate handler methods for each directive category
- **State machine** - Track current label scope, DUM block state
- **Factory pattern** - Create appropriate Atom types (Label, Data, Org, etc.)

---

## Critical Files Identified

### Files to Create
```
1. include/xasm++/syntax/merlin_syntax.h (~150 lines)
   - Purpose: MerlinSyntax class declaration
   - Rationale: Public interface for Merlin parser plugin
   - Contents: Class declaration, private state (scopes, DUM state), handler methods

2. src/syntax/merlin/merlin_syntax.cpp (~800-1,000 lines)
   - Purpose: MerlinSyntax implementation
   - Rationale: Core parsing logic, directive handlers
   - Contents: ParseLine(), directive handlers, label management, expression parsing

3. tests/unit/test_merlin_syntax.cpp (~500-800 lines)
   - Purpose: Comprehensive unit tests
   - Rationale: Validate each feature with TDD
   - Contents: ~50-80 tests covering all phases
```

### Files to Modify
```
1. src/CMakeLists.txt (~2 lines added)
   - Current: Builds syntax library with simple_syntax.cpp
   - Changes: Add src/syntax/merlin/merlin_syntax.cpp to xasm_syntax library

2. tests/unit/CMakeLists.txt (~5 lines added)
   - Current: Lists all unit test executables
   - Changes: Add test_merlin_syntax executable with merlin_syntax.cpp
```

### Files to Read (for context)
```
- include/xasm++/syntax/simple_syntax.h - Syntax plugin pattern
- src/syntax/simple_syntax.cpp - Parser implementation reference
- include/xasm++/assembler.h - Assembler API
- include/xasm++/symbol.h - Symbol table interface
- include/xasm++/atom.h - Atom types
- tests/unit/test_cpu6502.cpp - TDD pattern with GoogleTest
- ~/Projects/Vintage/tools/vasm-ext/syntax/merlin/syntax.c - Reference implementation
```

---

## Step-by-Step Implementation Plan

### Phase 1: Foundation

**Goal:** Basic Merlin parsing (comments, labels, core directives)

```
Step 1.1: Create header file (RED phase setup)
  - Action: Create include/xasm++/syntax/merlin_syntax.h with class skeleton
  - Files: merlin_syntax.h
  - Tests: None yet (just structure)
  - Verification: Compiles without errors

Step 1.2: Comment parsing tests (RED)
  - Action: Write failing tests for * and ; comments
  - Files: tests/unit/test_merlin_syntax.cpp
  - Tests:
    - Test_CommentWithAsterisk (RED - no impl yet)
    - Test_CommentWithSemicolon (RED)
    - Test_InlineComment (RED)
  - Verification: Tests compile but fail

Step 1.3: Comment parsing implementation (GREEN)
  - Action: Implement ParseLine() with comment stripping
  - Files: src/syntax/merlin/merlin_syntax.cpp
  - Tests: All comment tests now pass
  - Verification: ctest shows 3/3 passing

Step 1.4: Global label tests (RED)
  - Action: Write failing tests for global labels
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_GlobalLabel (RED)
    - Test_GlobalLabelWithInstruction (RED)
  - Verification: Tests fail (no label parsing yet)

Step 1.5: Global label implementation (GREEN)
  - Action: Implement label parsing in ParseLine()
  - Files: merlin_syntax.cpp
  - Tests: Label tests pass
  - Verification: 5/5 tests passing

Step 1.6: ORG directive tests (RED)
  - Action: Write tests for ORG directive
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_OrgDirective (RED)
    - Test_OrgWithHexValue (RED)
  - Verification: Tests fail

Step 1.7: ORG directive implementation (GREEN)
  - Action: Implement HandleOrg() method
  - Files: merlin_syntax.cpp
  - Tests: ORG tests pass
  - Verification: 7/7 tests passing

Step 1.8: Number format tests (RED)
  - Action: Write tests for $hex, %binary, decimal
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_HexNumber (RED)
    - Test_BinaryNumber (RED)
    - Test_DecimalNumber (RED)
  - Verification: Tests fail

Step 1.9: Number format implementation (GREEN)
  - Action: Implement ParseNumber() helper
  - Files: merlin_syntax.cpp
  - Tests: Number tests pass
  - Verification: 10/10 tests passing

Step 1.10: EQU directive tests (RED)
  - Action: Write tests for EQU
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_EquDirective (RED)
    - Test_EquWithExpression (RED)
  - Verification: Tests fail

Step 1.11: EQU directive implementation (GREEN)
  - Action: Implement HandleEqu()
  - Files: merlin_syntax.cpp
  - Tests: EQU tests pass
  - Verification: 12/12 tests passing

Step 1.12: DB directive tests (RED)
  - Action: Write tests for DB (define byte)
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_DbSingleByte (RED)
    - Test_DbMultipleBytes (RED)
  - Verification: Tests fail

Step 1.13: DB directive implementation (GREEN)
  - Action: Implement HandleDB()
  - Files: merlin_syntax.cpp
  - Tests: DB tests pass
  - Verification: 14/14 tests passing

Step 1.14: DW directive tests (RED)
  - Action: Write tests for DW (define word)
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_DwSingleWord (RED)
    - Test_DwMultipleWords (RED)
  - Verification: Tests fail

Step 1.15: DW directive implementation (GREEN)
  - Action: Implement HandleDW()
  - Files: merlin_syntax.cpp
  - Tests: DW tests pass
  - Verification: 16/16 tests passing

Step 1.16: HEX directive tests (RED)
  - Action: Write tests for HEX (hex literal string)
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_HexDirective (RED)
    - Test_HexWithSpaces (RED)
  - Verification: Tests fail

Step 1.17: HEX directive implementation (GREEN)
  - Action: Implement HandleHex()
  - Files: merlin_syntax.cpp
  - Tests: HEX tests pass
  - Verification: 18/18 tests passing

Step 1.18: REFACTOR Phase 1
  - Action: Clean up code, extract common helpers, add comments
  - Files: merlin_syntax.cpp, merlin_syntax.h
  - Tests: All 18 tests still pass
  - Verification: No regression, code quality improved

Step 1.19: Update CMakeLists.txt
  - Action: Add merlin_syntax.cpp to build
  - Files: src/CMakeLists.txt, tests/unit/CMakeLists.txt
  - Tests: All tests pass after rebuild
  - Verification: cmake --build build succeeds

Step 1.20: Commit Phase 1
  - Action: git add + commit with message
  - Files: All Phase 1 files
  - Tests: All tests passing (18 new + existing)
  - Verification: Clean git state
```

### Phase 2: Local Labels

**Goal:** :LOCAL label scoping

```
Step 2.1: Local label tests (RED)
  - Action: Write tests for :label syntax
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_LocalLabelDefinition (RED)
    - Test_LocalLabelReference (RED)
    - Test_LocalLabelScoping (RED - different scopes)
  - Verification: Tests fail (no local label support)

Step 2.2: Local label data structures
  - Action: Add LabelScope struct to header
  - Files: merlin_syntax.h
  - Tests: Compiles
  - Verification: No errors

Step 2.3: Local label implementation (GREEN)
  - Action: Implement local label parsing in ParseLine()
  - Files: merlin_syntax.cpp
  - Tests: Local label tests pass
  - Verification: 21/21 tests passing

Step 2.4: Forward reference tests (RED)
  - Action: Write tests for forward local label refs
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_LocalLabelForwardReference (RED)
    - Test_LocalLabelBackwardReference (RED)
  - Verification: Tests fail

Step 2.5: Forward reference implementation (GREEN)
  - Action: Implement forward ref resolution
  - Files: merlin_syntax.cpp
  - Tests: Forward ref tests pass
  - Verification: 23/23 tests passing

Step 2.6: Multi-scope tests (RED)
  - Action: Write tests for multiple scopes
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_MultipleScopesWithSameLocalLabel (RED)
    - Test_LocalLabelResetOnGlobal (RED)
  - Verification: Tests fail

Step 2.7: Multi-scope implementation (GREEN)
  - Action: Implement scope reset on global label
  - Files: merlin_syntax.cpp
  - Tests: Scope tests pass
  - Verification: 25/25 tests passing

Step 2.8: REFACTOR Phase 2
  - Action: Clean up scope management code
  - Files: merlin_syntax.cpp
  - Tests: All 25 tests still pass
  - Verification: No regression

Step 2.9: Commit Phase 2
  - Action: git add + commit
  - Files: All Phase 2 changes
  - Tests: All tests passing (25 new + existing)
  - Verification: Clean git state
```

### Phase 3: DUM Blocks & Variables

**Goal:** DUM/DEND and ]variable labels

```
Step 3.1: DUM/DEND tests (RED)
  - Action: Write tests for DUM blocks
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_DumBlock (RED)
    - Test_DumDend (RED)
  - Verification: Tests fail

Step 3.2: DUM block state
  - Action: Add in_dum_block_ flag to header
  - Files: merlin_syntax.h
  - Tests: Compiles
  - Verification: No errors

Step 3.3: DUM/DEND implementation (GREEN)
  - Action: Implement HandleDum() and HandleDend()
  - Files: merlin_syntax.cpp
  - Tests: DUM tests pass
  - Verification: 27/27 tests passing

Step 3.4: ]variable tests (RED)
  - Action: Write tests for ]variable labels
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_VariableLabelInDum (RED)
    - Test_VariableLabelReference (RED)
    - Test_VariableLabelOutsideDum (RED - should error)
  - Verification: Tests fail

Step 3.5: ]variable implementation (GREEN)
  - Action: Implement variable label parsing
  - Files: merlin_syntax.cpp
  - Tests: Variable tests pass
  - Verification: 30/30 tests passing

Step 3.6: DS directive tests (RED)
  - Action: Write tests for DS (define space)
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_DsDirective (RED)
    - Test_DsInDumBlock (RED)
  - Verification: Tests fail

Step 3.7: DS directive implementation (GREEN)
  - Action: Implement HandleDS()
  - Files: merlin_syntax.cpp
  - Tests: DS tests pass
  - Verification: 32/32 tests passing

Step 3.8: Integration test (RED)
  - Action: Write comprehensive test with all features
  - Files: test_merlin_syntax.cpp
  - Tests:
    - Test_CompletePhase3Example (RED)
  - Verification: Test fails

Step 3.9: Integration test (GREEN)
  - Action: Fix any integration issues
  - Files: merlin_syntax.cpp
  - Tests: Integration test passes
  - Verification: 33/33 tests passing

Step 3.10: REFACTOR Phase 3
  - Action: Clean up variable label management
  - Files: merlin_syntax.cpp
  - Tests: All 33 tests still pass
  - Verification: No regression

Step 3.11: Final self-review
  - Action: Review all code against standards
  - Files: All files
  - Tests: All tests passing
  - Verification: Ready for Tester/Reviewer

Step 3.12: Commit Phase 3
  - Action: git add + commit
  - Files: All Phase 3 changes
  - Tests: All tests passing (33 new + all existing)
  - Verification: Clean git state
```

---

## Testing Strategy

### Unit Tests (~33 total)
```
□ Comment parsing (3 tests)
  - * column 1 comment
  - ; inline comment
  - Mixed comments

□ Label parsing (7 tests)
  - Global label
  - :local label
  - ]variable label
  - Label scoping
  - Forward/backward references

□ Directives (15 tests)
  - ORG with different number formats
  - EQU with expressions
  - DB single/multiple bytes
  - DW single/multiple words
  - HEX directive
  - DS directive
  - DUM/DEND blocks

□ Number formats (3 tests)
  - $hex numbers
  - %binary numbers
  - Decimal numbers

□ Integration (5 tests)
  - Complete Phase 1 program
  - Complete Phase 2 program (with locals)
  - Complete Phase 3 program (with DUM)
  - Error cases (invalid syntax)
  - Edge cases (empty lines, whitespace)
```

### Integration Tests
```
□ Phase 1 complete program
  - Setup: Create MerlinSyntax parser
  - Execute: Parse complete program with ORG, labels, DB, DW, HEX
  - Verify: Correct atoms generated, symbols defined

□ Phase 2 with local labels
  - Setup: Create MerlinSyntax parser
  - Execute: Parse program with :local labels in multiple scopes
  - Verify: Correct scoping, unique labels per scope

□ Phase 3 with DUM blocks
  - Setup: Create MerlinSyntax parser
  - Execute: Parse program with DUM/DEND and ]variables
  - Verify: Variables defined correctly, DS allocates space
```

### Acceptance Tests
```
□ Assemble simple Merlin program
  - Given: Fresh MerlinSyntax instance
  - When: Parse basic program (Phase 1 example)
  - Then: Produces correct binary output

□ Assemble with local labels
  - Given: Program with subroutines using :loop
  - When: Parse and assemble
  - Then: Each :loop is unique per subroutine

□ Assemble with DUM blocks
  - Given: Program with zero-page variable definitions
  - When: Parse DUM block
  - Then: ]variables defined at correct addresses
```

### Coverage Target
```
- Overall: ≥85%
- Critical paths (parsing, directives): 95%+
- New code: ≥95%
```

---

## Dependencies and Prerequisites

### Technical Dependencies
```
□ [Library/package name] - [Version] - [Purpose]
□ [Service/API] - [What it provides]
□ [Tool] - [What it's used for]
```

### Task Dependencies
```
□ [Task/feature] must be completed first
□ [Infrastructure] must be ready
□ [Permission/access] must be granted
```

### Knowledge Dependencies
```
□ Understanding of [concept/technology]
□ Familiarity with [pattern/framework]
□ Access to [documentation/resource]
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** [Description]
- **Probability:** [Low/Medium/High]
- **Impact:** [Low/Medium/High]
- **Mitigation:** [Strategy]
- **Contingency:** [Backup plan]

**Risk 2:** [Description]
- **Probability:** [Low/Medium/High]
- **Impact:** [Low/Medium/High]
- **Mitigation:** [Strategy]
- **Contingency:** [Backup plan]

### Integration Risks

**Risk 1:** [Description]
- **Mitigation:** [Strategy]

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. [Step to revert changes]
2. [Step to restore state]
3. [Step to verify rollback]
```

**Rollback Verification:**
```
✓ [System returns to previous state]
✓ [All tests pass]
✓ [No data loss]
✓ [Services operational]
```

### Git Rollback
```
# If changes committed
git revert <commit-hash>

# If not committed
git restore .

# Verify
npm test / pytest / cargo test
```

---

## Performance Considerations

### Expected Performance Impact
```
- [Operation 1]: [Expected impact and why]
- [Operation 2]: [Expected impact and why]
```

### Performance Targets
```
- [Metric 1]: [Target value]
- [Metric 2]: [Target value]
```

### Monitoring Plan
```
□ [Metric to monitor]
□ [Tool/method for monitoring]
□ [Alert threshold]
```

---

## Security Considerations

### Security Checklist
```
□ Input validation implemented
□ Output sanitization applied
□ Authentication checked
□ Authorization verified
□ Sensitive data encrypted
□ Secrets not in code
□ SQL injection prevented
□ XSS prevented
□ CSRF protection (if web)
```

### Security Review Points
```
- [Point 1 requiring security review]
- [Point 2 requiring security review]
```

---

## Alternative Approaches Considered

### Alternative 1: [Name]
**Pros:**
- [Pro 1]
- [Pro 2]

**Cons:**
- [Con 1]
- [Con 2]

**Why Not Chosen:**
[Rationale]

### Alternative 2: [Name]
**Pros:**
- [Pro 1]

**Cons:**
- [Con 1]

**Why Not Chosen:**
[Rationale]

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Phase 1: [X] hours
- Step 1.1: [X] hours
- Step 1.2: [X] hours

Phase 2: [X] hours
- Step 2.1: [X] hours

Phase 3: [X] hours
- Step 3.1: [X] hours

Testing: [X] hours
Documentation: [X] hours

Total: [X] hours
```

---

## Success Metrics

### Completion Criteria
```
✓ All steps completed
✓ All tests passing
✓ Coverage targets met
✓ Performance acceptable
✓ Security validated
✓ Documentation complete
```

### Quality Metrics
```
- Test coverage: [Target %]
- Performance: [Target metrics]
- Code quality: [Linting/review score]
- Security: [No critical issues]
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [X] Author: Orchestrator (2026-01-24)
- [X] User/Stakeholder: Bryan Woodruff (2026-01-24)

**Revision History:**
```
v1.0 - 2026-01-24 - Initial plan
```

---

## Notes and Considerations

**TDD is MANDATORY:**
- Every feature starts with failing test (RED)
- Minimal implementation to pass (GREEN)
- Refactor for quality (REFACTOR)
- All existing tests must continue passing

**Incremental commits:**
- Phase 1: After step 1.20 (Foundation complete)
- Phase 2: After step 2.9 (Local labels complete)
- Phase 3: After step 3.12 (DUM blocks complete)

**Key Success Factors:**
- Follow SimpleSyntax pattern for consistency
- Comprehensive test coverage at each step
- Clean separation: .h declarations, .cpp implementations
- Document design decisions in code comments

---

**Plan Version:** 1.0
**Last Updated:** 2026-01-24

---

## Usage Instructions

This template should be instantiated at: `.ai/tasks/YYYY-MM-DD_task-name/10-plan.md`

**When to create:**
- After contract established
- Before implementation begins
- During planning phase of workflow

**Who creates it:**
- Orchestrator (delegates planning)
- Worker (for assigned tasks)
- Plan agent (for complex tasks)

**Key principles:**
- Be specific and actionable
- Break into small steps
- Consider risks upfront
- Plan for testing
- Document alternatives
- Get approval before implementing
