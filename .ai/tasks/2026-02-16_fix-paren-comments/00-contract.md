# Task Contract

**Task ID:** 2026-02-16_fix-paren-comments
**Beads Task:** xasm++-sznc
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix expression parser/comment handling for parenthetical inline comments.

### Problem

A2osX line 392: `S.MEM.F.INUSE .EQ %10000000  (Internal Flag)`

Error: `Unexpected character after expression: (`

### Root Cause Analysis Needed

The current HandleEq has enhanced comment detection, but this error suggests:
1. Comment stripping may not be catching parentheses, OR
2. Expression parser is being called with the full string before comment stripping

### Investigation

Check order of operations in HandleEq:
1. Does comment stripping happen BEFORE EvaluateExpression?
2. Does the comment detection logic handle parentheses?
3. Is expression parser rejecting the `(` character?

### Success Criteria

```
✓ Parenthetical comments work: .EQ value (comment)
✓ A2osX line 392 assembles
✓ Tests pass
✓ Zero warnings
```

**Estimated Effort:** 1 hour

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
