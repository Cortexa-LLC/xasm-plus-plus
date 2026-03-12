# Task Contract: Investigate and Fix Test Failures

**Task ID:** 2026-02-11_test-failures
**Beads Task:** xasm++-4y47
**Created:** 2026-02-11
**Requestor:** Orchestrator (architectural cleanup)
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Investigate and fix 29 failing tests (98% pass rate) to reach 99%+ pass rate and ensure architectural changes haven't introduced regressions.

### Current State

**Test Status:** 1397/1426 passing (98%)
**Failing Tests:** 29 across multiple categories

**Known Failure Categories:**
1. AssemblerTest - Branch relaxation (5 failures)
2. MerlinSyntaxTest - Macro tests (6 failures)
3. BinaryCompatTest - Forward references (2 failures)
4. ErrorReportingTest - Error messages (7 failures)
5. Various directive tests (9 failures)

### Desired State

- 99%+ pass rate (1412+ passing)
- All critical tests passing
- Known issues documented
- No regressions from Phase 6 refactoring

---

## Success Criteria

```
✓ Test pass rate ≥99% (≤14 failures)
✓ No regressions from Phase 6 refactoring
✓ All branch relaxation tests passing
✓ All macro tests passing OR documented as known issues
✓ Build clean (0 warnings)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Investigate all 29 test failures
□ Categorize failures (regression vs pre-existing)
□ Fix regressions immediately
□ Document known issues clearly
□ Verify fixes don't break other tests
```

### Quality Requirements
```
□ All tests passing after fixes
□ Build with 0 warnings
□ No new test failures introduced
□ Root cause documented for each failure
```

---

## Implementation Strategy

### Phase 1: Investigation (1-2 hours)

**Step 1.1: Build and Run Tests**
```bash
cmake --build build
cd build && ctest --output-on-failure
```

**Step 1.2: Categorize Failures**
- Branch relaxation (5) - Core assembler logic
- Macros (6) - Parser/macro expansion
- Binary compat (2) - Forward reference resolution
- Error reporting (7) - Error message formatting
- Directives (9) - Directive handler bugs

**Step 1.3: Identify Regressions**
- Check if failures existed before Phase 6
- Review git history for when tests started failing
- Priority: Fix regressions first

### Phase 2: Fix by Category (2-3 hours)

**Category 1: Branch Relaxation (Highest Priority)**
- Likely related to CPU coupling removal
- Check branch relaxation logic in cpu_6502.cpp
- Verify multi-pass assembly convergence

**Category 2: Macro Tests**
- Check if related to Phase 6c.1 Merlin extraction
- Verify macro parameter substitution
- Check local label scoping

**Category 3: Binary Compatibility**
- Forward reference resolution
- DW directive handling
- Multi-pass symbol resolution

**Category 4: Error Reporting**
- Error message format changes
- Location information
- Expected error format

**Category 5: Directive Tests**
- Handler registration issues
- Parameter parsing
- DirectiveContext usage

### Phase 3: Verification (1 hour)

**Step 3.1: Run Full Test Suite**
```bash
ctest --test-dir build
```

**Step 3.2: Verify No New Failures**
- Compare before/after results
- Ensure fixes didn't break other tests

**Step 3.3: Document Known Issues**
- Create KNOWN-ISSUES.md if needed
- Document workarounds
- Create follow-up tasks

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- 29 failures to investigate
- Multiple categories
- May involve multiple systems
- Estimated: 4-6 hours total

---

## Approvals

**Contract Approved By:**
- [X] Requestor: Orchestrator (2026-02-11)
- [ ] Agent: Engineer (TBD)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-11
