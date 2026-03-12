# Implementation Plan: Merlin Directive Registry Refactoring

**Task ID:** 2026-02-13_merlin-directive-refactor
**Workflow:** `.ai-pack/workflows/refactor.md`
**Role:** Engineer (delegated from Orchestrator)

---

## Workflow Selection

**Using:** Refactor workflow (behavior-preserving code improvement)

**Why:** Pure refactoring - no functional changes, just structural improvement for consistency.

---

## Approach

### Phase 1: Preparation
1. Read `src/syntax/merlin_syntax.cpp` - InitializeDirectiveRegistry() method
2. Count directive handlers (estimate ~40 lambdas)
3. Read `src/syntax/scmasm_syntax.cpp` - Reference implementation pattern
4. Verify all tests passing baseline (114 Merlin, 1538 total)

### Phase 2: Extraction (Mechanical Refactoring)

**CRITICAL: Create separate file like scmasm/edtasm/core patterns**

1. **Create new files:**
   - `src/syntax/merlin_directive_handlers.cpp` (implementations)
   - `include/xasm++/syntax/merlin_directive_handlers.h` (declarations)

2. **For each directive handler lambda:**
   - Extract lambda body to free function in `merlin` namespace
   - Function name: `Handle<Directive>` (e.g., `HandleOrg`, NOT `HandleOrgDirective`)
   - Signature: Match DirectiveHandler typedef exactly
   - Body: Exact copy of lambda body
   - Location: `merlin_directive_handlers.cpp`

3. **Update registry in merlin_syntax.cpp:**
   - Replace: `directive_registry_[X] = [this](...) { ... };`
   - With: `directive_registry_[X] = merlin::HandleX;`
   - Add: `#include "xasm++/syntax/merlin_directive_handlers.h"`

4. **Follow scmasm pattern exactly:**
   - Free functions in namespace, NOT member methods
   - Separate .cpp file, NOT in merlin_syntax.cpp
   - Clean separation like scmasm/edtasm/core

### Phase 3: Verification
1. Rebuild: `cmake --build build`
2. Run Merlin tests: `ctest --test-dir build -R Merlin`
3. Run full test suite: `ctest --test-dir build`
4. Verify 100% pass rate (114 Merlin, 1538 total)

### Phase 4: Commit
Single atomic commit with descriptive message

---

## Execution Strategy

**Sequential** - Single file refactoring, no parallelization needed.

**Steps:**
1. Baseline verification (tests passing)
2. Incremental extraction (one handler at a time, compile/test frequently)
3. Final verification (all tests)
4. Commit

---

## File Impact Analysis

**Files CREATED:**
- `src/syntax/merlin_directive_handlers.cpp` - ~40 free function implementations
- `include/xasm++/syntax/merlin_directive_handlers.h` - Function declarations

**Files MODIFIED:**
- `src/syntax/merlin_syntax.cpp` - Replace lambdas with function pointers, add include
- `src/CMakeLists.txt` - Add merlin_directive_handlers.cpp to build

**Files NOT Modified:**
- `include/xasm++/syntax/merlin_syntax.h` - No changes needed (using free functions)
- Tests (no behavioral changes)
- Other parsers (isolated refactoring)

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Breaking test | High | Incremental approach - compile/test after each handler |
| Method signature mismatch | Medium | Match DirectiveHandler typedef exactly |
| Missing const/reference | Low | Copy lambda signatures verbatim |
| Merge conflicts | Low | Work on single branch, atomic commit |

---

## Quality Gates

**Before starting:**
- [ ] All 114 Merlin tests passing
- [ ] All 1538 total tests passing

**During refactoring:**
- [ ] Compile after each batch of 5-10 handlers
- [ ] Run Merlin tests after each batch

**Before commit:**
- [ ] All tests passing (100%)
- [ ] No compiler warnings
- [ ] Code review (self-check)

---

## Estimated Timeline

- Preparation: 30 min
- Extraction (~40 handlers): 2-3 hours
- Verification: 30 min
- Commit: 10 min

**Total:** 3-4 hours

---

**Status:** READY FOR EXECUTION
