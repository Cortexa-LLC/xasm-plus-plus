# xasm++ Coding Standards

**Version:** 1.0.0
**Last Updated:** 2026-02-09
**Status:** Official

---

## Table of Contents

1. [Overview](#overview)
2. [C++17/20 Language Features](#c1720-language-features)
3. [Testing Requirements](#testing-requirements)
4. [Build and Quality Requirements](#build-and-quality-requirements)
5. [Code Organization Patterns](#code-organization-patterns)
6. [Error Handling](#error-handling)
7. [Plugin Architecture Patterns](#plugin-architecture-patterns)
8. [Documentation Requirements](#documentation-requirements)

---

## Overview

This document defines the coding standards for xasm++. It complements the [Style Guide](STYLE_GUIDE.md) by focusing on language feature usage, testing requirements, and architectural patterns.

### Design Principles

- **Modern C++17/20** - Use contemporary language features appropriately
- **SOLID Principles** - Maintain clean, extensible architecture
- **Test-Driven Development** - Write tests before implementation
- **Zero Warnings** - All code must compile without warnings
- **Plugin Architecture** - Extensible design for CPUs, syntaxes, and outputs

### Related Documentation

- **[Style Guide](STYLE_GUIDE.md)** - Naming conventions, formatting, and code organization
- **[Architecture Overview](architecture/00-overview.md)** - System design and abstractions
- **[Testing Strategy](architecture/06-testing-strategy.md)** - Testing approach and patterns

---

## C++17/20 Language Features

### Standard Version

**xasm++ uses C++20** as specified in `CMakeLists.txt`:

```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

### Recommended Features

#### Smart Pointers (C++11/14)

**MANDATORY:** Use smart pointers for ownership:

```cpp
// Ownership transfer
std::unique_ptr<Cpu6502> cpu = std::make_unique<Cpu6502>();

// Shared ownership
std::shared_ptr<Atom> atom = std::make_shared<InstructionAtom>("LDA", "#$42");

// Non-owning reference (observer)
CpuPlugin* cpu_ = nullptr;  // Does not own, safe to hold as raw pointer
```

**Rules:**
- Use `std::unique_ptr` for exclusive ownership
- Use `std::shared_ptr` for shared ownership
- Use raw pointers ONLY for non-owning references
- Never use `new`/`delete` directly in modern C++ code

#### std::optional (C++17)

**Use for optional values:**

```cpp
// Opcode table with optional opcodes per addressing mode
struct OpcodeInfo {
    std::optional<uint8_t> immediate;
    std::optional<uint8_t> zero_page;
    std::optional<uint8_t> zero_page_x;
    std::optional<uint8_t> absolute;
};

// Usage
OpcodeInfo lda_opcodes{
    .immediate = 0xA9,
    .zero_page = 0xA5,
    .absolute = 0xAD
};

if (lda_opcodes.immediate.has_value()) {
    uint8_t opcode = lda_opcodes.immediate.value();
}

// Preferred: value_or with default
uint8_t opcode = lda_opcodes.immediate.value_or(0xFF);
```

**When to use:**
- Return values that may not exist (prefer over `nullptr` or error codes)
- Optional configuration values
- Addressing modes that may not be supported
- Symbol lookups that may fail

#### enum class (C++11)

**MANDATORY:** Always use `enum class`, never plain `enum`:

```cpp
// Good: Scoped enumeration
enum class AddressingMode {
    Implied,
    Accumulator,
    Immediate,
    ZeroPage,
    Absolute,
    Relative
};

// Usage requires scope
AddressingMode mode = AddressingMode::Immediate;

// Bad: Plain enum (avoid)
enum AddressingMode { Implied, Accumulator };  // ❌ NEVER DO THIS
```

**Benefits:**
- No name collisions
- Type-safe (no implicit integer conversion)
- Clear intent at usage sites

#### constexpr and consteval (C++11/20)

**Use for compile-time constants:**

```cpp
// Compile-time constants
static constexpr int FAST_PHASE_LIMIT = 50;
static constexpr int MAX_PASSES = 500;
static constexpr uint32_t DEFAULT_ORG = 0x8000;

// constexpr functions (computed at compile-time when possible)
constexpr uint16_t MakeWord(uint8_t lo, uint8_t hi) {
    return static_cast<uint16_t>(lo) | (static_cast<uint16_t>(hi) << 8);
}

// consteval (C++20) - MUST be evaluated at compile-time
consteval int CalculateTableSize(int entries) {
    return entries * 8;  // Always computed at compile-time
}
```

**Rules:**
- Use `constexpr` for compile-time constants
- Use `constexpr` for functions that CAN be evaluated at compile-time
- Use `consteval` (C++20) for functions that MUST be evaluated at compile-time
- Avoid `#define` for constants (use `constexpr` instead)

#### Structured Bindings (C++17)

**Use for tuple-like types:**

```cpp
// Map iteration
for (const auto& [name, symbol] : symbol_table) {
    std::cout << name << " = $" << std::hex << symbol.value << "\n";
}

// Multiple return values
auto [success, error_msg] = ValidateInstruction(inst);

// Pair decomposition
std::map<std::string, int> counts;
auto [iter, inserted] = counts.insert({"label", 1});
```

**When to use:**
- Iterating over maps/pairs
- Functions returning `std::pair` or `std::tuple`
- Unpacking structured data

#### if/switch with Initializers (C++17)

**Use for scoped initialization:**

```cpp
// if with initializer
if (auto it = instruction_handlers_.find(mnemonic); it != instruction_handlers_.end()) {
    return it->second(operand, mode);
}

// switch with initializer
switch (auto mode = DetermineAddressingMode(operand); mode) {
    case AddressingMode::Immediate:
        return EncodeImmediate(operand);
    case AddressingMode::ZeroPage:
        return EncodeZeroPage(operand);
}
```

**Benefits:**
- Limits variable scope
- Clearer intent
- Reduces variable lifetime

#### Range-Based For Loops (C++11)

**PREFERRED:** Use range-based loops over iterator loops:

```cpp
// Good: Range-based loop
for (const auto& section : sections_) {
    ProcessSection(section);
}

for (auto& atom : section.atoms) {
    atom->Process();
}

// Avoid: Iterator loop (verbose)
for (auto it = sections_.begin(); it != sections_.end(); ++it) {
    ProcessSection(*it);
}

// Avoid: Index-based loop (error-prone)
for (size_t i = 0; i < sections_.size(); ++i) {
    ProcessSection(sections_[i]);
}
```

**When to use index loops:**
- Need index value for logic
- Parallel iteration over multiple containers
- Performance-critical code with measured benefit

#### auto Keyword (C++11)

**Use judiciously:**

```cpp
// Good: Type is obvious or verbose
auto it = instruction_handlers_.find(mnemonic);
auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
auto result = assembler.Assemble();

// Good: Long iterator types
for (auto it = map.begin(); it != map.end(); ++it) { ... }

// Avoid: Type clarity is important
std::vector<uint8_t> bytes;  // NOT: auto bytes = std::vector<uint8_t>{};
uint16_t address = 0x8000;   // NOT: auto address = 0x8000; (unclear type)
int count = 0;               // NOT: auto count = 0; (could be int, long, size_t?)
```

**Rules:**
- Use `auto` when type is obvious from right-hand side
- Use `auto` to avoid repeating long types
- Avoid `auto` when type clarity matters for readability
- Always use `auto` with `std::make_unique`/`std::make_shared`

#### Lambda Expressions (C++11/14/20)

**Use for callbacks and algorithms:**

```cpp
// Function object (callback)
using InstructionHandler = std::function<std::vector<uint8_t>(
    CpuPlugin*, uint16_t, AddressingMode)>;

instruction_handlers_["LDA"] = [](CpuPlugin* cpu, uint16_t operand, AddressingMode mode) {
    return cpu->EncodeLDA(operand, mode);
};

// Algorithm with lambda
std::sort(atoms.begin(), atoms.end(), [](const auto& a, const auto& b) {
    return a->address < b->address;
});

// Generic lambda (C++14)
auto encode = [](auto operand, auto mode) {
    return EncodeInstruction(operand, mode);
};

// Init capture (C++14)
auto processor = [symbols = symbols_](const std::string& name) {
    return symbols->Lookup(name);
};
```

**When to use:**
- Short callback functions
- Algorithm customization (sort, find_if, etc.)
- Closures with captured state

#### Designated Initializers (C++20)

**Use for struct initialization:**

```cpp
// C++20 designated initializers
struct AssemblerResult {
    bool success = true;
    int pass_count = 0;
    std::vector<AssemblerError> errors;
};

// Clear field initialization
AssemblerResult result{
    .success = false,
    .pass_count = 5,
    .errors = {error1, error2}
};
```

**Benefits:**
- Self-documenting initialization
- Order-independent (fields can be in any order)
- Robust to struct changes

#### Concepts (C++20) - Future Use

**Not currently used, but available:**

```cpp
// Define concept for CPU plugin
template<typename T>
concept CpuPluginType = requires(T cpu, uint32_t operand, AddressingMode mode) {
    { cpu.GetName() } -> std::convertible_to<std::string>;
    { cpu.EncodeInstruction(operand, mode) } -> std::same_as<std::vector<uint8_t>>;
};

// Use concept in template
template<CpuPluginType T>
class Assembler { ... };
```

**When to consider:**
- Type constraints for templates
- Documenting template requirements
- Improved error messages

### Features to Avoid

#### std::variant (Use Inheritance Instead)

**AVOID:** For this codebase, prefer inheritance and polymorphism:

```cpp
// ❌ Avoid: std::variant for atoms
using AtomVariant = std::variant<LabelAtom, InstructionAtom, DataAtom>;

// ✅ Preferred: Inheritance with shared_ptr
std::vector<std::shared_ptr<Atom>> atoms;
atoms.push_back(std::make_shared<LabelAtom>("start", 0x8000));
atoms.push_back(std::make_shared<InstructionAtom>("LDA", "#$42"));
```

**Rationale:**
- Plugin architecture benefits from polymorphism
- Virtual dispatch is clearer than `std::visit`
- Performance difference negligible for assembler use case

#### std::string_view (Not Needed Yet)

**Currently not used:**

```cpp
// Not currently needed, but acceptable for future use
void ProcessLabel(std::string_view label);
```

**Rationale:**
- Adds complexity without measured benefit
- String copies are not a bottleneck
- Consider when profiling shows string copy overhead

---

## Testing Requirements

### Test-Driven Development (TDD)

**MANDATORY:** All code must be developed using TDD:

```
1. RED:    Write a failing test
2. GREEN:  Write minimal code to pass
3. REFACTOR: Clean up code while keeping tests green
```

**Example TDD cycle:**

```cpp
// Step 1: RED - Write failing test
TEST(Cpu6502Test, EncodeLDA_Immediate) {
    Cpu6502 cpu;
    auto bytes = cpu.EncodeLDA(0x42, AddressingMode::Immediate);
    
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xA9);  // Opcode
    EXPECT_EQ(bytes[1], 0x42);  // Operand
}

// Step 2: GREEN - Implement minimal code
std::vector<uint8_t> Cpu6502::EncodeLDA(uint32_t operand, AddressingMode mode) {
    if (mode == AddressingMode::Immediate) {
        return {0xA9, static_cast<uint8_t>(operand)};
    }
    return {};
}

// Step 3: REFACTOR - Clean up (extract constants, improve design)
std::vector<uint8_t> Cpu6502::EncodeLDA(uint32_t operand, AddressingMode mode) {
    constexpr uint8_t LDA_IMMEDIATE = 0xA9;
    
    if (mode == AddressingMode::Immediate) {
        return {LDA_IMMEDIATE, static_cast<uint8_t>(operand)};
    }
    
    throw std::invalid_argument("Unsupported addressing mode for LDA");
}
```

### Test Framework

**Google Test** is the testing framework:

```cpp
#include <gtest/gtest.h>

// Test naming: TEST(TestSuiteName, TestName)
TEST(Cpu6502Test, Creation) {
    Cpu6502 cpu;
    EXPECT_EQ(cpu.GetName(), "6502");
}

TEST(Cpu6502Test, EncodeLDA_ZeroPage) {
    // Arrange
    Cpu6502 cpu;
    uint32_t operand = 0x80;
    AddressingMode mode = AddressingMode::ZeroPage;
    
    // Act
    auto bytes = cpu.EncodeLDA(operand, mode);
    
    // Assert
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xA5);  // LDA zero page opcode
    EXPECT_EQ(bytes[1], 0x80);  // Operand
}
```

### Test Organization

**Test Suite Naming:**
```
TEST(ComponentTest, FeatureDescription)
```

**Examples:**
- `TEST(AssemblerTest, Creation)`
- `TEST(Cpu6502Test, EncodeLDA_Immediate)`
- `TEST(SymbolTableTest, DefineAndLookup)`

**Test Structure (Arrange-Act-Assert):**

```cpp
TEST(AssemblerTest, MultiPassConvergence) {
    // Arrange - Set up test data
    Assembler assembler;
    Cpu6502 cpu;
    ConcreteSymbolTable symbols;
    assembler.SetCpuPlugin(&cpu);
    assembler.SetSymbolTable(&symbols);
    
    Section section(".text", 0x01, 0x8000);
    section.atoms.push_back(std::make_shared<LabelAtom>("loop", 0));
    section.atoms.push_back(std::make_shared<InstructionAtom>("INX", ""));
    section.atoms.push_back(std::make_shared<InstructionAtom>("BNE", "loop"));
    assembler.AddSection(section);
    
    // Act - Execute code under test
    AssemblerResult result = assembler.Assemble();
    
    // Assert - Verify expectations
    EXPECT_TRUE(result.success);
    EXPECT_GE(result.pass_count, 1);
    EXPECT_LE(result.pass_count, 10);
}
```

### Coverage Requirements

**Minimum Coverage Targets:**
- **Overall:** 85%+ line coverage
- **Core Engine:** 90%+ (assembler, symbol table, section)
- **CPU Plugins:** 85%+ per plugin
- **Utility Functions:** 80%+

**Measuring Coverage:**

```bash
# Build with coverage flags
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=--coverage ..
make

# Run tests
./tests/unit/test_assembler

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

**Coverage Tools:**
- **lcov/gcov** - Line coverage
- **SonarQube** - Comprehensive code quality metrics

### Test Types

#### Unit Tests

**Test individual components in isolation:**

```cpp
// tests/unit/test_cpu6502.cpp
TEST(Cpu6502Test, EncodeLDA_Immediate) {
    Cpu6502 cpu;  // Component under test
    
    auto bytes = cpu.EncodeLDA(0x42, AddressingMode::Immediate);
    
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xA9);
    EXPECT_EQ(bytes[1], 0x42);
}
```

**Characteristics:**
- Fast (milliseconds)
- No external dependencies
- Test one component
- Mock/stub dependencies

#### Integration Tests

**Test interaction between components:**

```cpp
// tests/integration/test_assembler_integration.cpp
TEST(AssemblerIntegrationTest, AssembleWithCpuPlugin) {
    // Multiple components working together
    Assembler assembler;
    Cpu6502 cpu;
    ConcreteSymbolTable symbols;
    
    assembler.SetCpuPlugin(&cpu);  // Integration point
    assembler.SetSymbolTable(&symbols);  // Integration point
    
    Section section(".text", 0x01, 0x8000);
    section.atoms.push_back(std::make_shared<InstructionAtom>("LDA", "#$42"));
    assembler.AddSection(section);
    
    AssemblerResult result = assembler.Assemble();
    
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.pass_count, 0);
}
```

**Characteristics:**
- Slower than unit tests
- Tests component interactions
- Real dependencies (no mocks)
- Validates integration points

#### End-to-End Tests

**Test complete workflows:**

```cpp
// tests/e2e/test_assembler_e2e.cpp
TEST(AssemblerE2ETest, CompleteAssemblyWorkflow) {
    // Complete workflow from source to binary
    Assembler assembler;
    Cpu6502 cpu;
    ConcreteSymbolTable symbols;
    
    // Parse source code (future)
    // Section section = ParseSource("program.asm");
    
    // Assemble
    assembler.SetCpuPlugin(&cpu);
    assembler.SetSymbolTable(&symbols);
    assembler.AddSection(section);
    
    AssemblerResult result = assembler.Assemble();
    
    // Generate output (future)
    // WriteBinary("program.bin", result.sections);
    
    EXPECT_TRUE(result.success);
}
```

**Characteristics:**
- Slowest tests
- Test entire system
- Real file I/O, real data
- User-facing scenarios

---

## Build and Quality Requirements

### Zero Warnings Policy

**MANDATORY:** Code must compile with zero warnings:

```bash
# Build with warnings as errors
cmake -DCMAKE_CXX_FLAGS="-Wall -Wextra -Werror" ..
make
```

**Required Warning Flags:**
- `-Wall` - Enable all common warnings
- `-Wextra` - Enable extra warnings
- `-Werror` - Treat warnings as errors

**Expected Output:**
```
[100%] Building CXX object src/CMakeFiles/xasm++.dir/assembler.cpp.o
[100%] Linking CXX executable xasm++
[100%] Built target xasm++
```

**No warnings allowed:**
```
# ❌ UNACCEPTABLE
warning: unused variable 'count' [-Wunused-variable]
warning: comparison between signed and unsigned [-Wsign-compare]
```

### Static Analysis

**clang-tidy Configuration:**

The project includes `.clang-tidy` configuration:

```yaml
Checks: >
  clang-diagnostic-*,
  clang-analyzer-*,
  bugprone-*,
  modernize-*,
  performance-*,
  readability-*,
  cppcoreguidelines-*
```

**Running clang-tidy:**

```bash
# Generate compile commands
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# Run clang-tidy
clang-tidy src/*.cpp -- -std=c++20
```

**Expected Output:**
```
0 warnings generated.
```

### Code Quality Metrics

**Target Metrics:**
- **Quality Grade:** A or better (SonarQube)
- **Code Duplication:** <5%
- **Cognitive Complexity:** <25 per function
- **Test Coverage:** 85%+
- **Compiler Warnings:** Zero
- **Technical Debt:** <5% debt ratio

**SonarQube Analysis:**

```bash
# Start SonarQube server (one-time setup)
cd .ai-pack
python3 scripts/setup-sonarqube.py

# Run analysis
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
sonar-scanner

# View results at http://localhost:9000
```

### Build System Requirements

**CMake Configuration:**

```cmake
# Minimum CMake version
cmake_minimum_required(VERSION 3.20)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

# Build types
set(CMAKE_BUILD_TYPE Debug)  # or Release
```

**Build Commands:**

```bash
# Configure
cmake -B build

# Build
cmake --build build -j8

# Run tests
cd build && ctest

# Install (future)
cmake --install build
```

---

## Code Organization Patterns

### Plugin Architecture

**Three-Layer Design:**

```
┌─────────────────┐
│ Syntax Plugins  │  Parse source syntax (SCMASM, Merlin, EDTASM)
└────────┬────────┘
         │
┌────────▼────────┐
│ Assembler Core  │  Multi-pass resolution, symbol table, atoms
└────────┬────────┘
         │
┌────────▼────────┐
│  CPU Plugins    │  Instruction encoding (6502, 6809, Z80, 68000)
└────────┬────────┘
         │
┌────────▼────────┐
│ Output Plugins  │  Binary output (raw, Intel HEX, S-record)
└─────────────────┘
```

### Plugin Interface Pattern

**All plugins implement abstract interfaces:**

```cpp
// Abstract base class (interface)
class CpuPlugin {
public:
    virtual ~CpuPlugin() = default;
    
    virtual std::string GetName() const = 0;
    virtual std::vector<uint8_t> EncodeInstruction(
        const std::string& mnemonic,
        uint32_t operand,
        AddressingMode mode) = 0;
};

// Concrete implementation
class Cpu6502 : public CpuPlugin {
public:
    std::string GetName() const override { return "6502"; }
    
    std::vector<uint8_t> EncodeInstruction(
        const std::string& mnemonic,
        uint32_t operand,
        AddressingMode mode) override;
        
private:
    std::unordered_map<std::string, OpcodeInfo> opcode_table_;
};
```

**Benefits:**
- Extensible (add new CPUs without modifying core)
- Testable (mock plugins in tests)
- SOLID (Open-Closed Principle)

### Atom Pattern

**Intermediate representation:**

```cpp
// Base atom class
class Atom {
public:
    AtomType type;
    uint32_t address = 0;
    
    virtual ~Atom() = default;
    virtual size_t GetSize() const = 0;
};

// Derived atoms
class LabelAtom : public Atom {
public:
    LabelAtom(const std::string& name, uint32_t addr);
    size_t GetSize() const override { return 0; }
    
private:
    std::string name_;
};

class InstructionAtom : public Atom {
public:
    InstructionAtom(const std::string& mnemonic, const std::string& operand);
    size_t GetSize() const override { return encoded_bytes.size(); }
    
    std::vector<uint8_t> encoded_bytes;
private:
    std::string mnemonic_;
    std::string operand_;
};
```

**Benefits:**
- Uniform representation of source code
- Supports multi-pass assembly
- Extensible for new atom types

### Section Organization

**Sections group related atoms:**

```cpp
struct Section {
    std::string name;           // ".text", ".data", ".bss"
    uint32_t attributes;        // Code, Data, BSS flags
    uint32_t org_address;       // Origin address
    std::vector<std::shared_ptr<Atom>> atoms;
};

// Usage
Section code_section(".text", 
    static_cast<uint32_t>(SectionAttributes::Code), 
    0x8000);
    
code_section.atoms.push_back(std::make_shared<LabelAtom>("start", 0x8000));
code_section.atoms.push_back(std::make_shared<InstructionAtom>("LDA", "#$42"));
```

**Benefits:**
- Organize code by memory region
- Support multiple sections per file
- Different origin addresses per section

---

## Error Handling

### Exception Policy

**Use exceptions for error handling:**

```cpp
// Throw for invalid input
std::vector<uint8_t> Cpu6502::EncodeLDA(uint32_t operand, AddressingMode mode) {
    if (mode == AddressingMode::Indirect) {
        throw std::invalid_argument("LDA does not support indirect addressing");
    }
    
    // Encode instruction
}

// Catch at appropriate level
try {
    auto bytes = cpu.EncodeLDA(operand, mode);
} catch (const std::invalid_argument& e) {
    result.success = false;
    result.errors.push_back({e.what(), location});
}
```

### Error Result Pattern

**For expected errors, use result structures:**

```cpp
struct AssemblerResult {
    bool success = true;
    int pass_count = 0;
    std::vector<AssemblerError> errors;
};

struct AssemblerError {
    std::string message;
    SourceLocation location;
};

// Usage
AssemblerResult result = assembler.Assemble();

if (!result.success) {
    for (const auto& error : result.errors) {
        std::cerr << error.location.filename << ":" 
                  << error.location.line << ": "
                  << error.message << "\n";
    }
}
```

**When to use:**
- Expected errors (syntax errors, undefined symbols)
- Multiple errors to collect
- User-facing error messages

### Error Messages

**Clear, actionable error messages:**

```cpp
// ❌ Bad: Cryptic error
throw std::runtime_error("Error");

// ❌ Bad: Technical jargon
throw std::runtime_error("Nullptr dereference in opcode lookup");

// ✅ Good: Clear, actionable
throw std::invalid_argument(
    "Undefined symbol 'loop' at line 42. "
    "Did you forget to define the label?"
);

// ✅ Good: Specific location
result.errors.push_back({
    "Branch target 'start' is out of range (-128 to +127 bytes)",
    {filename, line, column}
});
```

---

## Plugin Architecture Patterns

### CPU Plugin Pattern

**Implement CpuPlugin interface:**

```cpp
class Cpu6809 : public CpuPlugin {
public:
    Cpu6809();
    
    // Interface methods
    std::string GetName() const override { return "6809"; }
    
    std::vector<uint8_t> EncodeInstruction(
        const std::string& mnemonic,
        uint32_t operand,
        AddressingMode mode) override;
        
private:
    void InitializeOpcodeTable();
    
    std::unordered_map<std::string, OpcodeInfo> opcode_table_;
};
```

**Register in CpuRegistry (future):**

```cpp
// In plugin initialization
CpuRegistry::Register("6809", []() {
    return std::make_unique<Cpu6809>();
});
```

### Opcode Table Pattern

**Data-driven instruction encoding:**

```cpp
struct OpcodeInfo {
    std::optional<uint8_t> immediate;
    std::optional<uint8_t> zero_page;
    std::optional<uint8_t> zero_page_x;
    std::optional<uint8_t> absolute;
    std::optional<uint8_t> absolute_x;
    // ... more addressing modes
};

class Cpu6502 : public CpuPlugin {
private:
    void InitializeOpcodeTable() {
        opcode_table_["LDA"] = OpcodeInfo{
            .immediate = 0xA9,
            .zero_page = 0xA5,
            .zero_page_x = 0xB5,
            .absolute = 0xAD,
            .absolute_x = 0xBD,
            .absolute_y = 0xB9
        };
        
        opcode_table_["STA"] = OpcodeInfo{
            .zero_page = 0x85,
            .zero_page_x = 0x95,
            .absolute = 0x8D,
            .absolute_x = 0x9D,
            .absolute_y = 0x99
        };
    }
    
    std::unordered_map<std::string, OpcodeInfo> opcode_table_;
};
```

**Benefits:**
- Easy to add new instructions
- Self-documenting (opcodes visible in table)
- Data-driven (no complex branching)

### Strategy Pattern

**Encapsulate algorithms:**

```cpp
// Strategy interface
class BranchRelaxationStrategy {
public:
    virtual ~BranchRelaxationStrategy() = default;
    virtual bool ShouldRelax(int distance) const = 0;
    virtual std::vector<uint8_t> EncodeRelaxed(uint32_t target) const = 0;
};

// Concrete strategies
class ShortBranchStrategy : public BranchRelaxationStrategy {
public:
    bool ShouldRelax(int distance) const override {
        return distance < -128 || distance > 127;
    }
    
    std::vector<uint8_t> EncodeRelaxed(uint32_t target) const override {
        // Expand to long branch
    }
};
```

---

## Documentation Requirements

### File Headers

**Every file must have a header:**

```cpp
/**
 * @file assembler.h
 * @brief Multi-pass assembler engine for xasm++
 *
 * This file contains the core assembler engine that handles multi-pass
 * assembly with symbol resolution and instruction encoding.
 *
 * @note Phase 1: Minimal Viable Assembler - Assembler Engine
 */
```

### Class Documentation

**Doxygen-style comments for public APIs:**

```cpp
/**
 * @brief Multi-pass assembler engine
 *
 * The Assembler class is the core component that performs multi-pass assembly.
 * It coordinates symbol resolution, instruction encoding, and convergence
 * checking to handle forward references and branch relaxation.
 *
 * @par Multi-Pass Assembly
 * The assembler uses multiple passes to resolve:
 * - Forward references (labels defined after use)
 * - Branch relaxation (short branches that need to expand to long form)
 * - Instruction sizing (sizes unknown until labels are resolved)
 *
 * @par Usage Example
 * @code
 * Assembler asm;
 * Cpu6502 cpu;
 * asm.SetCpuPlugin(&cpu);
 * // ... add sections
 * AssemblerResult result = asm.Assemble();
 * @endcode
 */
class Assembler { ... };
```

### Method Documentation

**Document public methods:**

```cpp
/**
 * @brief Encode a LDA instruction with specified addressing mode
 * 
 * @param operand The operand value (address or immediate value)
 * @param mode The addressing mode to use
 * @return Vector of encoded bytes (1-3 bytes depending on mode)
 * @throws std::invalid_argument if mode is not supported for LDA
 */
std::vector<uint8_t> EncodeLDA(uint32_t operand, AddressingMode mode) const;
```

### Inline Comments

**Explain complex logic:**

```cpp
// Step 1: Resolve symbols in this pass
// Labels must be resolved before encoding instructions so that
// symbol references can be looked up during instruction encoding
ResolveSymbols(section.atoms, symbols, section.org_address, result);

// Step 2: Encode all instructions
// Returns sizes for convergence checking
std::vector<size_t> sizes = EncodeInstructions(symbols, result);

// Step 3: Check convergence
// If all instruction sizes match previous pass, we're done
if (CheckConvergence(previous_sizes, sizes)) {
    break;  // Converged, assembly complete
}
```

### TODO/FIXME Comments

**Track future work:**

```cpp
// TODO: Add support for 65C02 indirect zero page mode
// TODO(bryanw): Implement macro expansion in Phase 4

// FIXME: Branch relaxation needs to handle forward references properly
// FIXME: Optimize multi-pass convergence checking (currently O(n²))

// NOTE: This assumes little-endian byte order
// NOTE: 6502 branch instructions are always relative
```

---

## References

- **[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)** - Modern C++ best practices
- **[Google Test Documentation](https://google.github.io/googletest/)** - Testing framework
- **[SOLID Principles](https://en.wikipedia.org/wiki/SOLID)** - Object-oriented design
- **[Style Guide](STYLE_GUIDE.md)** - xasm++ naming and formatting conventions
- **[Architecture Overview](architecture/00-overview.md)** - System design

---

## Changelog

- **1.0.0** (2026-02-09) - Initial coding standards document
  - C++17/20 language features
  - Testing requirements (TDD, coverage)
  - Build and quality requirements (zero warnings)
  - Plugin architecture patterns
  - Error handling guidelines
  - Documentation requirements
