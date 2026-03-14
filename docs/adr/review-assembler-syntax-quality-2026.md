# Architect Review: Assembler and Syntax Parsing Code Quality

**Date:** 2026-03-14  
**Reviewer:** Architect Agent  
**Scope:** `src/core/assembler.cpp`, `src/common/expression_parser.cpp`,  
           `src/syntax/` directory, ADR-005 compliance  
**Build status at review:** ✅ Clean (0 warnings), 1912/1912 tests passing

---

## Executive Summary

The codebase is in **substantially improved health** compared to the ADR-005
incident. Violations V1, V2, V3, V4, V5, and V6 have been successfully
removed from shared code. However, two structural issues require engineering
attention:

1. **V7 and V8 (open ADR-005 violations)** — Syntax-specific identifier and
   expression constructs remain in `ExpressionParser::ParsePrimary` and
   `ExpressionParser::ParseIdentifier` without feature-flag gating.

2. **`assembler.cpp::ParseExpression` god-function** — A 307-line
   hand-rolled recursive descent parser remains in `assembler.cpp` with
   SCMASM-specific logic embedded. This is the same anti-pattern that caused
   the ADR-005 incident, just at a different call site.

There is also one **code smell** (duplicate SCMASM uppercase fallback logic)
and one **style inconsistency** (mixed comment register for ADR-005 status
in assembler.cpp).

---

## Finding 1 — V7: Z80 `[expr]` bracket notation in shared `ExpressionParser`

**File:** `src/common/expression_parser.cpp`, lines 301–315  
**Severity:** Medium (boundary violation per ADR-005)

### Code

```cpp
// Bracketed expression (Z80 alternative to parentheses)
if (Peek() == '[') {
  Consume();
  auto expr = ParseLogicalOr();
  SkipWhitespace();
  if (Peek() != ']') {
    throw std::runtime_error("Expected closing bracket");
  }
  Consume();
  return expr;
}
```

### Problem

`[…]` as an alternative grouping operator is a Z80-specific syntax feature. No
other supported syntax uses `[` as an expression grouping delimiter. This code
is active for every syntax mode, including SCMASM and Merlin, where `[` has
no defined meaning (or worse, could shadow future use).

### Risk

- Merlin's `]var` (V8) depends on `]` being consumed as part of an identifier.
  If `[` appears before `]` in a Merlin expression (e.g., `[FOO]`), V7 will
  evaluate it as a grouped expression, not an error. This masks malformed input.
- SCMASM does not define `[expr]` grouping. Passing a SCMASM expression through
  `ExpressionParser` that happens to contain `[` (e.g., in an array index
  context) will silently succeed with wrong semantics.

### Fix (ADR-005 Step 6, priority: LOW)

Add an optional feature flag to `ExpressionParser`:

```cpp
// In expression_parser.h
struct ParserFeatures {
  bool allow_bracket_grouping = false;  // Z80: [expr] as alternative parens
  bool allow_merlin_var_prefix = false; // Merlin: ]var identifiers
};

explicit ExpressionParser(const SymbolTable *symbols,
                          const INumberParser *number_parser = nullptr,
                          ParserFeatures features = {});
```

The `EdtasmM80PlusPlusSyntaxParser::ParseExpression` then passes
`{.allow_bracket_grouping = true}`. All other callers use the default (off).

---

## Finding 2 — V8: Merlin `]var` identifier prefix in shared `ExpressionParser`

**File:** `src/common/expression_parser.cpp`, lines 607–625 and 416–418  
**Severity:** Medium (boundary violation per ADR-005)

### Code

```cpp
// In ParseIdentifier():
// Identifier starts with letter, underscore, period, $, ?, or ] (Merlin vars)
if (!std::isalpha(Peek()) && Peek() != '_' && Peek() != '.' &&
    Peek() != '$' && Peek() != '?' && Peek() != ']') {
  throw std::runtime_error("Expected identifier");
}
```

```cpp
// In ParsePrimary():
// ']' prefix is valid for Merlin ]variable labels (DUM-block variables).
```

### Problem

Allowing `]` as an identifier-start character is a Merlin-specific convention
for DUM-block (`]`-prefixed) variables. No other syntax uses this convention.
Enabling it universally:

- Prevents `ExpressionParser` from reporting a useful error when `]` appears
  unexpectedly in SCMASM, EDTASM, or Flex sources.
- Means any unmatched `]` in any syntax mode will silently be treated as the
  start of an identifier, potentially creating a phantom symbol reference.

### Risk

Low in current usage (SCMASM/Flex parsers don't produce bare `]` in expression
strings), but the defence-in-depth is absent. A future change that inadvertently
feeds an ill-formed expression string could produce an obscure symbol-not-found
error instead of a parse error.

### Fix (ADR-005 Step 7, same `ParserFeatures` struct as V7)

Gate the `]` identifier start character and the `]`-prefix comment in
`ParsePrimary` behind `features_.allow_merlin_var_prefix`. Merlin's
`ParseExpression` passes `{.allow_merlin_var_prefix = true}`.

---

## Finding 3 — `assembler.cpp::ParseExpression` duplicates logic from `ExpressionParser`

**File:** `src/core/assembler.cpp`, lines 28–333  
**Severity:** High (structural code smell, not an ADR-005 violation per se)

### Description

`ParseExpression` is a 307-line static free function in `assembler.cpp`. It is
a bespoke hand-rolled recursive descent parser. It handles:

- Character literals (`"x"`, `'x'`)
- Hex (`$ABCD`), binary (`%1010`), decimal
- Unary `<`, `>`, `~`
- Binary `+`, `-`, `*`, `/`, `^`
- Parentheses grouping
- Symbol references (with SCMASM uppercase fallback)
- XOR and multiply at higher precedence than add/subtract
- A 307-line flat decision tree mixing all of the above

This function is called **26 times** throughout `assembler.cpp`, making it
the primary expression engine used during the instruction-encoding phase.

### Problem 1: Parallel implementation

`ExpressionParser` (`src/common/expression_parser.cpp`) is the *intended*
shared expression engine (624 lines, proper recursive descent precedence
tree). `ParseExpression` in `assembler.cpp` is a competing implementation.
Both are in production simultaneously:

- `ParseExpression` is called from the assembler core instruction loop.
- `ExpressionParser::Parse` is called from directive handlers and syntax parsers.

There is **no guarantee they produce the same results** for the same input.
Subtle precedence differences between the two parsers are latent bugs.

### Problem 2: SCMASM uppercase fallback leaking into the general path

```cpp
// Symbol reference - try original case first, then uppercase fallback.
// SCMASM normalizes all symbols to UPPERCASE at definition time, so a
// mixed-case reference like "TmpPtr2" must resolve to "TMPPTR2".
```

`ParseExpression` in `assembler.cpp` applies a SCMASM-specific case-folding
fallback to *every* symbol lookup. This is ADR-005 V1-class syntax bleeding:
a SCMASM behaviour baked into a shared path.

`ExpressionParser` (correctly) does NOT apply this fallback — it creates a
`SymbolExpr(ident)` and leaves resolution to evaluation time. The two parsers
have inconsistent symbol resolution semantics.

**This same pattern caused the ADR-005 incident.** The comment naming SCMASM
in the assembler's shared ParseExpression is a direct red flag.

### Fix (Medium-term)

1. Migrate `assembler.cpp`'s call sites to `ExpressionParser::Parse`.
2. Remove the SCMASM uppercase fallback from `assembler.cpp::ParseExpression`
   (or move it to a SCMASM-specific symbol-resolution hook in `SymbolTable`).
3. Once all callers of `assembler.cpp::ParseExpression` have been migrated,
   delete it.

**Suggested phasing:**
- Phase A: Add the SCMASM case-fold to `ConcreteSymbolTable::Lookup` behind
  a flag (or as the SCMASM symbol table subclass), not in the parser.
- Phase B: Replace the 26 call sites of `ParseExpression` in `assembler.cpp`
  with `ExpressionParser::Parse`, running the full regression suite after each.
- Phase C: Delete `ParseExpression` from `assembler.cpp`.

---

## Finding 4 — Comment-register inconsistency in `assembler.cpp::ParseExpression`

**File:** `src/core/assembler.cpp`, lines 88, 217, 319, 602  
**Severity:** Low (readability / maintenance)

Several comments in `ParseExpression` reference syntax-specific names in ways
that are now misleading:

| Line | Comment | Status |
|------|---------|--------|
| 88 | `Apple II Merlin \|0x80) are applied by the syntax parser, not here.` | ✅ Correct — explains V4 fix |
| 217 | `'/' at position 0 is the SCMASM unary high-byte operator (already handled)` | ✅ Correct — explains V3 removal |
| 319–320 | `SCMASM normalizes all symbols to UPPERCASE at definition time` | ⚠️ Problem — explains retained V1-class issue |
| 602–603 | `case first, then uppercase fallback since SCMASM normalizes` | ⚠️ Problem — same |

The lines 319 and 602 comments document the *retained* SCMASM violation. They
make the problem visible (good), but a CI linter (as specified in ADR-005
Section "Preventing Regression") would flag them as violations because they are
not excluded by the `grep -v "ADR-005"` pattern.

**Recommendation:** Add `// ADR-005: pending V1 migration` annotation to these
lines so the CI lint script continues to surface them as known-open work items
rather than hidden violations.

---

## Finding 5 — `ExpressionParser` instantiation pattern is safe but unguarded

**File:** Multiple syntax parsers  
**Severity:** Low (future-proofing concern)

`ExpressionParser` is instantiated in:

- `merlin_syntax.cpp:181` — with `&empty_symbols` (for number parsing only)
- `merlin_syntax.cpp` (ParseExpression) — with `&symbols`
- `scmasm_syntax.cpp:1706` — with `&symbols, &scmasm_number_parser_`
- `edtasm_m80_plusplus_syntax.cpp:918` — with `&symbols, &z80_number_parser_`
- `core_directive_handlers.cpp:119, 143, 198` — with `&symbols`

All callers use the default constructor (no feature flags), meaning V7 and V8
are **active for every caller** including SCMASM and directive handlers.

When V7/V8 are gated behind `ParserFeatures`, only `edtasm_m80_plusplus_syntax`
should set `allow_bracket_grouping = true`, and only `merlin_syntax` should set
`allow_merlin_var_prefix = true`. This is a mechanical change once the struct
exists.

---

## ADR-005 Compliance Summary

| ID | Description | Status |
|----|-------------|--------|
| V1 | Merlin inline string comment strip in `assembler.cpp::ParseExpression` | ✅ Removed (→ `merlin_syntax.cpp`) |
| V2 | Merlin `.Inc`/`.Dec` suffixes in `assembler.cpp::ParseExpression` | ✅ Removed (→ `merlin_syntax.cpp`) |
| V3 | SCMASM `/` high-byte alias in `ExpressionParser::ParseAddSub` | ✅ Removed |
| V4 | Merlin `\| 0x80` char encoding in `assembler.cpp::ParseExpression` | ✅ Removed (the incident) |
| V5 | SCMASM `=` vs `==` in `ExpressionParser::ParseComparison` | ✅ Removed (→ `scmasm_syntax.cpp`) |
| V6 | SCMASM `/` in `ExpressionParser::ParseMultiplicative` | ✅ Removed |
| V7 | Z80 `[expr]` bracket grouping in `ExpressionParser::ParsePrimary` | ⚠️ Open (LOW priority) |
| V8 | Merlin `]var` identifier prefix in `ExpressionParser::ParseIdentifier` | ⚠️ Open (LOW priority) |
| — | SCMASM uppercase fallback in `assembler.cpp::ParseExpression` | ⚠️ Open (MEDIUM priority, Finding 3) |

---

## Recommended Actions (Priority Order)

### P1 — Document remaining violations with ADR-005 annotations (1 hour)

Add `// ADR-005: V7 pending` and `// ADR-005: V8 pending` to the relevant
lines in `expression_parser.cpp`, and `// ADR-005: pending V1 migration`
to lines 319 and 602 of `assembler.cpp`. This makes the CI lint script
suppress them correctly while preserving visibility.

### P2 — Add `ParserFeatures` struct to `ExpressionParser` (half-day)

Introduce the feature-flag struct and gate V7 and V8 behind it. Update all
instantiation sites. Run full regression suite. This closes V7 and V8
cleanly without requiring Z80 tokeniser refactoring.

### P3 — Remove SCMASM uppercase fallback from `assembler.cpp::ParseExpression` (1 day)

Move the case-fold to `ConcreteSymbolTable::Lookup` as an optional mode,
or to a SCMASM-specific `SymbolTable` subclass. Validate against the full
SCMASM test corpus. This closes the last ADR-005 class violation in
`assembler.cpp`.

### P4 — Migrate assembler.cpp instruction loop to `ExpressionParser` (sprint)

Replace 26 call sites of `ParseExpression` in `assembler.cpp` with
`ExpressionParser::Parse`. This eliminates the parallel implementation
(Finding 3) and reduces `assembler.cpp` by ~350 LOC. High risk — requires
careful per-call-site verification and a reference binary diff test.

---

## What Is Working Well

- **V1–V6 removal is clean and complete.** The fixes follow ADR-005 precisely:
  syntax-specific logic moved to syntax parsers, shared code left clean.
- **`merlin_syntax.cpp::ParseExpression` is well-structured.** It
  pre-processes Merlin-specific constructs then delegates to `ExpressionParser`.
- **`ExpressionParser` INumberParser abstraction is correct.** It allows
  syntax-specific number formats (Z80's `0FFH`, SCMASM's `'B` binary) to be
  injected without polluting the shared grammar.
- **Test coverage is strong.** 1912 tests covering expression parsing, branch
  relaxation, char literals, forward references, and more.
- **Build is clean.** Zero warnings on all targets.

---

## Files Referenced

| File | Lines of concern |
|------|-----------------|
| `src/core/assembler.cpp` | 28–333 (ParseExpression), 319, 602 (SCMASM fallback) |
| `src/common/expression_parser.cpp` | 301–315 (V7), 607–625 (V8) |
| `include/xasm++/common/expression_parser.h` | 107–108 (constructor, no feature flags) |
| `src/syntax/merlin_syntax.cpp` | 191+ (V1/V2 correct placement) |
| `src/syntax/scmasm_syntax.cpp` | 1700+ (V5 correct placement) |
| `src/syntax/edtasm_m80_plusplus_syntax.cpp` | 918 (sole Z80 bracket consumer) |
| `docs/adr/005-syntax-core-boundary.md` | V7/V8 step definitions |
