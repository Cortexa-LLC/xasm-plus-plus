# Implementation Plan

**Task ID:** 2026-02-11_god-class-phase6c3-edtasm
**Created:** 2026-02-11
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Extract ~20 unique EDTASM-M80++ directive handlers from the 1797-line edtasm_m80_plusplus_syntax.cpp file, following the proven Phase 6b pattern. Group handlers into logical categories (segment control, conditional assembly, macro system, listing control) for better maintainability. Eliminate all magic numbers and strings using named constants from directive_constants.h and opcodes_z80.h.

The extraction will reduce the main parser file from 1797 lines to ~400-600 lines (just registration code), while creating a new edtasm_directive_handlers.cpp file (~1000-1200 lines) containing the extracted handler logic. All handlers will be registered with DirectiveRegistry maintaining identical behavior.

**Key Technical Decisions:**
- Follow Phase 6b pattern exactly: Free functions + DirectiveHandler wrappers
- Group handlers by category for readability and maintenance
- Use Z80Directives namespace for existing constants
- Extend directive_constants.h with EDTASM-specific directive names
- Handle stateful handlers (macros, conditionals) by passing parser reference

**Patterns to Use:**
- Free function pattern - Handler logic as free functions
- Wrapper pattern - DirectiveHandler signature wrappers
- Registry pattern - Register with DirectiveRegistry
- Constant extraction - Named constexpr constants

---

## Critical Files Identified

### Files to Modify
```
1. src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.cpp
   - Current: 1797 lines with all handlers inline as lambdas
   - Changes: Remove handler implementations, keep only registrations

2. src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.h
   - Current: Parser class declaration
   - Changes: Add forward declarations for handler functions

3. include/xasm++/directives/directive_constants.h
   - Current: 5 core directive names (from Phase 6b)
   - Changes: Add ~20 EDTASM-specific directive names as constexpr

4. tests/test_edtasm_directives.cpp
   - Current: Existing tests
   - Changes: Update tests to use new handlers, verify behavior unchanged
```

### Files to Create
```
1. src/syntax/edtasm_m80_plusplus/edtasm_directive_handlers.cpp
   - Purpose: Implement all EDTASM directive handler functions
   - Rationale: Separate handler logic from parser registration

2. src/syntax/edtasm_m80_plusplus/edtasm_directive_handlers.h
   - Purpose: Declare handler functions and wrappers
   - Rationale: Clean interface for registration
```

### Files to Read (for context)
```
- .ai/tasks/2026-02-10_god-class-phase6b-core/ENGINEER-REPORT.md - Pattern reference
- include/xasm++/cpu/opcodes_z80.h - Z80 opcodes for eliminating magic numbers
- include/xasm++/directives/z80_directives.h - Existing Z80 directive constants
- src/syntax/core_directive_handlers.cpp - Phase 6b implementation example
```

---

## Step-by-Step Implementation Plan

### Phase 1: Setup and Preparation (TDD: RED)
```
Step 1.1: Create new files with skeleton structure
  - Action: Create edtasm_directive_handlers.h/cpp with headers
  - Files: New handler files
  - Tests: Compile test (files build)
  - Verification: Project builds successfully

Step 1.2: Extend directive_constants.h with EDTASM directive names
  - Action: Add constexpr for all EDTASM directives
  - Files: directive_constants.h
  - Tests: Compile test
  - Verification: Constants accessible

Step 1.3: Write failing tests for first handler category (Segment Control)
  - Action: Write tests for ASEG, CSEG, DSEG
  - Files: test_edtasm_directives.cpp
  - Tests: Tests exist but fail (handlers not extracted yet)
  - Verification: Tests fail with expected error
```

### Phase 2: Extract Segment Control Handlers (TDD: GREEN)
```
Step 2.1: Extract ASEG handler
  - Action: Move ASEG lambda to free function + wrapper
  - Files: edtasm_directive_handlers.cpp/h, edtasm_m80_plusplus_syntax.cpp
  - Tests: ASEG tests should pass
  - Verification: ASEG directive works identically

Step 2.2: Extract CSEG handler
  - Action: Move CSEG lambda to free function + wrapper
  - Files: Handler files
  - Tests: CSEG tests should pass
  - Verification: CSEG directive works identically

Step 2.3: Extract DSEG handler
  - Action: Move DSEG lambda to free function + wrapper
  - Files: Handler files
  - Tests: DSEG tests should pass
  - Verification: All segment control tests passing
```

### Phase 3: Extract Conditional Assembly Handlers (TDD: RED-GREEN)
```
Step 3.1: Write failing tests for conditional directives
  - Action: Tests for IF, ELSE, ENDIF, IFDEF, IFNDEF, etc.
  - Files: test_edtasm_directives.cpp
  - Tests: Tests fail (not extracted yet)
  - Verification: Tests exist

Step 3.2: Extract IF/ELSE/ENDIF handlers
  - Action: Move conditional block handlers
  - Files: Handler files
  - Tests: Basic conditional tests pass
  - Verification: IF blocks work

Step 3.3: Extract comparison conditional handlers
  - Action: Move IFDEF, IFNDEF, IFEQ, IFNE, IFLT, IFGT, IFLE, IFGE
  - Files: Handler files
  - Tests: Comparison tests pass
  - Verification: All comparison conditionals work

Step 3.4: Extract special conditional handlers
  - Action: Move IF1, IF2, IFB, IFNB, IFIDN, IFDIF
  - Files: Handler files
  - Tests: Special conditional tests pass
  - Verification: All conditionals working
```

### Phase 4: Extract Macro System Handlers (TDD: RED-GREEN)
```
Step 4.1: Write failing tests for macro system
  - Action: Tests for MACRO, ENDM, LOCAL, EXITM, REPT, IRP, IRPC
  - Files: test_edtasm_directives.cpp
  - Tests: Tests fail
  - Verification: Tests exist

Step 4.2: Extract MACRO/ENDM/LOCAL/EXITM handlers
  - Action: Move macro definition/control handlers
  - Files: Handler files (handle parser state access)
  - Tests: Macro definition tests pass
  - Verification: Macro definition/expansion works

Step 4.3: Extract REPT/IRP/IRPC handlers
  - Action: Move repeat block handlers
  - Files: Handler files (handle parser state access)
  - Tests: Repeat block tests pass
  - Verification: All repeat constructs work
```

### Phase 5: Extract Listing Control Handlers (TDD: RED-GREEN)
```
Step 5.1: Write failing tests for listing control
  - Action: Tests for LIST, NOLIST, *LIST, LALL, SALL
  - Files: test_edtasm_directives.cpp
  - Tests: Tests fail
  - Verification: Tests exist

Step 5.2: Extract LIST family handlers
  - Action: Move LIST, NOLIST/XLIST, *LIST handlers
  - Files: Handler files
  - Tests: Listing control tests pass
  - Verification: Listing control works

Step 5.3: Extract LALL/SALL handlers
  - Action: Move listing expansion control handlers
  - Files: Handler files
  - Tests: Expansion control tests pass
  - Verification: Macro expansion control works

Step 5.4: Extract formatting directive handlers
  - Action: Move TITLE, SUBTTL, EJECT, SPACE, NAME, RADIX handlers
  - Files: Handler files
  - Tests: Formatting tests pass
  - Verification: All listing directives work
```

### Phase 6: Extract Remaining Handlers (TDD: RED-GREEN)
```
Step 6.1: Extract END handler
  - Action: Move END directive handler
  - Files: Handler files
  - Tests: END directive test passes
  - Verification: END works correctly
```

### Phase 7: Refactor and Clean Up (TDD: REFACTOR)
```
Step 7.1: Eliminate all magic numbers
  - Action: Replace hex values with opcodes_z80.h constants
  - Files: Handler files
  - Tests: All tests still pass
  - Verification: Zero magic numbers remain

Step 7.2: Eliminate all magic strings
  - Action: Replace string literals with directive_constants.h
  - Files: Handler files
  - Tests: All tests still pass
  - Verification: Zero magic strings remain

Step 7.3: Verify edtasm_m80_plusplus_syntax.cpp reduced
  - Action: Confirm file is ~400-600 lines (from 1797)
  - Files: Parser file
  - Tests: All tests still pass
  - Verification: File size significantly reduced

Step 7.4: Document handlers
  - Action: Add function documentation
  - Files: Handler files
  - Tests: Documentation complete
  - Verification: All handlers documented
```

---

## Testing Strategy

### Unit Tests
```
□ Segment Control - Test cases:
  - ASEG switches to absolute segment
  - CSEG switches to code segment
  - DSEG switches to data segment
  - Segment addresses track correctly

□ Conditional Assembly - Test cases:
  - IF/ELSE/ENDIF blocks evaluate correctly
  - IFDEF/IFNDEF detect symbol existence
  - IFEQ/IFNE compare values
  - IFLT/IFGT/IFLE/IFGE compare numerically
  - IF1/IF2 detect assembly pass
  - IFB/IFNB detect blank arguments
  - IFIDN/IFDIF compare strings
  - Nested conditionals work

□ Macro System - Test cases:
  - MACRO/ENDM define macros
  - LOCAL creates local symbols
  - EXITM exits macro early
  - REPT repeats block N times
  - IRP iterates over list
  - IRPC iterates over string
  - Macro parameters substitute correctly
  - Nested macros work

□ Listing Control - Test cases:
  - LIST enables listing
  - NOLIST/XLIST disable listing
  - *LIST ON/OFF toggle listing
  - LALL shows macro expansion
  - SALL hides macro expansion
  - TITLE sets listing title
  - SUBTTL sets subtitle
  - EJECT forces page break
  - SPACE adds blank lines
  - NAME sets module name
  - RADIX sets number base

□ Other - Test cases:
  - END terminates assembly
  - All directives register correctly
  - Multiple directive names map to same handler
```

### Integration Tests
```
□ Full EDTASM-M80++ program assembly
  - Setup: Create test .asm file with all directives
  - Execute: Assemble with EDTASM-M80++ parser
  - Verify: Output identical to pre-extraction

□ Macro expansion with conditionals
  - Setup: Complex macro with IF/ELSE inside
  - Execute: Expand macro
  - Verify: Conditional logic works in macro context

□ Nested repeat blocks
  - Setup: REPT inside IRP
  - Execute: Assemble
  - Verify: Correct nested expansion
```

### Acceptance Tests
```
□ Handler extraction complete
  - Given: All directives extracted
  - When: Run full test suite
  - Then: 97%+ tests passing

□ No magic numbers/strings
  - Given: All handlers extracted
  - When: Grep for magic patterns
  - Then: Zero magic numbers/strings found

□ File size reduced
  - Given: Extraction complete
  - When: Check edtasm_m80_plusplus_syntax.cpp
  - Then: File reduced from 1797 to ~400-600 lines
```

### Coverage Target
```
- Overall: 97%+
- Directive handlers: 95%+
- New handler functions: 90%+
```

---

## Dependencies and Prerequisites

### Technical Dependencies
```
□ DirectiveRegistry - Registration system
□ DirectiveContext - Context parameter type
□ Z80Directives namespace - Existing constants
□ directive_constants.h - Core constants from Phase 6b
```

### Task Dependencies
```
□ Phase 6b (xasm++-uqyd) - COMPLETE ✓
  - Established pattern
  - Created directive_constants.h
  - Proved DirectiveRegistry approach
```

### Knowledge Dependencies
```
□ Understanding of EDTASM-M80++ syntax
□ Familiarity with Z80 assembly
□ Knowledge of macro assembler concepts
□ Phase 6b pattern understanding
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Stateful handlers (macros, conditionals) difficult to extract
- **Probability:** Medium
- **Impact:** Medium (requires pattern adjustment)
- **Mitigation:** Pass parser reference via DirectiveContext or make handlers friends
- **Contingency:** Introduce EdtasmHandlerContext with needed state

**Risk 2:** Token limit exceeded (~30K tokens estimated)
- **Probability:** Medium
- **Impact:** Medium (blocks completion)
- **Mitigation:** Extract handlers incrementally, commit often
- **Contingency:** Complete core handlers first, remainder in follow-up

**Risk 3:** Tests break due to subtle behavior changes
- **Probability:** Low
- **Impact:** High (requires debugging)
- **Mitigation:** Extract one handler at a time, test immediately
- **Contingency:** Revert extraction, analyze difference, retry

### Integration Risks

**Risk 1:** Multiple directive names sharing handlers may break
- **Mitigation:** Test all aliases for each handler

**Risk 2:** Macro expansion state may not transfer cleanly
- **Mitigation:** Review macro state access carefully, maintain exact behavior

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. git revert <commit-hash> (or git restore . if uncommitted)
2. Verify tests pass
3. Verify edtasm_m80_plusplus_syntax.cpp restored to 1797 lines
```

**Rollback Verification:**
```
✓ All tests passing (97%+)
✓ Parser file back to original size
✓ Build succeeds
✓ No extraction artifacts remain
```

---

## Performance Considerations

### Expected Performance Impact
```
- Compilation time: Minimal impact (separate file compilation)
- Runtime performance: Zero impact (same code, different location)
- Binary size: Identical (no code changes, just reorganization)
```

### Performance Targets
```
- Assembly performance: No regression
- Test suite performance: No regression
```

---

## Alternative Approaches Considered

### Alternative 1: Extract to multiple category-specific files
**Pros:**
- Smaller files, easier to navigate
- Clear separation by directive category

**Cons:**
- More files to manage
- More complex build configuration
- Overkill for ~20 handlers

**Why Not Chosen:**
Single edtasm_directive_handlers.cpp with clear section comments is sufficient for ~20 handlers. Can refactor later if file grows.

### Alternative 2: Keep handlers inline, just extract helper functions
**Pros:**
- Minimal change to registration
- Less refactoring needed

**Cons:**
- Doesn't achieve god-class reduction goal
- Doesn't match Phase 6b pattern
- Still leaves large inline lambdas

**Why Not Chosen:**
Phase 6b established the pattern for full extraction. Consistency across parsers is important.

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Phase 1 (Setup): 0.5 hours
- File creation, constants, initial tests

Phase 2 (Segment Control): 0.5 hours
- 3 simple handlers

Phase 3 (Conditional Assembly): 1.5 hours
- ~17 conditional handlers (many similar)

Phase 4 (Macro System): 1.5 hours
- 7 handlers with complex state

Phase 5 (Listing Control): 1.0 hours
- ~11 handlers (mostly simple)

Phase 6 (Remaining): 0.5 hours
- 1 handler (END)

Phase 7 (Cleanup): 1.0 hours
- Magic elimination, documentation

Total: 6.5 hours
```

---

## Success Metrics

### Completion Criteria
```
✓ All ~20 unique handlers extracted
✓ All tests passing (97%+)
✓ Zero magic numbers/strings
✓ edtasm_m80_plusplus_syntax.cpp reduced to ~400-600 lines
✓ All 47 directive registrations still work
✓ Documentation complete
```

### Quality Metrics
```
- Test coverage: 97%+
- Handler coverage: 95%+
- Magic number count: 0
- Magic string count: 0
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [X] Author: Orchestrator [2026-02-11]
- [ ] Engineer: [Name] [Date]

---

## Notes and Considerations

**Handler State Access:**

Many EDTASM handlers access parser state:
- in_repeat_block_, repeat_param_, repeat_values_, repeat_body_ (for REPT/IRP/IRPC)
- in_conditional_block_, conditional_stack_ (for IF/ELSE/ENDIF)
- macro_table_ (for MACRO/ENDM)
- listing_enabled_ (for LIST/NOLIST)

**Recommended approach:** Pass parser reference via DirectiveContext or make EdtasmM80PlusPlusSyntaxParser a friend class of handler functions.

**Multiple Names Per Handler:**

Several handlers are registered with multiple names:
- LIST, ".LIST" → same handler
- NOLIST, XLIST, ".XLIST" → same handler
- etc.

Verify all aliases still work after extraction.

**Z80-Specific:**

EDTASM-M80++ is Z80-only, so can use opcodes_z80.h directly without CPU-specific checks.

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-11
