# Implementation Plan: SCMASM Handler Extraction

**Task:** Phase 6c.2 - SCMASM Handler Extraction
**Beads Task:** xasm++-1ilq

---

## Overview

Extract 11+ SCMASM directive handlers from ScmasmSyntaxParser class into free functions following Phase 6b pattern. Eliminate all magic numbers and strings.

---

## Execution Strategy

**Strategy:** SEQUENTIAL (single engineer)

**Rationale:** 
- Small batch (7 files)
- High cohesion - all SCMASM handlers
- Dependencies between constant definitions and handler implementations
- Token budget at limit (24.5K)

---

## Implementation Steps

### Step 1: Create Constant Definition Headers

**File:** `include/xasm++/directives/scmasm_directive_constants.h`

Create constants for all SCMASM directive names:
```cpp
namespace scmasm {
  namespace directives {
    constexpr const char* OR = ".OR";
    constexpr const char* EQ = ".EQ";
    constexpr const char* SE = ".SE";
    constexpr const char* AS = ".AS";
    constexpr const char* AT = ".AT";
    constexpr const char* AZ = ".AZ";
    constexpr const char* DA = ".DA";
    constexpr const char* DFB = ".DFB";
    constexpr const char* HS = ".HS";
    constexpr const char* BS = ".BS";
    constexpr const char* MA = ".MA";
    constexpr const char* ENDM = ".ENDM";
    constexpr const char* EM = ".EM";
  }
}
```

**File:** `include/xasm++/directives/scmasm_constants.h`

Create constants for character/ASCII processing:
```cpp
namespace scmasm {
  namespace constants {
    // High-bit rule constants (for ApplyHighBitRule)
    constexpr uint8_t HIGH_BIT_DELIMITER_THRESHOLD = 0x27;  // apostrophe
    constexpr uint8_t HIGH_BIT_MASK = 0x80;
    constexpr uint8_t LOW_7_BITS_MASK = 0x7F;
    
    // String processing
    constexpr uint8_t NULL_TERMINATOR = 0x00;
    
    // Data size constants
    constexpr size_t BITS_PER_BYTE = 8;
    constexpr size_t BITS_PER_WORD = 16;
    constexpr size_t BITS_PER_TRIBYTE = 24;
    constexpr size_t BITS_PER_DWORD = 32;
    
    // Macro limits
    constexpr size_t MAX_MACRO_DEPTH = 63;
  }
}
```

**Tests:** Add unit tests for constant values (verify correctness)

---

### Step 2: Create Handler Declaration Header

**File:** `include/xasm++/directives/scmasm_directive_handlers.h`

Declare all handler functions:
```cpp
#ifndef XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_HANDLERS_H
#define XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_HANDLERS_H

#include "xasm++/syntax/directive_registry.h"
#include <string>

namespace xasm {
namespace scmasm {

// Origin and addressing
void HandleOr(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

// Symbol definition
void HandleEq(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

void HandleSe(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

// String directives
void HandleAs(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

void HandleAt(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

void HandleAz(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

// Data directives
void HandleDa(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

void HandleHs(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

void HandleBs(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

// Macro directives
void HandleMa(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

void HandleEndm(const std::string& label,
                const std::string& operand,
                DirectiveContext& context);

} // namespace scmasm
} // namespace xasm

#endif
```

---

### Step 3: Extract Handler Implementations

**File:** `src/directives/scmasm/scmasm_directive_handlers.cpp`

Create directory if needed:
```bash
mkdir -p src/directives/scmasm/
```

Extract each handler from ScmasmSyntaxParser:

1. **HandleOr** - Origin setting
   - Copy from lambda in InitializeDirectiveRegistry
   - Convert to free function
   - Remove magic string ".OR"
   - Update references to use DirectiveContext

2. **HandleEq** - Constant definition
   - Extract EQ handler
   - Remove magic string ".EQ"
   - Preserve symbol definition logic

3. **HandleSe** - Variable definition
   - Extract SE handler
   - Remove magic string ".SE"
   - Preserve redefinition logic

4. **HandleAs** - ASCII string
   - Extract AS handler
   - Remove magic string ".AS"
   - Update ParseString and ApplyHighBitRule calls

5. **HandleAt** - ASCII text (high bit on last)
   - Extract AT handler
   - Remove magic string ".AT"
   - Replace 0x80 with HIGH_BIT_MASK

6. **HandleAz** - ASCII zero-terminated
   - Extract AZ handler
   - Remove magic string ".AZ"
   - Replace 0x00 with NULL_TERMINATOR

7. **HandleDa** - Define address/bytes with size prefixes
   - Extract DA handler
   - Remove magic string ".DA"
   - Complex logic for #, /, <, > prefixes
   - Replace size constants with named constants

8. **HandleHs** - Hex string
   - Extract HS handler
   - Remove magic string ".HS"
   - Replace magic number 2 with BITS_PER_BYTE/4

9. **HandleBs** - Binary string
   - Extract BS handler
   - Remove magic string ".BS"
   - Replace magic number 8 with BITS_PER_BYTE

10. **HandleMa** - Macro definition
    - Extract MA handler
    - Remove magic string ".MA"
    - Preserve macro state management

11. **HandleEndm** - End macro
    - Extract ENDM/EM handler
    - Remove magic strings ".ENDM" and ".EM"
    - Replace 63 with MAX_MACRO_DEPTH

**Key transformations:**
- Lambda captures become DirectiveContext members
- `this->method()` becomes direct function calls or context access
- Parser state accessed via context or passed as parameters

---

### Step 4: Update ScmasmSyntaxParser

**File:** `src/syntax/scmasm/scmasm_syntax.cpp`

1. Include new handler header:
```cpp
#include "xasm++/directives/scmasm_directive_handlers.h"
#include "xasm++/directives/scmasm_directive_constants.h"
#include "xasm++/directives/scmasm_constants.h"
```

2. Update InitializeDirectiveRegistry() to use free functions:
```cpp
void ScmasmSyntaxParser::InitializeDirectiveRegistry() {
  using namespace scmasm::directives;
  
  directive_registry_[OR] = scmasm::HandleOr;
  directive_registry_[EQ] = scmasm::HandleEq;
  directive_registry_[SE] = scmasm::HandleSe;
  directive_registry_[AS] = scmasm::HandleAs;
  directive_registry_[AT] = scmasm::HandleAt;
  directive_registry_[AZ] = scmasm::HandleAz;
  directive_registry_[DA] = scmasm::HandleDa;
  directive_registry_[DFB] = scmasm::HandleDa;  // Alias
  directive_registry_[HS] = scmasm::HandleHs;
  directive_registry_[BS] = scmasm::HandleBs;
  directive_registry_[MA] = scmasm::HandleMa;
  directive_registry_[ENDM] = scmasm::HandleEndm;
  directive_registry_[EM] = scmasm::HandleEndm;  // Alias
}
```

3. Remove old handler method declarations from class
4. Remove old handler method implementations
5. Update any remaining magic strings in ParseLine, etc.

---

### Step 5: Helper Functions

Some handlers need helper functions that were class methods. These need to be:

1. **ApplyHighBitRule** - Used by As, At, Az handlers
   - Extract as free function in scmasm_directive_handlers.cpp
   - Replace magic numbers with constants
   - Make it accessible to handlers

2. **ParseString** - Used by As, At, Az handlers
   - Extract as free function
   - Update to use ApplyHighBitRule free function

These helpers should be in anonymous namespace or scmasm::detail namespace.

---

### Step 6: Update CMakeLists.txt

Add new files to build:
```cmake
# In src/CMakeLists.txt or appropriate location
target_sources(xasm_syntax PRIVATE
  directives/scmasm/scmasm_directive_handlers.cpp
)
```

---

### Step 7: Run Tests

Execute test suite:
```bash
cd build_coverage
ctest -R scmasm
```

Verify:
- All SCMASM directive tests pass
- No regressions in existing functionality
- Coverage maintained at 97%+

---

### Step 8: Update Documentation

Update any relevant documentation:
- Architecture docs if handler extraction pattern needs documentation
- Code comments explaining new organization
- README if extraction pattern is noteworthy

---

## Testing Strategy

### Unit Tests

**Existing tests to verify:**
- `test_scmasm_syntax.cpp` - All directive parsing tests
- `test_scmasm_directive_registry.cpp` - Registry tests

**New tests to add:**
- Constant value verification tests
- Handler function tests (if not already covered)

### Integration Tests

**Existing tests to verify:**
- `test_scmasm_compat.cpp` - Binary compatibility tests
- Verify all .asm test files still assemble correctly

### TDD Workflow

Follow RED-GREEN-REFACTOR for each handler:

1. **RED:** Write test for extracted handler (if not exists)
2. **GREEN:** Extract handler to pass test
3. **REFACTOR:** Eliminate magic numbers/strings
4. **VERIFY:** Test still passes with constants

---

## File Checklist

**New Files:**
- [ ] `include/xasm++/directives/scmasm_directive_constants.h`
- [ ] `include/xasm++/directives/scmasm_constants.h`
- [ ] `include/xasm++/directives/scmasm_directive_handlers.h`
- [ ] `src/directives/scmasm/scmasm_directive_handlers.cpp`

**Modified Files:**
- [ ] `src/syntax/scmasm/scmasm_syntax.cpp` (InitializeDirectiveRegistry, remove old handlers)
- [ ] `include/xasm++/syntax/scmasm_syntax.h` (remove old handler declarations)
- [ ] `src/CMakeLists.txt` (add new source files)

**Test Files:**
- [ ] Verify `tests/unit/test_scmasm_syntax.cpp` still passes
- [ ] Verify `tests/unit/test_scmasm_directive_registry.cpp` still passes
- [ ] Verify `tests/integration/binary_compat/test_scmasm_compat.cpp` still passes

---

## Risk Mitigation

### Token Budget Risk

**Risk:** Approaching 24.5K token estimate (close to 32K limit)

**Mitigation:**
- Monitor token usage during implementation
- If approaching limit, split into two sub-tasks:
  - Sub-task 1: .OR, .EQ, .SE, .AS, .AT, .AZ (6 handlers)
  - Sub-task 2: .DA/.DFB, .HS, .BS, .MA, .ENDM/.EM (7 handlers)

### Test Failure Risk

**Risk:** Tests may fail after handler extraction

**Mitigation:**
- Follow TDD strictly (test each handler extraction)
- Run tests frequently
- Use git to track changes incrementally

### Magic Number Detection Risk

**Risk:** May miss some magic numbers during elimination

**Mitigation:**
- Use grep to search for literal numbers: `grep -rn "0x[0-9A-Fa-f]" src/directives/scmasm/`
- Use grep to search for literal strings: `grep -rn '"\.' src/directives/scmasm/`
- Code review by Inspector agent

---

## Success Metrics

- [ ] All 11 unique handlers extracted (13 total including aliases)
- [ ] Zero magic strings in extracted code
- [ ] Zero magic numbers in extracted code
- [ ] All tests passing (97%+ coverage)
- [ ] No compiler warnings
- [ ] Clean git history with meaningful commits

---

**Plan Status:** READY FOR EXECUTION
**Estimated Duration:** 2-3 hours
**Orchestrator:** Ready to delegate to Engineer

---

**Plan Approved:** ✓ Orchestrator 2026-02-11
