# Task Contract: Z80 Phase 12 - Macro System Implementation

**Task ID:** 2026-02-10_z80-phase12-macros
**Beads Task:** xasm++-j459
**Created:** 2026-02-10
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement complete macro system for EDTASM-M80++ parser supporting MACRO/ENDM, REPT, IRP, IRPC with parameter substitution and nested macros.

### Background and Context

Phase 10 identified macros as the #1 limitation. Macros are essential for:
- Code reuse (MACRO/ENDM)
- Repetition (REPT blocks)
- Iteration (IRP/IRPC)
- Local symbols (LOCAL)
This is a substantial feature used extensively in real Z80 code.

### Current State

No macro support. Users must hand-expand macros or use includes.

### Desired State

Full macro system:
- MACRO/ENDM macro definition and expansion
- REPT repeat blocks
- IRP/IRPC iteration macros
- LOCAL symbol support for unique labels
- EXITM early exit from macros
- Parameter substitution (& and %)
- Nested macros (macros calling macros)

---

## Success Criteria

```
✓ MACRO/ENDM working with parameters
✓ REPT working with count
✓ IRP/IRPC iteration working
✓ LOCAL symbols unique per expansion
✓ EXITM early exit working
✓ Parameter substitution (&param, %digit)
✓ Nested macros working
✓ All tests passing (97%+)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ MACRO name param1,param2 / ENDM
□ Macro invocation with arguments
□ REPT count / ENDM
□ IRP parameter,<list> / ENDM
□ IRPC parameter,<string> / ENDM
□ LOCAL label1,label2 (unique per expansion)
□ EXITM (early exit)
□ &param parameter substitution
□ % numeric parameter substitution
□ Nested macro calls
□ Error handling for undefined macros
```

### Quality Requirements
```
□ All tests passing
□ No warnings
□ TDD followed
□ Comprehensive test coverage
```

---

## Estimated Complexity

**Complexity:** Medium-Large

**Files:** 6 (macro processor, parser integration, symbol table, tests)
**Tokens:** 18K → ✅ SAFE

---

## Dependencies

**Depends on:** Nothing (can reference existing MacroProcessor from god-class Phase 5)
**Blocks:** Nothing

---

**Contract Approved:** ✓ Orchestrator 2026-02-10
