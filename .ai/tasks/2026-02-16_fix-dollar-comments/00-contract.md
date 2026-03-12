# Task Contract

**Task ID:** 2026-02-16_fix-dollar-comments
**Beads Task:** xasm++-hbcy
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Enhance comment detection to handle dollar-sign documentation patterns.

### Problem

Line: `ROM.MSLOT .EQ $07F8  $Cn, where n•slot using $C800`
Error: `Unexpected character after expression: $`

### Analysis

Current comment detection (in HandleEq) recognizes:
- `(` parenthetical comments
- `/` slash comments  
- Alphanumeric text

But doesn't recognize `$Cn` patterns used for documentation.

### Solution

Add detection for `$` followed by non-hex or lowercase as comment indicator.

Example: `$Cn` (lowercase n), `$C800` (valid hex - not a comment)

### Success Criteria

```
✓ Dollar-sign documentation comments work
✓ Valid hex values still parse correctly
✓ A2osX rom.ii.i:16 assembles
✓ Tests pass
```

**Estimated Effort:** <1 hour

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
