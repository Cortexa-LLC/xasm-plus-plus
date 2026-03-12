# Task Contract

**Task ID:** 2026-02-16_fix-complex-labels
**Beads Task:** xasm++-v6d4
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix parser to handle complex labels with multiple dots and numbers (e.g., `A2osX.Init1.128`).

### Current Behavior

Label `A2osX.Init1.128` fails with:
```
Parse error: <source>:76: Unexpected character after expression: 5
```

Parser is confused by `.128` suffix (multiple dots + numbers).

### Success Criteria

```
✓ Labels like A2osX.Init1.128 parse correctly
✓ Tests added for complex label patterns
✓ A2osX.S.txt assembles past line 76
✓ All existing tests still pass
✓ Zero compiler warnings
```

---

## Implementation

**Investigation:**
- Debug expression parser handling of dots in identifiers
- Check if dots are being misinterpreted as decimal points
- Test with various patterns: `A.B.1`, `A.1.B`, `A.1.2`

**Estimated Effort:** 2-3 hours

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
