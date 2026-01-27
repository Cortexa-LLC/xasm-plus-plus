# xasm++ Coding Style Guide

**Version:** 1.0.0
**Last Updated:** 2026-01-27
**Status:** Official

---

## Table of Contents

1. [Overview](#overview)
2. [Naming Conventions](#naming-conventions)
3. [Code Organization](#code-organization)
4. [Formatting](#formatting)
5. [Comments and Documentation](#comments-and-documentation)
6. [Language Features](#language-features)
7. [Testing](#testing)
8. [Exceptions](#exceptions)

---

## Overview

This style guide establishes naming conventions and coding standards for the xasm++ project. These conventions ensure consistency, readability, and maintainability across the codebase.

### Design Philosophy

- **Clarity over cleverness** - Code should be self-documenting
- **Consistency over personal preference** - Follow established patterns
- **SOLID principles** - Maintain clean architecture
- **Modern C++17/20** - Use contemporary language features appropriately

---

## Naming Conventions

### General Principles

1. **Names should be descriptive and unambiguous**
2. **Avoid abbreviations unless widely understood** (e.g., `cpu`, `org`, `addr`)
3. **Use American English spelling** (e.g., `Analyzer` not `Analyser`)
4. **Be consistent with existing code in the same file/module**

### Files and Directories

**Headers:**
```
snake_case.h
```

**Examples:**
```
assembler.h
cpu_6502.h
parse_utils.h
binary_output.h
command_line_options.h
```

**Implementation Files:**
```
snake_case.cpp
```

**Examples:**
```
assembler.cpp
cpu_6502.cpp
parse_utils.cpp
merlin_syntax.cpp
```

**Test Files:**
```
test_<component>.cpp
```

**Examples:**
```
test_assembler.cpp
test_cpu6502.cpp
test_symbol.cpp
test_expression.cpp
```

**Directory Structure:**
```
include/xasm++/           # Public headers
include/xasm++/cpu/       # CPU plugin headers
include/xasm++/output/    # Output plugin headers
include/xasm++/syntax/    # Syntax plugin headers
include/xasm++/cli/       # CLI headers
src/core/                 # Core implementation
src/cpu/m6502/            # 6502 family implementations
src/output/               # Output plugin implementations
src/syntax/               # Syntax plugin implementations
tests/unit/               # Unit tests
tests/integration/        # Integration tests
tests/e2e/                # End-to-end tests
docs/                     # Documentation
```

### Classes and Structs

**PascalCase** for all class and struct names:

```cpp
class Assembler;
class Cpu6502;
class BinaryOutput;
class ConcreteSymbolTable;
struct AssemblerResult;
struct AssemblerError;
struct SourceLocation;
enum class AtomType;
enum class AddressingMode;
```

**Rules:**
- Use nouns or noun phrases
- Classes are singular (e.g., `Section`, not `Sections`)
- Avoid prefixes like `C` or `I` for classes/interfaces
- Use `Abstract` prefix only for base classes that cannot be instantiated
- Use `Concrete` prefix when disambiguating from abstract base class

### Functions and Methods

**PascalCase** for all methods (public, private, and protected):

```cpp
// Public methods
void AddSection(const Section& section);
size_t GetSectionCount() const;
AssemblerResult Assemble();
void SetCpuPlugin(Cpu6502* cpu);
std::vector<uint8_t> EncodeLDA(uint32_t operand, AddressingMode mode) const;
bool IsDefined(const std::string& name) const;

// Private methods
void InitializeInstructionHandlers();
std::vector<size_t> EncodeInstructions();
bool CheckConvergence();
void ResolveSymbols();
```

**Rules:**
- Use verb phrases for methods that perform actions
- Use `Get`/`Set` for accessors/mutators
- Use `Is`/`Has`/`Can` for boolean predicates
- Avoid redundant prefixes (e.g., prefer `Size()` over `GetSize()` if clearly a getter)
- **All methods use PascalCase regardless of visibility** (consistent across entire codebase)

### Variables

**snake_case** for local variables and function parameters:

```cpp
int pass_count = 0;
uint32_t current_address = 0x8000;
std::string label_name;
AddressingMode mode = AddressingMode::Absolute;
const Section& section;  // parameter
```

**snake_case with trailing underscore** for private member variables:

```cpp
class Assembler {
private:
    std::vector<Section> sections_;
    Cpu6502* cpu_ = nullptr;
    SymbolTable* symbols_ = nullptr;
    std::unordered_map<std::string, InstructionHandler> instruction_handlers_;
    CpuMode cpu_mode_ = CpuMode::Cpu6502;
    bool m_flag_ = true;
    bool x_flag_ = true;
};
```

**Rules:**
- Member variables MUST have trailing underscore `_`
- No Hungarian notation (no `m_`, `p_` prefixes)
- Use descriptive names, avoid single letters except for loop counters
- Initialize member variables at declaration when possible

### Constants and Enums

**SCREAMING_SNAKE_CASE** for compile-time constants:

```cpp
static constexpr int FAST_PHASE_LIMIT = 50;
static constexpr int MAX_PASSES = 500;
const uint32_t DEFAULT_ORG = 0x8000;
```

**PascalCase** for enum classes and their values:

```cpp
enum class AtomType {
    Label,
    Instruction,
    Data,
    Space,
    Align,
    Org
};

enum class AddressingMode {
    Implied,
    Accumulator,
    Immediate,
    ZeroPage,
    ZeroPageX,
    ZeroPageY,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    Indirect,
    IndirectX,
    IndirectY,
    Relative
};

enum class CpuMode {
    Cpu6502,
    Cpu65C02,
    Cpu65C02Rock,
    Cpu65816
};
```

**Rules:**
- Always use `enum class` (never plain `enum`)
- Enum values use PascalCase
- Avoid ALL_CAPS for enum values (reserved for macros/constants)

### Namespaces

**snake_case** for namespace names:

```cpp
namespace xasm {
    // All project code
}

namespace xasm::cpu {
    // CPU plugins (alternative organization)
}
```

**Rules:**
- Project uses single top-level namespace: `xasm`
- Avoid deeply nested namespaces
- Never use `using namespace` in headers
- Prefer explicit qualification in implementation files

### Type Aliases

**PascalCase** for type aliases:

```cpp
using InstructionHandler = std::function<std::vector<uint8_t>(Cpu6502*, uint16_t, AddressingMode)>;
using SymbolMap = std::unordered_map<std::string, Symbol>;
```

### Macros (Avoid)

**SCREAMING_SNAKE_CASE** if absolutely necessary:

```cpp
#define XASM_VERSION_MAJOR 0
#define XASM_VERSION_MINOR 2
#define XASM_STRINGIFY(x) #x
```

**Rules:**
- Avoid macros whenever possible
- Prefer `constexpr` and `inline` functions
- Use include guards (not `#pragma once` for maximum portability)
- Prefix project-specific macros with `XASM_`

---

## Code Organization

### Header Structure

```cpp
// Brief description
// Phase information (if applicable)

#pragma once

// System includes (alphabetical)
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Project includes (alphabetical)
#include "xasm++/atom.h"
#include "xasm++/section.h"

namespace xasm {

// Forward declarations
class SymbolTable;

// Type aliases
using InstructionHandler = std::function<...>;

// Constants
constexpr int MAX_PASSES = 500;

// Enums
enum class AtomType { ... };

// Structs (simple data)
struct SourceLocation { ... };

// Classes
class Assembler {
public:
    // Public types
    
    // Constructors/Destructor
    Assembler();
    virtual ~Assembler() = default;
    
    // Public methods (grouped by functionality)
    void SetCpuPlugin(Cpu6502* cpu);
    void AddSection(const Section& section);
    AssemblerResult Assemble();
    
    // Accessors
    size_t GetSectionCount() const;

private:
    // Private methods
    void initialize_handlers();
    
    // Private member variables
    std::vector<Section> sections_;
    Cpu6502* cpu_ = nullptr;
};

} // namespace xasm
```

### Implementation File Structure

```cpp
// Implementation file comment

#include "xasm++/assembler.h"

// Additional includes
#include "xasm++/symbol.h"
#include <algorithm>
#include <sstream>

namespace xasm {

// Anonymous namespace for file-local helpers
namespace {

// Helper: Parse hex value ($1234 or $42)
static uint32_t ParseHex(const std::string& str) {
    // Implementation
}

// Helper: Trim whitespace
static std::string Trim(const std::string& str) {
    // Implementation
}

} // anonymous namespace

// Class implementation
Assembler::Assembler() {
    InitializeInstructionHandlers();
}

void Assembler::AddSection(const Section& section) {
    sections_.push_back(section);
}

// ... more methods

} // namespace xasm
```

---

## Formatting

### Indentation and Spacing

- **Use 4 spaces for indentation** (no tabs)
- **Maximum line length: 100 characters** (soft limit, can exceed for readability)
- **One statement per line**
- **Blank line between function definitions**

### Braces

**K&R style** (opening brace on same line, except for functions):

```cpp
// Functions: Opening brace on new line
void Function() {
    // body
}

// Classes: Opening brace on new line
class MyClass {
public:
    // members
};

// Control flow: Opening brace on same line
if (condition) {
    // body
} else {
    // body
}

for (int i = 0; i < n; ++i) {
    // body
}

while (condition) {
    // body
}

// Always use braces, even for single statements
if (condition) {
    single_statement();
}
```

### Pointer and Reference Alignment

Attach `*` and `&` to the **type** (left-aligned):

```cpp
int* pointer;
const std::string& reference;
Cpu6502* cpu_;
std::shared_ptr<Atom> atom;
```

**NOT:**
```cpp
int *pointer;    // Wrong
int * pointer;   // Wrong
int& reference;  // Wrong (inconsistent with above)
```

### Function Declarations

```cpp
// Short functions on one line
void SetValue(int val);
bool IsValid() const;

// Long signatures break after return type
std::vector<uint8_t> EncodeLDA(
    uint32_t operand,
    AddressingMode mode
) const;

// Very long signatures break at parameters
bool CheckConvergence(
    const std::vector<size_t>& previous_sizes,
    const std::vector<size_t>& current_sizes
) const;
```

### Initialization

Prefer **uniform initialization** with braces:

```cpp
// Preferred
std::vector<int> vec{1, 2, 3};
MyClass obj{arg1, arg2};
int value{42};

// Member initialization in constructor
Assembler::Assembler()
    : sections_(),
      cpu_(nullptr),
      symbols_(nullptr) {
    InitializeInstructionHandlers();
}
```

---

## Comments and Documentation

### File Headers

```cpp
// Brief description of file purpose
// Phase information (e.g., Phase 1: Minimal Viable Assembler)
```

### Function Comments

Use **Doxygen-style comments** for public APIs:

```cpp
/// @brief Encode a LDA instruction with specified addressing mode
/// @param operand The operand value (address or immediate value)
/// @param mode The addressing mode to use
/// @return Vector of encoded bytes (1-3 bytes depending on mode)
/// @throws std::invalid_argument if mode is not supported for LDA
std::vector<uint8_t> EncodeLDA(uint32_t operand, AddressingMode mode) const;
```

### Inline Comments

```cpp
// Use single-line comments for brief explanations
int pass = 0;  // Pass counter (0-indexed)

// Multi-line block comments for complex logic
// Step 1: Parse operand to determine addressing mode
// Step 2: Extract value from operand string
// Step 3: Encode using CPU plugin
```

### TODO/FIXME/NOTE

```cpp
// TODO: Add support for 65C02 indirect zero page mode
// FIXME: Branch relaxation needs to handle forward references
// NOTE: This assumes little-endian byte order
```

---

## Language Features

### Auto Keyword

Use `auto` when type is obvious or verbose:

```cpp
// Good uses of auto
auto it = instruction_handlers_.find(mnemonic);
auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
auto result = assembler.Assemble();

// Avoid auto when type clarity is important
std::vector<uint8_t> bytes;  // NOT: auto bytes = std::vector<uint8_t>{};
uint16_t address = 0x8000;   // NOT: auto address = 0x8000; (unclear type)
```

### Smart Pointers

Prefer **smart pointers** over raw pointers for ownership:

```cpp
// Ownership transfer
std::unique_ptr<Cpu6502> cpu = std::make_unique<Cpu6502>();

// Shared ownership
std::shared_ptr<Atom> atom = std::make_shared<LabelAtom>("start", 0x8000);

// Non-owning reference (observer)
Cpu6502* cpu_ = nullptr;  // Does not own, safe to hold
```

### Const Correctness

Mark methods and parameters `const` whenever possible:

```cpp
// Const methods (don't modify object state)
size_t GetSectionCount() const;
bool IsDefined(const std::string& name) const;

// Const parameters (not modified)
void AddSection(const Section& section);
void SetName(const std::string& name);  // Pass by const ref for strings

// Const member variables (initialized in constructor)
const int MAX_SIZE = 100;
```

### Range-Based For Loops

Prefer range-based loops over iterator loops:

```cpp
// Good
for (const auto& section : sections_) {
    ProcessSection(section);
}

for (auto& atom : section.atoms) {
    atom->Process();
}

// Avoid
for (size_t i = 0; i < sections_.size(); ++i) {
    ProcessSection(sections_[i]);
}
```

### nullptr

Use `nullptr` instead of `NULL` or `0`:

```cpp
Cpu6502* cpu_ = nullptr;  // Good
Cpu6502* cpu_ = NULL;      // Bad (C-style)
Cpu6502* cpu_ = 0;         // Bad
```

---

## Testing

### Test Naming

```cpp
// Format: TEST(TestSuiteName, TestName)
TEST(AssemblerTest, Creation) { ... }
TEST(Cpu6502Test, EncodeLDA_Immediate) { ... }
TEST(SymbolTableTest, DefineAndLookup) { ... }
```

**Rules:**
- Test suite name = `<Component>Test` in PascalCase
- Test name describes what is tested, use underscores for readability
- Group related tests in same suite

### Test Structure

```cpp
TEST(AssemblerTest, AssembleSingleSection) {
    // Arrange
    Assembler assembler;
    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
    auto label = std::make_shared<LabelAtom>("start", 0x8000);
    section.atoms.push_back(label);

    // Act
    assembler.AddSection(section);
    AssemblerResult result = assembler.Assemble();

    // Assert
    EXPECT_TRUE(result.success);
    EXPECT_GE(result.pass_count, 1);
}
```

**Follow Arrange-Act-Assert pattern:**
1. **Arrange** - Set up test data
2. **Act** - Execute the code under test
3. **Assert** - Verify expected outcomes

---

## Exceptions

### When to Deviate

1. **External APIs** - Match third-party library conventions
2. **Legacy code** - May maintain existing style for consistency
3. **Performance-critical code** - Clarity sometimes yields to performance
4. **Platform-specific code** - Follow platform conventions

### Documenting Deviations

If you must deviate from this guide, document why:

```cpp
// NOTE: Using raw pointer here for compatibility with C API
extern "C" void ProcessData(void* data);
```

---

## Enforcement

### Pre-commit Checks

1. **Formatting** - Automated formatting with clang-format (future)
2. **Linting** - Static analysis with clang-tidy (future)
3. **Tests** - All tests must pass before commit
4. **Build** - Must build with zero warnings

### Code Review

All code changes must:
- Follow this style guide
- Include appropriate tests
- Have clear commit messages
- Pass CI/CD checks

---

## Examples

### Good Example

```cpp
// Assembler - Multi-pass assembler engine
// Phase 1: Minimal Viable Assembler - Assembler Engine

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace xasm {

class Assembler {
public:
    static constexpr int MAX_PASSES = 500;

    Assembler();

    void SetCpuPlugin(Cpu6502* cpu);
    void AddSection(const Section& section);
    AssemblerResult Assemble();
    
    size_t GetSectionCount() const;

private:
    void InitializeInstructionHandlers();
    
    std::vector<Section> sections_;
    Cpu6502* cpu_ = nullptr;
};

} // namespace xasm
```

### Bad Example (Multiple Violations)

```cpp
// Don't do this!

#include <vector>
#include <string>  // Not alphabetical
#include "Assembler.h"  // Wrong capitalization

using namespace std;  // Never in headers!

class assembler {  // Wrong case
public:
    void addSection(Section& s);  // Inconsistent case, non-const ref
    int getSectionCount();  // Should be const
    
private:
    vector<Section> m_sections;  // Hungarian notation
    Cpu6502 *cpu;  // Wrong pointer alignment, missing underscore
};
```

---

## References

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html)

---

## Changelog

- **1.0.0** (2026-01-27) - Initial style guide based on existing codebase analysis
