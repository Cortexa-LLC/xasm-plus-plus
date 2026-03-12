# Work Log

**Task ID:** xasm-94e
**Started:** 2026-03-11
**Status:** Completed

---

## Work Sessions

### Session 1: 2026-03-11

#### Objectives for This Session
- Investigate Bug B off-by-1 issue
- Fix label address assignment for .DO and .LU directives
- Update tests to reflect correct behavior
- Verify all tests pass

#### Work Completed
✓ Identified root cause: LabelAtom was being added AFTER the block content in the atoms list
✓ Fixed HandleDo to insert LabelAtom at correct position (before block content)
✓ Fixed HandleLu with the same approach
✓ Updated test expectations to match correct behavior
✓ All 1864 tests passing

**Files Modified:**
- `src/syntax/scmasm_syntax.cpp` - Fixed HandleDo and HandleLu
  - Capture `start_address` at beginning of function
  - Capture `label_atom_position` before processing block
  - Insert LabelAtom at saved position (not push_back at end)
- `tests/unit/test_scmasm_conditionals.cpp` - Updated test expectations
  - `LabelOnDoLineGetsAddressAtStart` - Renamed and fixed expectations
  - `LabelOnDoLineWithElseTrue` - DIB should be at $3000, not $3004
  - `LabelOnDoLineWithElseFalse` - DIB should be at $3000, not $3002

**Root Cause:**
The previous Bug B fix moved label definition to after block processing, but it added the LabelAtom to the end of the atoms list. When Assembler::ResolveSymbols() processes atoms in order, it recomputes each label's address based on the current position in the atom stream. Since the LabelAtom was at the END (after all block content), the label got assigned the address AFTER the block instead of at the start.

**Solution:**
Insert the LabelAtom at the position in the atoms list corresponding to the START of the .DO/.LU block (i.e., the position before any block content is added). This ensures ResolveSymbols computes the correct address when it encounters the LabelAtom.

**Tests Added:**
None - existing tests were updated to expect correct behavior.

**Blockers:**
None

**Next Steps:**
- Task complete
- Ready for validation against A2osX source files
