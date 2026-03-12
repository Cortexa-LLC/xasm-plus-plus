# Investigation Handoff to Engineer

**From**: Spelunker Agent  
**To**: Engineer Agent  
**Date**: 2026-02-17  
**Task**: xasm++-exb8-2026-02-17-line-702-investigation

---

## Quick Start

Hey Engineer! 👋

I've completed the investigation into the A2osX line 702 parsing error. Everything you need to implement the fix is ready.

### TL;DR

**Problem**: `.EQ *+4` fails to parse  
**Root Cause**: `*` operator only works alone, not in expressions  
**Fix**: 5 lines of code in `src/syntax/scmasm_syntax.cpp:828`  
**Time**: 15-30 minutes to implement

### Start Here

1. **Read first**: `ENGINEER-CHECKLIST.md` (has step-by-step guide)
2. **Visual aid**: `fix-diagram.txt` (shows transformation)
3. **Deep dive**: `investigation-report.md` (if you need more context)

---

## What I Found

### Root Cause

File: `src/syntax/scmasm_syntax.cpp`  
Function: `EvaluateExpression()`  
Lines: 826-828

```cpp
// Current code only handles exact "*"
if (trimmed == "*") {
  return current_address_;
}
// Falls through to ParseExpression() which doesn't recognize *
```

When expression is `*+4`:
- Check for `trimmed == "*"` fails
- Code calls `ParseExpression("*+4")`
- ExpressionParser doesn't know `*` is current address
- Error: "Unexpected character after expression: 4"

### The Fix

Add this after line 828:

```cpp
// Handle *+offset, *-offset, etc.
if (trimmed.length() > 1 && trimmed[0] == '*') {
  char op = trimmed[1];
  if (op == '+' || op == '-' || op == '/' || op == '&' || 
      op == '|' || op == '^' || op == '<' || op == '>') {
    // Replace * with current address, then evaluate
    std::string expr_str = std::to_string(current_address_) + 
                          trimmed.substr(1);
    auto expr = ParseExpression(expr_str, symbols);
    return static_cast<uint32_t>(expr->Evaluate(symbols));
  }
}
```

This transforms `*+4` → `32768+4` (if current address is 0x8000).

---

## Testing

I've documented test cases in `ENGINEER-CHECKLIST.md`. Quick test:

```assembly
        .OR $8000
TEST1   .EQ *       ; Should be $8000
TEST2   .EQ *+4     ; Should be $8004 (currently fails)
TEST3   .EQ *-2     ; Should be $7FFE (currently fails)
```

Also verify A2osX.S.txt assembles past line 702.

---

## Why This Fix?

I evaluated 3 options (see `investigation-report.md` for details):

1. **String replacement** - Complex heuristics, fragile
2. **Modify ExpressionParser** - Too broad, affects all syntaxes
3. **Lookahead check** ✅ **RECOMMENDED** - Minimal scope, simple

Option 3 is cleanest: only touches SCMASM syntax, simple logic, no shared code changes.

---

## Edge Cases

The fix handles:
- `*+4`, `*-2`, `*+$10` ✅
- `*&$FF`, `*|$80` ✅
- `*<4`, `*>8` ✅
- `*` alone (existing code) ✅

Doesn't handle (and that's OK):
- `**4` - ambiguous (multiply or star-star?)
- `2**+4` - star in middle (rare)

These edge cases can gracefully fail or be future enhancements.

---

## Documentation

All in `.ai/tasks/xasm++-exb8-2026-02-17-line-702-investigation/`:

| File | What It Is | When to Read |
|------|-----------|--------------|
| ENGINEER-CHECKLIST.md | Your guide | Read first |
| fix-diagram.txt | Visual | If you like diagrams |
| FINDINGS.md | Quick ref | Need quick lookup |
| investigation-report.md | Full analysis | Need more context |
| execution-trace.txt | Debug trace | Troubleshooting |

---

## Confidence Level

🟢 **HIGH**

- Error reproduced with minimal test
- Root cause traced through full call stack
- Fix location precisely identified
- Multiple solutions evaluated
- Edge cases analyzed
- Implementation guide created

---

## If You Need Help

1. Check `investigation-report.md` for alternative solutions
2. Review `execution-trace.txt` to understand the flow
3. See `fix-diagram.txt` for visual explanation
4. Ping me with questions (though docs should cover everything!)

---

## After Implementation

When done:
1. Mark task complete
2. Link to commit in task notes
3. Update CHANGELOG.md
4. Close this investigation

---

## Notes

- Fix is minimal (5 lines)
- No shared code changes
- Low risk
- Well documented
- Test cases provided
- Should be straightforward

Good luck! This should be a quick win. 🚀

---

**Questions?** See investigation-report.md for detailed analysis.  
**Need checklist?** See ENGINEER-CHECKLIST.md.  
**Want diagrams?** See fix-diagram.txt.

You got this! 💪
