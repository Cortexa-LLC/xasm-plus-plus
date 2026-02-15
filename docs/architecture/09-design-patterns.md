# Design Patterns in xasm++

**Version:** 1.0  
**Date:** 2026-02-14  
**Status:** Reference Documentation

---

## Overview

This document catalogs the design patterns used throughout the xasm++ codebase, explaining their purpose, implementation, and benefits. Understanding these patterns helps maintain consistency and guides future development.

---

## Creational Patterns

### 1. Factory Method Pattern

**Intent:** Define an interface for creating objects, but let subclasses decide which class to instantiate.

**Status:** Planned (not yet implemented)

**Future Implementation:**

```cpp
// Plugin registry with factory methods
class CpuRegistry {
public:
  using FactoryFunc = std::function<std::unique_ptr<CpuPlugin>()>;
  
  static CpuRegistry& Instance() {
    static CpuRegistry instance;
    return instance;
  }
  
  void Register(const std::string& name, FactoryFunc factory) {
    factories_[name] = factory;
  }
  
  std::unique_ptr<CpuPlugin> Create(const std::string& name) {
    auto it = factories_.find(name);
    if (it == factories_.end()) {
      throw std::runtime_error("CPU plugin not found: " + name);
    }
    return it->second();  // Call factory function
  }

private:
  std::unordered_map<std::string, FactoryFunc> factories_;
};
```

**Registration Example:**

```cpp
// Auto-register at program startup
namespace {
  struct Cpu6502Registrar {
    Cpu6502Registrar() {
      CpuRegistry::Instance().Register("6502", 
          []() { return std::make_unique<Cpu6502>(); });
    }
  };
  static Cpu6502Registrar registrar;
}
```

**Benefits:**
- ✅ Runtime plugin selection
- ✅ Decouples plugin creation from usage
- ✅ Supports plugin discovery

**Drawbacks:**
- ⚠️ Requires registration mechanism
- ⚠️ Runtime overhead (function pointer call)

---

### 2. Singleton Pattern

**Intent:** Ensure a class has only one instance and provide a global point of access.

**Implementation:** Plugin registries

```cpp
class CpuRegistry {
public:
  static CpuRegistry& Instance() {
    static CpuRegistry instance;  // Thread-safe in C++11+
    return instance;
  }

private:
  CpuRegistry() = default;  // Private constructor
  CpuRegistry(const CpuRegistry&) = delete;  // No copying
  CpuRegistry& operator=(const CpuRegistry&) = delete;
};
```

**Usage:**

```cpp
CpuRegistry::Instance().Register("6502", /* factory */);
auto cpu = CpuRegistry::Instance().Create("6502");
```

**Benefits:**
- ✅ Single global instance
- ✅ Lazy initialization
- ✅ Thread-safe (C++11 magic statics)

**Drawbacks:**
- ⚠️ Global state (testability concern)
- ⚠️ Hidden dependencies

**Mitigation:** Use dependency injection in tests

```cpp
class Assembler {
  void SetCpuPlugin(CpuPlugin* cpu);  // Inject instead of using singleton
};
```

---

## Structural Patterns

### 3. Strategy Pattern ⭐ (Core Pattern)

**Intent:** Define a family of algorithms, encapsulate each one, and make them interchangeable.

**Implementation:** CPU Plugin System

**Interface:**

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

**Concrete Strategies:**

```cpp
class Cpu6502 : public CpuPlugin {
  std::string GetCpuFamily() const override { return "6502"; }
  std::vector<uint8_t> EncodeInstruction(/*...*/) const override {
    // 6502-specific encoding
  }
};

class Cpu6809 : public CpuPlugin {
  std::string GetCpuFamily() const override { return "6809"; }
  std::vector<uint8_t> EncodeInstruction(/*...*/) const override {
    // 6809-specific encoding
  }
};
```

**Context:**

```cpp
class Assembler {
public:
  void SetCpuPlugin(CpuPlugin* cpu) {
    cpu_ = cpu;  // Inject strategy
  }
  
  AssemblerResult Assemble() {
    // Use strategy
    auto bytes = cpu_->EncodeInstruction(/*...*/);
  }

private:
  CpuPlugin* cpu_;  // Strategy reference
};
```

**Benefits:**
- ✅ Add new CPUs without modifying assembler
- ✅ Runtime algorithm selection
- ✅ Each strategy independently testable
- ✅ Open/Closed Principle: Open for extension, closed for modification

**Real-World Usage:**

```cpp
Assembler asm;
Cpu6502 cpu_6502;
Cpu6809 cpu_6809;

// Assemble for 6502
asm.SetCpuPlugin(&cpu_6502);
asm.Assemble();

// Assemble for 6809
asm.SetCpuPlugin(&cpu_6809);
asm.Assemble();
```

---

### 4. Composite Pattern

**Intent:** Compose objects into tree structures to represent part-whole hierarchies.

**Implementation:** Expression Trees

**Component:**

```cpp
class Expression {
public:
  virtual ~Expression() = default;
  virtual int64_t Evaluate(const SymbolTable& symbols) const = 0;
  virtual bool IsConstant() const = 0;
};
```

**Leaf Nodes:**

```cpp
class LiteralExpr : public Expression {
public:
  explicit LiteralExpr(int64_t val) : value_(val) {}
  
  int64_t Evaluate(const SymbolTable&) const override {
    return value_;  // Leaf - no recursion
  }
  
  bool IsConstant() const override { return true; }

private:
  int64_t value_;
};

class SymbolExpr : public Expression {
public:
  explicit SymbolExpr(const std::string& sym) : symbol_(sym) {}
  
  int64_t Evaluate(const SymbolTable& symbols) const override {
    int64_t value;
    if (!symbols.Lookup(symbol_, value)) {
      throw std::runtime_error("Undefined symbol: " + symbol_);
    }
    return value;  // Leaf - lookup only
  }
  
  bool IsConstant() const override { return false; }

private:
  std::string symbol_;
};
```

**Composite Nodes:**

```cpp
class BinaryOpExpr : public Expression {
public:
  BinaryOpExpr(BinaryOp op,
               std::shared_ptr<Expression> left,
               std::shared_ptr<Expression> right)
    : op_(op), left_(left), right_(right) {}
  
  int64_t Evaluate(const SymbolTable& symbols) const override {
    // Recursive evaluation
    int64_t lval = left_->Evaluate(symbols);
    int64_t rval = right_->Evaluate(symbols);
    
    switch (op_) {
      case BinaryOp::Add: return lval + rval;
      case BinaryOp::Subtract: return lval - rval;
      // ...
    }
  }
  
  bool IsConstant() const override {
    return left_->IsConstant() && right_->IsConstant();
  }

private:
  BinaryOp op_;
  std::shared_ptr<Expression> left_;
  std::shared_ptr<Expression> right_;
};
```

**Usage:**

```cpp
// Build expression tree for: (start + 2) * 3
auto expr = std::make_shared<BinaryOpExpr>(
  BinaryOp::Multiply,
  std::make_shared<BinaryOpExpr>(
    BinaryOp::Add,
    std::make_shared<SymbolExpr>("start"),
    std::make_shared<LiteralExpr>(2)
  ),
  std::make_shared<LiteralExpr>(3)
);

// Evaluate
int64_t result = expr->Evaluate(symbols);
```

**Benefits:**
- ✅ Uniform treatment of simple and complex expressions
- ✅ Easy to add new operators
- ✅ Recursive evaluation natural
- ✅ Tree structure explicit

**Expression Tree Visualization:**

```
         [*]
        /   \
      [+]    3
     /   \
  start   2
```

---

### 5. Facade Pattern

**Intent:** Provide a unified interface to a set of interfaces in a subsystem.

**Implementation:** Assembler Class

The `Assembler` class provides a simplified interface to the complex multi-pass assembly process:

```cpp
class Assembler {
public:
  // Simplified API
  void SetCpuPlugin(CpuPlugin* cpu);
  void SetSymbolTable(SymbolTable* symbols);
  void AddSection(const Section& section);
  AssemblerResult Assemble();  // Hides complexity!

private:
  // Complex internal subsystems
  void Pass1_CollectSymbols();
  std::vector<size_t> EncodeAllInstructions(AssemblerResult& result);
  bool CheckConvergence(const std::vector<size_t>& prev,
                        const std::vector<size_t>& curr) const;
  void UpdateSymbolAddresses();
  AssemblerResult ContinueSafeOptimization(int pass, AssemblerResult& result);
};
```

**Client Usage (Simple):**

```cpp
Assembler asm;
Cpu6502 cpu;
ConcreteSymbolTable symbols;

asm.SetCpuPlugin(&cpu);
asm.SetSymbolTable(&symbols);
asm.AddSection(code_section);

AssemblerResult result = asm.Assemble();  // All complexity hidden!

if (result.success) {
  std::cout << "Assembled in " << result.pass_count << " passes\n";
}
```

**Benefits:**
- ✅ Hides multi-pass complexity from clients
- ✅ Simple API for common use case
- ✅ Internal complexity can change without affecting clients

---

## Behavioral Patterns

### 6. Template Method Pattern

**Intent:** Define the skeleton of an algorithm in a base class, letting subclasses override specific steps.

**Status:** Proposed for Syntax Parsers

**Proposed Implementation:**

```cpp
class BaseSyntaxParser {
public:
  // Template method - defines algorithm structure
  ParsedLine ParseLine(const std::string& line) {
    std::string trimmed = TrimWhitespace(line);
    
    // Step 1: Extract comment (varies by syntax)
    std::string comment = ExtractComment(trimmed);
    
    // Step 2: Extract label (varies by syntax)
    std::string label = ExtractLabel(trimmed);
    
    // Step 3: Split mnemonic and operand (common)
    auto [mnemonic, operand] = SplitMnemonicOperand(trimmed);
    
    return ParsedLine{label, mnemonic, operand, comment};
  }

protected:
  // Hooks for subclass customization
  virtual std::string ExtractComment(std::string& line) = 0;
  virtual std::string ExtractLabel(std::string& line) = 0;
  
  // Common implementation
  std::pair<std::string, std::string> SplitMnemonicOperand(
      const std::string& line) {
    // Common logic for all syntaxes
  }
};

class MerlinSyntaxParser : public BaseSyntaxParser {
protected:
  std::string ExtractComment(std::string& line) override {
    // Merlin-specific: '*' or ';' for comments
  }
  
  std::string ExtractLabel(std::string& line) override {
    // Merlin-specific: labels at column 1
  }
};

class ScmasmSyntaxParser : public BaseSyntaxParser {
protected:
  std::string ExtractComment(std::string& line) override {
    // SCMASM-specific: ';' for comments
  }
  
  std::string ExtractLabel(std::string& line) override {
    // SCMASM-specific: labels at column 1
  }
};
```

**Benefits:**
- ✅ Reuse common logic (SplitMnemonicOperand)
- ✅ Customize specific steps (ExtractComment, ExtractLabel)
- ✅ Consistent algorithm structure across syntaxes
- ✅ Eliminates ~40% code duplication

---

### 7. Visitor Pattern (Implicit)

**Intent:** Represent an operation to be performed on elements of an object structure.

**Implementation:** Atom Processing

While not explicitly using a Visitor class, the atom processing follows the Visitor pattern conceptually:

**Atom Hierarchy (Elements):**

```cpp
class Atom { AtomType type; };
class LabelAtom : public Atom { };
class InstructionAtom : public Atom { };
class DataAtom : public Atom { };
// ...
```

**"Visitors" (Different Operations):**

```cpp
// Visitor 1: Symbol Resolution
void Assembler::ResolveSymbols(std::vector<std::shared_ptr<Atom>>& atoms) {
  for (auto& atom : atoms) {
    switch (atom->type) {
      case AtomType::Label: {
        auto label = std::static_pointer_cast<LabelAtom>(atom);
        symbols_.Define(label->name, label->address);
        break;
      }
      case AtomType::Instruction: {
        // Process instruction
        break;
      }
      // ... other cases
    }
  }
}

// Visitor 2: Encoding
void Assembler::EncodeInstructions(std::vector<std::shared_ptr<Atom>>& atoms) {
  for (auto& atom : atoms) {
    switch (atom->type) {
      case AtomType::Instruction: {
        auto inst = std::static_pointer_cast<InstructionAtom>(atom);
        inst->encoded_bytes = cpu_->EncodeInstruction(/*...*/);
        break;
      }
      // ... other cases
    }
  }
}

// Visitor 3: Output Generation
void BinaryWriter::Write(const std::vector<std::shared_ptr<Atom>>& atoms) {
  for (auto& atom : atoms) {
    switch (atom->type) {
      case AtomType::Instruction: {
        auto inst = std::static_pointer_cast<InstructionAtom>(atom);
        out.write(reinterpret_cast<const char*>(inst->encoded_bytes.data()),
                 inst->encoded_bytes.size());
        break;
      }
      case AtomType::Data: {
        auto data = std::static_pointer_cast<DataAtom>(atom);
        out.write(reinterpret_cast<const char*>(data->data.data()),
                 data->data.size());
        break;
      }
      // ... other cases
    }
  }
}
```

**Benefits:**
- ✅ Separate algorithms (symbol resolution, encoding, output) from atom classes
- ✅ Add new operations without changing atom classes
- ✅ Multiple algorithms operate on same atom structure

**Potential Explicit Visitor (Future):**

```cpp
class AtomVisitor {
public:
  virtual void Visit(LabelAtom& atom) = 0;
  virtual void Visit(InstructionAtom& atom) = 0;
  virtual void Visit(DataAtom& atom) = 0;
  // ...
};

class Atom {
public:
  virtual void Accept(AtomVisitor& visitor) = 0;
};

class InstructionAtom : public Atom {
public:
  void Accept(AtomVisitor& visitor) override {
    visitor.Visit(*this);  // Double dispatch
  }
};
```

---

### 8. Iterator Pattern (Implicit)

**Intent:** Provide a way to access elements of an aggregate sequentially without exposing its underlying representation.

**Implementation:** Atom Chain Traversal

```cpp
class Assembler {
  std::vector<Section> sections_;
  
  void ProcessAtoms() {
    for (auto& section : sections_) {
      for (auto& atom : section.atoms) {  // Iterator pattern
        // Process atom
      }
    }
  }
};
```

**C++ Standard Library:** `std::vector<T>::iterator` provides this pattern

---

### 9. Observer Pattern

**Status:** Not currently used, but could be useful for:

**Future Use Case: Progress Reporting**

```cpp
class AssemblerObserver {
public:
  virtual void OnPassStart(int pass_number) = 0;
  virtual void OnPassComplete(int pass_number, bool converged) = 0;
  virtual void OnError(const AssemblerError& error) = 0;
};

class Assembler {
public:
  void AddObserver(AssemblerObserver* observer) {
    observers_.push_back(observer);
  }
  
  AssemblerResult Assemble() {
    for (int pass = 1; pass <= MAX_PASSES; ++pass) {
      NotifyPassStart(pass);
      
      // ... assembly logic ...
      
      NotifyPassComplete(pass, converged);
    }
  }

private:
  void NotifyPassStart(int pass) {
    for (auto* obs : observers_) {
      obs->OnPassStart(pass);
    }
  }
  
  std::vector<AssemblerObserver*> observers_;
};
```

**Benefits:**
- ✅ Decouple progress reporting from assembly logic
- ✅ Multiple observers (GUI, CLI, logging)
- ✅ Easy to add/remove observers

---

## Architectural Patterns

### 10. Dependency Injection

**Intent:** Pass dependencies to objects rather than having them create dependencies.

**Implementation:** Throughout xasm++

**Examples:**

```cpp
class Assembler {
public:
  // Inject dependencies via setters
  void SetCpuPlugin(CpuPlugin* cpu) { cpu_ = cpu; }
  void SetSymbolTable(SymbolTable* symbols) { symbols_ = symbols; }

private:
  CpuPlugin* cpu_;          // Injected dependency
  SymbolTable* symbols_;    // Injected dependency
};

// Usage
Assembler asm;
Cpu6502 cpu;
ConcreteSymbolTable symbols;

asm.SetCpuPlugin(&cpu);           // Inject
asm.SetSymbolTable(&symbols);     // Inject
```

**Benefits:**
- ✅ Testability: Inject mocks in tests
- ✅ Flexibility: Swap implementations at runtime
- ✅ Decoupling: Assembler doesn't know concrete types

**Testing Example:**

```cpp
class MockCpuPlugin : public CpuPlugin {
public:
  MOCK_METHOD(std::vector<uint8_t>, EncodeInstruction, 
              (const std::string&, uint32_t, const std::string&), 
              (const, override));
};

TEST(Assembler, EncodesInstructions) {
  Assembler asm;
  MockCpuPlugin mock_cpu;
  ConcreteSymbolTable symbols;
  
  EXPECT_CALL(mock_cpu, EncodeInstruction("LDA", 0x42, "#$42"))
      .WillOnce(Return(std::vector<uint8_t>{0xA9, 0x42}));
  
  asm.SetCpuPlugin(&mock_cpu);  // Inject mock
  asm.SetSymbolTable(&symbols);
  
  // ... test assembly ...
}
```

---

### 11. Layered Architecture

**Intent:** Organize system into layers with clear dependencies (higher layers depend on lower layers, not vice versa).

**Implementation:** xasm++ Layer Structure

```
┌─────────────────────────────────┐
│   Presentation Layer            │
│   (CLI, main.cpp)               │  ← Depends on Application Layer
└─────────────┬───────────────────┘
              │
              ▼
┌─────────────────────────────────┐
│   Application Layer             │
│   (Assembler, workflows)        │  ← Depends on Domain Layer
└─────────────┬───────────────────┘
              │
              ▼
┌─────────────────────────────────┐
│   Domain Layer                  │
│   (CPU plugins, syntax plugins) │  ← Depends on Core
└─────────────┬───────────────────┘
              │
              ▼
┌─────────────────────────────────┐
│   Core Layer                    │
│   (Atom, Expression, Symbol)    │  ← No dependencies (foundation)
└─────────────────────────────────┘
```

**Dependency Rules:**
- ✅ Higher layers depend on lower layers
- ❌ Lower layers never depend on higher layers
- ✅ Core layer has no dependencies (pure abstractions)

**Benefits:**
- ✅ Clear separation of concerns
- ✅ Testable layers (test lower layers independently)
- ✅ Maintainability (changes in high layers don't affect low layers)

---

### 12. Plugin Architecture

**Intent:** Allow extending system functionality without modifying core code.

**Implementation:** CPU Plugins, Syntax Plugins, Output Plugins

**Core Characteristics:**
- Abstract interfaces (CpuPlugin, SyntaxParser, OutputPlugin)
- Runtime plugin selection
- Registry-based plugin discovery (planned)

**Benefits:**
- ✅ Extensibility without core modifications
- ✅ Third-party plugins possible
- ✅ Each plugin independently developed and tested

---

## Anti-Patterns to Avoid

### 1. God Class ⚠️

**Definition:** A class that does too much, violating Single Responsibility Principle.

**Current Issue:** CPU plugin classes (Cpu6502, Cpu6809) contain 68-97 encoding methods.

**Example:**

```cpp
class Cpu6809 {
  // 97 encoding methods - TOO MANY RESPONSIBILITIES!
  std::vector<uint8_t> EncodeABX(...);
  std::vector<uint8_t> EncodeADCA_Immediate(...);
  std::vector<uint8_t> EncodeADCA_Direct(...);
  // ... 94 more methods
};
```

**Refactoring:**

```cpp
// Option 1: Addressing Mode Handlers
class Cpu6809 {
  ImmediateAddressingHandler immediate_;
  DirectAddressingHandler direct_;
  IndexedAddressingHandler indexed_;
  
  std::vector<uint8_t> EncodeInstruction(...) {
    // Dispatch to appropriate handler
  }
};

// Option 2: Data-Driven Tables
class Cpu6809 {
  std::unordered_map<std::string, OpcodeInfo> opcode_table_;
  
  std::vector<uint8_t> EncodeInstruction(...) {
    // Table lookup instead of 97 methods
  }
};
```

---

### 2. Magic Numbers

**Definition:** Hardcoded numeric constants without named constants.

**Bad Example:**

```cpp
if (operand > 255) {  // What does 255 mean?
  throw std::out_of_range("Operand out of range");
}
```

**Good Example:**

```cpp
constexpr uint32_t MAX_BYTE_VALUE = 0xFF;

if (operand > MAX_BYTE_VALUE) {  // Clear meaning
  throw std::out_of_range("Operand exceeds byte range");
}
```

---

### 3. Long Method

**Definition:** Methods with too many lines of code, hard to understand.

**Guideline:** Keep methods under 50 lines (ideally under 30).

**Refactoring:**

```cpp
// Before: Long method
std::vector<uint8_t> EncodeInstruction(...) {
  // 100 lines of parsing logic
  // 50 lines of validation
  // 75 lines of encoding
  // Total: 225 lines - TOO LONG!
}

// After: Extract methods
std::vector<uint8_t> EncodeInstruction(...) {
  AddressingMode mode = ParseAddressingMode(operand_str);
  ValidateOperand(operand, mode);
  return EncodeWithMode(mnemonic, mode, operand);
}

AddressingMode ParseAddressingMode(const std::string& operand_str) {
  // 30 lines
}

void ValidateOperand(uint32_t operand, AddressingMode mode) {
  // 20 lines
}

std::vector<uint8_t> EncodeWithMode(...) {
  // 40 lines
}
```

---

## Pattern Selection Guidelines

### When to Use Strategy Pattern

**Use When:**
- ✅ Multiple algorithms for the same operation
- ✅ Algorithm selection at runtime
- ✅ Algorithms need to be independently testable

**Example:** CPU encoding (6502 vs. 6809 vs. Z80)

---

### When to Use Template Method Pattern

**Use When:**
- ✅ Algorithm has common structure
- ✅ Steps vary by subclass
- ✅ Want to enforce algorithm skeleton

**Example:** Syntax parsing (Merlin vs. SCMASM - same structure, different steps)

---

### When to Use Composite Pattern

**Use When:**
- ✅ Part-whole hierarchies
- ✅ Uniform treatment of simple and complex objects
- ✅ Recursive structures

**Example:** Expression trees (literal, symbol, binary ops)

---

### When to Use Facade Pattern

**Use When:**
- ✅ Simplifying complex subsystem
- ✅ Providing high-level interface
- ✅ Decoupling client from subsystem details

**Example:** Assembler class (hides multi-pass complexity)

---

## References

- **Design Patterns: Elements of Reusable Object-Oriented Software** (Gang of Four)
- **Head First Design Patterns** (Freeman & Freeman)
- **Refactoring: Improving the Design of Existing Code** (Martin Fowler)
- **Clean Architecture** (Robert C. Martin)

---

**Document Version:** 1.0  
**Last Updated:** 2026-02-14  
**Related:** [Architecture README](README.md), [Plugin Architecture](plugin-architecture.md)
