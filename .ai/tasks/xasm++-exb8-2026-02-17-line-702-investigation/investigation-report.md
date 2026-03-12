# Investigation Report: A2osX Line 702 Parsing Error

**Date**: 2026-02-17  
**Investigator**: Spelunker Agent  
**Issue**: 'Unexpected character after expression: 4' at line 702

---

## Summary

The error occurs when parsing `.EQ *+4` expressions in SCMASM syntax. The root cause is that the `*` operator (current address) is only recognized when it appears alone, not in expressions like `*+4`.

---

## Root Cause

**Location**: `src/syntax/scmasm_syntax.cpp:821-880`  
**Function**: `ScmasmSyntaxParser::EvaluateExpression()`

### Problem

The function has special handling for `*` (current address operator):

```cpp
// Handle * (current address) - special case before general parsing
if (trimmed == "*") {
  return current_address_;
}
```

**However**, this only works when `*` is the **entire expression**. When the expression contains operations like `*+4`, the code falls through to the general `ParseExpression()` call, which uses `ExpressionParser`.

The `ExpressionParser` class (in `src/common/expression_parser.cpp:389-398`) recognizes `$` as the current location operator, but does **NOT** recognize `*`. 

When `ExpressionParser` encounters `*` in `*+4`:
1. It tries to parse `*` as a symbol/identifier
2. This fails because `*` is not a valid identifier
3. Later, when parsing the full expression, it interprets `*` as the multiply operator
4. This creates a syntax error because there's no left operand for the multiply

---

## Evidence

### Line 702 in A2osX.S.txt

```
A2osX.QC.B.BOOT	.EQ *+4					CLD JMP $FFFF
```

Breaking down the structure:
- Label: `A2osX.QC.B.BOOT` (TAB-separated)
- Directive: `.EQ *+4` (TAB-separated from comment)
- Comment: `CLD JMP $FFFF` (tab-separated, not semicolon)

The operand to `.EQ` is `*+4`, which means "current address plus 4".

### Error Message

```
Parse error: Unexpected character after expression: 4
```

This error comes from `ExpressionParser::Parse()` (line 32-34 in `expression_parser.cpp`):

```cpp
if (!AtEnd()) {
  throw std::runtime_error("Unexpected character after expression: " +
                         std::string(1, Peek()));
}
```

The parser successfully parses something, but then encounters the `4` character and doesn't know what to do with it.

---

## Execution Flow Traced

1. **Line 702 parsed**: `.EQ *+4` extracted as directive and operand
2. **HandleEq called**: Passes operand `*+4` to `EvaluateExpression()`
3. **EvaluateExpression check**: `trimmed` is `*+4`, not `*`, so special case doesn't trigger
4. **Falls through** to `ParseExpression(trimmed, symbols)`
5. **ExpressionParser::Parse()** called with input `*+4`
6. **ParsePrimary()** encounters `*`:
   - Not a number (`$`, `%`, digit)
   - Not `$` followed by non-hex (current location check fails)
   - Tries to parse as identifier, but `*` is invalid
7. **Error thrown**: "Unexpected character after expression: 4"

---

## Impact

This affects any SCMASM code that uses the `*` operator in expressions:
- `.EQ *+4` (define constant relative to current address)
- `.OR *+100` (set origin relative to current address)
- `.DA *+2` (data address relative to current address)
- Branch instructions: `BCC *+5` (branch relative to current address)

---

## Related Code Patterns

From previous work logs, there was a similar fix attempted for the `*` operator:
- The fix replaced `*` with `$` before passing to ExpressionParser
- However, this fix may not be in the current codebase or may have been reverted

---

## Recommendations

### Option 1: Pre-process `*` to `$` (Minimal Scope)

Before calling `ParseExpression()`, replace `*` with `$` in the expression string:

```cpp
// In EvaluateExpression(), before calling ParseExpression():
std::string expr_for_parser = trimmed;

// Replace * with $ for current address
// Need to be careful: only replace * when it's used as current address,
// not when it's the multiply operator
// Safe heuristic: * at start or after operator is current address
size_t pos = 0;
std::string result;
bool at_start = true;
while (pos < expr_for_parser.length()) {
  char c = expr_for_parser[pos];
  if (c == '*') {
    // Check if this is current address or multiply
    if (at_start || (pos > 0 && strchr("+-*/()[]<>=!&|^~", expr_for_parser[pos-1]))) {
      result += '$';  // Current address
    } else {
      result += '*';  // Multiply operator
    }
  } else {
    result += c;
  }
  at_start = (c == ' ' || c == '\t');
  pos++;
}
expr_for_parser = result;
```

**Pros**: Minimal code change, leverages existing `$` support  
**Cons**: Heuristic may have edge cases, string manipulation overhead

### Option 2: Extend ExpressionParser (Larger Scope)

Add `*` as an alternative current location operator in `ExpressionParser::ParsePrimary()`:

```cpp
// In expression_parser.cpp, ParsePrimary():
if (Peek() == '$' || Peek() == '*') {
  size_t saved_pos = pos_;
  char op = Peek();
  Consume(); // consume $ or *
  
  if (op == '*' && (std::isdigit(Peek()) || std::isalpha(Peek()))) {
    // * followed by operand - multiply operator, not current address
    // Restore and let it be handled by operator parsing
    pos_ = saved_pos;
  } else if (op == '$' && std::isxdigit(Peek())) {
    // $ followed by hex digit - number, not current location
    pos_ = saved_pos;
  } else {
    // $ or * not followed by operand - current location
    return std::make_shared<CurrentLocationExpr>();
  }
}
```

**Pros**: Cleaner, more robust, handles both `$` and `*` consistently  
**Cons**: Requires changes to shared ExpressionParser (affects all syntaxes)

### Option 3: Lookahead Check (Hybrid)

In `EvaluateExpression()`, check if expression starts with `*` and has an operator after it:

```cpp
// Handle * with operators: *+4, *-2, etc.
if (trimmed.length() > 1 && trimmed[0] == '*' && 
    (trimmed[1] == '+' || trimmed[1] == '-' || trimmed[1] == '*' || 
     trimmed[1] == '/' || trimmed[1] == '&' || trimmed[1] == '|')) {
  // Replace * with current address value
  std::string expr_with_value = std::to_string(current_address_) + trimmed.substr(1);
  auto expr = ParseExpression(expr_with_value, symbols);
  return static_cast<uint32_t>(expr->Evaluate(symbols));
}
```

**Pros**: Simple, targeted fix, no string manipulation heuristics  
**Cons**: Only handles `*` at start of expression, not in middle (e.g., `2**+4`)

---

## Recommended Fix

**Option 3 (Lookahead Check)** is recommended for minimal scope:

1. Add check in `EvaluateExpression()` before calling `ParseExpression()`
2. If expression starts with `*` followed by an operator, substitute current address value
3. This handles the common case (`.EQ *+4`) without complex heuristics

### Implementation

```cpp
// In scmasm_syntax.cpp, EvaluateExpression():

// Handle * (current address) - special case before general parsing
if (trimmed == "*") {
  return current_address_;
}

// NEW: Handle *+offset, *-offset, etc.
if (trimmed.length() > 1 && trimmed[0] == '*') {
  char op = trimmed[1];
  if (op == '+' || op == '-' || op == '/' || op == '&' || 
      op == '|' || op == '^' || op == '<' || op == '>') {
    // Replace * with current address, then evaluate
    std::string expr_str = std::to_string(current_address_) + trimmed.substr(1);
    auto expr = ParseExpression(expr_str, symbols);
    return static_cast<uint32_t>(expr->Evaluate(symbols));
  }
  // If * followed by *, it's multiply: *-offset (ambiguous)
  // Fall through to general parser (will likely fail, but that's expected)
}

// Continue with existing code...
```

---

## Test Cases

To verify the fix:

```assembly
        .OR $8000
TEST1   .EQ *           ; Should be $8000
TEST2   .EQ *+4         ; Should be $8004
TEST3   .EQ *-2         ; Should be $7FFE
        NOP
TEST4   .EQ *           ; Should be $8001
        JMP *+3         ; Should work
```

---

## Related Files

- **Main Issue**: `src/syntax/scmasm_syntax.cpp` (line 821-880)
- **Expression Parser**: `src/common/expression_parser.cpp` (line 389-398)
- **Test File**: `~/Projects/Vintage/Apple/A2osX/A2osX.S.txt` (line 702)

---

## Conclusion

✅ **Root cause identified**: `*` operator only handled for exact match, not in expressions  
✅ **Execution flow traced**: From `.EQ` directive through EvaluateExpression to ExpressionParser  
✅ **Evidence documented**: Code locations, error messages, test case  
✅ **Impact assessed**: Affects all SCMASM expressions using `*` with operators  
✅ **Fix recommended**: Lookahead check with minimal scope (Option 3)

---

**Next Step**: Engineer should implement Option 3 fix with test coverage.
