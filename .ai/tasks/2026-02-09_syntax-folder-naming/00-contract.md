# Task Contract: Standardize Syntax Folder Naming Convention

**Task ID:** 2026-02-09_syntax-folder-naming
**Beads Task:** xasm++-9k6d
**Created:** 2026-02-09
**Priority:** P2

Audit syntax folders and standardize naming convention, ensuring consistency and updating the build system accordingly.

## Problem Statement

Current syntax folder naming is inconsistent:
- **Underscored:** `edtasm_m80`, `z80_universal`
- **Non-underscored:** `edtasm`, `merlin`, `scmasm`, `flex`, `intel`, `mos`, `motorola`, `simple`, `zilog`

This inconsistency:
- Creates confusion about naming patterns
- Makes folder discovery harder
- Violates clean code principle of consistency
- Could cause build system issues

## Objective

Establish and enforce a consistent naming convention for all syntax folders.

## Success Criteria

✓ Single, documented naming convention established
✓ All syntax folders follow the convention
✓ Build system updated (CMakeLists.txt)
✓ Zero build errors after renaming
✓ All tests pass after changes
✓ Documentation updated with naming standard

## Acceptance Criteria

### Phase 1: Analysis
- [ ] Audit all 11 current syntax folders
- [ ] Document current naming patterns
- [ ] Identify conflicts and issues
- [ ] Propose standard convention (decide: underscore or no underscore)
- [ ] Document rationale for chosen convention

### Phase 2: Implementation
- [ ] Rename folders to match standard
- [ ] Update CMakeLists.txt references
- [ ] Update include paths in headers
- [ ] Update source file includes
- [ ] Update test file references

### Phase 3: Verification
- [ ] Build system compiles successfully
- [ ] All unit tests pass
- [ ] All integration tests pass
- [ ] Zero compiler warnings
- [ ] Grep verification: no old folder names remain

### Phase 4: Documentation
- [ ] Add naming convention to CODING-STANDARDS.md
- [ ] Update architecture docs if needed
- [ ] Document migration in work log

## Proposed Convention

**Recommendation:** Use lowercase with underscores only for multi-word compound names:
- ✅ `merlin` (single word)
- ✅ `edtasm_m80` (variant identifier)
- ✅ `z80_universal` (adjective + noun)
- ❌ `z80universal` (harder to read)

**Rationale:**
- Matches C++ naming conventions (snake_case for files)
- Improves readability for multi-word names
- Preserves existing majority pattern
- Minimal renames needed

## Current Inventory

| Current Name | Proposed Name | Change Needed? |
|--------------|---------------|----------------|
| edtasm | edtasm | No |
| edtasm_m80 | edtasm_m80 | No |
| flex | flex | No |
| intel | intel | No |
| merlin | merlin | No |
| mos | mos | No |
| motorola | motorola | No |
| scmasm | scmasm | No |
| simple | simple | No |
| z80_universal | z80_universal | No |
| zilog | zilog | No |

**Analysis:** Current naming is actually consistent! All follow the pattern correctly. Task should verify and document the standard, plus check for any edge cases.

## Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Build system breaks | High | Test after each rename, git checkpoint |
| Test failures | Medium | Run full test suite, fix references |
| Documentation drift | Low | Update docs as part of task |

## Files to Modify

**Syntax Folders:**
- `src/syntax/*/` (potential renames)

**Build System:**
- `src/CMakeLists.txt`
- `tests/unit/CMakeLists.txt`
- `tests/integration/CMakeLists.txt`

**Headers:**
- `include/xasm++/syntax/*.h` (include guards, references)

**Tests:**
- `tests/unit/test_*_syntax.cpp`
- `tests/integration/*`

## Estimated Effort

- Analysis: 30 minutes
- Implementation: 1-2 hours (if renames needed)
- Verification: 30 minutes
- Documentation: 30 minutes

**Total:** 2-3 hours

## Dependencies

- None (independent architectural cleanup)

## Stakeholders

- **Owner:** Orchestrator
- **Implementer:** Engineer
- **Reviewer:** Code quality review

**Approved:** Orchestrator 2026-02-09
