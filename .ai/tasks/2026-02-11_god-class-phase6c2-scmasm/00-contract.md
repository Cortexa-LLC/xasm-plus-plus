# Task Contract: God-Class Phase 6c.2 - SCMASM Handler Extraction

**Task ID:** 2026-02-11_god-class-phase6c2-scmasm
**Beads Task:** xasm++-1ilq
**Created:** 2026-02-11
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract approximately 25 SCMASM directive handlers from ScmasmSyntaxParser class into free functions, following the Phase 6b pattern. Register with DirectiveRegistry. Eliminate magic numbers and magic strings using named constants.

### Background and Context

Phase 6b established the proof-of-concept for extracting directive handlers as free functions. Phase 6c applies this pattern to extract ALL handlers from the three parser classes (Merlin, SCMASM, EDTASM-M80++). This is Phase 6c.2, focusing on SCMASM.

SCMASM parser currently has these directives registered in InitializeDirectiveRegistry():
- .OR - Set origin address
- .EQ - Define constant
- .SE - Set variable (redefinable)
- .AS - ASCII string
- .AT - ASCII text (high bit on last char)
- .AZ - ASCII zero-terminated
- .DA / .DFB - Define byte(s) with size prefix operators
- .HS - Hex string
- .BS - Binary string
- .MA - Begin macro definition
- .ENDM / .EM - End macro definition

Control flow directives (.DO, .ELSE, .FIN, .LU, .ENDU) are NOT extracted as they require special line-skipping logic.

**CRITICAL REQUIREMENT:** Eliminate magic numbers and magic strings:
- Directive names must use string constants (not ".OR", ".EQ" literals)
- Opcodes must use named constants from opcodes_*.h files
- Mnemonics must use string constants
- No hardcoded hex values (0x00, 0x27, 0x80, etc.) - use named constants
- No hardcoded sizes or limits - use named constants

### Current State

- Phase 6b complete: Proof-of-concept pattern established
- SCMASM parser has ~11 directives registered (plus 2 aliases)
- Directive handlers are lambda functions inside InitializeDirectiveRegistry()
- Magic strings and numbers exist in directive implementations (e.g., 0x27, 0x80, ".OR", ".EQ")
- Handlers use DirectiveContext signature

### Desired State

- All 11+ SCMASM directives extracted as free functions
- Functions in new file: src/directives/scmasm/scmasm_directive_handlers.cpp
- Header: include/xasm++/directives/scmasm_directive_handlers.h
- All handlers registered with DirectiveRegistry
- Zero magic numbers in extracted code
- Zero magic strings in extracted code
- Pattern validated for remaining Phase 6c work

---

## Success Criteria

```
✓ All 11+ SCMASM directives extracted (11 unique + 2 aliases = 13 total registrations)
✓ All extracted handlers use free function pattern
✓ All handlers registered with DirectiveRegistry
✓ Zero magic numbers in extracted code (use constants for 0x27, 0x80, 0x7F, etc.)
✓ Zero magic strings in extracted code (use constants for ".OR", ".EQ", etc.)
✓ All tests passing (97%+)
✓ ScmasmSyntaxParser class size reduced
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Extract .OR directive as free function
□ Extract .EQ directive as free function
□ Extract .SE directive as free function
□ Extract .AS directive as free function
□ Extract .AT directive as free function
□ Extract .AZ directive as free function
□ Extract .DA directive as free function
□ Extract .DFB directive (alias to .DA)
□ Extract .HS directive as free function
□ Extract .BS directive as free function
□ Extract .MA directive as free function
□ Extract .ENDM directive as free function
□ Extract .EM directive (alias to .ENDM)
□ Register all extracted handlers with DirectiveRegistry
□ Verify all extracted directives work identically to before
□ Preserve error handling and validation
```

### Code Quality Requirements (CRITICAL)
```
□ NO magic numbers - use named constants
  Example: Replace 0x27 with scmasm::HIGH_BIT_DELIMITER_THRESHOLD
  Example: Replace 0x80 with scmasm::HIGH_BIT_MASK
  Example: Replace 0x7F with scmasm::LOW_7_BITS_MASK

□ NO magic strings - use string constants
  Example: Replace ".OR" with scmasm::directives::OR
  Example: Replace ".EQ" with scmasm::directives::EQ

□ Directive names: Use constants from scmasm_directive_constants.h (create)

□ Character/ASCII constants: Define in scmasm_constants.h (create if needed)

□ Define constants in appropriate header files:
  - Directive names → include/xasm++/directives/scmasm_directive_constants.h
  - ASCII/character processing constants → include/xasm++/directives/scmasm_constants.h
  - High-bit rule constants → include/xasm++/directives/scmasm_constants.h
```

### Quality Requirements
```
□ All tests passing (97%+ target)
□ No warnings
□ TDD followed
□ Code formatted per project standards
□ Functions documented with clear comments
```

---

## Estimated Complexity

**Complexity:** Medium-High

**Files:** 6-8
- 1 parser file (ScmasmSyntaxParser - refactor InitializeDirectiveRegistry)
- 2 new handler files (scmasm_directive_handlers.h/.cpp)
- 2 new constant definition headers (scmasm_directive_constants.h, scmasm_constants.h)
- 2-3 test files (update existing or create new)

**Tokens:** 24K → ⚠️ APPROACHING LIMIT

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 7 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 7 files → ACCEPTABLE
```

### Batch Size Justification

Files: 7 (within acceptable range but requires justification)

**Why not decomposed further:**
- High cohesion - extracting all SCMASM handlers as one unit
- Single concern - complete SCMASM handler extraction
- Established pattern from Phase 6b reduces risk
- Dependencies: All handlers use common constants

**Contingency for token limits:**
- If token limit hit, will extract handlers in two groups:
  - Group 1: .OR, .EQ, .SE, .AS, .AT, .AZ (6 handlers)
  - Group 2: .DA/.DFB, .HS, .BS, .MA, .ENDM/.EM (7 handlers)

**Estimated tokens:** ~7 × 3500 = 24,500 tokens
**Status:** Within 25K-32K limit? ⚠️ CLOSE TO LIMIT

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
7 × 3,500 tokens = 24,500 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 24,500 tokens → APPROACHING LIMIT ⚠️
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [x] YES - Batch size 7 files AND token budget 24.5K AND WIP 1

**Risk Mitigation:**
- Engineer should monitor token usage
- If approaching 32K limit, request decomposition
- Orchestrator ready to split into 2 sub-tasks if needed

---

## Dependencies

**Depends On:** 
- xasm++-uqyd (Phase 6b - Core Handler Extraction) - ✓ COMPLETE
**Blocks:** 
- Phase 6c.3 (EDTASM handler extraction)
**Reference:** 
- `.ai/tasks/2026-02-10_god-class-phase6b-core/` (pattern reference)

---

## Resources and References

### Existing Constant Files
```
include/xasm++/cpu/opcodes_z80.h - Z80 opcode constants
include/xasm++/cpu/opcodes_6502.h - 6502 opcode constants
include/xasm++/cpu/opcodes_6809.h - 6809 opcode constants
```

### Files to Create
```
include/xasm++/directives/scmasm_directive_constants.h - Directive name constants
include/xasm++/directives/scmasm_constants.h - ASCII/character processing constants
src/directives/scmasm/scmasm_directive_handlers.cpp - Handler implementations
include/xasm++/directives/scmasm_directive_handlers.h - Handler declarations
```

### Pattern Reference
```
include/xasm++/syntax/directive_registry.h - DirectiveHandler signature
src/syntax/directive_registry.cpp - Registry implementation
src/directives/common/common_directive_handlers.cpp - Phase 6b examples
```

---

## Special Instructions

**Magic Number/String Elimination Strategy:**

1. **Audit Phase:** Scan extracted handlers for:
   - Literal strings (".OR", ".EQ", ".AS", etc.)
   - Hex constants (0x00, 0x27, 0x7F, 0x80, 0xFF, etc.)
   - Decimal constants with semantic meaning (8, 16, 32 for bit sizes)
   - Loop limits (63 for macro depth)

2. **Constant Definition Phase:**
   - Create scmasm_directive_constants.h with directive names
   - Create scmasm_constants.h with:
     - HIGH_BIT_DELIMITER_THRESHOLD = 0x27 (apostrophe)
     - HIGH_BIT_MASK = 0x80
     - LOW_7_BITS_MASK = 0x7F
     - MAX_MACRO_DEPTH = 63
     - BITS_PER_BYTE = 8
     - NULL_TERMINATOR = 0x00

3. **Replacement Phase:**
   - Replace all magic values with named constants
   - Use constexpr for compile-time constants
   - Group related constants in namespaces

**Example Pattern:**
```cpp
// Before (magic strings and numbers in ApplyHighBitRule):
uint8_t ScmasmSyntaxParser::ApplyHighBitRule(char c, char delimiter) {
  uint8_t result = static_cast<uint8_t>(c);
  if (delimiter < 0x27) {  // Magic number!
    result |= 0x80;  // Magic number!
  } else {
    result &= 0x7F;  // Magic number!
  }
  return result;
}

// After (named constants):
namespace scmasm {
  namespace constants {
    constexpr uint8_t HIGH_BIT_DELIMITER_THRESHOLD = 0x27;  // apostrophe
    constexpr uint8_t HIGH_BIT_MASK = 0x80;
    constexpr uint8_t LOW_7_BITS_MASK = 0x7F;
  }
}

uint8_t ApplyHighBitRule(char c, char delimiter) {
  uint8_t result = static_cast<uint8_t>(c);
  if (delimiter < constants::HIGH_BIT_DELIMITER_THRESHOLD) {
    result |= constants::HIGH_BIT_MASK;
  } else {
    result &= constants::LOW_7_BITS_MASK;
  }
  return result;
}
```

**Directive Name Constants Pattern:**
```cpp
// In scmasm_directive_constants.h:
namespace scmasm {
  namespace directives {
    // Directive names as constants (not string literals)
    constexpr const char* OR = ".OR";
    constexpr const char* EQ = ".EQ";
    constexpr const char* SE = ".SE";
    constexpr const char* AS = ".AS";
    constexpr const char* AT = ".AT";
    constexpr const char* AZ = ".AZ";
    constexpr const char* DA = ".DA";
    constexpr const char* DFB = ".DFB";  // Alias to .DA
    constexpr const char* HS = ".HS";
    constexpr const char* BS = ".BS";
    constexpr const char* MA = ".MA";
    constexpr const char* ENDM = ".ENDM";
    constexpr const char* EM = ".EM";    // Alias to .ENDM
  }
}

// Usage in registry initialization:
directive_registry_[directives::OR] = HandleOr;
directive_registry_[directives::EQ] = HandleEq;
```

**Handler Function Signature Pattern (from Phase 6b):**
```cpp
// Free function signature matching DirectiveHandler:
void HandleOr(const std::string& label,
              const std::string& operand,
              DirectiveContext& context);

// Register in InitializeDirectiveRegistry:
directive_registry_[directives::OR] = HandleOr;
```

---

**Contract Approved:** ✓ Orchestrator 2026-02-11
