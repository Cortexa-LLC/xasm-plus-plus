# Task Contract: Eliminate Merlin 3-Layer Indirection

**Task ID:** 2026-02-13_merlin-cleanup-layers
**Beads Task:** xasm++-f0o7
**Created:** 2026-02-13
**Owner:** Orchestrator → Engineer (delegated)
**Type:** Refactoring

---

## Problem Statement

Merlin currently has unnecessary 3-layer indirection that other parsers don't have:

**Current (Merlin - 3 layers):**
```
Registry → merlin::HandleOrg (wrapper)
         → MerlinSyntaxParser::HandleOrg (delegate)
         → HandleOrgDirective (actual work)
```

**Desired (SCMASM pattern - 2 layers):**
```
Registry → scmasm::HandleOr (actual work)
```

This creates:
- **Unnecessary complexity** - 3 function calls instead of 1
- **Performance overhead** - Extra stack frames
- **Maintenance burden** - Changes require updating 3 places
- **Inconsistency** - Other parsers use cleaner pattern

## Requirements

### Functional Requirements
- [ ] Move implementations from `merlin_directives.cpp` → `merlin_directive_handlers.cpp`
- [ ] Delete `merlin_directives.cpp` (715 lines → obsolete)
- [ ] Delete `merlin_directives.h` header
- [ ] Remove instance method delegates from `merlin_syntax.cpp`
- [ ] Update CMakeLists.txt (remove merlin_directives.cpp from build)
- [ ] Match SCMASM pattern exactly
- [ ] All 114 Merlin tests must continue passing

### Architecture Pattern

**Before:**
```cpp
// merlin_directive_handlers.cpp (wrapper)
void merlin::HandleOrg(label, operand, context) {
  auto *parser = static_cast<MerlinSyntaxParser*>(context.parser_state);
  parser->HandleOrg(operand, *context.section, *context.symbols);  // Layer 2
}

// merlin_syntax.cpp (delegate)
void MerlinSyntaxParser::HandleOrg(operand, section, symbols) {
  HandleOrgDirective(operand, section, symbols, current_address_);  // Layer 3
}

// merlin_directives.cpp (actual work)
void HandleOrgDirective(operand, section, symbols, current_address) {
  // Parse, validate, create atoms... (THE REAL WORK)
}
```

**After:**
```cpp
// merlin_directive_handlers.cpp (does actual work)
void merlin::HandleOrg(label, operand, context) {
  (void)label;

  if (operand.empty()) {
    throw std::runtime_error("ORG requires an address");
  }

  // Parse, validate, create atoms... (THE REAL WORK - moved from merlin_directives.cpp)
  ExpressionParser parser(context.symbols);
  auto expr = parser.Parse(operand);
  uint32_t address = expr->Evaluate();

  auto org_atom = std::make_shared<OrgAtom>(address);
  context.section->atoms.push_back(org_atom);
  *context.current_address = address;
}

// No wrapper, no delegate - just direct implementation
```

### Special Cases: Parser State Access

Some directives need parser state (macros, conditionals). For these:
- **Keep instance methods** for state-dependent operations
- **Call from free functions** via `context.parser_state`
- **Example:** Macros (HandleMa, HandleEm) - SCMASM does this too

## Success Criteria

- [ ] `merlin_directives.cpp` deleted (715 lines removed)
- [ ] `merlin_directives.h` deleted
- [ ] Instance method delegates removed from `merlin_syntax.cpp`
- [ ] CMakeLists.txt updated (remove merlin_directives.cpp)
- [ ] All 114 Merlin tests passing (100%)
- [ ] All 1538 total tests passing (100%)
- [ ] Code compiles without warnings
- [ ] Pattern matches SCMASM exactly

## Files to Modify

**Modified:**
- `src/syntax/merlin_directive_handlers.cpp` - Replace wrappers with full implementations
- `include/xasm++/directives/merlin_directive_handlers.h` - Update signatures if needed
- `src/syntax/merlin_syntax.cpp` - Remove instance method delegates
- `include/xasm++/syntax/merlin_syntax.h` - Remove method declarations
- `src/CMakeLists.txt` - Remove merlin_directives.cpp

**Deleted:**
- `src/syntax/merlin_directives.cpp` (715 lines)
- `include/xasm++/directives/merlin_directives.h`

## Constraints

- **No functional changes** - Pure refactoring
- **No test changes** - Tests should not need modification
- **Match SCMASM pattern** - Use as reference implementation
- **Single atomic commit** - For easy review/revert

## Estimated Effort

2-3 hours (move implementations, remove delegates, test)

---

**Status:** READY FOR IMPLEMENTATION
