# Task Contract: Instruction Dispatch Refactoring

**Task ID:** 2026-02-09_instruction-dispatch-refactor
**Beads Task:** xasm++-ogmf
**Created:** 2026-02-09
**Priority:** P2

Replace large if/elseif chains in syntax parsers with instruction registry pattern, eliminating code smell and improving extensibility.

## Problem Statement

Current syntax parsers (especially Z80Universal) use large if/elseif chains for instruction dispatch:

```cpp
// Code smell: 200+ line if/elseif chain
if (mnemonic == "LD") {
  // Handle LD...
} else if (mnemonic == "ADD") {
  // Handle ADD...
} else if (mnemonic == "SUB") {
  // Handle SUB...
} else if (mnemonic == "AND") {
  // Handle AND...
} // ... 100+ more elseif blocks
```

This creates multiple problems:
- **Cyclomatic complexity** extremely high
- **Difficult to extend** - must modify large function to add instruction
- **Hard to test** - single massive function
- **Violates Open/Closed Principle** - not open for extension, closed for modification
- **Poor separation of concerns** - instruction logic mixed with dispatch logic

## Objective

Replace if/elseif chains with **Command Pattern** or **Instruction Registry** pattern, making instruction handling modular, testable, and extensible.

## Success Criteria

✓ Zero if/elseif chains for instruction dispatch
✓ Each instruction handled by separate handler/command
✓ Registry pattern for instruction lookup
✓ Easy to add new instructions (open for extension)
✓ All existing tests pass
✓ Cyclomatic complexity reduced by 90%+
✓ Zero behavioral changes

## Acceptance Criteria

### Phase 1: Analysis & Design (2-3 hours)
- [ ] Count current if/elseif blocks in each parser
- [ ] Document cyclomatic complexity metrics (current)
- [ ] Analyze instruction categories (load, arithmetic, branch, etc.)
- [ ] Design instruction handler interface
- [ ] Choose pattern: Command vs Registry vs Strategy
- [ ] Document migration strategy

### Phase 2: Create Instruction Handler Framework (4-6 hours)
- [ ] Define `IInstructionHandler` interface
- [ ] Create `InstructionRegistry` class
- [ ] Implement registration mechanism
- [ ] Write unit tests for framework
- [ ] Document handler pattern

### Phase 3: Migrate Instructions (Per Parser) (8-12 hours)
- [ ] Start with Z80Universal (largest, most benefit)
- [ ] Convert 5-10 instructions to handlers (pilot)
- [ ] Verify tests pass
- [ ] Convert remaining instructions
- [ ] Remove if/elseif chain
- [ ] Repeat for Merlin and SCMASM (if time permits)

### Phase 4: Verification (1-2 hours)
- [ ] All parser tests passing
- [ ] Cyclomatic complexity metrics (after)
- [ ] Zero compiler warnings
- [ ] Documentation complete

## Design Approach

### Option A: Command Pattern (Recommended)

```cpp
// Handler interface
class IInstructionHandler {
public:
  virtual ~IInstructionHandler() = default;
  virtual std::vector<uint8_t> Handle(const Operands& operands) const = 0;
};

// Registry
class InstructionRegistry {
  std::unordered_map<std::string, std::unique_ptr<IInstructionHandler>> handlers_;

public:
  void Register(const std::string& mnemonic, std::unique_ptr<IInstructionHandler> handler);
  IInstructionHandler* Lookup(const std::string& mnemonic) const;
};

// Concrete handlers
class LDHandler : public IInstructionHandler {
  CpuZ80* cpu_;
public:
  explicit LDHandler(CpuZ80* cpu) : cpu_(cpu) {}

  std::vector<uint8_t> Handle(const Operands& operands) const override {
    // LD-specific logic here
    if (operands.dest == "A" && operands.src.is_immediate) {
      return cpu_->EncodeLD_A_n(operands.src.value);
    }
    // ... more LD variants
  }
};

// Usage in parser
class Z80UniversalSyntaxParser {
  InstructionRegistry registry_;

  void InitializeRegistry() {
    registry_.Register("LD", std::make_unique<LDHandler>(&cpu_));
    registry_.Register("ADD", std::make_unique<ADDHandler>(&cpu_));
    // ... register all instructions
  }

  std::vector<uint8_t> AssembleInstruction(const std::string& mnemonic, const Operands& operands) {
    auto* handler = registry_.Lookup(mnemonic);
    if (!handler) {
      throw std::runtime_error("Unknown instruction: " + mnemonic);
    }
    return handler->Handle(operands);
  }
};
```

### Option B: Strategy Pattern with Lambdas (Lighter Weight)

```cpp
using InstructionHandler = std::function<std::vector<uint8_t>(const Operands&)>;

class InstructionRegistry {
  std::unordered_map<std::string, InstructionHandler> handlers_;

public:
  void Register(const std::string& mnemonic, InstructionHandler handler) {
    handlers_[mnemonic] = handler;
  }

  std::vector<uint8_t> Execute(const std::string& mnemonic, const Operands& operands) const {
    auto it = handlers_.find(mnemonic);
    if (it == handlers_.end()) {
      throw std::runtime_error("Unknown instruction: " + mnemonic);
    }
    return it->second(operands);
  }
};

// Usage
void Z80UniversalSyntaxParser::InitializeRegistry() {
  registry_.Register("LD", [this](const Operands& ops) {
    if (ops.dest == "A" && ops.src.is_immediate) {
      return cpu_.EncodeLD_A_n(ops.src.value);
    }
    // ... more LD variants
  });

  registry_.Register("ADD", [this](const Operands& ops) {
    // ADD logic
  });
}
```

### Recommended: Option B (Lambdas)

**Rationale:**
- Simpler, less boilerplate
- Still achieves modularity
- Easier to migrate incrementally
- Handlers stay close to registration (better locality)
- No need for separate handler classes

## Metrics

**Before Refactoring (Z80Universal):**
- If/elseif chain: ~200 lines
- Cyclomatic complexity: ~150+
- Single function handles all instructions
- Adding instruction: Modify large function

**After Refactoring:**
- If/elseif chain: 0 lines
- Cyclomatic complexity: ~5 (registry lookup only)
- Each instruction: Separate handler
- Adding instruction: Register new handler (1 line)

**Improvement:**
- Complexity: 150 → 5 (97% reduction)
- Testability: High (each handler testable independently)
- Extensibility: Open for extension without modification

## Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Performance overhead | Low | Registry lookup is O(1), negligible |
| Breaking existing behavior | High | Comprehensive tests, incremental migration |
| Handler complexity explosion | Medium | Keep handlers simple, focused on dispatch only |
| Migration effort underestimated | Medium | Pilot with 5-10 instructions first |

## Files to Create/Modify

**New Files:**
- `include/xasm++/syntax/instruction_registry.h`
- `src/syntax/instruction_registry.cpp` (if needed)
- `tests/unit/test_instruction_registry.cpp`

**Modified Files:**
- `include/xasm++/syntax/z80_universal_syntax.h` (add registry member)
- `src/syntax/z80_universal/z80_universal_syntax.cpp` (replace if/elseif with registry)
- Similar changes for Merlin and SCMASM (if time permits)

## Estimated Effort

- Analysis & Design: 2-3 hours
- Framework creation: 4-6 hours
- Z80Universal migration: 8-12 hours
- Verification: 1-2 hours

**Total for Z80:** 15-23 hours (2-3 days)

**Optional extensions:**
- Merlin migration: +6-8 hours
- SCMASM migration: +6-8 hours

## Dependencies

- None (can run in parallel with Phase 2 integration)
- Recommended: Do after Phase 2 completes (cleaner codebase)

## Stakeholders

- **Owner:** Orchestrator
- **Implementer:** Engineer (refactoring specialist)
- **Reviewer:** Architect + Code quality review

**Approved:** Orchestrator 2026-02-09
