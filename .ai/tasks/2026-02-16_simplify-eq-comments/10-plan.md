# Implementation Plan

**Task:** Simplify .EQ comment handling
**Approach:** Replace complex logic with simple trim

---

## Steps

1. Find HandleEq in scmasm_directive_handlers.cpp (line ~206)
2. Remove complex comment detection (lines 210-248)
3. Replace with simple find_first_of whitespace trim
4. Test

## Code Change

**Before:** Complex heuristics (38 lines)
**After:** Simple trim (3 lines)

```cpp
std::string value_expr = operand;
size_t ws = value_expr.find_first_of(" \t");
if (ws != std::string::npos) value_expr = value_expr.substr(0, ws);
```

That's it!

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
