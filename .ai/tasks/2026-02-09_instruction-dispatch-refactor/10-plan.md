# Implementation Plan: Instruction Dispatch Refactoring

**Task:** 2026-02-09_instruction-dispatch-refactor
**Beads ID:** xasm++-ogmf
**Engineer:** AI Engineer
**Started:** 2026-02-09

## Analysis

After examining the codebase, I've identified that the **directive dispatch** in `Z80UniversalSyntaxParser::ParseLine()` uses a massive if/elseif chain (~40 branches) handling directives like ORG, EQU, DB, DW, IF, IFDEF, etc.

**Current Code Smell:**
```cpp
if (upper_mnemonic == "ORG") {
    HandleOrg(...);
} else if (upper_mnemonic == "EQU") {
    HandleEqu(...);
} else if (upper_mnemonic == "=" || upper_mnemonic == "SET" || upper_mnemonic == "DEFL") {
    HandleSet(...);
} else if (upper_mnemonic == "DB" || upper_mnemonic == "DEFB" || ...) {
    HandleDB(...);
} // ... 35+ more elseif branches
```

**Metrics:**
- Current chain: ~40 branches
- Cyclomatic complexity: ~50
- Adding new directive: Requires modifying 500+ line function

## Design Decision: Strategy Pattern with std::function

**Chosen approach:** Registry pattern using `std::function` (Option B from contract)

**Rationale:**
- Lighter weight than full Command pattern (no separate classes needed)
- Easy to migrate incrementally (can test each handler)
- Handlers stay close to registration (good locality)
- Each directive becomes independently testable
- Easy to add new directives (register in one place)

## Implementation Strategy

### Phase 1: Create Handler Framework (TDD)
1. **RED:** Write test for `DirectiveRegistry::Register()` and `Execute()`
2. **GREEN:** Implement minimal `DirectiveRegistry` class
3. **REFACTOR:** Clean up interface

### Phase 2: Migrate 3 Directives (Pilot)
1. **RED:** Write tests for ORG, EQU, DB handlers
2. **GREEN:** Extract handlers, register them, update dispatch
3. **REFACTOR:** Verify all tests pass
4. **Commit:** Checkpoint with 3 directives migrated

### Phase 3: Migrate Remaining Directives (Batch)
1. Migrate data directives: DW, D3, DWORD, DS
2. Migrate symbol directives: SET, PUBLIC, EXTERN
3. Migrate segment directives: CSEG, DSEG, ASEG, COMMON
4. Migrate conditional directives: IF, IFDEF, ELSE, ENDIF, etc.
5. Migrate macro directives: MACRO, ENDM, REPT, IRP, IRPC
6. **REFACTOR:** Remove if/elseif chain entirely

### Phase 4: Verification
1. Run all tests
2. Verify zero compiler warnings
3. Update metrics
4. Complete documentation

## Handler Signature

```cpp
using DirectiveHandler = std::function<void(
    const std::string& label,
    const std::string& operand,
    Section& section,
    ConcreteSymbolTable& symbols)>;
```

## Directory Structure

**New files to create:**
- `include/xasm++/syntax/directive_registry.h` - Registry class
- `src/syntax/directive_registry.cpp` - Registry implementation (if needed)
- `tests/unit/test_directive_registry.cpp` - Unit tests

**Modified files:**
- `src/syntax/z80_universal/z80_universal_syntax.cpp` - Replace if/elseif with registry
- `include/xasm++/syntax/z80_universal_syntax.h` - Add registry member

## Testing Approach

### Unit Tests for Registry
- Register directive handler
- Execute registered handler
- Error on unknown directive
- Multiple registrations
- Case-insensitive lookup

### Integration Tests
- All existing Z80 syntax tests must pass
- No behavioral changes
- Coverage maintained at 80%+

## Risks & Mitigation

| Risk | Mitigation |
|------|------------|
| Breaking existing tests | Migrate incrementally, test after each batch |
| Performance overhead | std::unordered_map lookup is O(1), negligible |
| Handler signature changes | Keep signature flexible with lambda captures |

## Success Criteria Checklist

- [ ] DirectiveRegistry class created and tested
- [ ] All 40+ directives migrated to handlers
- [ ] If/elseif chain removed from ParseLine()
- [ ] All existing tests pass (100%)
- [ ] Zero compiler warnings
- [ ] Cyclomatic complexity: 50 → <10 (80%+ reduction)
- [ ] Code coverage: Maintained at 80%+
- [ ] Documentation updated

## Estimated Effort

- Phase 1: 1 hour (framework + tests)
- Phase 2: 1 hour (pilot migration)
- Phase 3: 3-4 hours (remaining directives)
- Phase 4: 1 hour (verification)

**Total:** 6-7 hours

---

**Status:** Ready to implement
**Next Step:** Phase 1 - Create DirectiveRegistry with TDD
