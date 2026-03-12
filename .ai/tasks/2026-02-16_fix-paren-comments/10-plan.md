# Implementation Plan

**Task:** Fix parenthetical inline comments
**Workflow:** Bugfix
**Role:** Engineer

---

## Investigation Steps

1. Check current HandleEq implementation
2. Verify comment stripping happens before EvaluateExpression
3. Test if comment detection catches parentheses
4. Debug with test case: `.EQ %10000000 (test)`

## Likely Issues

**Option 1:** Comment detection doesn't recognize `(` as start of comment
- Current code checks for alphanumeric after whitespace
- `(` is not alphanumeric - won't trigger comment detection

**Option 2:** Trim() is stripping needed whitespace
- Check if whitespace is preserved for detection

## Fix Strategy

Enhance comment detection to recognize common comment patterns:
- `(comment)` - parenthetical
- `text comment` - word-based (already handled)

Update condition to check for `(` as comment indicator.

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
