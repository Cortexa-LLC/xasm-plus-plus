# Architecture Design: Function Complexity Reduction (SonarQube 147 Issues)

**Date:** 2026-04-13
**Status:** Proposed
**Author:** Architect Agent (xasm-bxk)

## Related Documents
- Contract: `.ai/tasks/xasm-bxk-20260413155214-complexity-arch/00-contract.md`
- ADR-007: [Complexity reduction patterns](../../docs/adr/007-complexity-reduction-patterns.md)
- Prior review: `docs/architecture/review-broader-code-quality-2026.md`

---

## Architecture Summary

**Problem:** SonarQube reports 147 cognitive/cyclomatic complexity violations across
15+ files, with 5 functions each exceeding complexity 50. The dominant patterns are:
giant `if`-chain dispatch in `EncodeInstruction`, interleaved state machines in
`EncodeInstructions`, long linear `ParsePrimary`/`ParseLine` token-type fans, and
deeply nested conditional loops in directive handlers (`HandleDa`, `HandleInb`).

**Key Constraints:**
- No observable behaviour change allowed (bit-exact output on all existing tests)
- C++17; no new heavy dependencies
- All refactors must remain independently testable
- Engineers implement; this document provides the blueprint only

**Approach:** Apply three targeted patterns — (1) table-driven dispatch to replace
`if`-chain fan-outs, (2) helper-extraction to split large state machines into named
sub-functions, (3) early-return / guard-clause to flatten nested conditionals. Each
pattern maps to a specific cluster of violations and can be applied file-by-file
without cross-cutting changes.

---

## Component Design

### Affected Components and Roles

| Component | File | Violation Pattern |
|-----------|------|-------------------|
| Cpu6502 encoder | `src/cpu/cpu_6502.cpp` | if-chain dispatch (complexity 147+) |
| Assembler encoder | `src/core/assembler.cpp` | switch/state-machine interleave |
| SCMASM parser | `src/syntax/scmasm_syntax.cpp` | ParseLine / ParsePrimary fans |
| Directive handlers | `src/syntax/scmasm_directive_handlers.cpp` | nested loops + conditions |
| Merlin parser | `src/syntax/merlin_syntax.cpp` | ParseLine fan-out |
| Expression parser | `src/common/expression_parser.cpp` | ParsePrimary token fan |

---

## Key Architecture Decisions

See `docs/adr/007-complexity-reduction-patterns.md` for full ADR.

### Decision 1 — Table-Driven Dispatch for `EncodeInstruction`
- **What:** Replace the 100+ `if (mnemonic == FOO) return EncodeFOO(...)` chain with
  a `static const std::unordered_map<std::string, EncoderFn>` lookup table.
- **Rationale:** Every new branch adds 1 to cyclomatic complexity. A map lookup is O(1)
  and adds zero complexity regardless of how many mnemonics are registered. The pattern
  is already used for the directive registry (`InitializeDirectiveRegistry`).
- **Trade-offs:** Slight start-up cost to build the map (negligible; built once).
  Harder to step through in a debugger (one indirect call instead of a named if-branch).
- **Alternatives rejected:**
  - `switch` on enum: would require a string→enum conversion table — same work, two
    levels of indirection instead of one.
  - Keeping the if-chain but splitting into helper groups: still high complexity,
    just spread across helpers.

### Decision 2 — Extract Named Sub-Procedures from `EncodeInstructions` (assembler.cpp)
- **What:** The main encode loop in `Assembler::EncodeInstructions` (~550 lines) handles
  six atom types inside one `switch`, each branch containing its own nested loops and
  try/catch blocks. Extract each case body into a private `EncodeXxxAtom()` method.
- **Rationale:** Each atom type has distinct logic (InstructionAtom, DataAtom, EquateAtom,
  CpuModeAtom, MxAtom, LabelAtom). Separating them lets SonarQube see individual
  functions with complexity ≤ 10 rather than one function with complexity ~60.
- **Trade-offs:** 6 new private methods; slightly more call overhead (negligible).
  Methods share mutable state (`current_address`, `virtual_address`) — pass by ref.

### Decision 3 — Guard-Clause Flattening for `HandleDa` and `HandleInb`
- **What:** The directive handlers use deeply nested `if/while/for` blocks because
  they check preconditions in the middle of loops. Invert conditions to throw/continue
  early, reducing nesting depth by 2–3 levels.
- **Rationale:** Cognitive complexity penalises nesting depth, not just branch count.
  A guard clause (`if (!ok) throw …; // rest is happy path`) reads more clearly and
  removes an indentation level from all subsequent code.
- **Trade-offs:** None meaningful; guard-clause is idiomatic C++.

### Decision 4 — Token-Type Helpers for `ParsePrimary` / `ParseLine`
- **What:** The linear sequence of `if (token is X) ... else if (token is Y) ...`
  blocks in `ParsePrimary` and `ParseLine` should be split into named predicate-and-
  handler pairs: `bool TryParseLiteral(...)`, `bool TryParseSymbol(...)`, etc., each
  called from the parent in sequence.
- **Rationale:** The sequence is inherently ordered and cannot become a dispatch table
  (several arms have fall-through semantics or shared state). Named helpers reduce the
  cognitive load on each reader without changing ordering guarantees.
- **Trade-offs:** More functions, slight indirection. Acceptable.

---

## Data Model

No data model changes. All refactoring is purely structural (same inputs, same outputs,
same data).

---

## API Contracts

No API surface changes. `EncodeInstruction`, `ParseLine`, `ParsePrimary`, and the
directive handler signatures remain identical. The new dispatch table is an
implementation detail internal to each class.

---

## Refactoring Roadmap (Implementation Guidance)

### Phase 1 — `cpu_6502.cpp::EncodeInstruction` (highest ROI)

```
BEFORE (schematic):
  std::vector<uint8_t> Cpu6502::EncodeInstruction(...) {
    if (clean_mnemonic == LDA) return EncodeLDA(operand, mode);
    if (clean_mnemonic == STA) return EncodeSTA(operand, mode);
    ... (100+ branches)
  }

AFTER:
  // In cpu_6502.cpp (or a new cpu_6502_dispatch.cpp):
  using EncoderFn = std::function<std::vector<uint8_t>(uint32_t, AddressingMode)>;

  static const std::unordered_map<std::string, EncoderFn>&
  Cpu6502::BuildEncoderTable() {
    // Capture `this` via lambda or use std::bind
    static const std::unordered_map<std::string, EncoderFn> kTable = {
      {LDA, [this](uint32_t op, AddressingMode m){ return EncodeLDA(op, m); }},
      {STA, [this](uint32_t op, AddressingMode m){ return EncodeSTA(op, m); }},
      ...
    };
    return kTable;
  }

  std::vector<uint8_t> Cpu6502::EncodeInstruction(...) {
    // ... mode detection (extract to DetectAddressingMode helper) ...
    const auto& table = GetEncoderTable();
    auto it = table.find(clean_mnemonic);
    if (it == table.end()) {
      throw std::runtime_error("Unknown mnemonic: " + clean_mnemonic);
    }
    return it->second(operand, mode);
  }
```

**Note on `this` capture:** `std::function` lambdas capturing `this` are fine for a
static table only if the Cpu6502 object outlives the table. Since the table is built
inside a member function and the map entries reference `this`, prefer a per-instance
table built in the constructor, or use a free-function table that calls member functions
via pointer.

**Simpler alternative** (avoids `this` capture problem):

```cpp
// Map mnemonic → member-function pointer
using EncFn = std::vector<uint8_t>(Cpu6502::*)(uint32_t, AddressingMode) const;
static const std::unordered_map<std::string, EncFn> kEncoderTable = {
  {LDA, &Cpu6502::EncodeLDA},
  {STA, &Cpu6502::EncodeSTA},
  ...
};
// Dispatch:
auto it = kEncoderTable.find(clean_mnemonic);
if (it != kEncoderTable.end()) {
  return (this->*it->second)(operand, mode);
}
```

Member-function pointers avoid the `std::function` overhead and the `this`-capture
lifetime issue. This is the **recommended approach**.

### Phase 2 — `assembler.cpp::EncodeInstructions`

Split the monolithic atom-dispatch loop. Each atom-type handler becomes a private
method:

```
Assembler::EncodeInstructionAtom(InstructionAtom&, uint32_t& addr, uint32_t& vaddr,
                                  std::vector<size_t>& sizes, ...)
Assembler::EncodeDataAtom(DataAtom&, uint32_t& addr, ...)
Assembler::EncodeEquateAtom(EquateAtom&, ConcreteSymbolTable&, ...)
Assembler::EncodeSpecialAtom(atom, ...)   // CpuMode / MxState
```

The main loop becomes a ~20-line dispatcher:
```cpp
for (auto& atom : section.atoms) {
  switch (atom->type) {
    case AtomType::Instruction: EncodeInstructionAtom(...); break;
    case AtomType::Data:        EncodeDataAtom(...);        break;
    ...
  }
}
```

### Phase 3 — `scmasm_directive_handlers.cpp::HandleDa` and `HandleInb`

**HandleDa** parses comma-separated expressions with embedded string literals and
inline comments (~290 lines). Decompose into:

```
ParseDaToken(source, pos)        → DaToken (value, is_string, comment_started)
EmitDaBytes(DaToken, context)    → void (appends atoms)
```

The outer loop becomes a simple iteration:
```cpp
while (!AtEnd(operand, pos)) {
  auto tok = ParseDaToken(operand, pos);
  if (tok.comment_started) break;
  EmitDaBytes(tok, context);
}
```

**HandleInb** has a 3-level nested conditional for path resolution. Apply guard-clause:
```cpp
// BEFORE:
if (!path.empty()) {
  if (fs::exists(path)) {
    // ... 40 more lines ...
  }
}
// AFTER:
if (path.empty()) throw std::runtime_error(".INB requires filename");
if (!fs::exists(resolved)) throw std::runtime_error("File not found: " + path);
// ... happy-path code at top level ...
```

### Phase 4 — `scmasm_syntax.cpp::ParseLine` and `merlin_syntax.cpp::ParseLine`

Extract sub-parsers as private `bool TryXxx(...)` methods that return false on
no-match and throw on partial match:

```
TryParseDirective(opcode, ...)  → bool
TryParseInstruction(opcode, ...) → bool
TryParseMacroInvocation(opcode, ...) → bool
```

`ParseLine` becomes a clean sequence of `if (!TryParseXxx(...)) ...` calls.

### Phase 5 — `expression_parser.cpp::ParsePrimary`

The ~340-line `ParsePrimary` handles: parenthesised expr, bracket expr, unary ops,
hex/dec/binary literals, char literals, PC symbol, named symbols, string literals.

Extract each into a `TryParseXxx` that returns `std::shared_ptr<Expression>` or
`nullptr`. `ParsePrimary` becomes:

```cpp
std::shared_ptr<Expression> ExpressionParser::ParsePrimary() {
  SkipWhitespace();
  if (auto e = TryParseGrouped())    return e;
  if (auto e = TryParseUnary())      return e;
  if (auto e = TryParseNumericLit()) return e;
  if (auto e = TryParseCharLit())    return e;
  if (auto e = TryParseSymbol())     return e;
  throw std::runtime_error("Unexpected token in expression");
}
```

---

## File-Level Complexity Reduction Estimates

| File | Current Est. Complexity | Target After Refactor | Strategy |
|------|--------------------------|-----------------------|----------|
| `cpu_6502.cpp::EncodeInstruction` | ~147 | ≤ 15 | Member-fn pointer table |
| `assembler.cpp::EncodeInstructions` | ~60 | ≤ 12 | Extract 5 atom handlers |
| `scmasm_syntax.cpp::ParseLine` | ~45 | ≤ 12 | TryParse sub-functions |
| `merlin_syntax.cpp::ParseLine` | ~40 | ≤ 12 | TryParse sub-functions |
| `scmasm_directive_handlers.cpp::HandleDa` | ~43 | ≤ 12 | Token + Emit split |
| `scmasm_directive_handlers.cpp::HandleInb` | ~54 | ≤ 15 | Guard-clause + helpers |
| `expression_parser.cpp::ParsePrimary` | ~35 | ≤ 10 | TryParse sub-functions |
| All other files (medium violations) | various | ≤ 15 | Guard-clause / extract |

---

## Open Questions / Risks

- **`this` capture in static table** — Member-function pointer approach avoids lifetime
  issues but requires all `Encode*` methods to have uniform signatures. Verify this
  before implementing. Owner: Engineer
- **`EncodeInstructionSpecial`** (line 2413) — also high complexity; same pattern as
  `EncodeInstruction` but smaller; can be handled in the same Phase 1 PR.
- **Merlin `ParseExpression`** (line 215, ~200 lines) — the pre-processing steps (inline
  comment stripping, prefix expansion) should be extracted as named helpers first, then
  the expression parse logic can follow the same TryParse pattern.
- **Test coverage before refactor** — Confirm that all existing unit/integration tests
  pass before any change lands. Complexity refactoring with failing tests is undefined
  territory. Owner: Engineer / CI
