# ADR-0001: Directive Handler Pattern - Free Functions over Lambdas

**Status:** Accepted
**Date:** 2026-02-14
**Deciders:** Development Team
**Context Source:** Architectural Review (xasm++-99f4)

---

## Context

During comprehensive architectural review, we identified inconsistent directive handler registration patterns across syntax parsers:

**Problem Examples:**
1. Lambda wrappers (`core_directive_handlers.cpp`) - indirect, harder to debug
2. Member methods with lambdas - 3-layer indirection
3. Inconsistent parameter orders - maintenance burden

**Impact:**
- Architectural inconsistency across 6 syntax parsers
- Harder to maintain and debug
- Parameter order confusion
- Violation of clean code principles

---

## Decision

**We adopt the free function pattern for ALL directive handlers:**

### Pattern: Free Functions in Namespace

```cpp
// Declaration (header)
namespace xasm {
namespace <syntax> {
void HandleOrg(const std::string &label,
               const std::string &operand,
               DirectiveContext &context);
void HandleEqu(const std::string &label,
               const std::string &operand,
               DirectiveContext &context);
}}

// Implementation
namespace xasm {
namespace <syntax> {
void HandleOrg(const std::string &label,
               const std::string &operand,
               DirectiveContext &context) {
  // Direct access to context members
  uint32_t address = ParseHex(operand);
  context.section->atoms.push_back(std::make_shared<OrgAtom>(address));
  *context.current_address = address;
}
}}

// Registration (NO LAMBDAS)
void <Syntax>Parser::InitializeDirectiveRegistry() {
  directive_registry_[ORG] = <syntax>::HandleOrg;
  directive_registry_[EQU] = <syntax>::HandleEqu;
  // ... direct assignment only
}
```

### Required Elements

1. **Free functions** - Not member methods, not lambdas
2. **Namespace isolation** - Each syntax parser has its own namespace (`merlin`, `edtasm`, `scmasm`, etc.)
3. **Standard signature** - `(label, operand, context)` order, always
4. **DirectiveContext** - Unified state access (section, symbols, current_address, parser_state)
5. **Direct assignment** - `registry[DIRECTIVE] = namespace::Handler` (no lambda wrappers)

---

## Consequences

### Positive

✅ **Consistency** - Same pattern across all syntax parsers
✅ **Debuggability** - Named functions appear in stack traces
✅ **Testability** - Functions can be unit tested independently
✅ **Maintainability** - Clear parameter order, single responsibility
✅ **Performance** - No lambda capture overhead
✅ **Clarity** - No 3-layer indirection

### Negative

⚠️ **Friend declarations** - Parser internals need friend declarations for handler access
⚠️ **Migration effort** - Existing lambda-based code requires refactoring

### Neutral

🔄 **Namespace pollution** - Mitigated by nested namespaces (`xasm::merlin::`, etc.)

---

## Implementation

### Phase 1: P0 Critical (Complete)
- ✅ `merlin_syntax.cpp` - Reference implementation
- ✅ `edtasm_m80_plusplus_syntax.cpp` - 31 lambdas → free functions (commit 73a690b)
- ✅ `edtasm_syntax.cpp` - Aligned to Merlin pattern (commit d047e8f)
- ✅ `simple_syntax.cpp` - 3 lambdas → free functions (commit f6eb2dd)
- ⏳ `core_directive_handlers.cpp` - In progress (xasm++-cws7)

### Phase 2: P1 Verification (After P0)
- Parameter order standardization
- DirectiveContext usage verification
- Documentation updates

---

## Compliance

### For New Code
**MANDATORY:**
- All new directive handlers MUST use free function pattern
- No lambda wrappers permitted
- Standard signature required: `(label, operand, context)`
- Namespace isolation required

### For Existing Code
**MIGRATION REQUIRED:**
- All non-compliant handlers flagged for refactor
- Lambda wrappers removed during next modification
- See `.ai/tasks/2026-02-14_architectural-review/` for full list

---

## Verification

### Automated Checks
```bash
# Search for lambda wrappers (should return zero matches)
grep -r "Register.*\[.*\].*(" src/syntax/*.cpp

# Verify direct assignment pattern
grep -r "directive_registry_\[.*\] = .*::" src/syntax/*.cpp
```

### Code Review Checklist
- [ ] Handler is free function in namespace
- [ ] Signature is `(label, operand, context)`
- [ ] Registration uses direct assignment
- [ ] No lambda wrappers
- [ ] DirectiveContext used for state access

---

## References

**Reference Implementation:** `src/syntax/merlin_directive_handlers.cpp` lines 458-500
**Architectural Review:** `.ai/tasks/2026-02-14_architectural-review/`
**Related ADRs:** ADR-0002 (Parameter Order), ADR-0003 (Magic Values)
**Commits:** 73a690b, d047e8f, f6eb2dd (migration examples)

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-02-14 | AI Architect | Initial decision based on architectural review |
