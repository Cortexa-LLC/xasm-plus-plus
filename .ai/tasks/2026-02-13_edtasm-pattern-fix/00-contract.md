# Task Contract: Fix edtasm_syntax to Match Merlin Directive Registry Pattern

**Task ID:** 2026-02-13_edtasm-pattern-fix
**Beads Task:** xasm++-lwa6
**Created:** 2026-02-14
**Owner:** Orchestrator → Engineer (delegated)
**Type:** Refactoring (Architectural Consistency)

---

## Problem Statement

The file `edtasm_syntax.cpp` was recently refactored to use a directive registry (commit 99c9a5f), but it **doesn't follow the established Merlin pattern**. This creates architectural inconsistency across syntax parsers.

**Current Implementation (WRONG):**
```cpp
// edtasm_syntax.h
using DirectiveHandler = std::function<void(
    const std::string &operands,         // ❌ operands FIRST (wrong order!)
    const std::string &label,            // ❌ label SECOND (wrong order!)
    Section &section,                    // ❌ Direct access (not DirectiveContext!)
    ConcreteSymbolTable &symbols)>;      // ❌ No DirectiveContext pattern

// Uses member methods with lambdas
void RegisterDirectives() {
  directive_registry_["ORG"] = [this](...) { HandleOrg(...); };
}

// Member methods
void HandleOrg(const std::string &operands, const std::string &label, ...);
```

**Target Pattern (CORRECT - Merlin):**
```cpp
// merlin_syntax.h
namespace merlin {
  void HandleOrg(const std::string &label,      // ✅ label FIRST
                 const std::string &operand,     // ✅ operand SECOND
                 DirectiveContext &context);     // ✅ DirectiveContext for state
}

using DirectiveHandler = std::function<void(
    const std::string &label,
    const std::string &operand,
    DirectiveContext &context)>;

// Direct assignment (no lambdas!)
void InitializeDirectiveRegistry() {
  directive_registry_[ORG] = merlin::HandleOrg;
}
```

This creates:
- **Architectural inconsistency** - Different from Merlin and SCMASM patterns
- **Wrong parameter order** - label should come before operand
- **No DirectiveContext** - Direct Section/SymbolTable access breaks pattern
- **Unnecessary complexity** - Lambda wrappers instead of direct assignment

---

## Requirements

### Functional Requirements
- [ ] Create `src/syntax/edtasm_directive_handlers.cpp`
- [ ] Create `include/xasm++/directives/edtasm_directive_handlers.h`
- [ ] Define free functions in `edtasm` namespace
- [ ] Use correct signature: `(label, operand, DirectiveContext&)`
- [ ] Update `EdtasmSyntaxParser::InitializeDirectiveRegistry()`
- [ ] Remove member method directive handlers
- [ ] Update header with forward declarations
- [ ] Add friend declarations for free functions
- [ ] Maintain identical behavior (pure refactoring)
- [ ] All tests must continue passing

### Architecture Pattern

**Reference:** `src/syntax/merlin_syntax.cpp` and `include/xasm++/syntax/merlin_syntax.h`

**Header Changes (edtasm_syntax.h):**
```cpp
// Forward declarations for edtasm namespace
namespace edtasm {
void HandleOrg(const std::string &, const std::string &, DirectiveContext &);
void HandleEnd(const std::string &, const std::string &, DirectiveContext &);
void HandleEqu(const std::string &, const std::string &, DirectiveContext &);
void HandleSet(const std::string &, const std::string &, DirectiveContext &);
void HandleFcb(const std::string &, const std::string &, DirectiveContext &);
void HandleFdb(const std::string &, const std::string &, DirectiveContext &);
void HandleFcc(const std::string &, const std::string &, DirectiveContext &);
void HandleRmb(const std::string &, const std::string &, DirectiveContext &);
void HandleSetdp(const std::string &, const std::string &, DirectiveContext &);
}

class EdtasmSyntaxParser {
public:
  // Friend declarations for handlers
  friend void edtasm::HandleOrg(const std::string &, const std::string &, DirectiveContext &);
  friend void edtasm::HandleEnd(const std::string &, const std::string &, DirectiveContext &);
  // ... etc for all 9 handlers

private:
  // Correct signature matching Merlin
  using DirectiveHandler = std::function<void(
      const std::string &label,      // ✅ label FIRST
      const std::string &operand,    // ✅ operand SECOND
      DirectiveContext &context)>;   // ✅ DirectiveContext

  std::unordered_map<std::string, DirectiveHandler> directive_registry_;
  void InitializeDirectiveRegistry();  // ✅ Renamed from RegisterDirectives

  // Remove all Handle* member methods - they become free functions
};
```

**Implementation Changes (edtasm_directive_handlers.cpp):**
```cpp
namespace xasm {
namespace edtasm {

void HandleOrg(const std::string &label,
               const std::string &operand,
               DirectiveContext &context) {
  // Implementation extracted from EdtasmSyntaxParser::HandleOrg
  // Access parser state via context.parser_state if needed
  uint32_t address = ParseNumber(operand);
  context.section->atoms.push_back(std::make_shared<OrgAtom>(address));
  *context.current_address = address;
}

// ... etc for all 9 handlers

} // namespace edtasm
} // namespace xasm
```

**Registry Initialization (edtasm_syntax.cpp):**
```cpp
void EdtasmSyntaxParser::InitializeDirectiveRegistry() {
  // Direct assignment - no lambdas!
  directive_registry_[ORG] = edtasm::HandleOrg;
  directive_registry_[END] = edtasm::HandleEnd;
  directive_registry_[EQU] = edtasm::HandleEqu;
  directive_registry_[SET] = edtasm::HandleSet;
  directive_registry_[FCB] = edtasm::HandleFcb;
  directive_registry_[FDB] = edtasm::HandleFdb;
  directive_registry_[FCC] = edtasm::HandleFcc;
  directive_registry_[RMB] = edtasm::HandleRmb;
  directive_registry_[SETDP] = edtasm::HandleSetdp;
}
```

---

## Directives to Migrate

9 directives need to be extracted to free functions:

1. **ORG** - Set origin address
2. **END** - End assembly
3. **EQU** - Define constant (immutable)
4. **SET** - Define variable (redefinable)
5. **FCB** - Form Constant Byte
6. **FDB** - Form Double Byte (16-bit, big-endian)
7. **FCC** - Form Constant Characters (flexible delimiter)
8. **RMB** - Reserve Memory Bytes
9. **SETDP** - Set Direct Page

---

## Success Criteria

- [ ] `edtasm_directive_handlers.cpp` created with 9 free functions
- [ ] `edtasm_directive_handlers.h` created with forward declarations
- [ ] `DirectiveHandler` signature matches Merlin exactly
- [ ] Parameter order corrected: (label, operand, context)
- [ ] Uses DirectiveContext instead of direct Section/SymbolTable access
- [ ] `InitializeDirectiveRegistry()` uses direct assignment (no lambdas)
- [ ] All 9 member methods removed from EdtasmSyntaxParser
- [ ] Friend declarations added to header
- [ ] All 23 EdtasmSyntaxTest tests passing (100%)
- [ ] Code compiles without warnings
- [ ] Pattern matches Merlin exactly
- [ ] Single atomic commit

---

## Comparison: Current vs Target

### Current Implementation (After 99c9a5f)
```cpp
// Wrong signature
using DirectiveHandler = std::function<void(
    const std::string &operands, const std::string &label,
    Section &section, ConcreteSymbolTable &symbols)>;

// Member methods
void EdtasmSyntaxParser::HandleOrg(
    const std::string &operands, const std::string &label,
    Section &section, ConcreteSymbolTable &symbols) { ... }

// Lambda wrappers
void RegisterDirectives() {
  directive_registry_["ORG"] = [this](auto& o, auto& l, auto& s, auto& syms) {
    HandleOrg(o, l, s, syms);
  };
}
```

### Target Pattern (Merlin)
```cpp
// Correct signature
using DirectiveHandler = std::function<void(
    const std::string &label, const std::string &operand,
    DirectiveContext &context)>;

// Free functions in namespace
namespace edtasm {
void HandleOrg(const std::string &label, const std::string &operand,
               DirectiveContext &context) { ... }
}

// Direct assignment
void InitializeDirectiveRegistry() {
  directive_registry_[ORG] = edtasm::HandleOrg;
}
```

---

## Constraints

- **No functional changes** - Pure architectural refactoring
- **No test changes** - Tests should not need modification
- **Match Merlin pattern exactly** - Use Merlin as reference
- **Single commit** - Atomic refactoring for easy review/revert

---

## Dependencies

**Depends on:** Commit 99c9a5f (directive registry infrastructure exists)
**Blocks:** None (architectural consistency improvement)

---

## Estimated Effort

2-3 hours (extract functions, update signatures, test)

---

**Status:** READY FOR IMPLEMENTATION

