# Quick Reference: Line 702 Bug Findings

## The Bug

**Error**: `Unexpected character after expression: 4` at line 702  
**Line**: `A2osX.QC.B.BOOT	.EQ *+4					CLD JMP $FFFF`  
**Symptom**: Expression `*+4` fails to parse

## Root Cause

**Location**: `src/syntax/scmasm_syntax.cpp:826-828`

```cpp
// Handle * (current address) - special case before general parsing
if (trimmed == "*") {
  return current_address_;
}
```

**Problem**: Only handles `*` when it's the **entire** expression.

When expression is `*+4`:
1. Check `trimmed == "*"` fails (it's `*+4`)
2. Falls through to `ParseExpression()`
3. `ExpressionParser` doesn't recognize `*` as current address
4. Parser tries to interpret `*` as multiply operator
5. Fails because no left operand exists
6. Error: "Unexpected character after expression: 4"

## The Fix

**Add after line 828** in `scmasm_syntax.cpp`:

```cpp
// Handle *+offset, *-offset, etc.
if (trimmed.length() > 1 && trimmed[0] == '*') {
  char op = trimmed[1];
  if (op == '+' || op == '-' || op == '/' || op == '&' || 
      op == '|' || op == '^' || op == '<' || op == '>') {
    // Replace * with current address, then evaluate
    std::string expr_str = std::to_string(current_address_) + trimmed.substr(1);
    auto expr = ParseExpression(expr_str, symbols);
    return static_cast<uint32_t>(expr->Evaluate(symbols));
  }
}
```

## Test Case

File: `test_star_operator.s`

```assembly
        .OR $8000
TEST1   .EQ *        ; Works (returns $8000)
TEST2   .EQ *+4      ; FAILS (should return $8004)
TEST3   .EQ *-2      ; FAILS (should return $7FFE)
```

## Impact

- 2 occurrences in A2osX.S.txt (lines 702, 703)
- Blocks A2osX assembly
- Rare pattern overall
- Workaround: use explicit addresses

## Files

- **Investigation**: `.ai/tasks/xasm++-exb8-2026-02-17-line-702-investigation/investigation-report.md`
- **Work Log**: `.ai/tasks/xasm++-exb8-2026-02-17-line-702-investigation/work-log.md`
- **Test**: `test_star_operator.s`
- **Source**: `src/syntax/scmasm_syntax.cpp:821-880`
