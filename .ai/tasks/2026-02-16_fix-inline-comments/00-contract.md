# Task Contract

**Task ID:** 2026-02-16_fix-inline-comments
**Beads Task:** xasm++-9hwl
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix parser to handle inline comments without semicolons on .EQ directive lines.

### Current Issue

A2osX assembly fails at line 252 in inc/macros.i:
```assembly
FPU.f			.EQ 180					float
```

**Error:** `Parse error: Unexpected character after expression: f`

**Root Cause:** The word "float" appears after the expression without a semicolon. The parser treats this as an unexpected token.

### Investigation Needed

**Question:** Does Merlin assembler support trailing text as implicit comments?

Many vintage assemblers allowed trailing descriptive text on certain directives:
```assembly
SYMBOL  .EQ  $1234    this is a comment (no semicolon needed)
```

This was common for:
- `.EQ` (equates/constants)
- `EQU` (alternate syntax)
- Sometimes `.DA`, `.DW`, etc.

### Possible Solutions

**Option A: Allow trailing text on .EQ lines**
- After parsing `.EQ value`, ignore remaining text on line
- Treat as implicit comment
- Most compatible with vintage code

**Option B: Improve error handling**
- Detect probable comment and suggest adding semicolon
- More strict, but clearer

**Option C: Support whitespace-delimited comments**
- If text appears after significant whitespace (multiple tabs/spaces), treat as comment
- Balance between compatibility and strictness

### Tasks

1. **Research Merlin behavior:**
   - Check if Merlin ignores trailing text on .EQ
   - Look for other examples in A2osX source

2. **Implement fix:**
   - Modify .EQ parsing to handle trailing text
   - Decide on approach (A, B, or C above)
   
3. **Add tests:**
   - .EQ with trailing comment (no semicolon)
   - .EQ with semicolon comment (should still work)
   - .EQ with no comment (should still work)

4. **Validate A2osX:**
   - Line 252 should now parse correctly
   - Continue to next issue

### Success Criteria

```
✓ Line 252 in inc/macros.i parses successfully
✓ .EQ with trailing text works
✓ .EQ with semicolon comments still works
✓ All existing tests pass
✓ Zero warnings
```

**Estimated Effort:** 1-2 hours

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
