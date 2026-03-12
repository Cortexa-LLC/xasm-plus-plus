# Implementation Plan

**Task ID:** 2026-02-12_edtasm-listing
**Created:** 2026-02-12
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Implement three EDTASM-M80++ listing control directives (LIST, NOLIST, *LIST) by adding handler functions and registering them with the DirectiveRegistry. The implementation will integrate with the existing `listing_output.cpp` infrastructure to control listing output during assembly.

The approach follows the established pattern from other EDTASM directive handlers: create handler functions that manipulate listing state, register them with the directive registry, and write tests to verify behavior. The listing output system already exists; this task only adds the control directives.

**Key Technical Decisions:**
- Follow existing directive handler pattern from `edtasm_directive_handlers.cpp`
- Use existing listing output API (no modifications to listing system needed)
- Register directives using DirectiveRegistry (consistent with other EDTASM directives)
- Implement state management for listing on/off toggle

**Patterns to Use:**
- Directive handler pattern - Consistent with existing EDTASM handlers
- DirectiveRegistry registration - Standard plugin registration mechanism
- TDD approach - Write tests first, implement to pass

---

## Implementation Steps

### Phase 1: Setup and Analysis

**Step 1.1:** Review existing listing output system
- Read `src/output/listing_output.cpp` to understand API
- Identify state management methods (enable/disable listing)
- Document integration points

**Step 1.2:** Review existing directive handlers
- Read `src/syntax/edtasm_directive_handlers.cpp` for pattern
- Understand handler signature and registration
- Review similar directives for reference

### Phase 2: TDD Implementation

**Step 2.1:** Write failing tests (RED)
- Add tests to `tests/unit/test_edtasm_m80_plusplus_syntax.cpp`
- Test LIST directive enables listing
- Test NOLIST directive disables listing
- Test *LIST directive special behavior
- Run tests - verify they fail

**Step 2.2:** Implement handlers (GREEN)
- Add `HandleListDirective()` to edtasm_directive_handlers.cpp
- Add `HandleNolistDirective()` to edtasm_directive_handlers.cpp
- Add `HandleStarListDirective()` to edtasm_directive_handlers.cpp
- Add declarations to edtasm_directive_handlers.h
- Integrate with listing_output API
- Run tests - verify they pass

**Step 2.3:** Refactor (REFACTOR)
- Extract common state management if needed
- Add code comments
- Ensure error handling is consistent
- Run tests - verify still passing

**Step 2.4:** Register directives
- Add registration calls in `edtasm_m80_plusplus_syntax.cpp`
- Use LIST, NOLIST, STAR_LIST constants from directive_constants.h
- Verify directives are recognized during parsing

### Phase 3: Integration Testing

**Step 3.1:** Build and test
- Run full build - verify zero warnings
- Run full test suite - verify 1425/1426 minimum
- Run specific EDTASM tests - verify all passing

**Step 3.2:** Manual verification
- Create test source file with LIST/NOLIST directives
- Assemble and verify listing output behavior
- Confirm state changes work as expected

### Phase 4: Documentation

**Step 4.1:** Update work log
- Document implementation progress
- Note any decisions or discoveries
- Record test results

**Step 4.2:** Create acceptance document
- Verify all acceptance criteria met
- Document any deviations
- Sign off on completion

---

## File Modifications

**Source Files (3 files):**
1. `src/syntax/edtasm_directive_handlers.cpp` - Add 3 handler implementations (~30 lines each)
2. `src/syntax/edtasm_directive_handlers.h` - Add 3 function declarations
3. `src/syntax/edtasm_m80_plusplus_syntax.cpp` - Add 3 directive registrations

**Test Files (1 file):**
4. `tests/unit/test_edtasm_m80_plusplus_syntax.cpp` - Add 3 test cases

**Total:** 4 files modified, ~120 lines added

---

## Integration Points

**Listing Output System:**
- `src/output/listing_output.cpp` - Existing listing infrastructure
- API methods for enable/disable (to be identified in Phase 1)

**Directive Infrastructure:**
- DirectiveRegistry - Standard registration mechanism
- Directive constants - Use LIST, NOLIST, STAR_LIST from directive_constants.h

**Test Infrastructure:**
- Google Test framework
- Existing EDTASM test suite

---

## Risk Mitigation

**Risk 1: Listing API not exposed**
- **Mitigation:** If API methods aren't public, add public methods to listing_output class
- **Fallback:** Use existing public API or create facade

**Risk 2: State management complexity**
- **Mitigation:** Follow pattern from similar directives (conditional state tracking)
- **Fallback:** Implement simple boolean flag for listing enabled/disabled

**Risk 3: *LIST behavior unclear**
- **Mitigation:** Research M80/ZMAC documentation for *LIST behavior
- **Fallback:** Implement as toggle (if listing on, turn off; if off, turn on)

---

## Testing Strategy

**Unit Tests:**
- Test each directive independently
- Verify state changes
- Test edge cases (multiple LIST/NOLIST calls)

**Integration Tests:**
- Test directive interaction with listing output
- Verify output format unchanged
- Test with macro expansion (if applicable)

**Regression Tests:**
- Run full test suite
- Verify no existing tests broken
- Maintain 99%+ pass rate

---

## Success Metrics

**Code Quality:**
- Zero compiler warnings
- All tests passing
- TDD process followed

**Functionality:**
- All 3 directives working
- Listing state correctly managed
- Integration with listing output verified

**Documentation:**
- Work log complete
- Acceptance criteria verified
- Code comments added

---

## Execution Strategy

**Sequential execution** (all steps depend on previous):
1. Analysis before implementation
2. Tests before code (TDD)
3. Implementation before integration
4. Verification before documentation

**Estimated time:** 2-3 hours
- Phase 1: 30 minutes
- Phase 2: 60-90 minutes
- Phase 3: 30 minutes
- Phase 4: 15 minutes

---

## Notes

- Directive constants (LIST, NOLIST, STAR_LIST) already exist in directive_constants.h
- Listing output infrastructure already exists
- Follow established pattern from existing EDTASM handlers
- Maintain consistency with other directive implementations
