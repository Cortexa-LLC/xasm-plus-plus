# Task Contract

**Task ID:** 2026-02-12_edtasm-listing
**Beads Task:** xasm++-8r43
**Created:** 2026-02-12
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement 3 EDTASM-M80++ listing control directives to integrate with existing listing output system.

### Background and Context

EDTASM-M80++ assembler includes directives for controlling listing output. The listing output infrastructure (`listing_output.cpp`) already exists, but the directives to control it are not yet implemented. This task adds the directive handlers to toggle listing state during assembly.

### Current State

- Listing output system exists in `src/output/listing_output.cpp`
- Directive handler infrastructure exists in `src/syntax/edtasm_directive_handlers.cpp`
- LIST, NOLIST, *LIST directives are not implemented
- No way to control listing output from source code

### Desired State

Three listing directives implemented and integrated:
- **LIST**: Enable listing output
- **NOLIST**: Disable listing output
- ***LIST**: Special listing control (toggle or specific behavior)

All directives properly registered and tested.

---

## Success Criteria

```
✓ LIST directive implemented and working
✓ NOLIST directive implemented and working
✓ *LIST directive implemented and working
✓ Integration with listing_output.cpp complete
✓ All tests passing (maintain 1425/1426 minimum)
✓ Zero compiler warnings
✓ Build succeeds
```

---

## Acceptance Criteria

### Functional Requirements
```
□ LIST directive enables listing output
□ NOLIST directive disables listing output
□ *LIST directive implements special control
□ Directives properly registered in DirectiveRegistry
□ State changes reflected in output
```

### Quality Requirements
```
□ Tests written for all 3 directives
□ Integration tests with listing output
□ No reduction in test pass rate
□ Zero new compiler warnings
□ TDD process followed (RED-GREEN-REFACTOR)
```

### Documentation Requirements
```
□ Work log updated with progress
□ Code comments explain behavior
□ Acceptance criteria verified
```

---

## Scope

### In Scope
- Implement 3 listing directive handlers
- Register directives with DirectiveRegistry
- Integrate with listing_output.cpp
- Toggle listing state
- Write tests for directive behavior

### Out of Scope
- Modifying listing output format
- Macro expansion control (separate TODO in listing_output.cpp)
- New listing features beyond on/off control

---

## Dependencies

**Requires:**
- Existing listing_output.cpp infrastructure
- DirectiveRegistry infrastructure
- Directive constants from directive_constants.h

**Blocks:**
- None

---

## Risk Assessment

**Risks:**
1. State management complexity (listing on/off per directive)
2. Integration points with listing system
3. Test coverage for state changes

**Mitigations:**
1. Follow existing directive handler pattern
2. Use existing listing output API
3. Write comprehensive tests for state transitions

---

## Technical Approach

**Files to Modify:**
1. `src/syntax/edtasm_directive_handlers.cpp` - Add 3 handlers
2. `src/syntax/edtasm_directive_handlers.h` - Add declarations
3. `src/syntax/edtasm_m80_plusplus_syntax.cpp` - Register directives
4. `tests/unit/test_edtasm_m80_plusplus_syntax.cpp` - Add tests

**Estimated:** 2-3 hours, ~9K tokens

---

## Stakeholders

- **Owner:** Engineer Agent
- **Reviewer:** User
- **Tester:** Test suite automation

---

## Timeline

**Estimated Effort:** 2-3 hours
**Target Completion:** 2026-02-12

---

## Related Documents

- **Reference:** `docs/planning/not-yet-implemented-items.md` (Section 3)
- **Existing Code:** `src/output/listing_output.cpp`
- **Directive Infrastructure:** `src/syntax/edtasm_directive_handlers.cpp`
- **Standards:** `.ai-pack/quality/clean-code/`
- **Workflow:** `.ai-pack/workflows/feature.md`
