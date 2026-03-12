# Implementation Plan: Syntax Folder Naming

**Task:** 2026-02-09_syntax-folder-naming
**Beads ID:** xasm++-9k6d
**Workflow:** Refactor (architecture cleanup)

## Workflow Selection

Using: `.ai-pack/workflows/refactor.md`

**Rationale:** This is a refactoring task focused on naming consistency without changing behavior.

## Implementation Approach

### Phase 1: Analysis and Verification (30 min)

**Goal:** Understand current state and verify convention compliance

**Steps:**
1. List all syntax folders with: `ls -1 src/syntax/*/`
2. Check for naming patterns:
   - Single word: lowercase (e.g., `merlin`, `flex`)
   - Compound word: snake_case (e.g., `z80_universal`, `edtasm_m80`)
3. Verify convention is actually consistent
4. Document findings in work log

**Expected Outcome:** Confirmation that current naming already follows pattern, or list of folders needing rename.

### Phase 2: Documentation (30 min)

**Goal:** Formalize the naming standard

**Steps:**
1. Create/update `docs/CODING-STANDARDS.md` with section:
   ```markdown
   ## Syntax Folder Naming Convention

   Syntax plugin folders use **lowercase snake_case**:
   - Single-word syntaxes: lowercase (e.g., `merlin`, `flex`, `zilog`)
   - Multi-word syntaxes: snake_case (e.g., `z80_universal`, `edtasm_m80`)

   Examples:
   - ✅ `merlin` (single word)
   - ✅ `z80_universal` (z80 + universal)
   - ✅ `edtasm_m80` (edtasm variant)
   - ❌ `Merlin` (capitalized)
   - ❌ `Z80Universal` (camelCase)
   ```

2. Add to work log with rationale

### Phase 3: Verification (if renames needed)

**Only execute if Phase 1 finds non-compliant folders**

**Steps:**
1. Create git checkpoint: `git add -A && git commit -m "checkpoint: before syntax rename"`
2. For each folder to rename:
   ```bash
   OLD_NAME="old_name"
   NEW_NAME="new_name"

   # Rename folder
   git mv src/syntax/$OLD_NAME src/syntax/$NEW_NAME

   # Update CMakeLists.txt
   sed -i '' "s/$OLD_NAME/$NEW_NAME/g" src/CMakeLists.txt

   # Update header guards
   sed -i '' "s/${OLD_NAME^^}/${NEW_NAME^^}/g" include/xasm++/syntax/*.h

   # Update includes
   find src tests -name "*.cpp" -o -name "*.h" | xargs sed -i '' "s/$OLD_NAME/$NEW_NAME/g"
   ```

3. Build and test after EACH rename:
   ```bash
   cmake --build build
   cd build && ctest --output-on-failure
   ```

4. Commit after successful rename:
   ```bash
   git add -A
   git commit -m "refactor: rename syntax/$OLD_NAME to syntax/$NEW_NAME"
   ```

### Phase 4: Final Verification (30 min)

**Steps:**
1. Full clean build:
   ```bash
   rm -rf build
   cmake -B build
   cmake --build build
   ```

2. Run complete test suite:
   ```bash
   cd build && ctest --output-on-failure
   ```

3. Verify no old names remain:
   ```bash
   grep -r "old_folder_name" src/ include/ tests/
   # Should return nothing
   ```

4. Check for warnings:
   ```bash
   cmake --build build 2>&1 | grep -i warning
   # Should return nothing
   ```

## Execution Strategy

**Sequential** (not parallel) because:
- Build system dependencies require ordered execution
- Each rename must be tested before next
- Git history should be clear (one commit per rename)

## TDD Approach

**This is a refactoring task** - tests already exist:
- ✅ Tests written: Existing syntax tests validate behavior
- 🔄 Refactor: Rename folders, update references
- ✅ Tests pass: Verify behavior unchanged

**RED-GREEN-REFACTOR:**
- RED: N/A (no new behavior)
- GREEN: N/A (tests already passing)
- REFACTOR: This entire task

## Risk Mitigation

**Risk:** Build breaks during rename
- **Mitigation:** Git checkpoint before changes, test after each rename

**Risk:** Tests fail after rename
- **Mitigation:** Incremental renames, rollback if needed

**Risk:** Documentation out of sync
- **Mitigation:** Update docs as part of task, not afterward

## Success Metrics

**Before:**
- [ ] Naming convention: Undocumented
- [ ] Consistency: Assumed but not verified
- [ ] Build system: Relies on current names

**After:**
- [x] Naming convention: Formally documented
- [x] Consistency: Verified and enforced
- [x] Build system: Updated and tested
- [x] All tests: Passing
- [x] Zero warnings: Confirmed

## Tools and Commands

```bash
# Analysis
ls -1d src/syntax/*/ | xargs -n1 basename | sort

# Verification (check for old names)
grep -r "old_name" src/ include/ tests/

# Build and test
cmake --build build && cd build && ctest

# Git operations
git mv src/syntax/old src/syntax/new
git add -A && git commit -m "refactor: standardize syntax naming"
```

## Notes

- **Current assessment:** Naming appears consistent already
- **Primary value:** Documentation and formal verification
- **Secondary value:** Establish pattern for future syntax plugins
