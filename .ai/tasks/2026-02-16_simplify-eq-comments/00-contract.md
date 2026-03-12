# Task Contract

**Task ID:** 2026-02-16_simplify-eq-comments
**Beads Task:** xasm++-ku41
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Simplify .EQ comment handling to strip everything after expression.

### Current Problem

HandleEq has complex comment detection logic (lines 210-248) that fails on edge cases.

### Simple Solution

After getting operand string, find first whitespace in the value expression.
Trim everything after that whitespace - it's all comments.

**File:** `src/syntax/scmasm_directive_handlers.cpp`
**Function:** `HandleEq()` around line 206

### Implementation

```cpp
void HandleEq(...) {
  RequireOperand(operand, ".EQ", context);
  
  // Simple: trim after first whitespace
  std::string value_expr = operand;
  size_t ws = value_expr.find_first_of(" \t");
  if (ws != std::string::npos) {
    value_expr = value_expr.substr(0, ws);
  }
  
  // Evaluate and define
  uint32_t value = EvaluateExpression(value_expr, ...);
  context.symbols->Define(label, ...);
}
```

### Success Criteria

```
✓ All A2osX .EQ lines parse
✓ Tests pass
```

**Estimated Effort:** 15 minutes

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
