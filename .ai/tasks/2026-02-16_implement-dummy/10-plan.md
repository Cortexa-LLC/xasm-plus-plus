# Implementation Plan

**Task:** Implement .DUMMY directive
**Workflow:** Feature
**Role:** Engineer

---

## Implementation Approach

### Step 1: Add Dummy Mode State

Add flag to ScmasmSyntaxParser:
```cpp
class ScmasmSyntaxParser {
private:
  bool in_dummy_section_ = false;
  
public:
  bool InDummySection() const { return in_dummy_section_; }
  void StartDummySection() { in_dummy_section_ = true; }
  void EndDummySection() { in_dummy_section_ = false; }
};
```

### Step 2: Implement .DUMMY Handler

```cpp
void HandleDummy(...) {
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
  parser->StartDummySection();
}
```

### Step 3: Implement .ED Handler

```cpp
void HandleEd(...) {
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
  parser->EndDummySection();
}
```

### Step 4: Modify Data Directives

Update .BS, .DA, .DW to check dummy mode:
```cpp
void HandleBs(...) {
  // ... calculate size ...
  
  if (auto* parser = dynamic_cast<ScmasmSyntaxParser*>(context.parser_state)) {
    if (parser->InDummySection()) {
      // Just advance address, don't emit
      context.section->AdvanceAddress(count);
      return;
    }
  }
  
  // Normal: emit bytes
  for (size_t i = 0; i < count; ++i) {
    context.section->EmitByte(0);
  }
}
```

### Step 5: Add Tests

- Test .DUMMY/.ED pairing
- Test symbol definition in dummy mode
- Test that no bytes emitted in dummy mode

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
