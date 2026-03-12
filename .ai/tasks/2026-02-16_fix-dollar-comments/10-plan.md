# Implementation Plan

**Task:** Fix dollar-sign comments
**Workflow:** Bugfix
**Role:** Engineer

---

## Implementation

Enhance HandleEq comment detection around line 235:

```cpp
if (next == '(' || next == '/') {
  looks_like_comment = true;
} else if (next == '$') {
  // Check if $ pattern looks like documentation
  if (next_pos + 1 < value_expr.length()) {
    char after = value_expr[next_pos + 1];
    if (!std::isxdigit(after) || std::islower(after)) {
      looks_like_comment = true;  // $Cn, $n, etc.
    }
  }
} else if (std::isalnum(next) && ...) {
  // Existing logic
}
```

## Testing

Test case: `.EQ $07F8  $Cn, where n•slot`

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
