# Quick Reference: * Operator Fix

## One-Line Summary
Fixed `.EQ *+4` parsing by adding lookahead check in scmasm_syntax.cpp after line 828.

## The Fix
```cpp
// In src/syntax/scmasm_syntax.cpp after line 828
if (trimmed.length() > 1 && trimmed[0] == '*') {
  char op = trimmed[1];
  if (op == '+' || op == '-' || op == '/' || op == '*' ||
      op == '&' || op == '|' || op == '^' || 
      op == '<' || op == '>') {
    std::string expr_str = std::to_string(current_address_) + 
                          trimmed.substr(1);
    auto expr = ParseExpression(expr_str, symbols);
    return static_cast<uint32_t>(expr->Evaluate(symbols));
  }
}
```

## What It Does
- **Input**: `.EQ *+4` at address 0x8000
- **Transform**: `*+4` → `"32768+4"`
- **Output**: 32772 (0x8004)

## Test It
```bash
# Build
cmake --build build

# Test
./build/bin/xasm++ --syntax scmasm test_star_operator.s -o test.bin

# Verify
./build/Testing/test_scmasm_syntax  # 137 tests pass
```

## Status
✅ Complete - All tests passing - Zero warnings - Ready for merge
