# Implementation Plan: Test Failure Investigation

**Workflow:** Bugfix (Inspector → Engineer)

---

## Execution Strategy

### Sequential Investigation by Category

1. Build and run full test suite
2. Categorize all 29 failures
3. Fix by priority (regressions first)
4. Verify no new failures
5. Document known issues

---

## Phases

### Phase 0: Setup (15 min)
- Build project
- Run tests to capture all failures
- Create failure categories

### Phase 1: Investigation (1-2 hours)
- Analyze each failure
- Identify root causes
- Determine regression vs pre-existing

### Phase 2: Fixes (2-3 hours)
- Fix regressions immediately
- Fix pre-existing if simple
- Document complex issues

### Phase 3: Verification (30 min)
- Run full test suite
- Verify 99%+ pass rate
- No new failures introduced

---

**Estimated Time:** 4-6 hours
