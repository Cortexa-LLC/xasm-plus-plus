# Implementation Plan

**Task ID:** 2026-02-10_scmasm-directive-registry
**Created:** 2026-02-10
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Apply the DirectiveRegistry pattern to SCMASM parser to replace the inline if/else directive dispatch with a registry-based approach. This refactoring improves extensibility and maintainability while following the established pattern already successfully implemented in Merlin and SimpleSyntax parsers.

The implementation will:
1. Add DirectiveRegistry member to ScmasmSyntaxParser class
2. Create InitializeDirectiveRegistry() method to register all ~20 directives
3. Refactor ParseLine() to use registry lookup instead of if/else chains
4. Maintain all existing functionality (no behavioral changes)

**Key Technical Decisions:**
- **Follow established pattern**: Use same DirectiveRegistry approach as Merlin/SimpleSyntax for consistency
- **Preserve behavior**: All existing tests must pass without modification
- **Lambda-based handlers**: Use lambda functions for directive implementations (matches current pattern)

**Patterns to Use:**
- **Registry Pattern** - Directive dispatch via std::unordered_map lookup (O(1))
- **Strategy Pattern** - Each directive handler is a std::function callback

---

## Critical Files Identified

### Files to Modify
```
1. include/xasm++/syntax/scmasm_syntax.h
   - Current: Parser class without DirectiveRegistry
   - Changes: Add directive_registry_ member, add InitializeDirectiveRegistry() declaration

2. src/syntax/scmasm/scmasm_syntax.cpp
   - Current: ParseLine() uses if/else chain for ~20 directives (.OR, .EQ, .SE, .AS, .AT, .AZ, .DA, .HS, .BS, .MA, .EM, .DO, .LU, etc.)
   - Changes: 
     - Implement InitializeDirectiveRegistry() to register all directives
     - Refactor ParseLine() to use directive_registry_.Execute()
     - Call InitializeDirectiveRegistry() from constructor

3. tests/unit/test_scmasm_syntax.cpp
   - Current: Existing tests for all directive functionality
   - Changes: None (tests should pass without modification - behavioral compatibility)
```

### Files to Read (for context)
```
- src/syntax/merlin/merlin_syntax.cpp - Reference implementation (29 directives registered)
- src/syntax/simple_syntax.cpp - Simple reference implementation (3 directives)
- include/xasm++/syntax/directive_registry.h - DirectiveRegistry API
- include/xasm++/directives/scmasm_directives.h - Directive name constants
```

---

## Step-by-Step Implementation Plan

### Phase 1: Add DirectiveRegistry Infrastructure
```
Step 1.1: Add includes and member to header
  - Action: Add #include "xasm++/syntax/directive_registry.h" to scmasm_syntax.h
  - Action: Add directive_registry_ member (private)
  - Action: Add InitializeDirectiveRegistry() method declaration (private)
  - Files: include/xasm++/syntax/scmasm_syntax.h
  - Tests: Build should compile
  - Verification: No compiler errors

Step 1.2: Add namespace using directive
  - Action: Add "using namespace SCMASMDirectives;" to scmasm_syntax.cpp
  - Files: src/syntax/scmasm/scmasm_syntax.cpp
  - Tests: Build should compile
  - Verification: Directive constants accessible
```

### Phase 2: Implement InitializeDirectiveRegistry()
```
Step 2.1: Create registry method skeleton
  - Action: Implement InitializeDirectiveRegistry() at end of .cpp file
  - Action: Call from constructor (after current_line_ = 0;)
  - Files: src/syntax/scmasm/scmasm_syntax.cpp
  - Tests: Build should compile
  - Verification: Constructor calls initialization

Step 2.2: Register Core Directives (.OR, .EQ, .SE)
  - Action: Register .OR with lambda calling HandleOr()
  - Action: Register .EQ with lambda calling HandleEq()
  - Action: Register .SE with lambda calling HandleSe()
  - Files: src/syntax/scmasm/scmasm_syntax.cpp
  - Tests: Existing .OR/.EQ/.SE tests should pass
  - Verification: Core directives work via registry

Step 2.3: Register String Directives (.AS, .AT, .AZ)
  - Action: Register .AS with lambda calling HandleAs()
  - Action: Register .AT with lambda calling HandleAt()
  - Action: Register .AZ with lambda calling HandleAz()
  - Files: src/syntax/scmasm/scmasm_syntax.cpp
  - Tests: String directive tests should pass
  - Verification: String directives work

Step 2.4: Register Data Directives (.DA, .HS, .BS, .DFB)
  - Action: Register .DA with lambda calling HandleDa()
  - Action: Register .DFB as alias for .DA
  - Action: Register .HS with lambda calling HandleHs()
  - Action: Register .BS with lambda calling HandleBs()
  - Files: src/syntax/scmasm/scmasm_syntax.cpp
  - Tests: Data directive tests should pass
  - Verification: Data directives work

Step 2.5: Register Macro Directives (.MA, .EM)
  - Action: Register .MA with lambda calling HandleMa()
  - Action: Register .EM with lambda calling HandleEm()
  - Files: src/syntax/scmasm/scmasm_syntax.cpp
  - Tests: Macro tests should pass
  - Verification: Macros work

Step 2.6: Register Conditional Directives (.DO, .ELSE, .FIN)
  - Action: Register .DO with lambda calling HandleDo()
  - Action: Register .ELSE and .FIN with error-throwing lambdas (they're handled by .DO)
  - Files: src/syntax/scmasm/scmasm_syntax.cpp
  - Tests: Conditional tests should pass
  - Verification: Conditionals work

Step 2.7: Register Loop Directives (.LU, .ENDU)
  - Action: Register .LU with lambda calling HandleLu()
  - Action: Register .ENDU with error-throwing lambda (handled by .LU)
  - Files: src/syntax/scmasm/scmasm_syntax.cpp
  - Tests: Loop tests should pass
  - Verification: Loops work
```

### Phase 3: Refactor ParseLine() Directive Dispatch
```
Step 3.1: Replace if/else chain with registry lookup
  - Action: In ParseLine(), after extracting opcode_upper, check if registered
  - Action: If registered, call directive_registry_.Execute() with DirectiveContext
  - Action: Keep legacy if/else chain as fallback initially (for testing)
  - Files: src/syntax/scmasm/scmasm_syntax.cpp
  - Tests: All directive tests should pass
  - Verification: Registry dispatch works

Step 3.2: Remove legacy if/else chain
  - Action: Delete the large if/else chain (lines ~290-340)
  - Action: Replace with registry.Execute() or error for unknown directive
  - Files: src/syntax/scmasm/scmasm_syntax.cpp
  - Tests: All tests should still pass
  - Verification: No regression

Step 3.3: Final testing
  - Action: Run full test suite
  - Action: Verify zero warnings
  - Files: All
  - Tests: ctest --output-on-failure
  - Verification: 100% test pass rate
```

---

## Testing Strategy

### Verification Approach
```
✓ No new tests needed - using existing tests for verification
✓ Behavioral compatibility: All existing tests must pass without modification
✓ This is a refactoring - behavior should be identical before and after
```

### Test Execution Plan
```
1. Before changes: Run tests and capture baseline
   - ctest -R scmasm --output-on-failure
   - Record test count and pass/fail status

2. After each phase: Run tests to verify no regression
   - Phase 1: Verify build
   - Phase 2: Verify each directive group works
   - Phase 3: Verify final refactoring

3. Final validation: Full test suite
   - ctest --output-on-failure
   - Verify same test count
   - Verify all tests pass
```

### Coverage Expectations
```
- No coverage change expected (same code paths exercised)
- Existing coverage should be maintained
- No new uncovered code introduced
```

---

## Dependencies and Prerequisites

### Technical Dependencies
```
✓ DirectiveRegistry class already exists (include/xasm++/syntax/directive_registry.h)
✓ SCMASM directive constants defined (include/xasm++/directives/scmasm_directives.h)
✓ Existing HandleXxx() methods work correctly
✓ DirectiveContext struct defined
```

### Task Dependencies
```
✓ No blocking dependencies
✓ Can proceed immediately
```

### Knowledge Dependencies
```
✓ Reference implementations available (Merlin, SimpleSyntax)
✓ Pattern well-established
✓ Engineer can follow template
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Breaking existing functionality during refactoring
- **Probability:** Low
- **Impact:** High
- **Mitigation:** 
  - Keep Handle methods unchanged (only refactor dispatch)
  - Run tests after each phase
  - Use existing implementations as reference
- **Contingency:** Git revert to last working state

**Risk 2:** Directive signature mismatch with DirectiveContext
- **Probability:** Low
- **Impact:** Medium
- **Mitigation:**
  - Review DirectiveRegistry::Execute() signature
  - Match DirectiveContext parameter passing
  - Reference Merlin implementation
- **Contingency:** Adapt DirectiveContext as needed

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. git restore include/xasm++/syntax/scmasm_syntax.h
2. git restore src/syntax/scmasm/scmasm_syntax.cpp
3. Rebuild: cmake --build build
4. Test: ctest -R scmasm
```

**Rollback Verification:**
```
✓ All tests pass
✓ Build succeeds
✓ Zero warnings
```

---

## Performance Considerations

### Expected Performance Impact
```
- Directive lookup: O(n) if/else → O(1) hash map
- Performance improvement expected (but negligible for assembler)
- No performance regression possible
```

### Performance Targets
```
- Same or better performance than current if/else chain
- No memory overhead concerns (single registry instance)
```

---

## Alternative Approaches Considered

### Alternative 1: Keep if/else chain
**Pros:**
- No changes needed
- Works currently

**Cons:**
- Hard to extend
- High cyclomatic complexity
- Inconsistent with other parsers (Merlin, SimpleSyntax)
- O(n) lookup instead of O(1)

**Why Not Chosen:**
Task explicitly requires DirectiveRegistry pattern for consistency and maintainability.

### Alternative 2: Virtual method dispatch
**Pros:**
- Object-oriented approach
- Polymorphic design

**Cons:**
- More complex
- Requires separate handler classes
- Overkill for this use case
- Inconsistent with established pattern

**Why Not Chosen:**
DirectiveRegistry with lambdas is the established pattern, simpler, and equally effective.

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Phase 1: 0.5 hours
- Add includes and member: 15 min
- Add namespace using: 15 min

Phase 2: 1.5 hours
- Skeleton method: 15 min
- Core directives: 15 min
- String directives: 15 min
- Data directives: 15 min
- Macro directives: 15 min
- Conditional directives: 15 min
- Loop directives: 15 min

Phase 3: 1.0 hour
- Refactor dispatch: 30 min
- Remove legacy chain: 15 min
- Final testing: 15 min

Total: 3.0 hours
```

---

## Success Metrics

### Completion Criteria
```
✓ All ~20 directives registered in DirectiveRegistry
✓ ParseLine() uses registry lookup
✓ All existing tests passing
✓ Zero compilation warnings
✓ Build succeeds
✓ Code follows established pattern (matches Merlin/SimpleSyntax)
```

### Quality Metrics
```
- Test pass rate: 100% (same as before)
- Compilation warnings: 0
- Code duplication: Reduced (centralized dispatch)
- Maintainability: Improved (easy to add new directives)
```

---

## Plan Approval

**Plan Status:** Approved (Orchestrator self-approval for delegation)

**Reviewed By:**
- [X] Author: Orchestrator [2026-02-10]

**Revision History:**
```
v1.0 - 2026-02-10 - Initial plan
```

---

## Notes and Considerations

**Implementation Notes:**
- Follow Merlin implementation as primary reference (most complete)
- SimpleSyntax implementation is simpler but less comprehensive
- DirectiveContext struct includes: label, operand, Section&, ConcreteSymbolTable&
- Handle methods already have correct signatures - just need to wrap in lambdas
- .ELSE, .FIN, .ENDU are terminator directives - handled by their opening directives

**Testing Strategy:**
- This is a refactoring task - behavioral compatibility is critical
- No new tests needed - existing tests validate correctness
- If any test fails, implementation is incorrect

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-10
