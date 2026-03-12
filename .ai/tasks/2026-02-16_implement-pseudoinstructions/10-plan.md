# Implementation Plan

**Task ID:** 2026-02-16_implement-pseudoinstructions
**Created:** 2026-02-16
**Plan Version:** 1.0

---

## Approach Summary

Implement SCMASM pseudo-instruction (macro invocation) support using `>` prefix.

**Key Decisions:**
- Investigate if `>` is macro invocation syntax in SCMASM
- Check if .MA/.EM macro definition already works
- Determine if macros already defined in inc/macros.i
- Implement macro expansion mechanism if needed

---

## Implementation Phases

### Phase 1: Investigation (1-2 hours)

**Understand the syntax:**
```
Step 1.1: Check inc/macros.i for macro definitions
Step 1.2: Determine if `>` prefix is SCMASM standard
Step 1.3: Verify .MA/.EM directives work
Step 1.4: Understand macro invocation mechanism
```

### Phase 2: Design (1 hour)

**Options:**
- Option A: `>` is macro call prefix - implement invocation
- Option B: Macros already work - fix parsing of `>` prefix
- Option C: Pseudo-instructions are built-in - implement as directives

### Phase 3: Implementation (2-4 hours)

**Based on investigation, implement chosen approach**

### Phase 4: Testing (1 hour)

**Verify:**
- A2osX.S.txt assembles 100%
- Macro expansion works correctly
- No test regressions

---

## Timeline Estimate

Total: 4-8 hours (investigation-heavy)

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
