# Implementation Plan: EDTASM Remaining Handlers

**Workflow:** Refactor (continuation of xasm++-hhbr)

---

## Strategy: Incremental by Type

1. Data directives (simple) - 3 hours
2. Macros (complex) - 3-4 hours
3. Remaining (misc) - 1 hour

**Total:** 7-8 hours

---

## Phases

### Phase 0: Setup (15 min)
- Review xasm++-hhbr work
- Identify which handlers still need implementation
- Plan order

### Phase 1: Data Directives (3 hours)
- Implement DB, DW, DS variants
- DEFB, DEFW, DEFM, DEFS
- DC, DM, DZ
- Test after each group of 5

### Phase 2: Macros (3-4 hours)
- MACRO, ENDM, LOCAL, EXITM
- REPT, IRP, IRPC
- May require macro processor state
- Reference Merlin macro implementation

### Phase 3: Remaining (1 hour)
- IF1, IF2, IFB, IFNB
- INCLUDE, PUBLIC, EXTERN
- Final test

---

**Estimated:** 7-8 hours
