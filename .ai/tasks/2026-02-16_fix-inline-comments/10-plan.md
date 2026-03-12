# Implementation Plan

**Task:** Fix inline comment parsing on .EQ lines
**Workflow:** Bugfix (.ai-pack/workflows/bugfix.md)
**Role:** Engineer

---

## Investigation Phase

### Files to Examine

1. **`src/syntax/scmasm_syntax.cpp`** - ParseLine()
   - How are comments stripped?
   - Where does "Unexpected character" error occur?

2. **`src/syntax/scmasm_directive_handlers.cpp`** - HandleEq()
   - How is operand parsed?
   - Does it expect single value or allow trailing text?

3. **`~/Projects/Vintage/Apple/A2osX/INC/macros.i`** - Line 252
   - Check if there are other similar patterns
   - Understand if this is common syntax

### Research Questions

- Does Merlin allow trailing text on .EQ lines?
- Is this common in other A2osX files?
- Are there semicolon comments on other .EQ lines?

### Expected Findings

- Parser currently expects end-of-line or comment after .EQ value
- Trailing text without semicolon causes "Unexpected character" error
- May need to strip trailing text after .EQ evaluation

## Implementation Phase

### Step 1: Understand Error Location

Find where "Unexpected character after expression" error is thrown:
```bash
grep -r "Unexpected character" src/
```

### Step 2: Check Current .EQ Handling

Read HandleEq() to understand:
- How operand is parsed
- What happens with trailing text
- If comments are already stripped

### Step 3: Implement Fix

**Approach:** Strip trailing text after .EQ value evaluation

**Modify HandleEq():**
```cpp
void HandleEq(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
    if (label.empty()) {
        throw std::runtime_error(".EQ requires a label");
    }
    
    // Extract just the value part (before any trailing text)
    std::string value_expr = operand;
    
    // Find first sequence of multiple spaces/tabs (likely comment separator)
    size_t comment_pos = value_expr.find("  "); // Two spaces
    if (comment_pos != std::string::npos) {
        value_expr = value_expr.substr(0, comment_pos);
    }
    
    // Evaluate expression
    int64_t value = context.parser_state->EvaluateExpression(
        value_expr, *context.symbols);
    
    // Define symbol
    context.symbols->Define(label, value);
}
```

**Alternative:** Strip trailing text in ParseLine() before calling directive handler.

### Step 4: Add Tests

**Test 1: .EQ with trailing comment**
```cpp
TEST_F(ScmasmSyntaxTest, EqWithTrailingComment) {
    std::string source = R"(
        VALUE .EQ 180    float
    )";
    // Should define VALUE = 180, ignore "float"
}
```

**Test 2: .EQ with semicolon comment**
```cpp
TEST_F(ScmasmSyntaxTest, EqWithSemicolonComment) {
    std::string source = R"(
        VALUE .EQ 180 ; this is a comment
    )";
    // Should still work
}
```

**Test 3: .EQ with no comment**
```cpp
TEST_F(ScmasmSyntaxTest, EqNoComment) {
    std::string source = R"(
        VALUE .EQ 180
    )";
    // Should work normally
}
```

## Testing Strategy

1. **Unit tests:** .EQ with various comment formats
2. **Integration test:** A2osX line 252
3. **Regression:** All existing tests pass

## Files to Modify

- `src/syntax/scmasm_directive_handlers.cpp` (HandleEq) OR
- `src/syntax/scmasm_syntax.cpp` (ParseLine comment stripping)
- `tests/unit/test_scmasm_syntax.cpp` (add tests)

## Decision: Where to Fix?

**Option 1: Fix in HandleEq()**
- Pros: Localized fix, only affects .EQ
- Cons: May need similar fixes for other directives

**Option 2: Fix in ParseLine()**
- Pros: Applies to all directives, more general
- Cons: May affect other parsing logic

**Recommendation:** Start with HandleEq() (Option 1) since it's specific to this directive. If we find similar issues with other directives, refactor to Option 2.

## Risks

- **Low risk:** Simple text stripping
- **Watch for:** Breaking existing .EQ uses that have expressions with spaces

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
