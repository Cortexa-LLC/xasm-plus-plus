# Task Contract

**Task ID:** 2026-02-16_implement-pseudoinstructions
**Beads Task:** xasm++-scrs
**Created:** 2026-02-16
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement SCMASM pseudo-instruction (macro) support for A2osX compatibility.

### Current State

A2osX.S.txt fails with 84 errors for pseudo-instructions like:
```
>LDYAI MSG.SYSMEM.KO
>STYA ZP.PARAM1
>LDYA ZP.PARAM1
>DEBUGOA
```

The `>` prefix indicates a pseudo-instruction (macro call in SCMASM syntax).

### Investigation

1. Check if macros defined in inc/macros.i
2. Understand SCMASM macro syntax (>>>)
3. Check if .MA/.EM macros already work
4. Determine if `>` is macro invocation or something else

### Success Criteria

```
✓ Pseudo-instructions recognized and expanded
✓ A2osX.S.txt assembles 100% (no errors)
✓ Output binary generated
✓ Tests added for pseudo-instruction support
✓ Zero compiler warnings
```

---

**Estimated Effort:** 4-8 hours (complex feature)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
