# Task Contract

**Task ID:** 2026-02-13_a2osx-p0-directives
**Created:** 2026-02-13
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer (Implementation)
**Workflow:** Feature
**Beads Task:** xasm++-zitw

---

## Task Description

Implement 5 critical P0 SCMASM directives required for A2oSX operating system build support.

### Background and Context

Gap analysis (xasm++-9p8u) identified 5 **P0 critical blockers**:

1. **.INB** (Include Binary) - 1131 uses (11.7%)
2. **.PS** (Pascal String) - 529 uses (5.5%)
3. **.DUMMY** (Dummy Section) - 231 uses (2.4%)
4. **.LIST** (Listing Control) - 192 uses (2.0%)
5. **.OP** (CPU Operation Mode) - 148 uses (1.5%)

Without these, A2oSX cannot build. Combined: 2,231 occurrences (23% of all directive usage).

### Current State

- xasm++ has 52% SCMASM coverage (13/25 directives)
- Missing P0 directives block A2oSX build
- Gap analysis complete with detailed semantics

### Desired State

- All 5 P0 directives implemented and tested
- xasm++ SCMASM coverage → 72% (18/25)
- A2oSX sample files assemble successfully
- Ready for full A2oSX build validation

---

## Success Criteria

```
✓ All 5 P0 directives implemented (.INB, .PS, .DUMMY, .LIST, .OP)
✓ Comprehensive unit tests for each directive
✓ All tests passing (100% pass rate)
✓ Code coverage ≥ 90% for new code
✓ Integration tests with A2oSX samples pass
✓ Documentation complete
✓ No regressions in existing SCMASM tests
```

---

## Acceptance Criteria

### Phase 1: Simple Directives
```
□ .PS (Pascal String) - Length-prefixed strings
  - Emits length byte + string bytes
  - Length validation (0-255)
  - Error on >255 characters

□ .INB (Include Binary) - File inclusion
  - Reads external binary file
  - Inserts bytes at current position
  - Handles relative/absolute paths
  - Error on missing file
  - Recursive depth protection

□ .LIST (Listing Control) - Stub OK
  - Stub implementation (no-op)
  - Parses ON/OFF parameter
```

### Phase 2: Complex Directives
```
□ .DUMMY (Dummy Section) - Structure definitions
  - Defines structure without emitting bytes
  - Symbol definitions within section
  - Restores state on .ED (End Dummy)
  - Nested sections supported

□ .OP (CPU Operation Mode) - CPU selection
  - Accepts 6502, 65C02, 65816
  - Switches active CPU
  - A2oSX requires 65C02 support
  - Validates opcodes against selected CPU
```

### Quality Requirements
```
□ All tests passing
□ Code coverage ≥ 90%
□ No memory leaks
□ No compiler warnings
□ Code review approved
□ TDD process followed
```

---

## Constraints and Dependencies

### Constraints
```
□ Follow existing SCMASM patterns
□ No breaking changes to existing directives
□ Support A2oSX usage patterns
```

### Dependencies
```
□ Gap analysis complete (xasm++-9p8u) ✅
□ A2oSX repository available ✅
□ xasm++ builds successfully ✅
```

### Out of Scope
```
✗ P1/P2 directives (separate tasks)
✗ Full A2oSX build validation (xasm++-e6h9)
✗ Listing output generation (.LIST stub only)
```

---

## Estimated Complexity

**Complexity:** Medium-to-Large

**Rationale:**
- 5 directives (mini-features)
- 800-1200 LOC estimate
- 8-10 files to modify
- New concepts: CPU switching, file inclusion, dummy sections
- 20-27 hours estimated

---

## Lean Flow Analysis

### Batch Size Assessment

**Files:** 8-10 files

**Breakdown:**
- `src/syntax/scmasm_directive_handlers.cpp` - 5 handlers (~400 lines)
- `include/xasm++/syntax/scmasm_syntax.h` - 5 declarations
- `src/syntax/scmasm_syntax.cpp` - Register 5 directives
- `tests/unit/test_scmasm_syntax.cpp` - Tests (~200 lines)
- `tests/integration/test_a2osx_samples.cpp` - NEW
- `src/cpu/*.cpp` - Possibly for .OP switching

**Assessment:**
```
8-10 files → ⚠️ ACCEPTABLE (within 6-14 range)
```

**Justification:**
- High cohesion (single goal: A2oSX P0 support)
- Phased implementation (TDD, one directive at a time)
- Shared patterns (all use same registration/test infrastructure)

**Phasing:**
- **Phase 1:** .PS, .INB, .LIST (4-6 files, ~20K tokens) ✅ SAFE
- **Phase 2:** .DUMMY, .OP (6-8 files, ~30K tokens) ⚠️ APPROACHING LIMIT

**Contingency:** If token limit hit in Phase 2, split into separate task

**Token Budget:**
- Phase 1: ~20K tokens → ✅ SAFE
- Phase 2: ~30K tokens → ⚠️ APPROACHING LIMIT
- Both phases: ~50K tokens → ❌ HIGH RISK
- **Mitigation:** Commit after Phase 1, then Phase 2

---

## Resources and References

### Key Files
```
- .ai/tasks/2026-02-12_a2osx-directive-analysis/directive-gap-analysis.md
- src/syntax/scmasm_directive_handlers.cpp
- tests/unit/test_scmasm_syntax.cpp
- ~/Projects/Vintage/Apple/A2osX/**/*.S.txt
```

### Usage Examples (grep A2oSX sources)
```
grep -r "\.PS" ~/Projects/Vintage/Apple/A2osX --include="*.S" | head -10
grep -r "\.INB" ~/Projects/Vintage/Apple/A2osX --include="*.S" | head -10
grep -r "\.DUMMY" ~/Projects/Vintage/Apple/A2osX --include="*.S" -A 3 | head -20
```

---

## Assumptions

```
1. Gap analysis semantics are correct
2. A2oSX repository available
3. Existing SCMASM patterns should be followed
4. .LIST can be stubbed (no listing output yet)
5. .INB handles binary files only (not assembly source)
6. Paths relative to source file location
```

---

## Risk Assessment

**Risk 1:** .OP requires major CPU abstraction changes
- Probability: Medium | Impact: High
- Mitigation: Investigate CPU abstraction first
- Contingency: Stub .OP if needed (sets flag, no validation)

**Risk 2:** Token limit exceeded
- Probability: Medium | Impact: Medium
- Mitigation: Phased approach with commits
- Contingency: Split Phase 2 into separate task

**Risk 3:** .DUMMY semantics unclear
- Probability: Low | Impact: Medium
- Mitigation: Study A2oSX examples thoroughly
- Contingency: Implement basic version, refine later

---

## Approvals

**Contract Approved By:**
- [X] Requestor: Bryan Woodruff (2026-02-13)
- [ ] Agent: Engineer (pending)

---

## Notes

- Implements P0 only (P1/P2 separate)
- Gap analysis provides detailed semantics
- TDD: test first, implement, verify, commit
- Each directive independent (any order within phases)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-13
