# Task Contract

**Task ID:** 2026-02-16_fix-symbol-dots
**Beads Task:** xasm++-ufcx
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix symbol/identifier parsing to support dots in symbol names for namespacing.

### Problem

A2osX line 59: `A2osX.LIBC .EQ A2osX.GP+00`

Error: `Unexpected character after expression: .`

### Investigation Needed

1. Check if expression parser already supports dots in identifiers
2. Check if symbol table supports dots in symbol names
3. Identify where parsing fails (expression parser, symbol parser, or both)

### Common Patterns in A2osX

- `A2osX.GP`, `A2osX.LIBC`, `A2osX.FPU` - Namespace prefixes
- `MSG.INIT0`, `MSG.SYSMEM` - Message identifiers
- Dots used extensively for organization

### Success Criteria

```
✓ Symbols with dots parse correctly
✓ A2osX line 59 assembles
✓ Tests pass
✓ Zero warnings
```

**Estimated Effort:** 1-2 hours

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
