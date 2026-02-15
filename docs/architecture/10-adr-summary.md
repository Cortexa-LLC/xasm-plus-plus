# Architecture Decision Records (ADR) Summary

**Version:** 1.0  
**Date:** 2026-02-14  
**Status:** Living Document

---

## Overview

This document summarizes all Architecture Decision Records (ADRs) for the xasm++ project. ADRs document significant architectural and design decisions, providing context for future developers and maintaining institutional knowledge.

**ADR Numbering:** Sequential across the project (not per-component).

**ADR Status Values:**
- **✅ Accepted** - Decision implemented and in use
- **📝 Proposed** - Decision proposed but not yet implemented
- **🔄 Superseded** - Decision replaced by a later ADR
- **❌ Rejected** - Decision considered and rejected

---

## Implemented Architecture Decisions

### ADR-001: CPU Plugin Architecture (Strategy Pattern)

**Status:** ✅ Accepted (Implemented)  
**Date:** 2026-01-19 (Implicit)  
**Deciders:** Architecture Team

#### Context

xasm++ needs to support multiple CPU architectures (6502, 6809, Z80, 68000, etc.) without modifying the core assembler for each new CPU. The assembler must:
- Support runtime CPU selection
- Allow independent testing of CPU encoders
- Enable third-party CPU plugin development
- Maintain clean separation between assembly logic and encoding logic

#### Decision

Use the **Strategy Pattern** with an abstract `CpuPlugin` base class:

```cpp
class CpuPlugin {
public:
  virtual ~CpuPlugin() = default;
  virtual std::string GetCpuFamily() const = 0;
  virtual std::vector<uint8_t> EncodeInstruction(
      const std::string& mnemonic,
      uint32_t operand,
      const std::string& operand_str) const = 0;
};
```

Concrete implementations (Cpu6502, Cpu6809, CpuZ80) inherit from this interface.

#### Rationale

**Alternatives Considered:**
1. **Monolithic assembler with CPU-specific code paths**
   - Rejected: Violates Open/Closed Principle, hard to extend
2. **Template-based compile-time selection**
   - Rejected: No runtime CPU selection, separate binaries needed
3. **Function pointers**
   - Rejected: Less type-safe, no polymorphic benefits

**Why Strategy Pattern:**
- ✅ Runtime CPU selection
- ✅ Open/Closed Principle: Add new CPUs without modifying core
- ✅ Each CPU independently testable
- ✅ Clean separation of concerns
- ✅ Polymorphism provides type safety

#### Consequences

**Positive:**
- ✅ Easy to add new CPU architectures (1-2 weeks per CPU)
- ✅ Core assembler never changes when adding CPUs
- ✅ Each CPU plugin can be developed independently
- ✅ Unit testing each CPU in isolation

**Negative:**
- ⚠️ Virtual function call overhead (negligible - not a hot path)
- ⚠️ Some code duplication across CPU plugins (addressing mode parsing)

**Neutral:**
- Requires dependency injection (assembler doesn't create CPU plugin)

#### Related Decisions
- ADR-002: Atom-based IR (interacts with encoding)
- ADR-005: Opcode organization (how CPU plugins structure internal data)

---

### ADR-002: Atom-Based Intermediate Representation

**Status:** ✅ Accepted (Implemented)  
**Date:** 2026-01-19 (Implicit)  
**Deciders:** Architecture Team

#### Context

The assembler needs an intermediate representation (IR) to decouple:
- Source parsing (syntax-specific) from instruction encoding (CPU-specific)
- Multi-pass assembly logic from I/O operations

Requirements:
- Support forward references (labels defined after use)
- Support branch relaxation (instruction sizes may change)
- Track source location for error reporting
- Support multiple passes with convergence checking

#### Decision

Use **Atom Chain** as the IR, inspired by vasm-ext:

```cpp
class Atom {
  AtomType type;              // Label, Instruction, Data, etc.
  SourceLocation location;    // File, line, column
  size_t size;                // Current size in bytes
  size_t last_size;           // Previous pass size
  uint32_t changes;           // Number of size changes
  std::shared_ptr<Atom> next; // Linked list
};
```

**Atom Types:**
- LabelAtom - Symbol definition
- InstructionAtom - CPU instruction
- DataAtom - Raw data bytes
- SpaceAtom - Reserved space
- AlignAtom - Alignment directive
- OrgAtom - Origin directive

#### Rationale

**Alternatives Considered:**
1. **AST (Abstract Syntax Tree)**
   - Rejected: Too tied to source syntax, hard to optimize
2. **Three-address code (TAC)**
   - Rejected: Inappropriate for assembler (not compilation)
3. **Direct encoding (no IR)**
   - Rejected: Can't handle multi-pass or forward references

**Why Atom Chain:**
- ✅ Proven design (vasm-ext has 15+ years production use)
- ✅ Natural fit for multi-pass assembly
- ✅ Clear separation: parsing creates atoms, assembler resolves them
- ✅ Easy to add new atom types (extensibility)
- ✅ Size tracking built-in (supports convergence checking)

#### Consequences

**Positive:**
- ✅ Clean separation between parsing and encoding
- ✅ Multi-pass assembly natural and straightforward
- ✅ Forward references handled elegantly
- ✅ Source location preserved for error messages
- ✅ Listing file generation straightforward

**Negative:**
- ⚠️ Memory overhead (linked list vs. vector)
- ⚠️ Pointer chasing (cache unfriendly vs. vector)

**Mitigation:**
- Consider switching to `std::vector<std::shared_ptr<Atom>>` for better cache locality (future optimization)

#### Related Decisions
- ADR-001: CPU plugin (atoms contain instructions to encode)
- ADR-003: Multi-pass algorithm (operates on atom chain)
- ADR-004: Syntax parser interface (creates atoms)

---

### ADR-003: Multi-Pass Assembly with Fast/Safe Phases

**Status:** ✅ Accepted (Implemented)  
**Date:** 2026-01-19 (Implicit)  
**Deciders:** Architecture Team

#### Context

Assembly requires multiple passes to resolve:
- Forward references (labels defined after use)
- Branch relaxation (instruction size depends on target distance)
- Expression evaluation (symbols depend on other symbols)

Challenge: Instruction sizes may oscillate (shrink then grow then shrink) in pathological cases.

Goal: Guarantee convergence while being fast for typical programs.

#### Decision

Use **two-phase multi-pass algorithm:**

**Phase 1: Fast Optimization (Passes 2-50)**
- Optimize ALL instructions simultaneously
- Fast convergence for typical programs
- Switch to safe mode if no convergence after 50 passes

**Phase 2: Safe Optimization (Passes 51-500)**
- Optimize ONE instruction per pass
- Prevents oscillation
- Guarantees eventual convergence

```cpp
constexpr int FAST_PHASE_LIMIT = 50;
constexpr int MAX_PASSES = 500;

for (int pass = 2; pass <= FAST_PHASE_LIMIT; ++pass) {
  EncodeAllInstructions();  // Optimize all simultaneously
  if (CheckConvergence()) break;
}

// If not converged, switch to safe mode
for (int pass = 51; pass <= MAX_PASSES; ++pass) {
  EncodeOneInstructionPerPass();  // Optimize one at a time
  if (CheckConvergence()) break;
}
```

#### Rationale

**Alternatives Considered:**
1. **Single-pass only**
   - Rejected: Can't handle forward references
2. **Unlimited multi-pass**
   - Rejected: May not converge (infinite loop risk)
3. **Fixed number of passes (e.g., 10)**
   - Rejected: Fails for complex programs with many dependencies
4. **Topological sort of dependencies**
   - Rejected: Complex implementation, doesn't handle all cases

**Why Two-Phase:**
- ✅ Fast for typical programs (2-3 passes)
- ✅ Guaranteed convergence in pathological cases
- ✅ Empirically proven in vasm-ext
- ✅ Simple to implement and understand

#### Consequences

**Positive:**
- ✅ Typical programs assemble in 2-3 passes (fast)
- ✅ Complex programs with many dependencies still converge
- ✅ Pathological cases guaranteed to converge (safe mode)
- ✅ No infinite loops possible (MAX_PASSES limit)

**Negative:**
- ⚠️ Pathological cases slow (50+ passes)
- ⚠️ One-instruction-per-pass in safe mode can be slow

**Neutral:**
- Trade-off between speed and robustness (acceptable)

**Empirical Data:**
| Program Type | Typical Pass Count | Fast Phase? |
|--------------|-------------------|-------------|
| No forward refs | 1 | N/A |
| Simple forward refs | 2-3 | Yes |
| Branch relaxation | 3-5 | Yes |
| Complex dependencies | 5-10 | Yes |
| Pathological | 50+ | No (safe mode) |

#### Related Decisions
- ADR-002: Atom-based IR (multi-pass operates on atoms)
- ADR-001: CPU plugin (encoding happens each pass)

---

## Proposed Architecture Decisions

### ADR-004: Syntax Parser Interface (Template Method Pattern)

**Status:** 📝 Proposed (Not Yet Implemented)  
**Date:** 2026-02-09  
**Deciders:** Architecture Team

#### Context

xasm++ needs to support multiple assembly syntaxes:
- Merlin (Apple IIgs)
- SCMASM (Apple II)
- EDTASM (CoCo)
- Motorola (68000)
- Z80 Universal

**Current Problem:**
- No common interface for syntax parsers
- ~40% code duplication across parsers (directive handling)
- Inconsistent behavior (each parser implements directives differently)
- Adding new syntax requires duplicating directive logic

#### Decision

Introduce **SyntaxParser interface** with **Template Method Pattern**:

```cpp
class SyntaxParser {
public:
  virtual ParsedLine ParseLine(const std::string& line) = 0;
  virtual bool IsDirective(const std::string& token) const = 0;
};

class BaseSyntaxParser : public SyntaxParser {
public:
  ParsedLine ParseLine(const std::string& line) override {
    // Template method - common structure
    std::string comment = ExtractComment(line);  // Hook 1
    std::string label = ExtractLabel(line);      // Hook 2
    auto [mnemonic, operand] = SplitMnemonicOperand(line);
    return ParsedLine{label, mnemonic, operand, comment};
  }

protected:
  // Hooks for subclass customization
  virtual std::string ExtractComment(std::string& line) = 0;
  virtual std::string ExtractLabel(std::string& line) = 0;
  
  // Common directive handling
  virtual void HandleORG(uint32_t address);
  virtual void HandleEQU(const std::string& symbol, uint32_t value);
  virtual void HandleDB(const std::vector<uint8_t>& bytes);
};

class MerlinSyntaxParser : public BaseSyntaxParser {
protected:
  std::string ExtractComment(std::string& line) override;  // Merlin-specific
  std::string ExtractLabel(std::string& line) override;    // Merlin-specific
};
```

#### Rationale

**Alternatives Considered:**
1. **Keep current independent parsers**
   - Rejected: Duplication, inconsistency, maintenance burden
2. **Strategy Pattern (composition)**
   - Rejected: More complex than needed, no shared structure
3. **Factory Pattern only**
   - Rejected: Doesn't address duplication

**Why Template Method:**
- ✅ Eliminates ~40% code duplication
- ✅ Consistent directive behavior across syntaxes
- ✅ Easy to add new syntax variants (inherit and override)
- ✅ Common logic tested once, syntax-specific logic tested separately

#### Consequences

**Positive:**
- ✅ Reduced duplication (~40% less code)
- ✅ Consistent directive handling
- ✅ Faster to implement new syntaxes (2-3 days vs. 1 week)
- ✅ Better testability (test common logic once)

**Negative:**
- ⚠️ Refactoring effort required (1-2 weeks to implement)
- ⚠️ Inheritance hierarchy (must understand template method pattern)

**Neutral:**
- Existing parsers need refactoring (but improves maintainability)

#### Implementation Plan

1. Create `SyntaxParser` interface
2. Create `BaseSyntaxParser` with common logic
3. Refactor `MerlinSyntaxParser` to inherit from base (pilot)
4. Refactor remaining parsers (ScmasmSyntaxParser, EdtasmSyntaxParser, etc.)
5. Add unit tests for base class
6. Update integration tests

**Estimated Effort:** 1-2 weeks

#### Related Decisions
- ADR-002: Atom-based IR (syntax parsers create atoms)

---

### ADR-005: Opcode Organization Pattern

**Status:** 📝 Proposed (Partially Implemented - 6502 uses modular pattern)  
**Date:** 2026-02-09  
**Deciders:** Architecture Team

#### Context

CPU plugin implementations currently have inconsistent opcode organization:

| CPU | Pattern | File Structure |
|-----|---------|---------------|
| 6502 | Modular | `cpu_6502.h` + `cpu_6502_branch_handler.h` |
| 6809 | Monolithic | `cpu_6809.h` (1027 lines, all-in-one) |
| Z80 | Monolithic | `cpu_z80.h` (751 lines, all-in-one) |

**Problem:**
- Inconsistent patterns confuse developers
- Large monolithic headers hard to navigate
- Opcode table changes mixed with logic changes (hard to review)

#### Decision

Adopt **modular opcode organization** (6502 pattern) for ALL CPU plugins:

```
include/xasm++/cpu/
  ├── cpu_6502.h              # Class definition only
  ├── opcodes_6502.h          # Opcode table constants
  ├── cpu_6502_branch_handler.h
  ├── cpu_6809.h              # Class definition only
  ├── opcodes_6809.h          # Opcode table constants (NEW)
  ├── cpu_z80.h               # Class definition only
  └── opcodes_z80.h           # Opcode table constants (NEW)
```

**Opcode Header Structure:**

```cpp
// opcodes_6502.h
namespace xasm {
namespace Opcodes6502 {
  // Addressing mode opcodes
  constexpr uint8_t LDA_IMMEDIATE = 0xA9;
  constexpr uint8_t LDA_ZEROPAGE = 0xA5;
  constexpr uint8_t LDA_ZEROPAGEX = 0xB5;
  constexpr uint8_t LDA_ABSOLUTE = 0xAD;
  constexpr uint8_t LDA_ABSOLUTEX = 0xBD;
  constexpr uint8_t LDA_ABSOLUTEY = 0xB9;
  constexpr uint8_t LDA_INDEXED_INDIRECT = 0xA1;
  constexpr uint8_t LDA_INDIRECT_INDEXED = 0xB1;
  // ... (rest of opcodes)
}
}
```

#### Rationale

**Why Modular:**
- ✅ **Consistency:** All CPUs use same pattern
- ✅ **Separation of concerns:** Data (opcodes) vs. logic (encoding)
- ✅ **Easier review:** Opcode table changes vs. logic changes separate
- ✅ **Better documentation:** Opcode header serves as reference
- ✅ **Smaller files:** Easier to navigate

**Why NOT monolithic:**
- ❌ Large files hard to navigate (1000+ lines)
- ❌ Opcode changes mixed with logic changes
- ❌ Inconsistent across CPUs (confusion)

#### Consequences

**Positive:**
- ✅ Consistent file organization across all CPUs
- ✅ Easier opcode table maintenance
- ✅ Clear separation of data vs. logic
- ✅ Better code review (can review opcode changes separately)

**Negative:**
- ⚠️ More files to manage (2 files per CPU instead of 1)
- ⚠️ Refactoring effort for 6809 and Z80

**Neutral:**
- 6502 already uses this pattern (no change needed)

#### Implementation Plan

1. Extract opcodes from `cpu_6809.h` to `opcodes_6809.h`
2. Extract opcodes from `cpu_z80.h` to `opcodes_z80.h`
3. Update includes in corresponding `.cpp` files
4. Verify tests still pass
5. Document pattern in coding standards

**Estimated Effort:** 2-3 days

#### Related Decisions
- ADR-001: CPU plugin architecture (affects CPU plugin structure)

---

### ADR-006: Error Handling Strategy

**Status:** 📝 Proposed (Partially Implemented - Needs Enforcement)  
**Date:** 2026-02-09  
**Deciders:** Architecture Team

#### Context

CPU plugins need to report errors (invalid operands, unsupported instructions, etc.).

**Current State:**
- Interface contract: Throw exceptions (`std::invalid_argument`, `std::out_of_range`)
- Implementation: 6502 and 6809 throw exceptions correctly
- Problem: Z80 plugin does NOT throw exceptions (interface violation)

**Requirements:**
- Consistent error handling across all plugins
- Clear error messages with context
- Type-safe error reporting
- Performance acceptable (errors are rare)

#### Decision

Use **C++ exceptions** for error reporting (as currently specified in interface):

```cpp
class CpuPlugin {
  /**
   * @throws std::invalid_argument if instruction/addressing mode not supported
   * @throws std::out_of_range if operand value out of range
   */
  virtual std::vector<uint8_t> EncodeInstruction(...) const = 0;
};
```

**All CPU plugins MUST throw appropriate exceptions:**

```cpp
std::vector<uint8_t> Cpu6502::EncodeInstruction(...) const {
  if (!HasOpcode(mnemonic)) {
    throw std::invalid_argument("Unsupported instruction: " + mnemonic);
  }
  
  if (operand > MAX_WORD_VALUE) {
    throw std::out_of_range(
        "Operand value " + std::to_string(operand) + " exceeds 16-bit range");
  }
  
  // ... encoding logic ...
}
```

#### Rationale

**Alternatives Considered:**
1. **Error codes (return std::optional<std::vector<uint8_t>>)**
   - Rejected: Verbose, easy to ignore errors, not idiomatic C++
2. **Result<T, Error> type (Rust-style)**
   - Rejected: Not idiomatic C++, requires additional template complexity
3. **Return empty vector on error**
   - Rejected: Ambiguous (empty instruction vs. error), no error message
4. **Callback error handler**
   - Rejected: More complex than needed, breaks control flow

**Why Exceptions:**
- ✅ Aligns with C++ exception model
- ✅ Clear error propagation up call stack
- ✅ Separates error handling from normal control flow
- ✅ Interface already specifies exceptions
- ✅ 6502 and 6809 already use exceptions correctly

**Performance Consideration:**
- Exceptions are **rare** in assembler (well-formed input is typical)
- Exception path performance doesn't matter (errors are exceptional)
- Normal path (no exceptions) has zero overhead

#### Consequences

**Positive:**
- ✅ Consistent error handling across all plugins
- ✅ Clear error messages with context
- ✅ Type-safe (compiler enforces exception specifications)
- ✅ Idiomatic C++ error handling

**Negative:**
- ⚠️ Requires fixing Z80 plugin (currently missing exceptions)
- ⚠️ Exception overhead on error path (acceptable - errors are rare)

**Neutral:**
- Exception safety required (RAII, smart pointers already used)

#### Implementation Plan

1. **Audit Z80 plugin for missing error checks**
2. **Add exception throws for:**
   - Unsupported instructions
   - Invalid addressing modes
   - Operands out of range
3. **Add unit tests for error conditions:**
   ```cpp
   TEST(CpuZ80, ThrowsOnInvalidOpcode) {
     CpuZ80 cpu;
     EXPECT_THROW(cpu.EncodeInstruction("INVALID", 0, ""),
                  std::invalid_argument);
   }
   ```
4. **Update documentation** with exception guarantees

**Estimated Effort:** 1-2 days

#### Related Decisions
- ADR-001: CPU plugin architecture (error handling is part of interface)

---

## Rejected Decisions

### ADR-007: Compile-Time CPU Selection (Rejected)

**Status:** ❌ Rejected  
**Date:** 2026-01-19  
**Deciders:** Architecture Team

#### Context

Early design consideration: Should CPU be selected at compile-time or runtime?

#### Decision Considered

Use C++ templates for compile-time CPU selection:

```cpp
template<typename CpuType>
class Assembler {
  CpuType cpu_;
  
  AssemblerResult Assemble() {
    auto bytes = cpu_.EncodeInstruction(/*...*/);
  }
};

// Usage:
Assembler<Cpu6502> asm6502;
Assembler<Cpu6809> asm6809;
```

#### Why Rejected

**Reasons:**
1. **No runtime selection** - Can't choose CPU at runtime (CLI argument)
2. **Separate binaries** - Would need xasm-6502, xasm-6809, etc.
3. **Increased complexity** - Template instantiation, longer compile times
4. **Inheritance impossible** - Can't have CPU family hierarchies (Cpu6502Base)
5. **Testing harder** - Can't inject mock CPU plugins

**Superseded By:** ADR-001 (Runtime polymorphism via Strategy Pattern)

---

### ADR-008: Direct Encoding Without IR (Rejected)

**Status:** ❌ Rejected  
**Date:** 2026-01-19  
**Deciders:** Architecture Team

#### Context

Early design consideration: Do we need an intermediate representation?

#### Decision Considered

Parse and encode directly in single pass:

```cpp
void Assembler::AssembleFile(const std::string& filename) {
  std::ifstream in(filename);
  std::string line;
  
  while (std::getline(in, line)) {
    if (IsLabel(line)) {
      DefineLabel(ExtractLabel(line), current_address_);
    } else if (IsInstruction(line)) {
      auto bytes = cpu_->EncodeInstruction(ParseInstruction(line));
      output_.write(bytes);  // Immediate output
      current_address_ += bytes.size();
    }
  }
}
```

#### Why Rejected

**Problems:**
1. **Forward references impossible** - Label addresses not known
2. **No branch relaxation** - Can't adjust instruction sizes
3. **No multi-pass** - Can't optimize instruction sizes
4. **No listing file** - Source not preserved
5. **Error recovery hard** - Can't continue after errors

**Superseded By:** ADR-002 (Atom-based IR)

---

## Future Decisions (Deferred)

### ADR-009: Parallel Encoding (Future)

**Status:** 🔮 Future Consideration  
**Date:** TBD

**Context:** Large programs could benefit from parallel encoding

**Options:**
1. Section-level parallelism (encode sections in parallel)
2. Instruction-level parallelism (encode instructions in parallel)

**Deferred Because:** Premature optimization - benchmark first

---

### ADR-010: LSP Support (Future)

**Status:** 🔮 Future Consideration  
**Date:** TBD

**Context:** IDE integration via Language Server Protocol

**Considerations:**
- Real-time syntax checking
- Auto-completion
- Jump-to-definition
- Hover documentation

**Deferred Because:** Phase 5+ feature, core assembler first

---

## ADR Process

### When to Create an ADR

Create an ADR when making decisions about:
- ✅ Core architecture (patterns, layers, modules)
- ✅ Major algorithms (multi-pass, branch relaxation)
- ✅ Plugin interfaces (CPU, syntax, output)
- ✅ Error handling strategies
- ✅ Performance trade-offs
- ✅ Technology selections (dependencies, build tools)

**Do NOT create ADRs for:**
- ❌ Implementation details (variable names, formatting)
- ❌ Bug fixes (unless architecture impact)
- ❌ Minor refactorings (unless pattern change)

### ADR Template

```markdown
# ADR-XXX: [Decision Title]

**Status:** [Proposed | Accepted | Rejected | Superseded]
**Date:** YYYY-MM-DD
**Deciders:** [Who participated in decision]

## Context
[What's the issue? What factors are in play? What constraints?]

## Decision
[What was decided? Be specific.]

## Rationale
[Why this decision over alternatives?]

## Alternatives Considered
[What other options were evaluated?]
- **Option A:** [Description] - Why not chosen
- **Option B:** [Description] - Why not chosen

## Consequences
**Positive:**
- [Benefit 1]
- [Benefit 2]

**Negative:**
- [Trade-off 1]
- [Trade-off 2]

**Neutral:**
- [Implication 1]

## Related Decisions
[Links to related ADRs]
```

---

## ADR Index

| ADR | Title | Status | Date |
|-----|-------|--------|------|
| ADR-001 | CPU Plugin Architecture (Strategy Pattern) | ✅ Accepted | 2026-01-19 |
| ADR-002 | Atom-Based Intermediate Representation | ✅ Accepted | 2026-01-19 |
| ADR-003 | Multi-Pass Assembly with Fast/Safe Phases | ✅ Accepted | 2026-01-19 |
| ADR-004 | Syntax Parser Interface (Template Method) | 📝 Proposed | 2026-02-09 |
| ADR-005 | Opcode Organization Pattern | 📝 Proposed | 2026-02-09 |
| ADR-006 | Error Handling Strategy | 📝 Proposed | 2026-02-09 |
| ADR-007 | Compile-Time CPU Selection | ❌ Rejected | 2026-01-19 |
| ADR-008 | Direct Encoding Without IR | ❌ Rejected | 2026-01-19 |
| ADR-009 | Parallel Encoding | 🔮 Future | TBD |
| ADR-010 | LSP Support | 🔮 Future | TBD |

---

## References

- [Architecture README](README.md) - System architecture overview
- [Plugin Architecture](plugin-architecture.md) - Plugin system details
- [Design Patterns](design-patterns.md) - Pattern catalog
- [Multi-Pass Assembly](multipass-assembly.md) - Assembly algorithm details

---

**Document Version:** 1.0  
**Last Updated:** 2026-02-14  
**Maintained By:** xasm++ Architecture Team
