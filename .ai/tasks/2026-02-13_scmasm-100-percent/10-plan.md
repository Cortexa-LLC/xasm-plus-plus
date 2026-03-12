# Plan: Phase 3 - Complete SCMASM 100% Coverage

**Task:** xasm++-f2n8
**Workflow:** Feature (incremental implementation)
**Role:** Engineer
**Approach:** TDD with incremental directive implementation

---

## Execution Strategy

**Sequential implementation** (directives build on each other):
1. Simple string/metadata directives first (.CS, .CZ, .TF, .EP)
2. Simple data directives (.HX, .TA)
3. Complex control flow last (.DO/.FIN, .AC)

**Rationale:** Build confidence with simple directives, tackle complex ones with momentum.

---

## Implementation Phases

### Phase 3.1: String Directives (4-8 hours)

**Directives:** .CS, .CZ
**Complexity:** Medium (escape sequence parsing)

**Steps:**
1. RED: Write tests for .CS escape sequences
2. GREEN: Implement HandleCs with escape parser
3. REFACTOR: Extract escape parser as helper
4. RED: Write tests for .CZ (null terminator)
5. GREEN: Implement HandleCz (calls HandleCs + adds null)
6. Commit: "feat: implement .CS and .CZ directives"

**vasm-ext reference:** Lines 1256-1349

---

### Phase 3.2: Metadata Directives (2-3 hours)

**Directives:** .TF, .EP
**Complexity:** Low (simple storage)

**Steps:**
1. RED: Write tests for .TF (title parsing)
2. GREEN: Implement HandleTf (stub or metadata storage)
3. RED: Write tests for .EP (entry point)
4. GREEN: Implement HandleEp (metadata storage)
5. Commit: "feat: implement .TF and .EP directives"

**vasm-ext reference:** Lines 2171 (.TF), no specific line for .EP

---

### Phase 3.3: Data Directives (2-3 hours)

**Directives:** .HX, .TA
**Complexity:** Low (simple logic)

**Steps:**
1. RED: Write tests for .HX (nibble packing)
2. GREEN: Implement HandleHx (pack nibbles)
3. RED: Write tests for .TA (no-op)
4. GREEN: Implement HandleTa (no-op or warning)
5. Commit: "feat: implement .HX and .TA directives"

**vasm-ext reference:** Lines 1352+ (.HX), Lines 1453-1470 (.TA)

---

### Phase 3.4: Conditional Assembly (6-8 hours)

**Directives:** .DO, .FIN
**Complexity:** High (control flow, nesting, expression evaluation)

**Steps:**
1. RED: Write tests for basic .DO/.FIN (true/false)
2. GREEN: Implement conditional block tracking (stack)
3. RED: Write tests for nested conditionals
4. GREEN: Implement nesting support
5. RED: Write tests for expression evaluation
6. GREEN: Integrate with expression evaluator
7. Commit: "feat: implement .DO/.FIN conditional assembly"

**Implementation notes:**
- May need to integrate with existing macro conditional system
- Requires expression evaluator for condition
- Stack for tracking nested blocks

---

### Phase 3.5: ASCII Compression (6-8 hours)

**Directives:** .AC
**Complexity:** Very High (state machine, 3 tables, nibble packing)

**Steps:**
1. RED: Write tests for table initialization (.AC 0)
2. GREEN: Implement compression state (3 tables + nibble flag)
3. RED: Write tests for table definition (.AC 1/2/3)
4. GREEN: Implement table storage
5. RED: Write tests for single-nibble encoding (table 1)
6. GREEN: Implement table 1 compression
7. RED: Write tests for double-nibble encoding (table 2)
8. GREEN: Implement table 2 compression
9. RED: Write tests for triple-nibble encoding (table 3)
10. GREEN: Implement table 3 compression
11. RED: Write tests for state persistence
12. GREEN: Verify nibble state persists across directives
13. RED: Write tests for blank count encoding
14. GREEN: Implement digit-to-blank-count logic
15. Commit: "feat: implement .AC ASCII compression"

**vasm-ext reference:** Lines 929-1119 (very detailed implementation)

**Implementation notes:**
- Requires parser-level state (3 tables + nibble position)
- Most complex directive in SCMASM
- Low usage (2 occurrences) but high fidelity requirement

---

## Token Budget Analysis

**Files to modify:** ~4 files
**Estimated lines:** ~850 lines

**Breakdown:**
- `scmasm_directive_handlers.cpp`: ~500 lines (7 handlers + helpers)
- `scmasm_directive_handlers.h`: ~20 lines (declarations)
- `scmasm_syntax.cpp`: ~30 lines (registration)
- `test_scmasm_syntax.cpp`: ~300 lines (30+ tests)

**Token estimate:** ~17K-20K (safe for single task)

**Batch size:** ✅ Acceptable (4 files, well under 15-file limit)

---

## Dependencies

**Required:**
- Phase 1 complete ✅
- Phase 2 complete ✅
- Expression evaluator (existing) - for .DO/.FIN
- Parser state management (existing) - for .AC

---

## Verification Steps

1. **After each directive:**
   - Run tests: `ctest -R SCMASM.*<DirectiveName>`
   - Verify zero warnings: `cmake --build build 2>&1 | grep -i warning`
   - Commit changes

2. **After Phase 3 complete:**
   - Run full test suite: `ctest --output-on-failure`
   - Verify coverage: 100% (25/25 directives)
   - Update work log and acceptance document
   - Close Beads task xasm++-f2n8

---

## Success Criteria

- ✅ All 7 directives implemented
- ✅ 30+ new tests, all passing
- ✅ 1582+ total tests passing (1552 + 30)
- ✅ Zero compiler warnings
- ✅ Zero regressions
- ✅ 100% SCMASM directive coverage
- ✅ A2oSX-ready (all used directives supported)

---

## Risk Mitigation

**Risk:** .AC complexity too high (6-8 hours estimate)
**Mitigation:** Implement last, use vasm-ext reference code, accept longer time if needed

**Risk:** .DO/.FIN requires parser refactor
**Mitigation:** Check existing conditional infrastructure first, reuse if possible

**Risk:** Escape sequence bugs in .CS
**Mitigation:** Comprehensive test coverage, vasm-ext reference implementation

---

## References

- vasm-ext source: `~/Projects/Vintage/tools/vasm-ext/syntax/scmasm/syntax.c`
- Comparison doc: `.ai/tasks/2026-02-13_a2osx-p0-directives/vasm-ext-comparison.md`
- Gap analysis: `.ai/tasks/2026-02-12_a2osx-directive-analysis/directive-gap-analysis.md`

---

**Plan approved by:** Orchestrator
**Ready for execution by:** Engineer
**Estimated:** 20-29 hours conservative, 8-12 hours actual
