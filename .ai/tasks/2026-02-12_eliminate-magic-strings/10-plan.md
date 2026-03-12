# Implementation Plan

**Task:** Eliminate magic strings across syntax parsers
**Workflow:** Refactor (.ai-pack/workflows/refactor.md)
**Execution:** Sequential (dependencies between constant definitions and usage)

---

## Phase 1: Preparation

### Step 1.1: Identify Constants Needed
- [ ] Review existing constants in `directive_constants.h`
- [ ] Identify new constants needed (HIGH/LOW, ON/OFF, DIRECT_PAGE_MAX)

### Step 1.2: Define Missing Constants
- [ ] Add HIGH/LOW constants to expression parser or common header
- [ ] Add ON/OFF constants to appropriate header
- [ ] Add DIRECT_PAGE_MAX to CPU constants

---

## Phase 2: Implementation (By File)

### Step 2.1: FLEX Syntax (src/syntax/flex_syntax.cpp) - 20 instances
- [ ] Add using declarations for directive constants
- [ ] Replace all 17 dir_upper comparisons with constants
- [ ] Replace 3 opcode_upper comparisons with constants
- [ ] Test: Build and run tests

### Step 2.2: EDTASM Syntax (src/syntax/edtasm_syntax.cpp) - 9 instances
- [ ] Add using declarations for directive constants
- [ ] Replace all 9 dir_upper comparisons with constants
- [ ] Test: Build and run tests

### Step 2.3: Expression Parser (src/common/expression_parser.cpp) - 6 instances
- [ ] Add HIGH/LOW constants
- [ ] Replace all 6 string comparisons with constants
- [ ] Test: Build and run tests

### Step 2.4: Merlin Directives (src/syntax/merlin_directives.cpp) - 2 instances
- [ ] Add ON/OFF constants or use existing
- [ ] Replace both comparisons with constants
- [ ] Test: Build and run tests

### Step 2.5: Merlin Syntax (src/syntax/merlin_syntax.cpp) - 5 instances
- [ ] Add using declarations for pseudo-op constants
- [ ] Replace all 5 comparisons with constants
- [ ] Test: Build and run tests

### Step 2.6: CPU 6809 (src/cpu/cpu_6809.cpp) - 1 instance
- [ ] Define DIRECT_PAGE_MAX constant
- [ ] Replace 0x100 with constant
- [ ] Test: Build and run tests

---

## Phase 3: Verification

### Step 3.1: Full Test Suite
- [ ] Run complete test suite
- [ ] Verify 1425/1426 tests passing (no regressions)
- [ ] Check for zero new warnings

### Step 3.2: Code Review
- [ ] Review all changes for consistency
- [ ] Verify constants used correctly
- [ ] Check namespace imports

---

## Phase 4: Documentation

### Step 4.1: Update Work Log
- [ ] Document implementation progress
- [ ] Note any issues encountered
- [ ] Record test results

### Step 4.2: Update Acceptance
- [ ] Mark all acceptance criteria as met
- [ ] Document any deviations
- [ ] Final sign-off

---

## Execution Strategy

**Sequential execution** required because:
1. Constant definitions must exist before usage
2. Testing after each file ensures no cascading failures
3. Namespace conflicts need immediate resolution

**Estimated time:** 1-2 hours total
- Phase 1: 15 minutes
- Phase 2: 45-60 minutes (6 files)
- Phase 3: 15 minutes
- Phase 4: 15 minutes

---

## Risk Mitigation

1. **String mismatch risk:** Test after each file to catch issues early
2. **Case sensitivity risk:** Ensure constants match original strings exactly (uppercase)
3. **Namespace conflicts:** Add using declarations incrementally, test each

---

## Dependencies

- All changes can be made in parallel after constants are defined
- However, safer to do sequentially with testing between each file

---

## Notes

- Most directive constants already exist in `directive_constants.h`
- Need to create new constants for HIGH/LOW, ON/OFF, DIRECT_PAGE_MAX
- Maintain existing behavior exactly - this is pure refactoring
