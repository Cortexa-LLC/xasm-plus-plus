# Implementation Plan: Directive Handler Migration

**Task ID:** 2026-02-09_god-class-phase6-directive
**Created:** 2026-02-10
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Migrate parsers to use the existing DirectiveRegistry infrastructure with DirectiveContext signature. This completes the partial migration started in earlier phases.

**Current Situation:**
- DirectiveRegistry class exists with new `DirectiveContext` signature
- Parsers still use old local `directive_registry_` with old signature
- Need to migrate parsers to use centralized DirectiveRegistry

**Migration Strategy:**
- Incremental: One parser at a time (Merlin first as proof of concept)
- Pattern: Replace local registry with DirectiveRegistry integration
- Signature: Adapt old handler calls to use DirectiveContext

**Key Technical Decisions:**
1. **Use existing DirectiveRegistry** - No new abstraction needed
   - Rationale: Infrastructure already exists and well-designed
   
2. **DirectiveContext signature** - Migrate to new signature
   - Rationale: This is the approved pattern from ADR-002
   
3. **Adapter pattern for migration** - Create helper to convert contexts
   - Rationale: Enables incremental migration without breaking tests

**Patterns to Use:**
- **Adapter Pattern** - Convert DirectiveContext ↔ (Section&, SymbolTable&)
- **Registry Pattern** - Use existing DirectiveRegistry for dispatch
- **Incremental Migration** - One parser at a time, TDD verification

---

## Critical Files Identified

### Files to Modify

1. **src/syntax/merlin/merlin_syntax.cpp** (FIRST - proof of concept)
   - Current: Uses local `directive_registry_` with old signature
   - Changes: Integrate with DirectiveRegistry, migrate handlers
   
2. **include/xasm++/syntax/merlin_syntax.h**
   - Current: Defines local DirectiveHandler type and registry
   - Changes: Remove local registry, use global DirectiveRegistry

3. **src/syntax/scmasm/scmasm_syntax.cpp** (SECOND - apply pattern)
   - Current: Uses local directive handling
   - Changes: Apply same migration pattern as Merlin

4. **include/xasm++/syntax/scmasm_syntax.h**
   - Changes: Remove local registry references

5. **src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.cpp** (THIRD)
   - Changes: Apply migration pattern

6. **include/xasm++/syntax/edtasm_m80_plusplus_syntax.h**
   - Changes: Remove local registry references

### Files to Read (for context)

- **include/xasm++/syntax/directive_registry.h** - Target pattern
- **src/syntax/directive_registry.cpp** - Implementation reference
- **docs/adr/002-component-based-parser-refactoring.md** - Architecture guidance

---

## Step-by-Step Implementation Plan

### Phase 1: Merlin Parser Migration (Proof of Concept)

**Step 1.1: Create DirectiveContext adapter helper**
  - Action: Add helper method to convert parser state → DirectiveContext
  - Files: `include/xasm++/syntax/merlin_syntax.h`, `src/syntax/merlin/merlin_syntax.cpp`
  - Tests: Add unit test for context creation
  - Verification: Helper compiles and creates valid DirectiveContext
  
```cpp
// In merlin_syntax.h
DirectiveContext CreateDirectiveContext();

// In merlin_syntax.cpp
DirectiveContext MerlinSyntaxParser::CreateDirectiveContext() {
  DirectiveContext ctx;
  ctx.section = &current_section_;
  ctx.symbols = &symbol_table_;
  ctx.current_address = &current_address_;
  ctx.parser_state = this;  // For parser-specific state
  return ctx;
}
```

**Step 1.2: Write failing test for ORG directive with DirectiveRegistry**
  - Action: Create test that calls DirectiveRegistry.Execute("ORG", ...)
  - Files: Test file (find existing test structure)
  - Tests: Test expects ORG to set address correctly
  - Verification: Test fails (RED phase)

**Step 1.3: Migrate ORG handler to DirectiveRegistry**
  - Action: 
    1. Remove ORG from local `directive_registry_`
    2. Register ORG in DirectiveRegistry during initialization
    3. Adapt handler to use DirectiveContext
  - Files: `src/syntax/merlin/merlin_syntax.cpp`
  - Verification: Test passes (GREEN phase)
  
```cpp
// Before (OLD):
directive_registry_["ORG"] = [this](const std::string &label,
                                     const std::string &operands,
                                     Section &section,
                                     ConcreteSymbolTable &symbols) {
  HandleOrg(operands, section, symbols);
};

// After (NEW):
global_directive_registry_.Register("ORG", 
  [this](const std::string &label,
         const std::string &operand,
         DirectiveContext &ctx) {
    HandleOrg(operand, *ctx.section, *ctx.symbols);
  });
```

**Step 1.4: Refactor ORG handler (if needed)**
  - Action: Clean up any duplication or improve clarity
  - Verification: Tests still pass (REFACTOR phase)

**Step 1.5: Migrate EQU directive (second example)**
  - Action: Apply same pattern to EQU
  - Tests: Add/verify EQU tests with DirectiveRegistry
  - Verification: Tests pass

**Step 1.6: Migrate remaining 29 Merlin directives**
  - Action: Batch migrate similar directives
  - Groups:
    - Data directives (DB, DW, DS, HEX) - similar pattern
    - String directives (ASC, DCI, INV, FLS) - similar pattern
    - Control directives (DO, ELSE, FIN) - similar pattern
    - Other directives
  - Tests: Verify existing test suite still passes
  - Verification: All 31 Merlin directives use DirectiveRegistry

**Step 1.7: Remove local directive_registry_**
  - Action: Delete local `directive_registry_` member and type definition
  - Files: `include/xasm++/syntax/merlin_syntax.h`
  - Verification: Code compiles, tests pass

**Step 1.8: Update dispatch logic to use DirectiveRegistry**
  - Action: Modify line parsing to call `global_directive_registry_.Execute()`
  - Files: `src/syntax/merlin/merlin_syntax.cpp`
  - Verification: All parser tests pass (97%+ target)

### Phase 2: SCMASM Parser Migration

**Step 2.1: Apply Merlin pattern to SCMASM**
  - Action: Repeat Phase 1 steps for SCMASM parser
  - Files: `src/syntax/scmasm/scmasm_syntax.cpp`, `include/xasm++/syntax/scmasm_syntax.h`
  - Directives: ~13 directives to migrate
  - Verification: SCMASM tests pass

### Phase 3: EDTASM Parser Migration

**Step 3.1: Apply pattern to EDTASM**
  - Action: Repeat Phase 1 steps for EDTASM parser
  - Files: `src/syntax/edtasm_m80_plusplus/`
  - Verification: EDTASM tests pass

### Phase 4: Verification and Documentation

**Step 4.1: Run full test suite**
  - Action: Execute all parser tests
  - Verification: 97%+ pass rate maintained

**Step 4.2: Update documentation**
  - Action: Document DirectiveRegistry integration
  - Files: Add comments, update README if needed
  - Verification: Documentation accurate

**Step 4.3: Clean up and commit**
  - Action: Remove any dead code, finalize changes
  - Verification: Clean git diff, all tests passing

---

## Testing Strategy

### Unit Tests

**For each directive handler:**
- ✓ Happy path (valid operands)
- ✓ Edge cases (boundary values)
- ✓ Error cases (invalid operands)

**Example for ORG:**
```cpp
TEST(DirectiveRegistry, OrgSetsAddress) {
  DirectiveRegistry registry;
  DirectiveContext ctx = CreateTestContext();
  
  registry.Execute("ORG", "", "1000H", ctx);
  
  EXPECT_EQ(*ctx.current_address, 0x1000);
}

TEST(DirectiveRegistry, OrgInvalidOperand) {
  DirectiveRegistry registry;
  DirectiveContext ctx = CreateTestContext();
  
  EXPECT_THROW(
    registry.Execute("ORG", "", "INVALID", ctx),
    std::runtime_error
  );
}
```

### Integration Tests

**Parser integration:**
- ✓ Parse complete assembly file using DirectiveRegistry
- ✓ Verify all directives dispatched correctly
- ✓ Verify output matches expected binary

### Regression Tests

**Existing test suites:**
- ✓ All Merlin parser tests pass
- ✓ All SCMASM parser tests pass
- ✓ All EDTASM parser tests pass
- ✓ Overall test pass rate: 97%+

### Coverage Target

- Overall: 85%+ (maintain current)
- DirectiveRegistry integration: 90%+
- Directive handlers: 85%+ (maintain current)

---

## Dependencies and Prerequisites

### Technical Dependencies

✓ DirectiveRegistry class (already exists)
✓ DirectiveContext struct (already exists)
✓ Existing test infrastructure

### Task Dependencies

✓ DirectiveRegistry implementation (completed in earlier phase)
✓ ADR-002 approval (architecture defined)

### Knowledge Dependencies

✓ Understanding of DirectiveRegistry pattern
✓ Familiarity with parser structure
✓ TDD workflow

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1: Breaking existing functionality**
- **Probability:** Medium
- **Impact:** High
- **Mitigation:** 
  - Incremental migration (one parser at a time)
  - Maintain existing tests
  - Run tests after each migration step
- **Contingency:** Revert to previous state if tests fail

**Risk 2: Performance degradation**
- **Probability:** Low
- **Impact:** Low
- **Mitigation:** 
  - DirectiveRegistry uses std::unordered_map (O(1) lookup)
  - Existing pattern also uses map lookup
  - No expected performance change
- **Contingency:** Profile if performance issues detected

**Risk 3: Signature incompatibilities**
- **Probability:** Medium
- **Impact:** Medium
- **Mitigation:**
  - Use adapter pattern for context conversion
  - Keep existing HandleOrg/HandleEqu methods unchanged initially
  - Gradual refactoring after migration complete
- **Contingency:** Maintain adapter layer if needed

---

## Rollback Plan

### If Migration Fails

**Rollback Steps:**
```bash
# Revert changes
git restore .

# Or revert specific commits
git revert <commit-hash>

# Verify tests pass
ctest --output-on-failure
```

**Rollback Verification:**
- ✓ All tests pass
- ✓ No compilation errors
- ✓ Parser functionality unchanged

---

## Performance Considerations

### Expected Performance Impact

**Directive dispatch:**
- Before: std::unordered_map lookup (O(1))
- After: std::unordered_map lookup (O(1))
- Expected impact: **None** (same lookup mechanism)

**Memory:**
- Before: Local registry per parser (~3 instances)
- After: Shared DirectiveRegistry (~1 instance)
- Expected impact: **Slight reduction** in memory

### Performance Targets

- Parse time: No degradation (<1% acceptable)
- Memory: No significant change

---

## Alternative Approaches Considered

### Alternative 1: Extract Handlers to Classes

**Approach:** Create DirectiveHandler base class with subclasses

**Pros:**
- More OOP-style
- Each handler is separate class

**Cons:**
- More files (31+ new files for Merlin alone)
- More boilerplate
- Overkill for simple handlers

**Why Not Chosen:** 
- Current lambda approach works well
- Not worth the complexity for this phase
- Can extract to classes later if needed

### Alternative 2: Keep Dual Signatures

**Approach:** Support both old and new signatures

**Pros:**
- No migration needed
- Backward compatible

**Cons:**
- Technical debt persists
- Two patterns to maintain
- Confusion for developers

**Why Not Chosen:**
- Doesn't achieve goal of unification
- Perpetuates inconsistency

---

## Timeline Estimate

**Phase 1 (Merlin):** 4-6 hours
- Context adapter: 0.5 hour
- ORG migration (TDD): 1 hour
- EQU migration: 0.5 hour
- Remaining 29 directives: 2-3 hours
- Testing and verification: 1 hour

**Phase 2 (SCMASM):** 2-3 hours
- Apply pattern: 1.5 hours
- Testing: 0.5-1 hour

**Phase 3 (EDTASM):** 2-3 hours
- Apply pattern: 1.5 hours
- Testing: 0.5-1 hour

**Phase 4 (Verification):** 1 hour
- Full test suite: 0.5 hour
- Documentation: 0.5 hour

**Total: 9-13 hours**

---

## Success Metrics

### Completion Criteria

✓ All 31 Merlin directives use DirectiveRegistry
✓ All SCMASM directives use DirectiveRegistry
✓ All EDTASM directives use DirectiveRegistry
✓ Local directive_registry_ removed from all parsers
✓ All tests passing (97%+ pass rate)
✓ No compilation warnings

### Quality Metrics

- Test coverage: Maintain 85%+
- Performance: No degradation
- Code quality: No new linting issues
- All parsers use consistent pattern

---

## Plan Approval

**Plan Status:** Approved by Orchestrator

**Reviewed By:**
- [X] Orchestrator: 2026-02-10

**Revision History:**
- v1.0 - 2026-02-10 - Initial plan (Orchestrator)

---

## Notes

**Key Insight:** This is NOT new architecture - it's completing a partial migration. DirectiveRegistry infrastructure already exists, we just need to integrate parsers with it.

**Migration Order Rationale:**
1. Merlin first - Most directives (31), good test coverage
2. SCMASM second - Fewer directives (13), apply proven pattern
3. EDTASM third - Apply proven pattern

**Success Criteria:** When complete, all parsers will use DirectiveRegistry consistently, enabling easy addition of new directives without modifying parser classes.

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-10
