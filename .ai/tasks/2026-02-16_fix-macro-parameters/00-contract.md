# Task Contract

**Task ID:** 2026-02-16_fix-macro-parameters
**Beads Task:** xasm++-f0hn
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Verify and fix macro parameter handling based on investigation findings.

### Current State (from xasm++-bk49)

- ✅ .MA/.EM directives implemented
- ✅ `>` prefix recognized
- ❓ Parameter syntax needs verification
- 📋 56 A2osX macros documented

### Tasks

1. **Verify parameter syntax:**
   - Test `]1`-`]9` parameters
   - Test `]#` parameter count
   - Compare with `\N` syntax (may be old)

2. **Fix any issues found**

3. **Test with A2osX macros:**
   - >LDYAI, >STYA, >LDYA
   - Verify expansion works

### Success Criteria

```
✓ Parameter substitution works (]N syntax)
✓ Parameter count works (]#)
✓ A2osX macros expand correctly
✓ Tests added
✓ Zero warnings
```

**Estimated Effort:** 2-3 hours

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
