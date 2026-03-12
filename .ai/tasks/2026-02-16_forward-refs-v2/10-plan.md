# Implementation Plan

**Task:** Forward reference resolution for .DA
**Workflow:** Feature
**Role:** Engineer

---

## Key Insight from xasm++-kbgs

DataAtom has two constructors:
1. `DataAtom(std::vector<uint8_t> data)` - Immediate (current use)
2. `DataAtom(std::vector<std::string> expressions, DataSize size)` - Deferred (needed)

## Implementation

### Step 1: Modify HandleDa

Convert from immediate evaluation to expression storage:

```cpp
void HandleDa(...) {
  // Parse comma-separated values
  std::vector<std::string> expressions;
  // Split operand by commas
  
  for (auto& expr : raw_expressions) {
    // Convert SCMASM operators to generic
    std::string converted = ConvertOperators(expr);  // # -> <, / -> >
    expressions.push_back(converted);
  }
  
  // Create DataAtom with expressions (deferred evaluation)
  auto atom = std::make_shared<DataAtom>(expressions, DataSize::Word);
  context.section->AddAtom(atom);
}
```

### Step 2: Operator Conversion

```cpp
std::string ConvertOperators(const std::string& expr) {
  std::string result = expr;
  // # (low byte) -> < (generic low byte)
  // / (high byte) -> > (generic high byte)
  return result;
}
```

### Step 3: Add Tests

- Forward reference in .DA
- Byte extraction with forward references (#LABEL, /LABEL)
- Multiple forward references
- A2osX pattern test

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
