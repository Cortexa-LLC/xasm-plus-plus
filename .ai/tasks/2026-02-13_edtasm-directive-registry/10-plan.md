# Implementation Plan: Refactor edtasm_syntax to Use Directive Registry

**Task ID:** 2026-02-13_edtasm-directive-registry
**Workflow:** `.ai-pack/workflows/refactor.md`
**Role:** Engineer (delegated from Orchestrator)

---

## Workflow Selection

**Using:** Refactor workflow (behavior-preserving architectural improvement)

**Why:** Pure refactoring to improve architecture consistency and performance.

---

## Approach

### Phase 1: Analysis & Baseline
1. Read reference implementations:
   - `src/syntax/scmasm_syntax.cpp` (registry pattern)
   - `src/syntax/merlin_syntax.cpp` (registry pattern)
   - `include/xasm++/syntax/scmasm_syntax.h` (header structure)
2. Read current implementation:
   - `src/syntax/edtasm_syntax.cpp` (if/else chain)
   - `include/xasm++/syntax/edtasm_syntax.h`
3. Count directives in if/else chain (for complete registration list)
4. Verify all tests passing baseline

### Phase 2: Header Modifications
1. Add `directive_registry_` member variable to `EdtasmSyntax` class
2. Add `register_directives()` private method declaration
3. Ensure constructor is declared (may need to add explicit constructor)

### Phase 3: Implementation - Add Registry Infrastructure
1. **Add constructor if needed:**
   ```cpp
   EdtasmSyntax::EdtasmSyntax() {
       register_directives();
   }
   ```

2. **Implement register_directives() method:**
   - Create method skeleton
   - Register each directive with lambda wrapper
   - Match scmasm pattern exactly

### Phase 4: Replace if/else Chain
1. Save original parse_directive method for reference
2. Replace entire method body with single line:
   ```cpp
   return directive_registry_.parse_directive(keyword, label, operand);
   ```
3. Remove or comment out old if/else chain

### Phase 5: Verification
1. Rebuild: `cmake --build build`
2. Run edtasm tests: `ctest --test-dir build -R edtasm`
3. Run full test suite: `ctest --test-dir build`
4. Verify 100% pass rate
5. Check no compiler warnings
6. Performance test (optional): Verify O(1) lookup

### Phase 6: Commit
Single atomic commit with descriptive message

---

## Execution Strategy

**Sequential** - Each phase builds on previous:
1. Baseline verification (Phase 1)
2. Header changes (Phase 2) - compile to verify syntax
3. Add infrastructure (Phase 3) - compile and test
4. Replace if/else (Phase 4) - compile and test
5. Final verification (Phase 5)
6. Commit (Phase 6)

**Incremental Testing:**
- Compile after header changes
- Compile after adding constructor
- Compile after register_directives skeleton
- Run tests after complete registry implementation
- Run full test suite before commit

---

## File Impact Analysis

**Files Modified:**
- `include/xasm++/syntax/edtasm_syntax.h` - Add directive_registry_ member, register_directives() method (~3 lines)
- `src/syntax/edtasm_syntax.cpp` - Add constructor, register_directives() method, replace parse_directive (~20 lines added, ~50 lines removed)

**Files NOT Modified:**
- Tests (no behavioral changes)
- Other syntax parsers (isolated change)
- edtasm_directive_handlers.cpp (already uses registry correctly)

**Net Impact:** ~30 lines removed (cleaner, more maintainable code)

---

## Detailed Implementation Steps

### Step 1: Header Changes (edtasm_syntax.h)

**Find the EdtasmSyntax class definition:**
```cpp
class EdtasmSyntax : public SyntaxParser {
public:
    // Add or modify constructor
    EdtasmSyntax();

    // ... existing public methods

private:
    // ADD these two lines:
    DirectiveRegistry directive_registry_;
    void register_directives();

    // Keep existing handler methods...
};
```

### Step 2: Add Constructor (edtasm_syntax.cpp)

**Add at appropriate location:**
```cpp
EdtasmSyntax::EdtasmSyntax() {
    register_directives();
}
```

### Step 3: Implement register_directives()

**Pattern to follow (from scmasm_syntax.cpp):**
```cpp
void EdtasmSyntax::register_directives() {
    // Register all directives found in if/else chain

    directive_registry_.register_directive("ORG",
        [this](const std::string& label, const std::string& operand) {
            return handle_org_directive(label, operand);
        });

    directive_registry_.register_directive("EQU",
        [this](const std::string& label, const std::string& operand) {
            return handle_equ_directive(label, operand);
        });

    directive_registry_.register_directive("DB",
        [this](const std::string& label, const std::string& operand) {
            return handle_db_directive(label, operand);
        });

    directive_registry_.register_directive("DW",
        [this](const std::string& label, const std::string& operand) {
            return handle_dw_directive(label, operand);
        });

    directive_registry_.register_directive("DS",
        [this](const std::string& label, const std::string& operand) {
            return handle_ds_directive(label, operand);
        });

    directive_registry_.register_directive("ASC",
        [this](const std::string& label, const std::string& operand) {
            return handle_asc_directive(label, operand);
        });

    directive_registry_.register_directive("DCI",
        [this](const std::string& label, const std::string& operand) {
            return handle_dci_directive(label, operand);
        });

    directive_registry_.register_directive("INV",
        [this](const std::string& label, const std::string& operand) {
            return handle_inv_directive(label, operand);
        });

    directive_registry_.register_directive("FLS",
        [this](const std::string& label, const std::string& operand) {
            return handle_fls_directive(label, operand);
        });

    directive_registry_.register_directive("HEX",
        [this](const std::string& label, const std::string& operand) {
            return handle_hex_directive(label, operand);
        });

    // Add any other directives found in if/else chain...
}
```

### Step 4: Replace parse_directive Method

**Before (lines 35-77 in edtasm_syntax.cpp):**
```cpp
DirectiveParseResult EdtasmSyntax::parse_directive(
    const std::string& keyword,
    const std::string& label,
    const std::string& operand) {

    if (keyword == "ORG" || keyword == "org") {
        return handle_org_directive(label, operand);
    } else if (keyword == "EQU" || keyword == "equ") {
        return handle_equ_directive(label, operand);
    } else if (keyword == "DB" || keyword == "db") {
        return handle_db_directive(label, operand);
    }
    // ... 40+ more lines of if/else
}
```

**After:**
```cpp
DirectiveParseResult EdtasmSyntax::parse_directive(
    const std::string& keyword,
    const std::string& label,
    const std::string& operand) {

    // Single O(1) lookup in registry
    return directive_registry_.parse_directive(keyword, label, operand);
}
```

---

## Case Sensitivity Handling

**Important:** The current code checks both cases: `"ORG" || "org"`.

Check how scmasm handles this:
- If registry handles case-insensitively: Register once with uppercase
- If not: Register both uppercase and lowercase variants

**Example if case-sensitive:**
```cpp
directive_registry_.register_directive("ORG", handler);
directive_registry_.register_directive("org", handler);  // lowercase alias
```

---

## Verification Tests

### Functional Tests
- All existing edtasm tests must pass
- Verify directive parsing still works
- Check case-insensitive directive names (ORG vs org)
- Verify unknown directive handling

### Performance Tests (Optional)
- Measure parse_directive execution time before/after
- Should see improvement for programs with many directives

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Break directive parsing | High | Test after each phase, incremental approach |
| Case sensitivity issues | Medium | Check registry behavior, test both cases |
| Missing directive | Medium | Count directives in if/else, verify all registered |
| Signature mismatch | Low | Follow scmasm pattern exactly |

---

## Quality Gates

**Before starting:**
- [ ] All edtasm tests passing
- [ ] All total tests passing

**During refactoring:**
- [ ] Compile after header changes
- [ ] Compile after constructor addition
- [ ] Compile after register_directives skeleton
- [ ] Run tests after complete implementation

**Before commit:**
- [ ] All tests passing (100%)
- [ ] No compiler warnings
- [ ] Code review (compare with scmasm pattern)
- [ ] Verify all directives registered

---

## Reference Comparison

### scmasm_syntax.h (REFERENCE):
```cpp
class ScmasmSyntax : public SyntaxParser {
private:
    DirectiveRegistry directive_registry_;
    void register_directives();
```

### edtasm_syntax.h (TARGET):
```cpp
class EdtasmSyntax : public SyntaxParser {
private:
    DirectiveRegistry directive_registry_;  // ADD
    void register_directives();             // ADD
```

---

## Estimated Timeline

- Analysis & baseline: 30 min
- Header modifications: 20 min
- Add constructor: 10 min
- Implement register_directives: 2 hours
- Replace parse_directive: 30 min
- Verification & testing: 1 hour
- Commit: 10 min

**Total:** 4-5 hours

---

**Status:** READY FOR EXECUTION
