# Implementation Plan: Convert simple_syntax lambdas to free functions

**Task:** 2026-02-14_simple-syntax-lambdas  
**Created:** 2026-02-14  
**Status:** Ready for execution

---

## Overview

This plan details the step-by-step process to extract 3 lambda directive handlers from `simple_syntax.cpp` to free functions in a separate file, following the Merlin pattern.

---

## Execution Strategy

**Approach:** Sequential implementation (single developer)

**Estimated Time:** 1 hour

**Batch Size:** 3 files (within optimal range)

**Parallelization:** Not needed (small refactoring task)

---

## Implementation Steps

### Phase 1: Create Header File (15 minutes)

**File:** `include/xasm++/directives/simple_directive_handlers.h`

**Actions:**
1. Create file with header guard
2. Add file documentation (Doxygen)
3. Include necessary headers:
   - `<string>`
   - `"xasm++/syntax/directive_registry.h"` (for DirectiveContext)
4. Declare namespace `xasm::simple`
5. Declare 3 handler functions:
   ```cpp
   void HandleOrg(const std::string& label, const std::string& operand, DirectiveContext& context);
   void HandleDb(const std::string& label, const std::string& operand, DirectiveContext& context);
   void HandleDw(const std::string& label, const std::string& operand, DirectiveContext& context);
   ```

**Reference:** `include/xasm++/directives/merlin_directive_handlers.h`

**Verification:**
- [ ] File compiles
- [ ] Header guard present
- [ ] Namespace correct

---

### Phase 2: Create Implementation File (30 minutes)

**File:** `src/syntax/simple_directive_handlers.cpp`

**Actions:**

**2.1: File Setup**
1. Add file documentation
2. Include headers:
   ```cpp
   #include "xasm++/directives/simple_directive_handlers.h"
   #include "xasm++/parse_utils.h"  // For ParseHex
   #include "xasm++/atom.h"
   #include <sstream>
   #include <algorithm>
   #include <cctype>
   ```
3. Open namespace `xasm::simple`

**2.2: Helper Functions**
1. Copy helper functions from `simple_syntax.cpp`:
   - `static std::string Trim(const std::string&)`
   - `static std::string ToUpper(const std::string&)` (if needed)

**2.3: HandleOrg Implementation**
Extract lambda body:
```cpp
void HandleOrg(const std::string& /*label*/, 
               const std::string& operand,
               DirectiveContext& context) {
  uint32_t address = ParseHex(operand);
  context.section->atoms.push_back(std::make_shared<OrgAtom>(address));
  *context.current_address = address;
}
```

**2.4: HandleDb Implementation**
Extract lambda body:
```cpp
void HandleDb(const std::string& /*label*/,
              const std::string& operand,
              DirectiveContext& context) {
  std::vector<uint8_t> bytes;
  std::istringstream ops(operand);
  std::string value;

  while (std::getline(ops, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      bytes.push_back(static_cast<uint8_t>(ParseHex(value)));
    }
  }

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}
```

**2.5: HandleDw Implementation**
Extract lambda body:
```cpp
void HandleDw(const std::string& /*label*/,
              const std::string& operand,
              DirectiveContext& context) {
  std::vector<uint8_t> bytes;
  std::istringstream ops(operand);
  std::string value;

  while (std::getline(ops, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      uint32_t word = ParseHex(value);
      bytes.push_back(static_cast<uint8_t>(word & 0xFF));        // Low byte
      bytes.push_back(static_cast<uint8_t>((word >> 8) & 0xFF)); // High byte
    }
  }

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}
```

**Reference:** Copy exact logic from lambda bodies (lines 57-108 in simple_syntax.cpp)

**Verification:**
- [ ] File compiles
- [ ] All handlers implemented
- [ ] Helper functions present

---

### Phase 3: Update simple_syntax.cpp (10 minutes)

**File:** `src/syntax/simple_syntax.cpp`

**Actions:**

**3.1: Add Include**
```cpp
#include "xasm++/directives/simple_directive_handlers.h"
```

**3.2: Update InitializeDirectives()**
Replace lambda registrations:
```cpp
void SimpleSyntaxParser::InitializeDirectives() {
  // Register .ORG directive
  directive_registry_.Register("ORG", simple::HandleOrg);

  // Register .DB directive
  directive_registry_.Register("DB", simple::HandleDb);

  // Register .DW directive
  directive_registry_.Register("DW", simple::HandleDw);
}
```

**3.3: Remove Helper Functions (Optional)**
If Trim, ToUpper are only used by lambdas:
- Consider removing from simple_syntax.cpp
- They're now in simple_directive_handlers.cpp

**Verification:**
- [ ] File compiles
- [ ] No lambdas remain in InitializeDirectives()
- [ ] Registration calls are correct

---

### Phase 4: Update Build Configuration (5 minutes)

**File:** `CMakeLists.txt`

**Actions:**
1. Find the source file list for xasm_syntax library
2. Add `src/syntax/simple_directive_handlers.cpp`

**Location:** Look for `add_library(xasm_syntax ...)` or similar

**Example:**
```cmake
set(XASM_SYNTAX_SOURCES
  src/syntax/simple_syntax.cpp
  src/syntax/simple_directive_handlers.cpp  # ADD THIS
  src/syntax/merlin_syntax.cpp
  ...
)
```

**Verification:**
- [ ] CMake configuration succeeds
- [ ] Build succeeds

---

### Phase 5: Verification (10 minutes)

**Actions:**

**5.1: Build Test**
```bash
cd build
cmake --build . 2>&1 | tee build.log
# Verify zero warnings/errors
```

**5.2: Run Tests**
```bash
cd build
ctest -R SimpleSyntax -V
# Verify all SimpleSyntax tests pass
```

**5.3: Full Test Suite**
```bash
cd build
ctest
# Verify no regressions in other tests
```

**5.4: Code Review Checklist**
- [ ] Files follow project style
- [ ] Doxygen comments present
- [ ] No code duplication
- [ ] Consistent with Merlin pattern

**Verification:**
- [ ] All tests pass
- [ ] Zero build warnings
- [ ] Code review ready

---

## File Modification Summary

| File | Type | Action |
|------|------|--------|
| `include/xasm++/directives/simple_directive_handlers.h` | New | Create header with 3 declarations |
| `src/syntax/simple_directive_handlers.cpp` | New | Create implementation with 3 handlers |
| `src/syntax/simple_syntax.cpp` | Modified | Replace lambdas with function calls |
| `CMakeLists.txt` | Modified | Add new source file |

**Total Files:** 4 (2 new, 2 modified)

---

## Test Strategy

### Existing Tests
- `SimpleSyntaxTest` suite validates directive behavior
- Tests use SimpleSyntaxParser API (unchanged)
- Tests verify `.org`, `.db`, `.dw` directives work correctly

### No New Tests Needed
- Handler logic unchanged (exact extraction)
- API unchanged (Parse() method signature same)
- Test coverage already exists

### Validation Approach
1. Run existing tests
2. Compare output with baseline
3. Verify no behavioral changes

---

## Rollback Plan

If issues arise:

1. **Build Errors:**
   - Revert CMakeLists.txt change
   - Remove new files
   - Restore simple_syntax.cpp from git

2. **Test Failures:**
   - Compare handler implementations with lambda bodies
   - Check for copy/paste errors
   - Verify helper functions work correctly

3. **Complete Rollback:**
   ```bash
   git checkout src/syntax/simple_syntax.cpp
   git clean -fd include/xasm++/directives/simple_directive_handlers.h
   git clean -fd src/syntax/simple_directive_handlers.cpp
   git checkout CMakeLists.txt
   ```

---

## Success Criteria

- ✅ All files created/modified as specified
- ✅ Build succeeds with zero warnings
- ✅ All tests pass (especially SimpleSyntaxTest)
- ✅ Code follows Merlin pattern
- ✅ No behavioral changes to directives
- ✅ Code review approval

---

## Notes

### Design Decisions

**Q: Why separate file?**
A: Follows established pattern (Merlin), improves testability, reduces file size

**Q: Why `simple` namespace?**
A: Mirrors `merlin` namespace, prevents name collisions, clear organization

**Q: Why keep helper functions in .cpp?**
A: File-local helpers (static), no need for header exposure

### Implementation Tips

1. Copy exact lambda bodies (don't refactor logic)
2. Preserve comments from lambdas
3. Match Merlin file structure exactly
4. Test after each phase

---

**Plan Approved:** TBD  
**Ready to Execute:** Yes
