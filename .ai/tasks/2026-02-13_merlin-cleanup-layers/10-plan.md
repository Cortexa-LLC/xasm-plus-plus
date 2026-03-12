# Implementation Plan: Eliminate Merlin 3-Layer Indirection

**Task ID:** 2026-02-13_merlin-cleanup-layers
**Workflow:** `.ai-pack/workflows/refactor.md`
**Role:** Engineer (delegated from Orchestrator)

---

## Workflow Selection

**Using:** Refactor workflow (behavior-preserving architectural improvement)

**Why:** Pure refactoring to eliminate unnecessary indirection and match established SCMASM pattern.

---

## Approach

### Phase 1: Analysis & Baseline
1. Read `src/syntax/scmasm_directive_handlers.cpp` - Reference implementation
2. Read `src/syntax/merlin_directives.cpp` - Source of implementations to move
3. Read `src/syntax/merlin_directive_handlers.cpp` - Target file (currently wrappers)
4. Count directives to migrate (~30 handlers)
5. Verify all tests passing baseline (114 Merlin, 1538 total)

### Phase 2: Move Implementations
For each directive handler:

1. **Copy implementation from merlin_directives.cpp**
   - Get the actual logic (parsing, validation, atom creation)
   - Preserve all error messages, comments, and behavior

2. **Replace wrapper in merlin_directive_handlers.cpp**
   - Remove: `parser->HandleX(...)` call
   - Insert: Full implementation from merlin_directives.cpp
   - Adapt: Access state via `context.*` instead of parser members
   - Match: SCMASM style (no parser delegation)

3. **Access patterns:**
   ```cpp
   // Instead of: parser->current_address_
   // Use: *context.current_address

   // Instead of: parser->symbols_
   // Use: *context.symbols

   // Instead of: parser->section_
   // Use: *context.section
   ```

### Phase 3: Remove Obsolete Code
1. **Delete files:**
   - `rm src/syntax/merlin_directives.cpp` (715 lines)
   - `rm include/xasm++/directives/merlin_directives.h`

2. **Remove instance methods from merlin_syntax.cpp:**
   - Delete: All `MerlinSyntaxParser::HandleX()` delegate methods
   - Keep: Only stateful methods needed by handlers (macros, conditionals)

3. **Remove declarations from merlin_syntax.h:**
   - Delete: Method declarations for deleted delegates
   - Keep: Only methods still needed

4. **Update CMakeLists.txt:**
   - Remove: `syntax/merlin_directives.cpp` from source list

5. **Remove includes:**
   - Delete: `#include "xasm++/directives/merlin_directives.h"` from merlin_syntax.cpp

### Phase 4: Verification
1. Rebuild: `cmake --build build`
2. Run Merlin tests: `ctest --test-dir build -R Merlin`
3. Run full test suite: `ctest --test-dir build`
4. Verify 100% pass rate (114 Merlin, 1538 total)
5. Check no compiler warnings

### Phase 5: Commit
Single atomic commit with descriptive message

---

## Execution Strategy

**Sequential** - Must complete in order:
1. Baseline verification
2. Move implementations (incremental, test frequently)
3. Delete obsolete code
4. Final verification
5. Commit

**Incremental Testing:**
- Compile after every 5-10 handlers migrated
- Run Merlin tests after each batch
- Catch errors early

---

## File Impact Analysis

**Files Modified:**
- `src/syntax/merlin_directive_handlers.cpp` - Replace wrappers with full implementations (~1500 lines)
- `src/syntax/merlin_syntax.cpp` - Remove delegate methods (~300 lines removed)
- `include/xasm++/syntax/merlin_syntax.h` - Remove method declarations (~30 lines removed)
- `src/CMakeLists.txt` - Remove merlin_directives.cpp from build

**Files Deleted:**
- `src/syntax/merlin_directives.cpp` (715 lines)
- `include/xasm++/directives/merlin_directives.h` (~200 lines)

**Net Impact:** ~1200 lines removed

---

## Key Adaptations

### State Access Changes

**Before (3-layer):**
```cpp
// merlin_syntax.cpp
void MerlinSyntaxParser::HandleOrg(operand, section, symbols) {
  HandleOrgDirective(operand, section, symbols, current_address_);
  //                                              ^^^^^^^^^^^^^^^^ member var
}
```

**After (2-layer):**
```cpp
// merlin_directive_handlers.cpp
void merlin::HandleOrg(label, operand, context) {
  // Access via context
  *context.current_address = address;
  //^^^^^^^^^^^^^^^^^^^^^^^^^ context pointer
}
```

### Error Message Format

**Preserve existing error formatting:**
- Use `context.current_file` and `context.current_line` for error messages
- Match existing error format exactly
- No changes to error text

### Special Cases: Stateful Operations

Some operations need parser instance methods (like SCMASM does):
- **Macros:** HandlePMC, HandleEOM, HandleMAC - keep instance methods
- **Conditionals:** HandleDo, HandleElse, HandleFin - may need instance state
- **LUP blocks:** HandleLup - needs loop state

Call these via `context.parser_state` like SCMASM does:
```cpp
auto *parser = static_cast<MerlinSyntaxParser*>(context.parser_state);
parser->HandlePMC(label);  // OK - needs instance state
```

---

## Reference Implementation

**Study scmasm_directive_handlers.cpp for patterns:**
- How to access context fields
- How to handle parser state when needed
- Error message formatting
- Comment style
- Helper function organization

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Break functionality | High | Incremental approach, test after each batch |
| Miss state dependencies | Medium | Identify parser state access upfront |
| Error format changes | Low | Copy implementations verbatim, preserve messages |
| Linker errors | Low | Update CMakeLists.txt, remove includes |

---

## Quality Gates

**Before starting:**
- [ ] All 114 Merlin tests passing
- [ ] All 1538 total tests passing

**During migration:**
- [ ] Compile after each batch of 5-10 handlers
- [ ] Run Merlin tests after each batch
- [ ] Fix errors immediately before continuing

**Before commit:**
- [ ] All tests passing (100%)
- [ ] No compiler warnings
- [ ] Code review (self-check against SCMASM pattern)
- [ ] Verify obsolete files deleted

---

## Estimated Timeline

- Analysis & baseline: 20 min
- Move implementations (~30 handlers): 1.5-2 hours
- Delete obsolete code: 20 min
- Verification: 20 min
- Commit: 10 min

**Total:** 2-3 hours

---

**Status:** READY FOR EXECUTION
