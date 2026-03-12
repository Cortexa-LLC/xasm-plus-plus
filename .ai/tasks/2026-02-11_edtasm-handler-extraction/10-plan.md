# Implementation Plan: EDTASM Handler Extraction

**Task:** Complete Phase 6c.3 - Extract 47 EDTASM directive handlers
**Workflow:** Refactor
**Estimated Time:** 6-8 hours

---

## Execution Strategy

### Strategy Choice: Sequential Implementation

**Rationale:**
- 47 handlers is too many for parallel extraction
- Incremental testing needed (build + test after every 5-10 handlers)
- Pattern is mechanical and repetitive (good for sequential)
- Single engineer can complete efficiently

---

## Phase Breakdown

### Phase 0: Preparation (30 minutes)

**Step 0.1: Read Pattern References** (15 min)
- Read `.ai/tasks/2026-02-10_god-class-phase6b-core/ENGINEER-REPORT.md`
- Read `.ai/tasks/2026-02-11_god-class-phase6c1-merlin/40-completion-summary.md`
- Understand the free function + registration pattern

**Step 0.2: Analyze Current State** (15 min)
- Read `edtasm_directive_handlers.h` (declarations)
- Read `edtasm_directive_handlers.cpp` (skeleton)
- Read `InitializeDirectiveRegistry()` in `edtasm_m80_plusplus_syntax.cpp`
- List all 47 handlers to extract
- Identify magic numbers/strings to eliminate

---

### Phase 1: Setup Constants (30 minutes)

**Step 1.1: Extend directive_constants.h** (15 min)
- Add EDTASM directive name constants
- Create `edtasm::directives` namespace
- Define all 47 directive names (IF, ELSE, ENDIF, DB, DW, etc.)

**Step 1.2: Verify opcodes_z80.h** (15 min)
- Check if all Z80 opcodes are defined
- Add any missing constants
- Document which opcodes are needed

---

### Phase 2: Handler Implementation (4-5 hours)

**Incremental Approach:**
- Extract handlers in groups of 5-10
- Build and test after each group
- Fix any issues before continuing

**Group 1: Data Definition Directives** (1 hour)
- DB, DW, DS, DEFB, DEFW, DEFM, DEFS (~8 handlers)
- These are similar, can extract quickly
- Checkpoint: Build + test

**Group 2: Conditional Assembly** (1.5 hours)
- IF, ELSE, ENDIF, IFT, IFF, IFDEF, IFNDEF (~13 handlers)
- More complex logic
- Checkpoint: Build + test

**Group 3: Macro System** (1 hour)
- MACRO, ENDM, LOCAL, EXITM, REPT, IRP, IRPC (~7 handlers)
- May have parser state dependencies
- Checkpoint: Build + test

**Group 4: Listing Control** (30 min)
- LIST, NOLIST, TITLE, PAGE (~3 handlers)
- Simple implementations
- Checkpoint: Build + test

**Group 5: Miscellaneous** (1 hour)
- ORG, EQU, SET, INCLUDE, END, PUBLIC, EXTERN (~9 handlers)
- Various complexity levels
- Checkpoint: Build + test

**Group 6: Remaining Handlers** (1 hour)
- Any handlers not yet implemented
- Edge cases and special directives
- Final checkpoint: Build + test

---

### Phase 3: Registry Integration (1 hour)

**Step 3.1: Update InitializeDirectiveRegistry()** (30 min)
- Replace inline lambdas with calls to free functions
- Keep lambda wrapper, delegate to handler
- Example:
  ```cpp
  registry_.Register(DIR_IF, [this](Section& section, SymbolTable& symbols,
                                     const std::string& line) {
      std::string operand = ParseOperand(line);
      HandleIfDirective(section, symbols, operand);
  });
  ```

**Step 3.2: Verify All Handlers Registered** (15 min)
- Check that all 47 handlers have registry entries
- Verify no duplicate registrations
- Confirm directive names match constants

**Step 3.3: Remove Old Lambda Bodies** (15 min)
- Clean up any leftover inline implementations
- Verify parser class is now ~600 lines (down from 1858)

---

### Phase 4: Final Verification (1-2 hours)

**Step 4.1: Full Test Suite** (30 min)
```bash
cmake --build build
ctest --test-dir build -R EdtasmM80PlusPlusSyntaxTest
```

**Step 4.2: Fix Any Test Failures** (30-60 min)
- Analyze failures
- Compare with original lambda logic
- Fix and retest

**Step 4.3: Build Verification** (15 min)
- Verify 0 warnings
- Check no unused variables/functions
- Confirm all includes correct

**Step 4.4: Magic String/Number Audit** (15 min)
- Grep for hardcoded strings in handlers
- Grep for magic hex numbers
- Verify all use constants

---

## Success Checkpoints

After each group of handlers:
- ✅ Build succeeds with 0 warnings
- ✅ Tests still pass (96%+)
- ✅ No magic strings in new code
- ✅ No magic numbers in new code

Final checkpoint:
- ✅ All 47 handlers implemented
- ✅ Registry updated
- ✅ Parser reduced to ~600 lines
- ✅ All tests passing
- ✅ 0 warnings
- ✅ Zero magic strings/numbers

---

## Rollback Plan

If extraction fails for a handler:
1. Revert changes for that handler
2. Leave lambda implementation in place
3. Document why extraction failed
4. Continue with other handlers
5. Revisit problematic handler at end

---

## Tools and Commands

**Build:**
```bash
cmake --build build
```

**Test:**
```bash
ctest --test-dir build -R EdtasmM80PlusPlusSyntaxTest
ctest --test-dir build -R EdtasmDirectiveHandlersTest  # If exists
```

**Search for Magic Strings:**
```bash
grep -n '"[A-Z][A-Z]"' src/directives/edtasm/edtasm_directive_handlers.cpp
```

**Search for Magic Numbers:**
```bash
grep -n '0x[0-9A-Fa-f]' src/directives/edtasm/edtasm_directive_handlers.cpp
```

**Check File Size:**
```bash
wc -l src/syntax/edtasm/edtasm_m80_plusplus_syntax.cpp
# Target: ~600 lines (down from 1858)
```

---

## Dependencies

**Depends On:**
- xasm++-5j00 (skeleton infrastructure) - ✅ Complete
- Phase 6b pattern - ✅ Established
- Phase 6c.1 Merlin - ✅ Complete (reference)
- Phase 6c.2 SCMASM - ✅ Complete (reference)

**Blocks:**
- Nothing (completes Phase 6c)

---

## Risk Mitigation

**Risk: Time overrun (>8 hours)**
- Mitigation: Focus on core functionality, can polish in follow-up
- Fallback: Extract first 30 handlers, create follow-up task for remaining

**Risk: Test failures cascade**
- Mitigation: Incremental testing prevents cascade
- Fallback: Revert last group, fix issues, retry

**Risk: Pattern doesn't fit some handlers**
- Mitigation: Document exceptions, keep as member functions if needed
- Note: Phase 6c.1 kept some handlers as members (macros with state)

---

**Plan Approved:** 2026-02-11
**Next Action:** Begin Phase 0 (Preparation)
