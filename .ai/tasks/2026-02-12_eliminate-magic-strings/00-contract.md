# Task Contract

**Task ID:** 2026-02-12_eliminate-magic-strings
**Beads Task:** xasm++-n3wv
**Created:** 2026-02-12
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Eliminate all magic string literals and magic numbers across syntax parsers and expression parser by replacing them with named constants.

### Background and Context

During comprehensive code review, discovered ~50 instances of magic strings (directive names, function names, option values) and magic numbers across the codebase. These violate clean code principles:
- Hard to maintain (string changes require finding all usages)
- Error-prone (typos in strings won't be caught at compile time)
- Lacks semantic meaning (what does "0x100" represent?)

Most directive constants already exist in `include/xasm++/directives/directive_constants.h` but are not being used consistently.

### Current State

- FLEX syntax: 20 string literals for directive/opcode comparisons
- EDTASM syntax: 9 string literals for directive comparisons
- Expression parser: 6 string literals for function names (HIGH/LOW with case variants)
- Merlin directives: 2 string literals for option values (ON/OFF)
- Merlin syntax: 5 string literals for pseudo-op comparisons
- CPU 6809: 1 magic number (0x100) for direct page check

### Desired State

All string literals replaced with named constants from appropriate headers:
- Use directive constants from `directive_constants.h`
- Create constants for HIGH/LOW function names
- Create constants for ON/OFF option values
- Define DIRECT_PAGE_MAX constant for 0x100

---

## Success Criteria

```
✓ All ~50 magic strings replaced with named constants
✓ All tests passing (1425/1426 minimum)
✓ Zero new compiler warnings
✓ Build succeeds with no errors
```

---

## Acceptance Criteria

### Functional Requirements
```
□ FLEX syntax: All 20 directive/opcode strings replaced
□ EDTASM syntax: All 9 directive strings replaced
□ Expression parser: All 6 function name strings replaced
□ Merlin directives: Both ON/OFF strings replaced
□ Merlin syntax: All 5 pseudo-op strings replaced
□ CPU 6809: Magic number 0x100 replaced with constant
```

### Quality Requirements
```
□ No reduction in test pass rate (maintain 1425/1426)
□ Zero new compiler warnings
□ Constants defined in appropriate headers
□ Using declarations added where needed
```

### Documentation Requirements
```
□ Work log updated with progress
□ Any new constants documented with comments
```

---

## Scope

### In Scope
- Replace string literals with constants
- Define new constants where needed (HIGH/LOW, ON/OFF, DIRECT_PAGE_MAX)
- Add using declarations for namespaced constants
- Test to ensure no regressions

### Out of Scope
- Refactoring parser logic
- Adding new features
- Changing test implementations
- Fixing pre-existing test failures

---

## Dependencies

**Requires:**
- Existing directive constants in `directive_constants.h`

**Blocks:**
- None

---

## Risk Assessment

**Risks:**
1. String constant mismatches could break parsing
2. Case sensitivity issues (upper vs lower)
3. Namespace conflicts

**Mitigations:**
1. Run full test suite after each file
2. Ensure constants match original strings exactly
3. Use appropriate using declarations

---

## Stakeholders

- **Owner:** Claude (Engineer role)
- **Reviewer:** User
- **Tester:** Test suite automation

---

## Timeline

**Estimated Effort:** 1-2 hours
**Target Completion:** 2026-02-12

---

## Related Documents

- **Standards:** `.ai-pack/quality/clean-code/`
- **Workflow:** `.ai-pack/workflows/refactor.md`
- **Magic Strings Report:** `/tmp/magic_strings_report.txt`
