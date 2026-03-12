# Task Contract

**Task ID:** 2026-02-12_edtasm-macros
**Beads Task:** xasm++-pdmh
**Created:** 2026-02-12
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement 7 EDTASM-M80++ macro system directives for macro definition, expansion, and iteration.

### Background and Context

Macros are essential for code reuse in assembly. EDTASM-M80++ supports macro definition (MACRO/ENDM), local symbols (LOCAL), early exit (EXITM), repetition (REPT), and parameter iteration (IRP/IRPC). A similar macro processor already exists for FLEX ASM and can be adapted for EDTASM-M80++ syntax.

### Current State

- No macro system implemented for EDTASM-M80++
- FLEX ASM macro processor exists and can be reused
- Macro infrastructure pattern established

### Desired State

7 macro directives fully implemented:
- MACRO/ENDM - Define and end macros
- LOCAL - Local label uniquification
- EXITM - Early macro exit
- REPT - Repeat block N times
- IRP - Iterate over parameter list
- IRPC - Iterate over characters

---

## Success Criteria

```
✓ All 7 macro directives implemented
✓ Parameter substitution (& and % operators)
✓ Local label uniquification
✓ Nested macro support
✓ All tests passing
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ MACRO/ENDM define and expand macros
□ LOCAL creates unique labels per expansion
□ EXITM exits macro early
□ REPT repeats block N times
□ IRP iterates over comma-separated list
□ IRPC iterates over string characters
□ Parameter substitution works (&param, %param)
□ Nested macros supported
```

### Quality Requirements
```
□ Can reuse FLEX macro processor pattern
□ TDD process followed
□ Comprehensive macro tests
□ No reduction in test pass rate
```

---

## Technical Approach

**Files to Create/Modify:**
- Macro processor (adapt from FLEX)
- 7 directive handlers
- Parameter substitution engine
- Local label generator

**Estimated:** 8-12 hours, 5-6 files, ~24K tokens

**Reference:** FLEX ASM macro processor (src/syntax/flex_syntax.cpp)

---

## Related Documents

- **Reference:** docs/planning/not-yet-implemented-items.md (Section 2)
- **Existing Implementation:** FLEX macro processor
