# ADR-0002: Directive Handler Parameter Order Standardization

**Status:** Accepted
**Date:** 2026-02-14
**Deciders:** Development Team
**Context Source:** Architectural Review (xasm++-99f4)
**Depends On:** ADR-0001 (Directive Handler Pattern)

---

## Context

The architectural review revealed inconsistent parameter ordering across directive handlers:

**Inconsistencies Found:**
1. Some handlers: `(operand, label, section, symbols)` - operand first
2. Some handlers: `(label, operand, context)` - label first
3. Legacy handlers: Individual parameters instead of context object
4. Lambda wrappers reordering parameters to match inconsistent signatures

**Problems:**
- Confusion when implementing new handlers
- Copy-paste errors when adapting from other parsers
- Lambda wrappers needed to adapt signatures
- Maintenance burden

---

## Decision

**Standard signature for ALL directive handlers:**

```cpp
void HandlerName(const std::string &label,
                const std::string &operand,
                DirectiveContext &context);
```

### Parameter Order Rationale

1. **`label` first** - Assembly syntax: `LABEL DIRECTIVE OPERAND`
   - Mirrors source code order
   - Label is first thing assembler sees

2. **`operand` second** - Natural reading order
   - Follows label in source
   - Primary data for most directives

3. **`context` last** - Context object pattern
   - Standard C++ idiom (context/options last)
   - Bundles all state: section, symbols, current_address, parser_state

---

## DirectiveContext Definition

```cpp
struct DirectiveContext {
  Section *section;                    // Current assembly section
  SymbolTable *symbols;                // Symbol table
  uint32_t *current_address;           // Current PC/address
  void *parser_state;                  // Parser-specific state (opaque)
};
```

**Benefits:**
- Single point of modification (add members without signature changes)
- Clear ownership (all state in one place)
- Easier mocking for tests
- Type-safe parser_state via static_cast

---

## Migration Strategy

### Phase 1: Establish Standard (P0)
1. ✅ Define DirectiveContext struct
2. ✅ Convert all handlers to standard signature
3. ✅ Remove parameter-reordering lambdas
4. ⏳ Update core_directive_handlers.cpp (xasm++-cws7)

### Phase 2: Verification (P1)
1. Audit all syntax parsers for compliance
2. Update call sites
3. Remove legacy signatures
4. Document in coding standards

---

## Consequences

### Positive

✅ **Consistency** - Same signature everywhere
✅ **Clarity** - Matches assembly syntax order
✅ **Maintainability** - Less confusion, fewer copy-paste errors
✅ **Extensibility** - Easy to add context members
✅ **Type safety** - Single context object

### Negative

⚠️ **Migration effort** - Existing handlers need parameter reordering
⚠️ **Breaking change** - Incompatible with old signatures

### Neutral

🔄 **Parser state casting** - Requires static_cast for parser-specific state (acceptable tradeoff)

---

## Implementation Examples

### Before (WRONG - Inconsistent)

```cpp
// Handler 1: Operand first
void HandleInc(const std::string &operands,
               const std::string &label,
               Section &section,
               SymbolTable &symbols);

// Handler 2: Label first, different parameters
void HandleOrg(const std::string &label,
               const std::string &operand,
               Section &section);

// Registration: Lambda reorders parameters
registry.Register(INC, [&](const std::string &label,
                            const std::string &operand,
                            DirectiveContext &ctx) {
  HandleInc(operand, label, *ctx.section, *ctx.symbols);  // Reorder!
});
```

### After (CORRECT - Standard)

```cpp
// All handlers: Standard signature
void HandleInc(const std::string &label,
               const std::string &operand,
               DirectiveContext &context);

void HandleOrg(const std::string &label,
               const std::string &operand,
               DirectiveContext &context);

// Registration: Direct assignment
registry.Register(INC, HandleInc);  // No lambda needed
registry.Register(ORG, HandleOrg);
```

---

## Compliance

### For New Code
**MANDATORY:**
- Parameter order: `(label, operand, context)` - NO EXCEPTIONS
- Use DirectiveContext, not individual parameters
- No parameter reordering in registration

### For Existing Code
**MIGRATION REQUIRED:**
- All non-compliant handlers flagged
- Signature changes tracked in task xasm++-cws7 (P0)
- Lambda wrappers removed

---

## Verification

### Automated Checks

```bash
# Check all handlers have standard signature
grep -A3 "void Handle" include/xasm++/directives/*.h | \
  grep "DirectiveContext" | wc -l

# Should match total handler count
grep "void Handle" include/xasm++/directives/*.h | wc -l
```

### Code Review Checklist
- [ ] Signature is `(label, operand, context)` exactly
- [ ] Parameters in correct order (not reversed)
- [ ] Uses DirectiveContext, not individual args
- [ ] No lambda parameter reordering

---

## Examples from Codebase

### Reference Implementation: Merlin

```cpp
namespace xasm {
namespace merlin {
void HandleOrg(const std::string &label,
               const std::string &operand,
               DirectiveContext &context) {
  if (!label.empty()) {
    throw std::runtime_error("ORG directive cannot have a label");
  }
  uint32_t address = ParseHex(operand);
  context.section->atoms.push_back(std::make_shared<OrgAtom>(address));
  *context.current_address = address;
}
}}
```

### With Parser-Specific State

```cpp
namespace xasm {
namespace edtasm {
void HandleRept(const std::string &label,
                const std::string &operand,
                DirectiveContext &context) {
  auto parser = static_cast<EdtasmParser *>(context.parser_state);
  parser->rept_count_ = std::stoi(operand);
  parser->in_repeat_block_ = RepeatType::REPT;
  // ... etc
}
}}
```

---

## References

**Related ADRs:**
- ADR-0001: Directive Handler Pattern (prerequisite)
- ADR-0003: Magic Value Elimination (complementary)

**Implementation:**
- `.ai/tasks/2026-02-14_core-directive-lambdas/` - P0 migration task

**Reference Code:**
- `src/syntax/merlin_directive_handlers.cpp` - Standard signature example
- `include/xasm++/syntax/directive_context.h` - DirectiveContext definition

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-02-14 | AI Architect | Initial decision based on architectural review |
