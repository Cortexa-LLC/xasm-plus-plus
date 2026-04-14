# ADR-007: Complexity Reduction Patterns

**Status:** Proposed
**Date:** 2026-04-13
**Deciders:** Architect Agent (xasm-bxk task)

## Context

SonarQube reports 147 cognitive/cyclomatic complexity violations after all
mechanical clang-tidy fixes are complete. The five highest-complexity functions
exceed complexity 50 each. Standard SonarQube thresholds flag any function above
15 (cyclomatic) or 25 (cognitive). The violations concentrate in three structural
patterns:

1. **Long if-chain dispatch** — `EncodeInstruction` uses 100+ sequential
   `if (mnemonic == X) return EncodeX()` branches.
2. **Interleaved state machines** — `EncodeInstructions` handles six atom types
   inside one giant switch, each arm containing its own nested loops.
3. **Token-type fan-outs** — `ParseLine`, `ParsePrimary`, `HandleDa` use deeply
   nested if/else to classify tokens, mixing detection and action logic.

## Decision

Apply three targeted structural patterns to reduce complexity without behaviour
change:

### Pattern A — Member-Function Pointer Dispatch Table

Replace sequential `if (mnemonic == X) return EncodeX()` chains with a `static
const std::unordered_map<std::string, EncFn>` where `EncFn` is a member-function
pointer. The dispatch loop becomes a single `map::find` + indirect call.

```cpp
using EncFn = std::vector<uint8_t>(Cpu6502::*)(uint32_t, AddressingMode) const;
static const std::unordered_map<std::string, EncFn> kEncoderTable = { ... };
```

**Primary targets:** `Cpu6502::EncodeInstruction`, `Cpu6502::EncodeInstructionSpecial`

### Pattern B — Named Sub-Procedure Extraction

Extract each logical arm of a complex switch/if into a dedicated private method.
The parent function becomes a thin dispatcher ≤ 20 lines; complexity concentrates
in small, single-responsibility helpers.

**Primary targets:** `Assembler::EncodeInstructions` (6 atom types),
`ScmasmSyntaxParser::ParseLine`, `MerlinSyntaxParser::ParseLine`

### Pattern C — Guard-Clause Flattening

Invert nested precondition checks to throw/continue/return early. The happy path
sits at the shallowest nesting level; deep nesting disappears.

**Primary targets:** `HandleDa`, `HandleInb`, `MerlinSyntaxParser::ParseExpression`

### Pattern D — TryParse Decomposition (for parsers)

Extract each token-class arm of `ParsePrimary` / similar into a named
`std::shared_ptr<Expression> TryParseXxx()` that returns `nullptr` on non-match.
The parent calls them in sequence:

```cpp
if (auto e = TryParseGrouped())    return e;
if (auto e = TryParseNumericLit()) return e;
...
```

**Primary targets:** `ExpressionParser::ParsePrimary`

## Rationale

- **Pattern A** is the highest-ROI change: one table replaces 100+ branches.
  Precedent already exists in this codebase (`InitializeDirectiveRegistry`).
- **Pattern B** follows the Single Responsibility Principle: each atom type has
  distinct semantics, and isolating them improves testability.
- **Patterns C and D** are micro-refactors with no design risk; they are idiomatic
  C++ and universally accepted.
- All four patterns preserve the external interface (no signature changes to public
  or virtual functions).

## Consequences

**Positive:**
- SonarQube violations reduced from 147 to an estimated < 20 (residual from
  medium-complexity files not targeted in Phase 1–5)
- Smaller functions are unit-testable in isolation
- Easier to add new mnemonics / directives without touching a giant function

**Negative:**
- Member-function pointer syntax is moderately unfamiliar to C++ newcomers
- Extracting sub-functions increases total function count (acceptable trade-off)
- Static dispatch table built at program start; negligible cost

**Neutral:**
- No external API changes
- No data model changes
- Test suite passes unchanged (verification step before any PR)

## Alternatives Considered

**Option A: `switch` on mnemonic enum** — Would require a string→enum map, adding
an extra lookup step. Net complexity is the same; two data structures instead of
one. Rejected.

**Option B: Virtual method per mnemonic (visitor pattern)** — Would create ~100
virtual methods or a parallel class hierarchy. Massive over-engineering for what is
essentially a lookup problem. Rejected.

**Option C: Split files only (no pattern change)** — Moving large functions to
separate `.cpp` files without structural change would reduce file-level metrics but
not function-level complexity, which is what SonarQube reports. Rejected.

## Related ADRs

- ADR-001: Directive handler pattern (establishes the handler-registry precedent)
- ADR-002: Component-based parser refactoring
- ADR-005: Syntax/core boundary
