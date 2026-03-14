# ADR-005: Syntax/Core Boundary — Prohibiting Syntax-Specific Logic in Shared Code

**Status:** Proposed  
**Date:** 2026-03-13  
**Deciders:** Architect Agent, pending engineering review  
**Context Source:** Incident post-mortem (commit 0302137) + codebase audit  
**Task:** xasm-4kd

---

## Context

### The Incident

Commit **0302137** added Merlin-style character literal encoding (`| 0x80`) to
`assembler.cpp::ParseExpression` — a static free function shared by **all** syntax
modes. This immediately corrupted SCMASM output: a regression run that previously
produced **83 identical binaries** dropped to **8** (a 90% regression).

The fix (commit d538f83) moved the correct behaviour into
`MerlinSyntaxParser::ParseExpression`, but the broken code in the shared function
remained. The regression was caught only because a large automated validation corpus
exists; without it, this class of defect would ship silently.

### Why It Keeps Happening

There is no explicit architectural rule that says "shared code must not reference any
single syntax". Engineers fixing a Merlin-specific bug look for the closest code that
touches expressions and patch it there — often the shared `ParseExpression` or
`ExpressionParser`, because those are where expression evaluation already occurs.
Without a documented boundary, every bug fix is one wrong edit away from cross-syntax
contamination.

---

## Audit: Violations at Time of Review (2026-03-13)

The following syntax-specific constructs were found in shared/core code:

### File: `src/core/assembler.cpp` — function `ParseExpression`

| # | Lines | What | Syntax Owner | Harm |
|---|-------|------|--------------|------|
| V1 | 31–50 | Strip Merlin inline string comments (`99 "stabbed"` → `99`) | Merlin | Corrupts any non-Merlin expression containing a quoted string suffix |
| V2 | 53–67 | Merlin `.Inc`/`.Dec` symbol suffixes (`LABEL.Inc` → `LABEL+1`) | Merlin | Incorrectly reinterprets `.inc`/`.dec` sub-fields in SCMASM/EDTASM identifiers |
| V3 | 122–140 | SCMASM `/expr` high-byte operator (`/ADDR` → high byte) | SCMASM | Intercepts the division operator `/` for Merlin/EDTASM/Z80 expressions |
| **V4** | **142–175** | **Merlin char literal `\| 0x80`** (`"A"` → `$C1`) | **Merlin** | **THE INCIDENT — caused 83→8 identical SCMASM binary regression** |

### File: `src/common/expression_parser.cpp` — `ExpressionParser` class

| # | Location | What | Syntax Owner | Harm |
|---|----------|------|--------------|------|
| V5 | `ParseRelational` ~l.109 | SCMASM single `=` as equality operator | SCMASM | Interprets assignment `=` as equality comparison in Merlin/EDTASM contexts |
| V6 | `ParseUnary` ~l.286–300 | SCMASM `/expr` high-byte operator | SCMASM | Same as V3; duplicate in the shared expression class |
| V7 | `ParsePrimary` ~l.317 | Z80 `[expr]` bracket notation as alternate parens | Z80 | Overrides bracket semantics for all syntaxes (Merlin uses `]` as var prefix) |
| V8 | `ParsePrimary` ~l.432, l.623 | Merlin `]var` identifier prefix (DUM-block vars) | Merlin | Allows `]` to begin identifiers in all syntaxes, breaking downstream parsers |

**Total: 8 violations across 2 shared files.**

---

## Decision

### Rule 1 — The Syntax/Core Boundary

> **Shared code (`src/core/`, `src/common/`) MUST NOT contain any logic whose
> correctness depends on which syntax mode is active.**

A piece of logic "depends on a syntax mode" if:

- It was introduced specifically to support one assembler's source conventions
  (e.g. Apple II high-bit encoding, Merlin comment style)
- Its presence changes the behaviour for any *other* syntax (i.e. it is not
  universally applicable)
- Its removal would have zero effect on any syntax other than the one it targets

### Rule 2 — Where Syntax-Specific Logic Lives

All syntax-specific logic MUST reside in one of:

| Layer | Where | Examples |
|-------|-------|---------|
| **Syntax parser** | `src/syntax/<name>_syntax.cpp` | Line tokenisation, comment stripping, character encoding |
| **Syntax expression override** | `<SyntaxParser>::ParseExpression` | Char literal encoding, operator aliasing, suffix rewriting |
| **Syntax number parser** | `INumberParser` subclass per syntax | `0FFh`, `%1010`, `@177`, etc. |
| **Syntax directive handler** | `src/syntax/<name>_directive_handlers.cpp` | Directives unique to one syntax |

### Rule 3 — The Syntax Hook Contract (How to Extend Without Polluting)

`ExpressionParser` MUST provide a clean extension interface so that syntax-specific
pre/post processing can be injected without modifying shared code:

```
┌───────────────────────────────────────────────────────────┐
│  SyntaxParser::ParseExpression(raw_str, symbols)          │
│   1. Pre-process: strip syntax comments, canonicalise     │ ← syntax layer
│                   aliases (e.g. /expr → >expr for SCMASM) │
│   2. Call ExpressionParser::Parse(canonical_str, symbols) │ ← shared core
│   3. Post-process: apply syntax transforms to AST/value   │ ← syntax layer
└───────────────────────────────────────────────────────────┘
```

`ExpressionParser` handles:
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`
- Comparisons: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Unary: low-byte `<`, high-byte `>`, unary minus `-`
- Literals: decimal, `$hex`, `%binary`, `0x`-prefixed hex
- Symbols: alphanumeric identifiers, `.`-prefixed local labels
- Parenthesised sub-expressions: `(expr)`

`ExpressionParser` does NOT handle:
- Apple II high-bit character encoding — belongs in Merlin pre-process step
- SCMASM `/` high-byte alias — Merlin pre-process canonicalises to `>`
- SCMASM `=` equality alias — canonicalise to `==` before calling parser
- Z80 `[addr]` bracket notation — Z80 tokeniser handles this before expression parsing
- Merlin `]var` variable names — Merlin pre-process must expand to a canonical form
- Syntax-specific comment syntax — handled by `StripComments()` in each syntax class

### Rule 4 — `INumberParser` Extension Point

Syntax-specific number formats (e.g. Z80 `0FFh` suffix hex, EDTASM `#100` decimal
prefix) MUST be injected via `INumberParser`. The shared `ExpressionParser` already
supports this interface. All syntax-specific number formats must use it.

### Rule 5 — No Conditional Branches on Syntax Identity in Core

Constructs of the form:

```cpp
if (syntax_mode_ == SyntaxMode::Merlin) { ... }
if (syntax_type == kSCMASM) { ... }
// or equivalent string comparisons, enum checks, etc.
```

are PROHIBITED in `src/core/` and `src/common/`. Encountering one is a blocking
code-review failure.

---

## Migration Plan for Existing Violations

The 8 violations above must be remediated. Recommended order (lowest risk first):

### Step 1 — Move V4 (the incident violation) — Priority: IMMEDIATE

Remove the `| 0x80` char literal block from `assembler.cpp::ParseExpression`.
`MerlinSyntaxParser::ParseExpression` already contains the correct implementation
(added in d538f83). The shared function must call the syntax-level handler or receive
pre-processed input.

**Risk:** Low — the correct implementation already exists in the right place.

### Step 2 — Move V1 (Merlin inline comment strip) — Priority: HIGH

Remove the `"stabbed"` comment-stripping logic from `assembler.cpp::ParseExpression`.
This MUST be done in `MerlinSyntaxParser::StripComments` (or a new pre-process step)
before the expression string reaches the shared function.

**Risk:** Low — `MerlinSyntaxParser` already has `StripComments`.

### Step 3 — Move V2 (Merlin `.Inc`/`.Dec` suffixes) — Priority: HIGH

Remove `.Inc`/`.Dec` suffix rewriting from `assembler.cpp::ParseExpression`.
Add pre-processing in `MerlinSyntaxParser::ParseExpression` to canonicalise
`LABEL.Inc` → `LABEL+1` before calling the shared parser.

**Risk:** Low — purely a text pre-processing step.

### Step 4 — Consolidate V3/V6 (SCMASM `/` alias) — Priority: MEDIUM

Remove the SCMASM `/expr` high-byte handling from both `assembler.cpp` and
`expression_parser.cpp`. Add pre-processing in `ScmasmSyntaxParser::ParseExpression`
to canonicalise `/EXPR` → `>EXPR` before invoking the shared parser.

**Risk:** Medium — must ensure SCMASM division is still handled correctly. Requires
context-aware substitution (leading `/` is high-byte; `/` after an operand is
division).

### Step 5 — Move V5 (SCMASM `=` equality) — Priority: MEDIUM

Remove single `=` as equality from `ExpressionParser::ParseRelational`. Add
pre-processing in `ScmasmSyntaxParser` to substitute `=` → `==` in expression
strings where appropriate (i.e. not inside labels/strings).

**Risk:** Medium — `=` appears in many contexts; substitution must be precise.

### Step 6 — Move V7 (Z80 `[expr]` brackets) — Priority: LOW

Remove `[…]` bracket handling from `ExpressionParser::ParsePrimary`. Z80 indirect
addressing uses `[reg]` which is a tokenisation concern, not an expression-grammar
concern. The Z80 tokeniser should expand or preserve this before expression evaluation.

**Risk:** Medium — requires Z80 tokeniser changes and validation against Z80 test corpus.

### Step 7 — Move V8 (Merlin `]var` identifier prefix) — Priority: LOW

Remove `]` as a valid identifier-start from `ExpressionParser::ParsePrimary`.
`MerlinSyntaxParser::ParseExpression` should pre-expand `]VARNAME` to `__DUM_VARNAME`
(or another canonical form) before calling the shared parser.

**Risk:** Medium — DUM-block variable semantics are complex; requires Merlin test corpus
validation.

---

## Enforcement

### Code Review Gate

All PRs that touch `src/core/` or `src/common/` MUST be checked against this rule:

> **Is there any reference to a named syntax (`Merlin`, `SCMASM`, `EDTASM`, `Z80`,
> `Z80Universal`, etc.) or any logic that only applies to a single syntax?**

If yes: BLOCK. Direct the author to the syntax layer.

### Automated Check (Recommended)

A linting script (or CI check) should `grep` for known syntax names in `src/core/`
and `src/common/`:

```bash
#!/bin/bash
# Check for syntax-specific names in shared code
OFFENDING=$(grep -rn \
  "Merlin\|merlin\|SCMASM\|scmasm\|EDTASM\|edtasm\|Z80\|Apple.II\|kMerlin\|kSCMASM" \
  src/core/ src/common/ \
  | grep -v "^.*//.*[Ss]ee\|^.*//.*ADR-005")
if [ -n "$OFFENDING" ]; then
  echo "ERROR: Syntax-specific references found in core/common:"
  echo "$OFFENDING"
  exit 1
fi
```

This should fail CI on any new violation.

---

## Alternatives Considered

### Alternative A: Add a `SyntaxMode` parameter to `ParseExpression`

**Rejected.** This would allow the violations to proliferate cleanly (all existing
code would pass a mode flag instead of being moved), but it fundamentally keeps
syntax-specific logic in shared code. A `switch (mode)` in a shared function is the
same architectural violation in a different dress.

### Alternative B: Per-syntax `ExpressionParser` subclasses

**Rejected.** Subclassing `ExpressionParser` per syntax would re-introduce the
god-class and code duplication problems that the shared `ExpressionParser` was
created to solve (see ADR-002). The pre-processing hook model (Rule 3 above) achieves
isolation without subclassing or duplication.

### Alternative C: Template-parameterised `ExpressionParser<SyntaxTraits>`

**Rejected for now.** Template parameterisation would provide compile-time isolation
but at the cost of significantly increased complexity and slower build times. This
remains an open option if the pre-processing hook model proves insufficient.

---

## Consequences

### Positive

- **Regression safety**: A bug fix for Merlin can no longer corrupt SCMASM output.
  The 83→8 incident class becomes structurally impossible.
- **Testability**: Each syntax layer's pre-processing can be unit-tested independently
  of the shared parser.
- **Clarity of ownership**: Any engineer reading `assembler.cpp` or
  `expression_parser.cpp` sees only universal grammar rules, not scattered
  syntax-specific patches.
- **Lower fix cost**: Fixing a Merlin char-literal bug requires touching only Merlin
  files — not shared code that affects 5 other syntaxes.

### Negative / Trade-offs

- **Short-term migration work**: The 8 existing violations must be remediated. Each
  requires careful testing against the full validation corpus.
- **Pre-processing complexity**: Each syntax parser gains a thin pre-processing step.
  This adds code, though less than the current violations warrant.
- **Canonicalisation for SCMASM `/` and `=`**: Context-aware string substitution
  before parsing is non-trivial and must handle edge cases (identifiers containing
  `=`, etc.).

---

## References

- Incident commit: `0302137` (`fix(merlin): char literals +$80…`)
- Remediation commit: `d538f83` (`fix(merlin): four correctness bugs…`)
- ADR-002: Component-Based Parser Refactoring
- `include/xasm++/common/expression_parser.h` — `INumberParser` extension interface
- `src/syntax/merlin_syntax.cpp` — Example of correct syntax-layer implementation
