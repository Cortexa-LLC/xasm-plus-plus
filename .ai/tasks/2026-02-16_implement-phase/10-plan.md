# Implementation Plan

**Task:** Implement .PH/.EP directives
**Workflow:** Feature
**Role:** Engineer

---

## Implementation

### Step 1: Add State to Parser

```cpp
class ScmasmSyntaxParser {
  bool in_phase_ = false;
  uint32_t phase_address_ = 0;
  uint32_t saved_address_ = 0;
  
public:
  void StartPhase(uint32_t addr);
  void EndPhase();
  bool InPhase() const;
};
```

### Step 2: Implement Handlers

```cpp
void HandlePh(const std::string&, const std::string& operand, ...) {
  uint32_t addr = EvaluateExpression(operand, ...);
  parser->StartPhase(addr);
}

void HandleEp(...) {
  parser->EndPhase();
}
```

### Step 3: Override GetCurrentAddress

When in phase mode, return phase_address instead of actual address.

### Step 4: Register Directives

Add PH and EP to directive registry.

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
