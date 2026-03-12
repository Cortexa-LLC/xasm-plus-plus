# Implementation Plan

**Task:** Implement forward reference resolution
**Workflow:** Feature (.ai-pack/workflows/feature.md)
**Role:** Engineer

---

## Phase 1: Investigation

### Files to Examine

1. **`src/core/assembler.cpp`** - Multi-pass logic
   - How many passes are run?
   - When is symbol table built?
   - How are passes triggered?

2. **`src/syntax/scmasm_directive_handlers.cpp`** - HandleDa()
   - Current implementation
   - When does EvaluateExpression() get called?

3. **`include/xasm++/core/symbol_table.h`** - Symbol storage
   - How are undefined symbols tracked?
   - Forward reference support?

4. **`include/xasm++/core/section.h`** - Code emission
   - Can bytes be emitted conditionally?
   - Placeholder byte support?

### Expected Findings

- Multi-pass assembler exists (confirmed from earlier grep)
- Symbol table built on first pass
- Forward references may already be partially supported
- .DA directive needs modification to defer evaluation

## Phase 2: Implementation

### Step 1: Understand Current Multi-Pass

Read assembler.cpp to understand:
- Pass loop structure
- Symbol table population timing
- Forward reference detection

### Step 2: Modify .DA Handler

**Current code (approximate):**
```cpp
void HandleDa(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
    // Evaluates IMMEDIATELY - throws if undefined
    int64_t value = context.parser_state->EvaluateExpression(operand, *context.symbols);
    context.section->EmitByte(value & 0xFF);
    context.section->EmitByte((value >> 8) & 0xFF);
}
```

**New approach:**
```cpp
void HandleDa(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
    try {
        // Try to evaluate
        int64_t value = context.parser_state->EvaluateExpression(operand, *context.symbols);
        context.section->EmitByte(value & 0xFF);
        context.section->EmitByte((value >> 8) & 0xFF);
    } catch (const std::exception &e) {
        // Check if it's an undefined symbol
        if (context.is_first_pass) {
            // First pass: emit placeholder bytes
            context.section->EmitByte(0x00);
            context.section->EmitByte(0x00);
            // Mark for later resolution
            context.forward_references.push_back({operand, current_address});
        } else {
            // Later pass: still undefined = error
            throw;
        }
    }
}
```

### Step 3: Track Pass Number

Ensure DirectiveContext knows which pass:
```cpp
struct DirectiveContext {
    // ... existing fields ...
    bool is_first_pass;
    int pass_number;
};
```

### Step 4: Add Tests

**Test 1: Simple forward reference**
```cpp
TEST_F(ScmasmSyntaxTest, ForwardReference) {
    std::string source = R"(
        .OR $0800
        lda label    ; Use before definition
    label:
        rts
    )";
    // Should assemble successfully
}
```

**Test 2: Forward reference in .DA**
```cpp
TEST_F(ScmasmSyntaxTest, ForwardReferenceInDA) {
    std::string source = R"(
        .OR $0800
        .DA forward_symbol
    forward_symbol:
        .DA $1234
    )";
    // Should assemble successfully
}
```

**Test 3: A2osX line 76**
```cpp
TEST_F(ScmasmSyntaxTest, A2osXForwardReferenceDA) {
    std::string source = R"(
        .OR $0800
        .DA MLIGETPREFIX01
        ; ... 553 lines of code ...
    MLIGETPREFIX01:
        rts
    )";
    // Should assemble successfully
}
```

## Phase 3: Testing

### Unit Tests
- Forward reference in instructions
- Forward reference in .DA directive
- Multiple forward references
- Circular references (should error)

### Integration Test
- A2osX line 76+ assembly
- Full A2osX.S.txt build

### Regression
- All existing tests pass (1,738 tests)

## Files to Modify

- `src/core/assembler.cpp` (may need pass tracking)
- `src/syntax/scmasm_directive_handlers.cpp` (HandleDa)
- `include/xasm++/directives/directive_context.h` (add pass tracking)
- `tests/unit/test_scmasm_syntax.cpp` (add tests)

## Risks

- **Medium risk:** Multi-pass logic may need refactoring
- **Low risk:** .DA directive change is localized
- **Watch for:** Circular reference infinite loops

## Alternative: ExpressionPatch Approach

Instead of emitting placeholders, store patches:

```cpp
struct ExpressionPatch {
    std::string expression;
    uint16_t address;
    size_t byte_count;
};

std::vector<ExpressionPatch> patches;
```

On later passes, apply patches after symbols defined.

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
