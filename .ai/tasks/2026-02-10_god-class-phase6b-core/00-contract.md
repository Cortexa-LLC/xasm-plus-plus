# Task Contract: God-Class Phase 6b - Core Handler Extraction

**Task ID:** 2026-02-10_god-class-phase6b-core
**Beads Task:** xasm++-uqyd
**Created:** 2026-02-10
**Requestor:** Orchestrator (from decomposition of xasm++-3mwi)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract 5-10 common directive handlers (ORG, EQU, DB, DW, DS) from parser classes into free functions as proof-of-concept. Register with DirectiveRegistry. Eliminate magic numbers and magic strings using named constants.

### Background and Context

Phase 6a unified all directive signatures to use DirectiveContext. Now we can extract handlers from parser classes into standalone functions, following the pattern established in DirectiveRegistry.

This phase serves as proof-of-concept for Phase 6c's full extraction.

**CRITICAL REQUIREMENT:** Eliminate magic numbers and magic strings:
- Directive names must use string constants (not "ORG", "EQU" literals)
- Opcodes must use named constants from opcodes_*.h files
- Mnemonics must use string constants
- No hardcoded hex values (0x4C, 0x20, etc.) - use named constants

### Current State

- Phase 6a complete: All parsers use DirectiveContext signature
- Directive handlers are lambda functions inside parser classes
- ~75 total handlers across 3 parsers (Merlin, SCMASM, EDTASM-M80++)
- Some magic strings and numbers exist in directive implementations

### Desired State

- 5-10 common directives extracted as free functions
- Functions registered with DirectiveRegistry
- All magic numbers replaced with named constants
- All magic strings replaced with string constants
- Pattern proven for Phase 6c full extraction

---

## Success Criteria

```
✓ 5-10 common directives extracted (ORG, EQU, DB, DW, DS minimum)
✓ All extracted handlers use free function pattern
✓ All handlers registered with DirectiveRegistry
✓ Zero magic numbers in extracted code
✓ Zero magic strings in extracted code
✓ All tests passing (97%+)
✓ Proof-of-concept validates pattern for Phase 6c
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Extract ORG directive as free function
□ Extract EQU directive as free function
□ Extract DB/DEFB/BYTE directive as free function
□ Extract DW/DEFW/WORD directive as free function
□ Extract DS/DEFS/BLOCK directive as free function
□ Register all extracted handlers with DirectiveRegistry
□ Verify all extracted directives work identically to before
□ Start with Merlin parser (31 directives total, extract 5-10)
```

### Code Quality Requirements (CRITICAL)
```
□ NO magic numbers - use named constants
  Example: Replace 0x4C with opcodes::JMP_ABSOLUTE

□ NO magic strings - use string constants
  Example: Replace "ORG" with directives::ORG_NAME

□ Directive names: Use constants from directive_constants.h

□ Opcodes: Use constants from opcodes_z80.h, opcodes_6502.h, opcodes_6809.h

□ Mnemonics: Use string constants from mnemonic_constants.h (create if needed)

□ Define constants in appropriate header files:
  - Directive names → include/xasm++/directives/directive_constants.h
  - CPU opcodes → Already exist in include/xasm++/cpu/opcodes_*.h
  - Mnemonics → include/xasm++/cpu/mnemonic_constants.h (if needed)
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

**Complexity:** Medium

**Files:** 5-8
- 3 parser files (extract handlers)
- 1-2 new handler files (free functions)
- 1-2 constant definition headers
- 1-2 test files

**Tokens:** 18K → ✅ SAFE

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
- High cohesion - extracting related directive handlers
- Single concern - proof-of-concept for handler extraction pattern
- Already minimal viable batch for meaningful validation

**Contingency for token limits:**
- If token limit hit, will extract fewer directives (3-4 instead of 5-10)

**Estimated tokens:** ~7 × 3000 = 21,000 tokens
**Status:** Within 25K-32K limit? Yes ✅

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
7 × 3,000 tokens = 21,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 21,000 tokens → APPROACHING LIMIT ⚠️
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [x] YES - Batch size 7 files AND token budget 21K AND WIP 1

---

## Dependencies

**Depends On:** xasm++-98cb (Phase 6a - Signature Unification) - ✓ COMPLETE
**Blocks:** xasm++-ci14 (Phase 6c - Full Handler Extraction)
**Reference:** `.ai/tasks/2026-02-09_god-class-phase6-directive/ESCALATION.md`

---

## Resources and References

### Existing Constant Files
```
include/xasm++/cpu/opcodes_z80.h - Z80 opcode constants (already exists)
include/xasm++/cpu/opcodes_6502.h - 6502 opcode constants (already exists)
include/xasm++/cpu/opcodes_6809.h - 6809 opcode constants (already exists)
```

### Files to Create
```
include/xasm++/directives/directive_constants.h - Directive name constants
include/xasm++/cpu/mnemonic_constants.h - Mnemonic string constants (if needed)
```

### Pattern Reference
```
include/xasm++/syntax/directive_registry.h - DirectiveHandler signature
src/syntax/directive_registry.cpp - Registry implementation
```

---

## Special Instructions

**Magic Number/String Elimination Strategy:**

1. **Audit Phase:** Scan extracted handlers for:
   - Literal strings ("ORG", "EQU", "JMP", etc.)
   - Hex constants (0x00, 0xFF, etc.)
   - Decimal constants with semantic meaning

2. **Constant Definition Phase:**
   - Create directive_constants.h with directive names
   - Reuse existing opcodes_*.h files
   - Create mnemonic_constants.h if needed

3. **Replacement Phase:**
   - Replace all magic values with named constants
   - Use constexpr for compile-time constants
   - Group related constants in namespaces

**Example Pattern:**
```cpp
// Before (magic strings and numbers):
if (directive == "ORG") {
    section.SetOrigin(0x8000);
    output.WriteByte(0x4C);  // JMP opcode
}

// After (named constants):
namespace directives {
    constexpr const char* ORG = "ORG";
    constexpr uint16_t DEFAULT_ORIGIN = 0x8000;
}

if (directive == directives::ORG) {
    section.SetOrigin(directives::DEFAULT_ORIGIN);
    output.WriteByte(opcodes::m6502::JMP_ABSOLUTE);
}
```

---

**Contract Approved:** ✓ Orchestrator 2026-02-10
