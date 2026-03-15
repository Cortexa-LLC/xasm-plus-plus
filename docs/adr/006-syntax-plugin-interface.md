# ADR-006: SyntaxPlugin — Common Interface for Assembly Syntax Parsers

**Status:** Proposed  
**Date:** 2026-03-14  
**Deciders:** Architect Agent, pending engineering review  
**Context Source:** M5 finding in `docs/adr/review-broader-code-quality-2026.md` + codebase audit  
**Task:** xasm-9k5

---

## Context

### Current State

xasm++ supports six distinct assembly syntaxes:

| Class | File | CPU family |
|---|---|---|
| `SimpleSyntaxParser` | `simple_syntax.h` | generic |
| `MerlinSyntaxParser` | `merlin_syntax.h` | 6502 |
| `ScmasmSyntaxParser` | `scmasm_syntax.h` | 6502 |
| `EdtasmSyntaxParser` | `edtasm_syntax.h` | 6809 |
| `FlexAsmSyntax` | `flex_syntax.h` | 6809 |
| `EdtasmM80PlusPlusSyntaxParser` | `edtasm_m80_plusplus_syntax.h` | Z80 |

Every parser is a standalone concrete class.  They share **no base class, no
abstract interface, and no common naming convention**.  As a result:

1. **`src/main.cpp` lines 147–270** contain a 120-line `if/else if` ladder that
   instantiates the correct parser, configures it, calls `Parse()`, and extracts
   results — with nearly identical code duplicated for every branch.

2. Any code that needs to work across parsers (e.g. a future `--list` flag, a
   unit-test harness, a language-server bridge) must either hardcode every
   parser variant or live inside the `main.cpp` ladder.

3. Adding a new syntax requires touching the `main.cpp` ladder, not just writing
   a new parser class.

4. The `CpuPlugin` base class (ADR-001 era) already demonstrates this pattern
   works cleanly for CPU back-ends; the syntax front-ends have no equivalent.

### Prior Art: CpuPlugin

`include/xasm++/cpu/cpu_plugin.h` defines a pure-virtual base that all CPU
encoders implement.  `main.cpp` holds a `CpuPlugin *cpu` and calls `cpu->Encode()`
without knowing whether it is talking to `Cpu6502`, `Cpu6809`, or `CpuZ80`.
The `SyntaxPlugin` ADR mirrors that model for the parsing front-end.

### Common Interface Candidates (Observed Across All Parsers)

Survey of the six parser headers reveals these methods appear — with identical or
near-identical signatures — in every parser:

| Method | Signature | Notes |
|---|---|---|
| `Parse` | `void Parse(const std::string &source, Section &section, ConcreteSymbolTable &symbols)` | Exact same signature in all six parsers |

The following methods appear in some parsers and have clear generalisation paths:

| Method | Present in | Generalisation |
|---|---|---|
| `SetCpu(CpuPlugin*)` | Merlin (typed `Cpu6502*`), Scmasm (`CpuPlugin*`), EdtasmM80++ (`CpuZ80*`) | Lift to `CpuPlugin*` in interface |
| `SetIncludePaths(const std::vector<std::string>&)` | Scmasm | Include in interface; default no-op |
| `SetPathMappings(const std::map<std::string,std::string>&)` | Scmasm | Scmasm-specific; omit from base |
| `SetRw18Mode(bool)` | Merlin | Merlin-specific; omit from base |
| `HasUsrArgs()` / `GetUsrArgs()` | Merlin | Merlin-specific; omit from base |
| `GetTfOutput()` / `SetTfOutput()` | Scmasm | Scmasm-specific; omit from base |
| `GetCpu()` | Merlin | Not universally meaningful; omit from base |

Purely syntax-specific accessors (`SetRw18Mode`, `HasUsrArgs`, `GetTfOutput`, etc.)
are **not** candidates for the base interface; callers that need them must
dynamic_cast to the concrete type.  Only genuinely universal operations belong in
the base.

---

## Decision

### ADR-006: Introduce `SyntaxPlugin` as a Pure-Virtual Base Class

We will introduce `include/xasm++/syntax/syntax_plugin.h` containing an abstract
base class `SyntaxPlugin`.  All six existing parsers will inherit from it.
`main.cpp` will use `std::unique_ptr<SyntaxPlugin>` and call through the base
wherever the generic interface suffices.

---

## Interface Design

```cpp
// include/xasm++/syntax/syntax_plugin.h

#pragma once

#include "xasm++/cpu/cpu_plugin.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <string>
#include <vector>

namespace xasm {

/**
 * @brief Abstract base class for assembly syntax parsers (SyntaxPlugin).
 *
 * Each concrete syntax (Merlin, SCMASM, EDTASM, FLEX, Z80, Simple, …) derives
 * from SyntaxPlugin and implements at minimum Parse().
 *
 * DESIGN PRINCIPLES
 * -----------------
 * 1. Only universal operations live here.  Syntax-specific features (Merlin
 *    RW18 mode, SCMASM TF output, …) remain on the concrete class and are
 *    accessed via dynamic_cast.
 * 2. The interface mirrors the existing CpuPlugin pattern; callers need not
 *    know which concrete parser they are holding.
 * 3. SetCpu() is included because every real-world syntax requires CPU
 *    knowledge to validate opcodes; parsers that target a fixed CPU may
 *    provide an empty body or a no-op override.
 * 4. SetIncludePaths() is included because include/PUT directives are a
 *    cross-cutting concern; parsers with no include support may ignore it.
 */
class SyntaxPlugin {
public:
  virtual ~SyntaxPlugin() = default;

  // -----------------------------------------------------------------------
  // Identification
  // -----------------------------------------------------------------------

  /**
   * @brief Returns a short canonical name for this syntax (e.g. "merlin",
   *        "scmasm", "edtasm", "flex", "z80", "simple").
   *
   * Used for error messages, `--list-syntaxes`, and diagnostic output.
   */
  virtual std::string Name() const = 0;

  // -----------------------------------------------------------------------
  // Configuration (called before Parse())
  // -----------------------------------------------------------------------

  /**
   * @brief Attach a CPU plugin for opcode validation.
   *
   * @param cpu  Non-owning pointer; must remain valid for the lifetime of
   *             Parse().  Passing nullptr is legal for syntaxes that do not
   *             validate opcodes.
   */
  virtual void SetCpu(CpuPlugin *cpu) = 0;

  /**
   * @brief Supply a list of directories to search for included files.
   *
   * @param paths  Ordered list of include-search directories.  The parser
   *               searches them in order.  Parsers that do not support
   *               include directives may ignore this call.
   */
  virtual void SetIncludePaths(const std::vector<std::string> &paths) = 0;

  // -----------------------------------------------------------------------
  // Parsing
  // -----------------------------------------------------------------------

  /**
   * @brief Parse assembly source into atoms and symbols.
   *
   * This is the primary entry point.  Implementors should:
   *  - Tokenise @p source line by line.
   *  - Populate @p section with instruction/data atoms.
   *  - Populate @p symbols with label and constant definitions.
   *  - Report errors via the shared error infrastructure (throw or collect).
   *
   * @param source   Full source text (may be multi-line).
   * @param section  Output section to populate.
   * @param symbols  Symbol table to update.
   */
  virtual void Parse(const std::string &source, Section &section,
                     ConcreteSymbolTable &symbols) = 0;

  // -----------------------------------------------------------------------
  // Non-copyable, non-movable (holds raw pointers to external state)
  // -----------------------------------------------------------------------
  SyntaxPlugin(const SyntaxPlugin &) = delete;
  SyntaxPlugin &operator=(const SyntaxPlugin &) = delete;
  SyntaxPlugin(SyntaxPlugin &&) = delete;
  SyntaxPlugin &operator=(SyntaxPlugin &&) = delete;

protected:
  SyntaxPlugin() = default;
};

} // namespace xasm
```

---

## Migration Plan

### Phase 1 — Add `SyntaxPlugin` header (no behaviour change)

Create `include/xasm++/syntax/syntax_plugin.h` with the interface above.  No
existing code is changed; tests continue to pass.

### Phase 2 — Inherit in all six parsers

For each parser, add `: public SyntaxPlugin` to the class definition and
supply the three pure-virtual overrides:

| Parser | `Name()` returns | `SetCpu()` note | `SetIncludePaths()` note |
|---|---|---|---|
| `SimpleSyntaxParser` | `"simple"` | Store pointer; no-op if no opcode engine | No-op (no include support) |
| `MerlinSyntaxParser` | `"merlin"` | `static_cast<Cpu6502*>` after accepting `CpuPlugin*` | No-op (Merlin uses PUT with CWD) |
| `ScmasmSyntaxParser` | `"scmasm"` | Already `CpuPlugin*` — rename / delegate | Already exists — delegate |
| `EdtasmSyntaxParser` | `"edtasm"` | Store pointer; cast to `Cpu6809*` internally | No-op (EDTASM uses NAM, not paths) |
| `FlexAsmSyntax` | `"flex"` | Store pointer; cast to `Cpu6809*` internally | No-op |
| `EdtasmM80PlusPlusSyntaxParser` | `"z80"` | Cast to `CpuZ80*` internally | No-op |

**Compatibility note on `SetCpu`:**  
Merlin currently exposes `void SetCpu(Cpu6502*)`.  After Phase 2 the class must
implement *both*:
- `void SetCpu(CpuPlugin*) override` — the base-interface override, performs
  `static_cast<Cpu6502*>` and stores the result.
- `void SetCpu(Cpu6502*)` — the existing typed helper, kept for existing direct
  callers.  It simply calls `SetCpu(static_cast<CpuPlugin*>(cpu))`.

The same dual-overload pattern applies to `EdtasmSyntaxParser` (`Cpu6809*`) and
`EdtasmM80PlusPlusSyntaxParser` (`CpuZ80*`).

### Phase 3 — Simplify `main.cpp`

Replace the 120-line `if/else if` ladder with a factory:

```cpp
// src/syntax/syntax_factory.h  (new)
std::unique_ptr<SyntaxPlugin> CreateSyntaxPlugin(const std::string &syntax_name);
```

```cpp
// src/main.cpp  (simplified)
auto parser = CreateSyntaxPlugin(opts.syntax);  // factory
parser->SetCpu(cpu.get());
parser->SetIncludePaths(opts.include);
parser->Parse(source, section, symbols);
// syntax-specific config obtained via dynamic_cast where needed
```

The factory is a plain `switch`/`if`-on-string that returns the correct concrete
type wrapped in `unique_ptr<SyntaxPlugin>`.  It lives in `src/syntax/` to
keep `main.cpp` free of construction details.

---

## Alternatives Considered

### A — Non-virtual template (policy-based design)

Use a template `Assembler<SyntaxT, CpuT>` and resolve calls at compile time.

**Rejected because:**  
The syntax is selected at runtime from the `--syntax` flag.  Compile-time
dispatch cannot handle this without exhaustive explicit instantiations, which
reproduces the `if/else` ladder at the template level.

### B — Std::variant dispatch

Hold a `std::variant<MerlinSyntaxParser, ScmasmSyntaxParser, …>` and use
`std::visit`.

**Rejected because:**  
`std::visit` requires the same exhaustive per-type logic and is not more
extensible than the current `if/else` ladder; adding a new syntax still requires
editing the variant type alias.

### C — Duck-typing via `std::function` wrappers

Store six `std::function` members in a struct and populate them per-syntax.

**Rejected because:**  
Indirect call overhead is the same as virtual dispatch, type safety is worse,
and the pattern is unfamiliar and harder to document than a standard virtual
base.

### D — Inheritance but no `Name()` or `SetIncludePaths()` in base

Include only `Parse()` and `SetCpu()` in the pure-virtual interface.

**Considered.**  `Name()` was added because it eliminates fragile
`dynamic_cast`-for-identification patterns.  `SetIncludePaths()` was added
because it is a near-universal configuration step (present today in Scmasm;
likely needed in future syntaxes) and the cost of a no-op override is
negligible.

---

## Consequences

### Positive

- **Eliminates the `main.cpp` `if/else` ladder** (120 lines → ~5 lines after
  factory extraction).
- **Enables generic test harnesses** that exercise any parser through the
  common interface.
- **Makes adding a new syntax safe** — the compiler will enforce that the new
  class implements all three pure virtuals before it compiles.
- **ADR-005 compliance** — the interface boundary is pure-virtual; no
  syntax-specific logic can leak into `SyntaxPlugin` itself because it has no
  implementation.
- **Mirrors existing `CpuPlugin` pattern**, which is already well-understood by
  the team.

### Negative / Risks

- **`Cpu6502*` vs `CpuPlugin*` cast in Merlin** is a mild type-safety regression
  caught at runtime rather than compile time.  Mitigation: add a `dynamic_cast`
  assertion in debug builds inside `MerlinSyntaxParser::SetCpu(CpuPlugin*)`.
- **Six parsers must be touched** in Phase 2.  This is mechanical but carries
  non-zero risk of silent regressions.  Mitigation: run the full test suite
  (currently 1928 tests) after each parser conversion.
- **`FlexAsmSyntax` name** differs from the `*SyntaxParser` convention on other
  parsers.  Phase 2 is an opportunity to rename it `FlexSyntaxParser` — but
  that is out of scope for this ADR; file a separate task if desired.

---

## Scope of This ADR

This ADR **specifies the interface and migration plan**.  It does **not** commit
to a timeline for Phase 2 or Phase 3; those are implementation tasks.

The following items are **explicitly out of scope**:

- The `SyntaxFactory` implementation detail (API shape only sketched above).
- Renaming `FlexAsmSyntax` to `FlexSyntaxParser`.
- Any changes to CPU plugins.
- Thread-safety of syntax parsers.
- Any new syntax parsers.

---

## Enforcement

### Code Review Gate

All new syntax parsers MUST:

1. Inherit `SyntaxPlugin`.
2. Implement `Name()`, `SetCpu(CpuPlugin*)`, `SetIncludePaths()`, and `Parse()`.
3. Not add syntax-specific logic to `SyntaxPlugin` itself (ADR-005 boundary).

Reviewers should reject any PR that instantiates a concrete parser in
`main.cpp` directly rather than going through the factory.

### Linting

After Phase 2 lands, add a comment header to `SyntaxPlugin` noting that it is
the **only** file in `include/xasm++/syntax/` that may include
`xasm++/cpu/cpu_plugin.h` at the plugin-interface level; all other syntax
headers receive `CpuPlugin*` via constructor injection or `SetCpu()`.

---

## Open Questions

1. **Should `SyntaxPlugin` expose `GetErrors()` / `GetWarnings()`?**  
   Currently parsers throw or collect errors through internal state; no parser
   exposes a unified error-retrieval API through the base.  A unified error
   collection interface would be valuable but is deferred to a follow-up ADR.

2. **Should `SyntaxPlugin` include `ParseLine()` in the interface?**  
   All parsers expose a `ParseLine()` helper, but it is `protected` or internal
   in most.  Exposing it publicly would enable incremental / REPL-style use.
   Deferred pending a concrete use-case.

3. **`Name()` discoverability** — should there be a `SyntaxRegistry` analogous
   to `DirectiveRegistry` that maps name strings to factory functions?  Out of
   scope here; tracked as a potential M6 follow-up.

---

## Related ADRs

- **ADR-001** — Directive handler pattern (same plugin-style motivation)
- **ADR-005** — Syntax/core boundary (SyntaxPlugin must not carry syntax logic)
- **Review** — `docs/adr/review-broader-code-quality-2026.md` §M5 (origin of
  this ADR)
