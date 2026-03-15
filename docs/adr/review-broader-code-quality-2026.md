# Architect Review: Broader Code Quality Refactoring Opportunities

**Date:** 2026-03-14  
**Reviewer:** Architect Agent  
**Scope:** Full codebase survey beyond `src/core/` and `src/syntax/`  
**Build status at review:** ✅ Clean (0 warnings), 1928/1928 tests passing  
**Predecessor review:** `docs/adr/review-assembler-syntax-quality-2026.md`

---

## Executive Summary

The codebase has made substantial progress through the ADR-005 remediation and
prior god-class refactoring work.  This review surveys **all remaining
refactoring opportunities** across the full source tree, identifying patterns
that were out of scope for the previous `assembler.cpp`/`syntax/` focused
review.

The biggest concentration of technical debt is now in `src/cpu/cpu_6809.cpp`
(2043 lines, 255 identical indexed-addressing dispatch blocks), followed by
pervasive duplication of `Trim` utility functions (7 implementations when one
canonical `xasm::util::Trim` already exists).  These two issues account for
roughly 1600 lines of unnecessary code.

**Severity overview:**

| Severity | Count | Description |
|----------|-------|-------------|
| 🔴 High | 2 | Structural duplication blocking maintainability |
| 🟠 Medium | 5 | Inconsistent use of shared utilities |
| 🟡 Low | 4 | Minor cleanup / dead code |

---

## High-Severity Issues

### H1 — `cpu_6809.cpp`: Massive instruction-encoder repetition (2043 lines)

**File:** `src/cpu/cpu_6809.cpp`  
**Lines:** ~107–1650  
**Pattern:** 94 separate `Encode*()` functions, each independently handling
the same four addressing modes (Immediate, Direct, Extended, Indexed).  The
Indexed-addressing block is byte-for-byte identical in every function — only
the opcode constants differ.

**Measurement:**

```
grep -c "case AddressingMode6809::Indexed"  src/cpu/cpu_6809.cpp  → 255
grep -c "if (mnemonic =="                   src/cpu/cpu_6809.cpp  →  93
```

**Root cause:** Each per-instruction `Encode*` function re-implements the
mode-dispatch table from scratch rather than delegating to a shared helper.

**Proposed refactoring (table-driven):**

```cpp
// 1. Represent each instruction as a set of four opcode constants:
struct OpcodeSet8 {
    uint8_t imm;   ///< Immediate mode opcode
    uint8_t dir;   ///< Direct page mode opcode
    uint8_t ext;   ///< Extended (absolute) mode opcode
    uint8_t idx;   ///< Indexed base opcode (post-byte appended separately)
};

// 2. One generic encoder dispatched by the set:
std::vector<uint8_t> Cpu6809::EncodeWithModes(
    uint32_t operand,
    AddressingMode6809 mode,
    OpcodeSet8 opcodes) const;

// 3. Each public method becomes a one-liner:
std::vector<uint8_t> Cpu6809::EncodeLDA(uint32_t operand,
                                         AddressingMode6809 mode) const {
    return EncodeWithModes(operand, mode,
                           {Opcodes6809::LDA_IMM, Opcodes6809::LDA_DIR,
                            Opcodes6809::LDA_EXT, Opcodes6809::LDA_IDX});
}
```

**Estimated reduction:** from 2043 lines to ≈300 lines (−85%).  
**Risk:** Low — behaviour is purely data-driven.  Tests already cover the
indexed path exhaustively (`test_cpu6809_indexed.cpp`).

**Also in scope:** `EncodeInstruction()` at line ~1680 uses a 93-deep
`if (mnemonic ==)` chain.  This should become an
`std::unordered_map<std::string, EncodeFunc>` dispatch table — consistent
with the precedent already set in `cpu_6502.cpp` (branch opcode lookup table,
task `cpu_6502.cpp branch opcode lookup table refactoring`).

---

### H2 — `cpu_6809.cpp`: Private `ToBigEndian()` shadowing `encoding_utils.h`

**File:** `src/cpu/cpu_6809.cpp` lines 32–38  
**Issue:** A private member function `Cpu6809::ToBigEndian(uint16_t)` exists
and is used throughout the file, despite `include/xasm++/cpu/encoding_utils.h`
already providing `xasm::encoding::ToBigEndian16()` for exactly this purpose.
Both functions produce identical output.

```cpp
// cpu_6809.cpp (private — should be removed)
std::vector<uint8_t> Cpu6809::ToBigEndian(uint16_t value) const {
    return {static_cast<uint8_t>((value >> 8) & 0xFF),
            static_cast<uint8_t>(value & 0xFF)};
}

// encoding_utils.h (canonical — already present)
constexpr std::array<uint8_t, 2> ToBigEndian16(uint16_t value) {
    return {static_cast<uint8_t>((value >> 8) & 0xFF),
            static_cast<uint8_t>(value & 0xFF)};
}
```

**Fix:** Add `#include "xasm++/cpu/encoding_utils.h"` to `cpu_6809.cpp`, 
replace all calls to `ToBigEndian()` with `encoding::ToBigEndian16()`,
and delete the private member function and its declaration in `cpu_6809.h`.

---

## Medium-Severity Issues

### M1 — Six duplicate `Trim()` implementations

**Pattern:** `xasm::util::Trim()` is the canonical implementation in
`include/xasm++/util/string_utils.h` / `src/utils/string_utils.cpp`.  Despite
this, six additional file-local `Trim` functions exist, each identical to the
canonical version:

| File | Line | Type |
|------|------|------|
| `src/syntax/core_directive_handlers.cpp` | 30 | anonymous namespace function |
| `src/syntax/edtasm_simple_directive_handlers.cpp` | 37 | anonymous namespace function |
| `src/syntax/scmasm_directive_handlers.cpp` | 45 | anonymous namespace function |
| `src/syntax/scmasm_syntax.cpp` | 711 | `static TrimRight()` (partial) |
| `src/syntax/simple_directive_handlers.cpp` | 31 | `static Trim()` |
| `src/syntax/simple_syntax.cpp` | 16 | `static Trim()` |

`src/cpu/cpu_6502.cpp` also has **two** inline `trim` lambdas at lines 2125
and 2542 — duplicate within the same file.

**Fix:**
1. Add `#include "xasm++/util/string_utils.h"` to each offending file.
2. Replace each local `Trim(s)` call with `xasm::util::Trim(s)`.
3. Delete the local implementations.
4. For `scmasm_syntax.cpp::TrimRight()` — if not replaceable by
   `xasm::util::Trim`, add `TrimRight()` to `string_utils.h`.
5. For `cpu_6502.cpp` — replace both trim lambdas at lines 2125 and 2542
   with calls to `xasm::util::Trim`.

**Estimated reduction:** ~60 lines of duplicated boilerplate.

---

### M2 — `directive_error_utils.h` is included but unused

**File:** `src/syntax/scmasm_directive_handlers.cpp`  
**Line 12:** `#include "xasm++/directives/directive_error_utils.h"`  
**Issue:** The header is included but `directive_utils::` is never referenced
in this file (confirmed by `grep`).  The file throws 19 hand-rolled
`std::runtime_error` / `std::invalid_argument` exceptions.

**Fix (two options, choose one):**
- **Option A (quick):** Remove the unused `#include`.  
- **Option B (better):** Adopt `directive_error_utils.h` — replace the 19
  hand-rolled `throw` statements with calls to `directive_utils::Throw*`
  helpers.  This is what `merlin_directive_handlers.cpp` already does
  (`using namespace xasm::directive_utils;`).

This should be tracked as a follow-up improvement regardless.

---

### M3 — `simple_syntax.cpp` / `simple_directive_handlers.cpp` not using shared utilities

**Files:** `src/syntax/simple_syntax.cpp`, `src/syntax/simple_directive_handlers.cpp`  
**Issue:** Neither file includes `string_utils.h` or `parse_utils.h`.  Both
define their own `static Trim()` and `ToUpper()` helpers.

This is likely an oversight from when `simple_syntax` was written before the
shared utilities existed, but it now represents an orphaned island of code that
is out of step with the rest of the codebase.

**Fix:**
- Add `#include "xasm++/util/string_utils.h"` to both files.
- Replace local `Trim()` with `xasm::util::Trim()`.
- Replace local `ToUpper()` with `xasm::util::ToUpper()`.
- Delete the local helper functions.

---

### M4 — `Cpu6502::GetName()` duplicates `Cpu6502::GetCpuFamily()`

**Files:** `include/xasm++/cpu/cpu_6502.h` (lines 126, 136),  
           `include/xasm++/cpu/cpu_6809.h` (lines 108, 118)

Both classes implement two methods returning identical strings:

```cpp
// cpu_6502.h
std::string GetCpuFamily() const override { return "6502"; }  // line 126
std::string GetName()      const          { return "6502"; }  // line 136
```

`GetName()` is **not** declared in the `CpuPlugin` interface.  It appears to
be a leftover from before `GetCpuFamily()` was standardized.

**Fix:**
1. Audit call sites of `GetName()` vs `GetCpuFamily()`.
2. Migrate all callers of `GetName()` to `GetCpuFamily()`.
3. Remove `GetName()` from both `Cpu6502` and `Cpu6809`.

---

### M5 — No common syntax parser interface

**Files:** All parsers in `include/xasm++/syntax/*.h`

Currently each syntax parser class (`MerlinSyntaxParser`, `ScmasmSyntaxParser`,
`EdtasmSyntaxParser`, `FlexAsmSyntax`, `EdtasmM80PlusPlusSyntaxParser`,
`SimpleSyntaxParser`) is a standalone concrete class with no shared base class
or interface.

`src/main.cpp` lines 147–270 contain a large `if/else if` ladder that
instantiates the correct parser variant and repeats nearly identical assembly
pipeline setup for each.  This makes it hard to add new syntaxes without
modifying `main.cpp`.

**Proposed interface (non-breaking — no immediate action required):**

```cpp
// include/xasm++/syntax/syntax_plugin.h  (new file)
class SyntaxPlugin {
public:
    virtual ~SyntaxPlugin() = default;
    virtual void SetCpu(CpuPlugin* cpu) = 0;
    virtual void SetIncludePaths(const std::vector<std::string>& paths) = 0;
    virtual void Parse(const std::string& source,
                       Section& section,
                       ConcreteSymbolTable& symbols) = 0;
    virtual std::string GetName() const = 0;
};
```

Once the interface exists, `main.cpp` can reduce to a factory lookup:
```cpp
auto parser = syntax_factory.Create(opts.syntax);
parser->SetCpu(cpu);
parser->Parse(source, section, symbols);
```

**Effort:** Medium.  Requires adding the interface and adapting each parser —
but no functional changes.

---

## Low-Severity Issues

### L1 — TODO comments tracking unimplemented directives

Seven `// TODO: Implement...` comments remain in directive handlers:

| File | Line | TODO |
|------|------|------|
| `src/syntax/edtasm_directive_handlers.cpp` | 251 | DD directive (DWord) |
| `src/syntax/edtasm_directive_handlers.cpp` | 501 | File inclusion |
| `src/syntax/edtasm_directive_handlers.cpp` | 646, 654 | Macro listing control |
| `src/syntax/scmasm_directive_handlers.cpp` | 1418 | Store current address |
| (others in scmasm) | various | additional stubs |

**Fix:** Convert each TODO to a tracked Beads task.  Delete the comments once
tickets exist (or clearly mark as `// Not planned:` for deferred items).

---

### L2 — `scmasm_syntax.cpp`: inline `static` set declarations inside functions

**File:** `src/syntax/scmasm_syntax.cpp` lines 645, 916, 1274  
**Pattern:**
```cpp
static const std::unordered_set<std::string> kDataEmittingDirectives = {"DB", ...};
```
These are `static` sets declared at function scope, but they are conceptually
module-level constants.  Promoting them to file-scope `static const` or to
named constants in a companion header would improve readability and make them
discoverable for tests.

---

### L3 — `cpu_65816.cpp`: 23 identical guard checks

**File:** `src/cpu/cpu_65816.cpp`  
**Count:** 23 occurrences of:
```cpp
if (cpu_mode_ != CpuMode::Cpu65816) {
    throw std::invalid_argument(...);
}
```

These are copy-pasted across every 65816-only function.  A single inline
helper would eliminate the repetition:

```cpp
inline void AssertCpu65816() const {
    if (cpu_mode_ != CpuMode::Cpu65816)
        throw std::invalid_argument("Instruction requires 65816 mode");
}
```

---

### L4 — Z80 bit-operation opcode arithmetic uses bare hex literals

**File:** `src/cpu/cpu_z80.cpp` lines 281–291  
**Code:**
```cpp
uint8_t opcode = 0x40 + (bit << 3) + reg;   // BIT
uint8_t opcode = 0xC0 + (bit << 3) + reg;   // SET
uint8_t opcode = 0x80 + (bit << 3) + reg;   // RES
```
The constants `0x40`, `0xC0`, `0x80` are the Z80 `CB`-prefix base opcodes for
BIT/SET/RES respectively.  They should be named constants in `opcodes_z80.h`
to document intent:

```cpp
// opcodes_z80.h additions
constexpr uint8_t kCbBitBase = 0x40;
constexpr uint8_t kCbSetBase = 0xC0;
constexpr uint8_t kCbResBase = 0x80;
```

---

## Recommended Actions (Priority Order)

### P1 — Consolidate `Trim()` duplicates (2–3 hours)

**Impact:** Highest bang-for-buck.  Removes ~60 lines of identical boilerplate
from 6 files, enforces DRY, and reduces future defect surface.

**Files to touch:**
- `src/syntax/core_directive_handlers.cpp` — add `string_utils.h` include
- `src/syntax/edtasm_simple_directive_handlers.cpp`
- `src/syntax/scmasm_directive_handlers.cpp`
- `src/syntax/simple_directive_handlers.cpp`
- `src/syntax/simple_syntax.cpp`
- `src/cpu/cpu_6502.cpp` (two inline lambdas at lines 2125, 2542)

Consider extending `string_utils.h` with `TrimRight()` to absorb the partial
variant in `scmasm_syntax.cpp`.

---

### P2 — Remove `Cpu6809::ToBigEndian()` and use `encoding_utils.h` (1 hour)

Simple mechanical substitution.  Prerequisite for P3.

**Files to touch:**
- `src/cpu/cpu_6809.cpp` — add `encoding_utils.h` include; replace all
  `ToBigEndian(v)` calls with `encoding::ToBigEndian16(v)` (note: return type
  changes from `std::vector` to `std::array` — insert pattern changes slightly)
- `include/xasm++/cpu/cpu_6809.h` — remove `ToBigEndian` declaration

---

### P3 — Refactor `cpu_6809.cpp` to table-driven encoding (1–2 days)

This is the largest structural improvement available.  Reduces the file from
2043 to ~300 lines while preserving all existing tests.

**Approach:**
1. Define `struct OpcodeSet8` and `struct OpcodeSet16` (for 16-bit instructions
   like LDD/LDX) in `opcodes_6809.h`.
2. Implement `EncodeWithModes8()` and `EncodeWithModes16()` as private helpers
   in `Cpu6809`.
3. Replace each `Encode*` body with a one-liner delegation (see H1 above).
4. Replace `EncodeInstruction()`'s 93-deep `if` chain with an
   `unordered_map<string, function<vector<uint8_t>(...)>>` dispatch table.

**Do not start P3 before P2** — `ToBigEndian` migration is a prerequisite.

---

### P4 — `AssertCpu65816()` helper in `cpu_65816.cpp` (30 minutes)

Trivial mechanical cleanup.  23 copy-pasted guard checks → 1 helper call each.

---

### P5 — Remove/adopt `directive_error_utils.h` in `scmasm_directive_handlers.cpp` (2–4 hours)

Standardizes error throwing across all directive handlers.  Optional but
improves consistency.

---

### P6 — Convert TODO comments to tracked tasks (30 minutes)

Each TODO becomes a Beads task.  The comment either points to the ticket ID or
is removed.

---

### P7 — `SyntaxPlugin` interface (3–5 days, architect decision required)

This is the largest architectural change and should be evaluated by the
Architect before assigning.  It is a prerequisite for making `main.cpp`
extensible.  Not recommended for immediate implementation — raise an ADR first.

---

## Items Already Resolved (not regression risks)

The following were flagged in earlier reviews and are confirmed resolved:

- ✅ ADR-005 V1–V6: All expression parser violations resolved
- ✅ Branch opcode if-chain in `cpu_6502.cpp` → `unordered_map` (task completed)
- ✅ `ParseNumericLiteral` duplication in `merlin_syntax.cpp` (task completed)
- ✅ Lambda anti-pattern in directive handlers (resolved in Phase 6b/6c)
- ✅ `ParseExpression` local static in `assembler.cpp` → shared `ExpressionParser`
- ✅ `expression_utils.h` shared `ParseAndEvaluate` utilities (15 call sites)

---

## Test Coverage Notes

Current test count: **1928 tests** (up from 1912 at last review).  
Coverage observations:

- `cpu_6809.cpp` is well covered by `test_cpu6809.cpp` (1167 lines) and
  `test_cpu6809_indexed.cpp` (766 lines).  Refactoring H1 is safe.
- `cpu_65816.cpp` guard-check refactoring (L3) is covered by 65816 tests.
- `simple_syntax.cpp` utility cleanup (M3) is low-risk as SimpleSyntax is
  the reference implementation used across many tests.

No new tests are required for P1 through P4 — all are mechanical substitutions
of equivalent logic.

---

## Summary Table

| ID | Severity | File | Issue | Effort | Action |
|----|----------|------|-------|--------|--------|
| H1 | 🔴 High | `cpu_6809.cpp` | 94 Encode* functions × identical mode dispatch | 2 days | P3 |
| H2 | 🔴 High | `cpu_6809.cpp` | Private `ToBigEndian()` shadows `encoding_utils.h` | 1 hr | P2 |
| M1 | 🟠 Medium | 6 files | Duplicate `Trim()` implementations | 3 hrs | P1 |
| M2 | 🟠 Medium | `scmasm_directive_handlers.cpp` | Unused include + 40 raw throws | 4 hrs | P5 |
| M3 | 🟠 Medium | `simple_syntax.cpp`, `simple_directive_handlers.cpp` | Not using shared utilities | 1 hr | P1 |
| M4 | 🟠 Medium | `cpu_6502.h`, `cpu_6809.h` | `GetName()` duplicates `GetCpuFamily()` | 1 hr | — |
| M5 | 🟠 Medium | all syntax parsers | No common `SyntaxPlugin` interface | 5 days | P7 |
| L1 | 🟡 Low | edtasm/scmasm handlers | 7 TODO comments | 30 min | P6 |
| L2 | 🟡 Low | `scmasm_syntax.cpp` | Inline static sets in functions | 30 min | — |
| L3 | 🟡 Low | `cpu_65816.cpp` | 23 identical guard checks | 30 min | P4 |
| L4 | 🟡 Low | `cpu_z80.cpp` | Bare hex literals in bit-op math | 15 min | — |
