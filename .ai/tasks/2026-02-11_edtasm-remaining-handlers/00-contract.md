# Task Contract: EDTASM Phase 6c.3 - Complete Remaining Handlers

**Task ID:** 2026-02-11_edtasm-remaining-handlers
**Beads Task:** xasm++-fax8
**Created:** 2026-02-11
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Refactor (continuation)

---

## Task Description

Complete EDTASM directive handler extraction by implementing the remaining ~21 handlers. Previous work (xasm++-hhbr) extracted 26 handlers (55%). This task completes the remaining 45%.

### Previous Work (xasm++-hhbr)

**Completed (26 handlers):**
- Segment Control (3): ASEG, CSEG, DSEG
- Conditionals (11): IF, ELSE, ENDIF, IFDEF, IFNDEF, IFEQ, IFNE, IFLT, IFLE, IFGT, IFGE
- Listing Control (8): LIST, NOLIST, *LIST, LALL, SALL, TITLE, EJECT, SPACE
- Miscellaneous (4): NAME, RADIX, *RADIX, END

**Status:** 96% tests passing (118/123)

### Remaining Work (~21 handlers)

Based on skeleton in `edtasm_directive_handlers.h`:

**Data Directives (~8):**
- DB, DW, DS (EDTASM variants)
- DEFB, DEFW, DEFM, DEFS
- DC, DM, DZ

**Macro Directives (~7):**
- MACRO, ENDM
- LOCAL, EXITM
- REPT, IRP, IRPC

**Additional Conditionals (~3):**
- IF1, IF2 (pass conditionals)
- IFB, IFNB (blank checks)

**Other (~3):**
- INCLUDE
- PUBLIC, EXTERN

---

## Success Criteria

```
✓ All remaining handlers implemented (not just skeletons)
✓ Registry updated to call handlers
✓ Build succeeds with 0 warnings
✓ Tests: 96%+ maintained or improved
✓ Phase 6c.3 complete (all EDTASM handlers extracted)
```

---

## Implementation Strategy

### Incremental Approach (Groups of 5)

**Group 1: Data Directives** (2 hours)
- DB, DW, DS, DEFB, DEFW (~5 handlers)
- Test after group

**Group 2: Remaining Data** (1 hour)
- DEFM, DEFS, DC, DM, DZ (~4 handlers)
- Test after group

**Group 3: Macros** (2-3 hours)
- MACRO, ENDM, LOCAL, EXITM, REPT (~5 handlers)
- May be complex - similar to Merlin macros
- Test after group

**Group 4: Advanced Macros** (1 hour)
- IRP, IRPC (~2 handlers)
- Test after group

**Group 5: Remaining** (1 hour)
- IF1, IF2, IFB, IFNB, INCLUDE, PUBLIC, EXTERN (~5 handlers)
- Test after group

---

## Reference Pattern

Follow xasm++-hhbr pattern:
1. Implement handler function body
2. Update RegisterEdtasmDirectiveHandlers()
3. Test incrementally
4. Build with 0 warnings

---

## Estimated Complexity

**Complexity:** Medium-Large

**Rationale:**
- ~21 handlers to implement
- Macros may be complex
- Data directives relatively simple
- Estimated: 6-8 hours

**Files:** 3-4 files (same as before)
**Tokens:** ~20K (SAFE)

---

## Approvals

**Contract Approved By:**
- [X] Requestor: Orchestrator (2026-02-11)
- [ ] Agent: Engineer (TBD)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-11
