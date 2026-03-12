# Task Contract: Refactor edtasm_syntax to Use Directive Registry

**Task ID:** 2026-02-13_edtasm-directive-registry
**Beads Task:** xasm++-lv49
**Created:** 2026-02-13
**Owner:** Orchestrator → Engineer (delegated)
**Type:** Refactoring (Architectural)

---

## Problem Statement

The file `edtasm_syntax.cpp` uses an if/else chain for directive handling instead of the directive registry pattern used by scmasm and merlin. This creates architectural inconsistency and performance issues.

**Current Pattern (WRONG - if/else chain):**
```cpp
DirectiveParseResult EdtasmSyntax::parse_directive(...) {
    if (keyword == "ORG" || keyword == "org") {
        return handle_org_directive(label, operand);
    } else if (keyword == "EQU" || keyword == "equ") {
        return handle_equ_directive(label, operand);
    } else if (keyword == "DB" || keyword == "db") {
        return handle_db_directive(label, operand);
    }
    // ... 10+ more if/else statements
}
```

**Target Pattern (CORRECT - registry lookup):**
```cpp
EdtasmSyntax::EdtasmSyntax() {
    register_directives();
}

void EdtasmSyntax::register_directives() {
    directive_registry_.register_directive("ORG",
        [this](auto& l, auto& o) { return handle_org_directive(l, o); });
    directive_registry_.register_directive("EQU",
        [this](auto& l, auto& o) { return handle_equ_directive(l, o); });
    // ... all other directives
}

DirectiveParseResult EdtasmSyntax::parse_directive(...) {
    return directive_registry_.parse_directive(keyword, label, operand);
}
```

This creates:
- **Architectural inconsistency** - Different from scmasm and merlin
- **Performance issue** - O(n) linear search vs O(1) hash lookup
- **Maintenance burden** - Adding directives requires modifying if/else chain
- **Testability issues** - Harder to test individual directives

## Requirements

### Functional Requirements
- [ ] Add `directive_registry_` member to `EdtasmSyntax` class
- [ ] Add `register_directives()` private method
- [ ] Register all directives in constructor
- [ ] Replace if/else chain with single registry lookup
- [ ] Match scmasm_syntax.cpp pattern exactly
- [ ] Maintain identical behavior (pure refactoring)
- [ ] All tests must continue passing

### Architecture Pattern

**Reference:** `src/syntax/scmasm_syntax.cpp` and `src/syntax/merlin_syntax.cpp`

**Header Changes (edtasm_syntax.h):**
```cpp
class EdtasmSyntax : public SyntaxParser {
public:
    EdtasmSyntax();  // Add directive registration
    // ... existing public methods

private:
    DirectiveRegistry directive_registry_;  // ADD THIS
    void register_directives();             // ADD THIS

    // Keep existing handler methods...
};
```

**Implementation Changes (edtasm_syntax.cpp):**
```cpp
EdtasmSyntax::EdtasmSyntax() {
    register_directives();
}

void EdtasmSyntax::register_directives() {
    // Register all directives
    directive_registry_.register_directive("ORG",
        [this](const std::string& l, const std::string& o) {
            return handle_org_directive(l, o);
        });

    directive_registry_.register_directive("EQU",
        [this](const std::string& l, const std::string& o) {
            return handle_equ_directive(l, o);
        });

    // ... register all other directives (10+ total)
}

DirectiveParseResult EdtasmSyntax::parse_directive(
    const std::string& keyword,
    const std::string& label,
    const std::string& operand) {

    // REPLACE entire if/else chain with:
    return directive_registry_.parse_directive(keyword, label, operand);
}
```

## Directives to Register

Based on investigation, these directives need registration:

1. ORG - Set origin
2. EQU - Define constant
3. DB - Define byte(s)
4. DW - Define word(s)
5. DS - Define space
6. ASC - ASCII string
7. DCI - DCI string
8. INV - Inverted string
9. FLS - Flash string
10. HEX - Hex data
11. (+ any others found in if/else chain)

## Success Criteria

- [ ] `directive_registry_` member added to EdtasmSyntax class
- [ ] `register_directives()` method implemented
- [ ] All directives registered in constructor
- [ ] if/else chain replaced with single lookup
- [ ] All edtasm tests passing (100%)
- [ ] All total tests passing (100%)
- [ ] Code compiles without warnings
- [ ] Pattern matches scmasm_syntax.cpp exactly
- [ ] Single atomic commit

## Comparison: Before vs After

### Before (Current - O(n) lookup):
- **Lines of code:** ~50 in parse_directive
- **Performance:** O(n) where n = number of directives
- **Maintainability:** Hard - must modify if/else chain
- **Consistency:** Different from scmasm/merlin

### After (Target - O(1) lookup):
- **Lines of code:** ~3 in parse_directive, ~15 in register_directives
- **Performance:** O(1) hash table lookup
- **Maintainability:** Easy - add one line to register_directives
- **Consistency:** Matches scmasm/merlin exactly

## Constraints

- **No functional changes** - Pure refactoring
- **No test changes** - Tests should not need modification
- **Match established pattern** - Use scmasm as reference
- **Single commit** - Atomic refactoring for easy review/revert

## Dependencies

**None** - Standalone refactoring

## Estimated Effort

4-6 hours (architectural refactoring, testing)

---

**Status:** READY FOR IMPLEMENTATION
