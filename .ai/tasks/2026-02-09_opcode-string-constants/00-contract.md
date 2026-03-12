# Task Contract: Opcode String Constants

**Task ID:** 2026-02-09_opcode-string-constants
**Beads Task:** xasm++-30s4
**Created:** 2026-02-09
**Priority:** P2

Replace magic strings for instruction names with named constants, eliminating string literal duplication and typo risks.

## Problem Statement

Current code uses string literals for instruction names throughout parsers:

```cpp
// Code smell: Magic strings scattered everywhere
if (mnemonic == "LD") { ... }       // String literal "LD"
if (mnemonic == "ADD") { ... }      // String literal "ADD"
if (instr == "SUB") { ... }         // Different variable name, same concept
if (op == "AND") { ... }            // More inconsistency

// In tests:
REQUIRE(parser.Parse("LD A,42"));   // String literal "LD"
REQUIRE(parser.Parse("ADD A,B"));   // String literal "ADD"

// In error messages:
throw std::runtime_error("Invalid LD operands");  // String literal "LD"
```

Problems:
- **Typo risk** - `"LDD"` instead of `"LD"` compiles but fails at runtime
- **Duplication** - Same string appears dozens of times
- **Hard to refactor** - Changing instruction name requires find/replace
- **No autocomplete** - IDEs can't help with string literals
- **Error-prone** - String comparisons are case-sensitive

## Objective

Define instruction name constants in opcode headers, use throughout codebase.

## Success Criteria

✓ All instruction names defined as constants
✓ Zero magic strings for instruction names in parser code
✓ Zero magic strings for instruction names in tests
✓ Constants organized by CPU architecture
✓ All existing tests pass
✓ Zero compiler warnings

## Acceptance Criteria

### Phase 1: Define Constants (1-2 hours)
- [ ] Add to `include/xasm++/cpu/opcodes_z80.h`:
  ```cpp
  namespace Z80Mnemonics {
    constexpr const char* LD = "LD";
    constexpr const char* ADD = "ADD";
    constexpr const char* SUB = "SUB";
    // ... all Z80 instructions
  }
  ```
- [ ] Add to `include/xasm++/cpu/opcodes_6502.h`
- [ ] Add to `include/xasm++/cpu/opcodes_6809.h`
- [ ] Document naming convention

### Phase 2: Replace in Parsers (2-3 hours)
- [ ] Replace magic strings in Z80Universal parser
- [ ] Replace magic strings in Merlin parser
- [ ] Replace magic strings in SCMASM parser
- [ ] Replace magic strings in other parsers
- [ ] Verify all tests pass after each parser

### Phase 3: Replace in Tests (1-2 hours)
- [ ] Replace magic strings in unit tests
- [ ] Replace magic strings in integration tests
- [ ] Verify all tests pass

### Phase 4: Replace in Error Messages (1 hour)
- [ ] Replace magic strings in error/warning messages
- [ ] Verify error messages still clear

### Phase 5: Verification (1 hour)
- [ ] Grep for remaining magic strings
- [ ] Zero instruction name literals found
- [ ] All tests passing
- [ ] Zero compiler warnings

## Design Approach

### Option A: Namespaced Constants (Recommended)

```cpp
// include/xasm++/cpu/opcodes_z80.h
namespace xasm {
namespace Z80Mnemonics {

// Organized by category
namespace Load {
  constexpr const char* LD = "LD";
  constexpr const char* PUSH = "PUSH";
  constexpr const char* POP = "POP";
}

namespace Arithmetic {
  constexpr const char* ADD = "ADD";
  constexpr const char* SUB = "SUB";
  constexpr const char* ADC = "ADC";
  constexpr const char* SBC = "SBC";
  constexpr const char* INC = "INC";
  constexpr const char* DEC = "DEC";
}

namespace Logical {
  constexpr const char* AND = "AND";
  constexpr const char* OR = "OR";
  constexpr const char* XOR = "XOR";
  constexpr const char* CP = "CP";
}

namespace Branch {
  constexpr const char* JP = "JP";
  constexpr const char* JR = "JR";
  constexpr const char* CALL = "CALL";
  constexpr const char* RET = "RET";
}

namespace Rotate {
  constexpr const char* RLCA = "RLCA";
  constexpr const char* RRCA = "RRCA";
  constexpr const char* RLA = "RLA";
  constexpr const char* RRA = "RRA";
}

namespace Bit {
  constexpr const char* BIT = "BIT";
  constexpr const char* SET = "SET";
  constexpr const char* RES = "RES";
}

} // namespace Z80Mnemonics
} // namespace xasm
```

Usage:
```cpp
using namespace xasm::Z80Mnemonics;

// In parser
if (mnemonic == Load::LD) { ... }
if (mnemonic == Arithmetic::ADD) { ... }

// In tests
REQUIRE(parser.Parse(Load::LD + std::string(" A,42")));
```

### Option B: Flat Namespace (Simpler)

```cpp
// include/xasm++/cpu/opcodes_z80.h
namespace xasm::Z80Mnemonics {
  constexpr const char* LD = "LD";
  constexpr const char* ADD = "ADD";
  constexpr const char* SUB = "SUB";
  // ... all instructions flat
}
```

Usage:
```cpp
using namespace xasm::Z80Mnemonics;

if (mnemonic == LD) { ... }
if (mnemonic == ADD) { ... }
```

### Recommended: Option B (Flat)

**Rationale:**
- Simpler, less nesting
- Z80 instructions don't have naming conflicts
- Easier to use
- Still organized by file (one per CPU)

**With commenting for organization:**
```cpp
namespace xasm::Z80Mnemonics {
  // Load/Store
  constexpr const char* LD = "LD";
  constexpr const char* PUSH = "PUSH";
  constexpr const char* POP = "POP";

  // Arithmetic
  constexpr const char* ADD = "ADD";
  constexpr const char* SUB = "SUB";
  // ...
}
```

## Example Migration

**Before:**
```cpp
// Parser
if (mnemonic == "LD") {
  if (operands == "A,n") {
    return EncodeLD_A_n(value);
  }
}

// Test
TEST_CASE("Z80 - LD A,n") {
  auto result = cpu.Parse("LD A,42");
  // ...
}

// Error
throw std::runtime_error("Invalid LD operands");
```

**After:**
```cpp
#include "xasm++/cpu/opcodes_z80.h"
using namespace xasm::Z80Mnemonics;

// Parser
if (mnemonic == LD) {
  if (operands == "A,n") {
    return EncodeLD_A_n(value);
  }
}

// Test
TEST_CASE("Z80 - LD A,n") {
  auto result = cpu.Parse(LD + std::string(" A,42"));
  // Or: std::string(LD) + " A,42"
  // ...
}

// Error
throw std::runtime_error(std::string("Invalid ") + LD + " operands");
```

## Metrics

**Before:**
- Magic strings: ~500+ occurrences across parsers and tests
- Typo risk: High (no compile-time checking)
- Refactoring difficulty: High (find/replace error-prone)

**After:**
- Magic strings: 0 (only in constant definitions)
- Typo risk: Zero (compile-time checking)
- Refactoring difficulty: Low (change constant, all uses update)

**Code Quality:**
- ✅ DRY principle
- ✅ Type safety
- ✅ Refactoring safety
- ✅ IDE autocomplete support

## Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| String concatenation verbosity | Low | Small price for safety |
| Breaking existing behavior | Low | Tests validate behavior unchanged |
| Migration effort underestimated | Low | Simple find/replace mostly |

## Files to Modify

**Opcode Headers (add constants):**
- `include/xasm++/cpu/opcodes_z80.h`
- `include/xasm++/cpu/opcodes_6502.h`
- `include/xasm++/cpu/opcodes_6809.h`

**Parsers (replace strings):**
- `src/syntax/z80_universal/z80_universal_syntax.cpp`
- `src/syntax/merlin/merlin_syntax.cpp`
- `src/syntax/scmasm/scmasm_syntax.cpp`
- All other parser implementations

**Tests (replace strings):**
- `tests/unit/test_cpu_z80.cpp`
- `tests/unit/test_cpu_6502.cpp`
- `tests/unit/test_cpu_6809.cpp`
- All parser test files

## Estimated Effort

- Define constants (all CPUs): 1-2 hours
- Replace in parsers: 2-3 hours
- Replace in tests: 1-2 hours
- Replace in error messages: 1 hour
- Verification: 1 hour

**Total:** 6-9 hours (1 day)

## Dependencies

- None (can run in parallel with other work)
- Recommended: Do alongside instruction dispatch refactoring

## Stakeholders

- **Owner:** Orchestrator
- **Implementer:** Engineer
- **Reviewer:** Code quality review

**Approved:** Orchestrator 2026-02-09
