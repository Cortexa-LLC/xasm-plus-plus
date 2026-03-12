# Task Contract: Merlin Directive Registry Refactoring

**Task ID:** 2026-02-13_merlin-directive-refactor
**Beads Task:** xasm++-v161
**Created:** 2026-02-13
**Owner:** Orchestrator → Engineer (delegated)
**Type:** Refactoring

---

## Problem Statement

The Merlin syntax parser uses ~40 inline lambdas in the directive registry, which is inconsistent with the pattern used in other parsers (scmasm, edtasm). This creates:

- **Inconsistency** - Different patterns across codebase
- **Reduced readability** - Large InitializeDirectiveRegistry() method
- **Harder debugging** - Lambdas don't show meaningful names in stack traces
- **Harder testing** - Can't test individual handlers in isolation

## Requirements

### Functional Requirements
- [ ] Refactor all directive registry lambdas to named methods
- [ ] Follow the scmasm pattern: `directive_registry_[X] = &MerlinSyntaxParser::HandleX;`
- [ ] Maintain identical behavior (zero functional changes)
- [ ] All 114 Merlin tests must continue passing

### Pattern Examples

**Current (lambda in merlin_syntax.cpp):**
```cpp
directive_registry_[ORG] = [this](const std::string &label,
                                  const std::string &operand,
                                  DirectiveContext &ctx) {
  (void)label;
  HandleOrg(operand, *ctx.section, *ctx.symbols);
};
```

**Desired (free function in separate file):**
```cpp
// In merlin_directive_handlers.h:
namespace merlin {
  void HandleOrg(const std::string &label, const std::string &operand,
                 DirectiveContext &ctx);
}

// In merlin_directive_handlers.cpp:
namespace merlin {
  void HandleOrg(const std::string &label, const std::string &operand,
                 DirectiveContext &ctx) {
    (void)label;
    // Implementation here
  }
}

// In merlin_syntax.cpp InitializeDirectiveRegistry():
directive_registry_[ORG] = merlin::HandleOrg;
```

### Reference Implementation

See `src/syntax/scmasm_directive_handlers.cpp` for the exact pattern:
```cpp
// scmasm_directive_handlers.cpp
namespace scmasm {
  void HandleOr(...) { ... }
  void HandleEq(...) { ... }
}

// scmasm_syntax.cpp
directive_registry_[OR] = scmasm::HandleOr;
directive_registry_[EQ] = scmasm::HandleEq;
```

**Key Requirements:**
- Create separate file: `src/syntax/merlin_directive_handlers.cpp`
- Create header: `include/xasm++/syntax/merlin_directive_handlers.h`
- Use `namespace merlin { }` for all handlers
- Free functions, NOT member methods
- Follow scmasm/edtasm/core pattern exactly

## Success Criteria

- [ ] All ~40 directive handlers extracted to named methods
- [ ] Method declarations added to `include/xasm++/syntax/merlin_syntax.h`
- [ ] Method implementations in `src/syntax/merlin_syntax.cpp`
- [ ] All 114 Merlin syntax tests passing (100%)
- [ ] All 1538 total tests passing (100%)
- [ ] Code compiles without warnings
- [ ] Git history shows single atomic commit

## Constraints

- **No functional changes** - This is pure refactoring
- **No test changes** - Tests should not need modification
- **Maintain existing behavior** - Byte-for-byte identical assembly output
- **Single commit** - Atomic refactoring for easy review/revert

## Stakeholders

- **User:** Requested consistency across directive handlers
- **Orchestrator:** Managing task coordination
- **Engineer:** Implementing refactoring

## Estimated Effort

3-4 hours (mechanical refactoring, ~40 handlers)

---

**Status:** READY FOR IMPLEMENTATION
