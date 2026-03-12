# Data Models and Architecture Relationships

**Date:** 2026-02-09
**Project:** xasm++ Architecture Review
**Purpose:** Document key data structures, interfaces, and relationships

---

## Core Interfaces

### 1. CpuPlugin Interface (Abstract Base Class)

```cpp
namespace xasm {

class CpuPlugin {
public:
  virtual ~CpuPlugin() = default;

  // Core interface methods
  virtual std::string GetCpuFamily() const = 0;
  virtual std::vector<std::string> GetSupportedVariants() const = 0;
  
  virtual std::vector<uint8_t> 
    EncodeInstruction(const std::string& mnemonic,
                     uint32_t operand,
                     const std::string& operand_str) const = 0;

protected:
  CpuPlugin() = default;
  
  // Non-copyable, non-movable
  CpuPlugin(const CpuPlugin&) = delete;
  CpuPlugin& operator=(const CpuPlugin&) = delete;
  CpuPlugin(CpuPlugin&&) = delete;
  CpuPlugin& operator=(CpuPlugin&&) = delete;
};

} // namespace xasm
```

**Design Notes:**
- Pure abstract interface (Strategy pattern)
- Non-copyable/non-movable by design (plugins are singletons)
- Exception-based error reporting (documented in interface)

---

## CPU Plugin Implementations

### Cpu6502 Class Structure

```cpp
namespace xasm {

class Cpu6502 : public CpuPlugin {
public:
  Cpu6502();
  ~Cpu6502() override = default;

  // Interface implementation
  std::string GetCpuFamily() const override;
  std::vector<std::string> GetSupportedVariants() const override;
  std::vector<uint8_t> EncodeInstruction(
    const std::string& mnemonic,
    uint32_t operand,
    const std::string& operand_str) const override;

private:
  // Addressing mode detection
  enum class AddressingMode {
    Implied,
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

  AddressingMode DetectAddressingMode(const std::string& operand_str) const;

  // 75 individual encoding methods (CODE SMELL: Large Class)
  std::vector<uint8_t> EncodeADC_Immediate(uint32_t operand) const;
  std::vector<uint8_t> EncodeADC_ZeroPage(uint32_t operand) const;
  std::vector<uint8_t> EncodeADC_ZeroPageX(uint32_t operand) const;
  std::vector<uint8_t> EncodeADC_Absolute(uint32_t operand) const;
  std::vector<uint8_t> EncodeADC_AbsoluteX(uint32_t operand) const;
  std::vector<uint8_t> EncodeADC_AbsoluteY(uint32_t operand) const;
  std::vector<uint8_t> EncodeADC_IndirectX(uint32_t operand) const;
  std::vector<uint8_t> EncodeADC_IndirectY(uint32_t operand) const;
  // ... 67 more methods

  // Helper classes (GOOD: Separation of concerns)
  Cpu6502BranchHandler branch_handler_;
};

} // namespace xasm
```

**Metrics:**
- File size: 637 lines
- Public methods: 3 (interface)
- Private methods: 75+ (encoding)
- Dependencies: cpu_plugin.h, cpu_6502_branch_handler.h

**Issues:**
- ✅ Branch handler extracted (good pattern)
- ⚠️ 75 encoding methods (should be refactored)
- ✅ Clear addressing mode enum
- ⚠️ Likely missing separate opcode header (unlike name suggests)

---

### Cpu6809 Class Structure

```cpp
namespace xasm {

class Cpu6809 : public CpuPlugin {
public:
  Cpu6809();
  ~Cpu6809() override = default;

  // Interface implementation
  std::string GetCpuFamily() const override;
  std::vector<std::string> GetSupportedVariants() const override;
  std::vector<uint8_t> EncodeInstruction(
    const std::string& mnemonic,
    uint32_t operand,
    const std::string& operand_str) const override;

private:
  // 97 individual encoding methods (CODE SMELL: Large Class)
  std::vector<uint8_t> EncodeABX(...);
  std::vector<uint8_t> EncodeADCA_Immediate(...);
  std::vector<uint8_t> EncodeADCA_Direct(...);
  std::vector<uint8_t> EncodeADCA_Indexed(...);
  std::vector<uint8_t> EncodeADCA_Extended(...);
  // ... 92 more methods

  // No separate helper classes (ISSUE: Could benefit from extraction)
};

} // namespace xasm
```

**Metrics:**
- File size: 1027 lines (LARGEST)
- Public methods: 3 (interface)
- Private methods: 97+ (encoding)
- Dependencies: cpu_plugin.h only

**Issues:**
- ⚠️ Largest class (1027 lines)
- ⚠️ 97 encoding methods (highest count)
- ⚠️ No helper classes extracted
- ⚠️ Opcode definitions likely inline (not modular like 6502)

---

### CpuZ80 Class Structure

```cpp
namespace xasm {

class CpuZ80 : public CpuPlugin {
public:
  CpuZ80();
  ~CpuZ80() override = default;

  // Interface implementation
  std::string GetCpuFamily() const override;
  std::vector<std::string> GetSupportedVariants() const override;
  std::vector<uint8_t> EncodeInstruction(
    const std::string& mnemonic,
    uint32_t operand,
    const std::string& operand_str) const override;

private:
  // 68 individual encoding methods
  // ... similar structure to above

  // CRITICAL ISSUE: No exception throwing found in implementation
};

} // namespace xasm
```

**Metrics:**
- File size: 751 lines
- Public methods: 3 (interface)
- Private methods: 68+ (encoding)
- Dependencies: cpu_plugin.h only

**Issues:**
- ⚠️ No exception throwing (interface contract violation)
- ⚠️ Missing <optional> include (but 6502 has it)
- ⚠️ 68 encoding methods (should be refactored)
- ⚠️ Opcode definitions likely inline

---

## Syntax Parser Classes (No Common Interface)

### Current Structure (Anti-Pattern: No Inheritance)

```cpp
// NO COMMON BASE CLASS - ISSUE

class SimpleSyntaxParser {
public:
  // Independent implementation
  void ParseLine(const std::string& line);
  // Likely duplicates directive handling
};

class MerlinSyntaxParser {
public:
  // Independent implementation
  void ParseLine(const std::string& line);
  // Likely duplicates directive handling (30 process/handle methods)
};

class EdtasmSyntaxParser {
public:
  // Independent implementation
  void ParseLine(const std::string& line);
  // Likely duplicates directive handling
};

class ScmasmSyntaxParser {
public:
  // Independent implementation
  void ParseLine(const std::string& line);
  // Likely duplicates directive handling (13 process/handle methods)
};

class FlexAsmSyntax {
public:
  // Independent implementation
  void ParseLine(const std::string& line);
  // Likely duplicates directive handling
};

class Z80UniversalSyntaxParser {
public:
  // Independent implementation
  void ParseLine(const std::string& line);
  // Likely duplicates directive handling
};
```

**Issues:**
- ⚠️ No common interface (can't use polymorphism)
- ⚠️ Directive handling duplicated across all 6 parsers
- ⚠️ No consistent behavior guarantee
- ⚠️ High maintenance burden (change requires 6 edits)

---

## Proposed Refactoring: Syntax Parser Interface

```cpp
namespace xasm {
namespace syntax {

// PROPOSED: Common interface
class SyntaxParser {
public:
  virtual ~SyntaxParser() = default;

  struct ParsedLine {
    std::optional<std::string> label;
    std::optional<std::string> mnemonic;
    std::optional<std::string> operand;
    std::optional<std::string> comment;
    bool is_directive;
  };

  virtual ParsedLine ParseLine(const std::string& line) = 0;
  virtual bool IsDirective(const std::string& token) const = 0;
  virtual std::string GetSyntaxName() const = 0;

protected:
  SyntaxParser() = default;
};

// PROPOSED: Base implementation with common logic
class BaseSyntaxParser : public SyntaxParser {
protected:
  // Common directive handlers (Template Method pattern)
  virtual void HandleORG(uint32_t address);
  virtual void HandleEQU(const std::string& symbol, uint32_t value);
  virtual void HandleDB(const std::vector<uint8_t>& bytes);
  virtual void HandleDW(const std::vector<uint16_t>& words);
  virtual void HandleDS(size_t count);
  
  // Syntax-specific hooks (must be overridden)
  virtual std::string ExtractLabel(const std::string& line) = 0;
  virtual std::string ExtractComment(const std::string& line) = 0;
  virtual char GetCommentChar() const = 0;

  // Common utilities
  std::string TrimWhitespace(const std::string& str) const;
  std::vector<std::string> SplitTokens(const std::string& line) const;
};

// PROPOSED: Concrete implementations become simpler
class MerlinSyntaxParser : public BaseSyntaxParser {
public:
  std::string GetSyntaxName() const override { return "Merlin"; }

protected:
  // Only implement syntax-specific logic
  std::string ExtractLabel(const std::string& line) override {
    // Merlin: Label in first column, no colon required
    if (!line.empty() && !std::isspace(line[0])) {
      size_t space = line.find_first_of(" \t");
      return line.substr(0, space);
    }
    return "";
  }

  std::string ExtractComment(const std::string& line) override {
    size_t pos = line.find(';');
    return (pos != std::string::npos) ? line.substr(pos) : "";
  }

  char GetCommentChar() const override { return ';'; }
};

class EdtasmSyntaxParser : public BaseSyntaxParser {
public:
  std::string GetSyntaxName() const override { return "EDTASM"; }

protected:
  // Only implement syntax-specific logic
  std::string ExtractLabel(const std::string& line) override {
    // EDTASM: Label must end with colon
    size_t colon = line.find(':');
    if (colon != std::string::npos) {
      return line.substr(0, colon);
    }
    return "";
  }

  std::string ExtractComment(const std::string& line) override {
    size_t pos = line.find('*');
    return (pos != std::string::npos) ? line.substr(pos) : "";
  }

  char GetCommentChar() const override { return '*'; }
};

} // namespace syntax
} // namespace xasm
```

**Benefits of Proposed Refactoring:**
- ✅ Eliminates ~80% of duplicated code
- ✅ Consistent directive behavior across syntaxes
- ✅ Easier to add new syntax variants
- ✅ Polymorphic usage possible (syntax selection at runtime)
- ✅ Better testability (test common logic once)

---

## Data Flow Architecture

### Current Assembly Pipeline

```
┌─────────────────────────────────────────────────────┐
│  Input: Assembly Source File (.asm)                 │
└─────────────────┬───────────────────────────────────┘
                  │
                  v
┌─────────────────────────────────────────────────────┐
│  Syntax Parser (selected by user or auto-detect)    │
│  ▸ Tokenizes lines                                  │
│  ▸ Extracts labels, mnemonics, operands             │
│  ▸ Handles syntax-specific conventions              │
└─────────────────┬───────────────────────────────────┘
                  │
                  v
┌─────────────────────────────────────────────────────┐
│  Macro Processor                                    │
│  ▸ Expands macros                                   │
│  ▸ Handles conditional assembly                     │
└─────────────────┬───────────────────────────────────┘
                  │
                  v
┌─────────────────────────────────────────────────────┐
│  Symbol Table / Expression Evaluator                │
│  ▸ Resolves labels                                  │
│  ▸ Evaluates expressions                            │
│  ▸ Multi-pass resolution (forward references)       │
└─────────────────┬───────────────────────────────────┘
                  │
                  v
┌─────────────────────────────────────────────────────┐
│  CPU Plugin (selected by directive or CLI)          │
│  ▸ EncodeInstruction(mnemonic, operand, mode)       │
│  ▸ Returns vector<uint8_t> of machine code          │
│  ▸ Handles addressing modes                         │
└─────────────────┬───────────────────────────────────┘
                  │
                  v
┌─────────────────────────────────────────────────────┐
│  Output Writer (selected by CLI option)             │
│  ▸ Binary (.bin)                                    │
│  ▸ S-Record (.srec, .s19)                           │
│  ▸ Intel HEX (.hex)                                 │
│  ▸ etc.                                             │
└─────────────────┬───────────────────────────────────┘
                  │
                  v
┌─────────────────────────────────────────────────────┐
│  Output: Executable Binary                          │
└─────────────────────────────────────────────────────┘
```

### Data Structures in Pipeline

```cpp
// Stage 1: Syntax Parser Output
struct ParsedLine {
  std::optional<std::string> label;       // "START"
  std::optional<std::string> mnemonic;    // "LDA"
  std::optional<std::string> operand;     // "#$42"
  std::optional<std::string> comment;     // "; Load accumulator"
  bool is_directive;                      // false
  size_t line_number;                     // 10
};

// Stage 2: Symbol Table Entry
struct Symbol {
  std::string name;       // "START"
  uint32_t address;       // 0x8000
  bool is_defined;        // true
  bool is_exported;       // false
  size_t line_defined;    // 10
};

// Stage 3: Instruction Encoding Request
struct InstructionRequest {
  std::string mnemonic;       // "LDA"
  std::string operand_str;    // "#$42"
  uint32_t operand_value;     // 0x42 (evaluated)
  uint32_t address;           // Current assembly address
};

// Stage 4: CPU Plugin Output
std::vector<uint8_t> machine_code;  // {0xA9, 0x42} for LDA #$42

// Stage 5: Output Writer Input
struct AssembledSection {
  uint32_t start_address;               // 0x8000
  std::vector<uint8_t> data;            // {0xA9, 0x42, ...}
  std::map<std::string, uint32_t> symbols;  // {"START": 0x8000}
};
```

---

## Class Relationships (UML Conceptual)

```
┌──────────────────┐
│   CpuPlugin      │ (Abstract Base)
│  (Strategy)      │
└────────┬─────────┘
         │
         │ inherits
         │
    ┌────┴────┬─────────┬────────┐
    │         │         │         │
┌───▼────┐ ┌──▼─────┐ ┌─▼──────┐ │
│Cpu6502 │ │Cpu6809 │ │CpuZ80  │ │ ... (future CPUs)
└────────┘ └────────┘ └────────┘ │
    │                             │
    │ uses                        │
    ▼                             │
┌────────────────────┐            │
│Cpu6502BranchHandler│            │
│  (Helper)          │            │
└────────────────────┘            │
                                  │
                                  │
┌──────────────────┐              │
│  SyntaxParser    │ (PROPOSED Abstract Base)
│  (Strategy)      │
└────────┬─────────┘
         │
         │ inherits
         │
    ┌────┴────┬─────────┬────────┬─────────┬─────────┐
    │         │         │         │         │         │
┌───▼─────┐┌─▼──────┐┌─▼──────┐┌─▼──────┐┌─▼──────┐┌─▼──────┐
│Simple   ││Merlin  ││Edtasm  ││Scmasm  ││FlexAsm ││Z80Univ │
│Syntax   ││Syntax  ││Syntax  ││Syntax  ││Syntax  ││Syntax  │
└─────────┘└────────┘└────────┘└────────┘└────────┘└────────┘

┌──────────────────┐
│  OutputWriter    │ (Likely exists)
│  (Strategy?)     │
└────────┬─────────┘
         │
         │ inherits
         │
    ┌────┴────┬─────────┬────────┐
    │         │         │         │
┌───▼────┐ ┌──▼─────┐ ┌─▼──────┐
│Binary  │ │SRecord │ │IntelHex│
│Writer  │ │Writer  │ │Writer  │
└────────┘ └────────┘ └────────┘
```

---

## Constants and Configuration

### CPU Constants (cpu_constants.h)

```cpp
namespace xasm {
namespace cpu {

// CPU Variant Names
inline constexpr const char* CPU_6502 = "6502";
inline constexpr const char* CPU_65C02 = "65c02";
inline constexpr const char* CPU_65C02_ROCK = "65c02rock";
inline constexpr const char* CPU_65816 = "65816";
inline constexpr const char* CPU_6809 = "6809";

// CPU Family Identifiers
inline constexpr const char* FAMILY_6502 = "6502";
inline constexpr const char* FAMILY_6809 = "6809";

} // namespace cpu
} // namespace xasm
```

**Good Practice:**
- ✅ Avoids magic strings
- ✅ Centralized constants
- ⚠️ Could add Z80 constants (currently missing)

---

## Addressing Mode Enumerations

### 6502 Addressing Modes

```cpp
enum class AddressingMode {
  Implied,       // CLC
  Immediate,     // LDA #$42
  ZeroPage,      // LDA $42
  ZeroPageX,     // LDA $42,X
  ZeroPageY,     // LDX $42,Y
  Absolute,      // LDA $4242
  AbsoluteX,     // LDA $4242,X
  AbsoluteY,     // LDA $4242,Y
  Indirect,      // JMP ($4242)
  IndirectX,     // LDA ($42,X)
  IndirectY,     // LDA ($42),Y
  Relative       // BEQ label (branches)
};
```

### 6809 Addressing Modes (Inferred)

```cpp
enum class AddressingMode {
  Inherent,      // NOP
  Immediate,     // LDA #$42
  Direct,        // LDA $42 (zero page equivalent)
  Extended,      // LDA $4242 (absolute)
  Indexed,       // LDA ,X (various indexed modes)
  Relative,      // BEQ label
  // 6809 has complex indexed modes (5-bit, 8-bit, 16-bit offsets)
};
```

### Z80 Addressing Modes (Inferred)

```cpp
enum class AddressingMode {
  Implied,       // NOP
  Immediate,     // LD A,$42
  Register,      // LD A,B
  Indirect,      // LD A,(HL)
  IndexedIX,     // LD A,(IX+$10)
  IndexedIY,     // LD A,(IY+$10)
  Extended,      // LD A,($4242)
  // Z80 has many register pair modes
};
```

**Note:** Addressing mode enums are likely defined differently in each CPU class, contributing to pattern inconsistency.

---

## Memory Model and Address Spaces

### Assembly Address Space

```cpp
struct AddressSpace {
  uint32_t origin;              // Starting address (set by ORG)
  uint32_t current_address;     // Current assembly address
  std::vector<uint8_t> data;    // Assembled bytes
  
  // Sections (if supported)
  std::map<std::string, Section> sections;
};

struct Section {
  std::string name;       // ".text", ".data", etc.
  uint32_t start;         // Section start address
  uint32_t size;          // Section size
  std::vector<uint8_t> data;  // Section data
};
```

### Symbol Table Model

```cpp
class SymbolTable {
public:
  void Define(const std::string& name, uint32_t value);
  uint32_t Lookup(const std::string& name) const;
  bool IsDefined(const std::string& name) const;
  
  // Multi-pass support
  void MarkUnresolved(const std::string& name, size_t line);
  std::vector<std::string> GetUnresolved() const;

private:
  struct SymbolEntry {
    uint32_t value;
    bool is_defined;
    size_t line_defined;
    std::vector<size_t> references;  // Lines where used
  };
  
  std::map<std::string, SymbolEntry> symbols_;
};
```

---

## Error Model

### Current Error Handling

```cpp
// Interface contract
class CpuPlugin {
  /**
   * @throws std::invalid_argument if instruction/addressing mode not supported
   * @throws std::out_of_range if operand value out of range
   */
  virtual std::vector<uint8_t> EncodeInstruction(...) const = 0;
};

// Implementation (6502, 6809)
throw std::invalid_argument("Unsupported instruction: " + mnemonic);

// Implementation (Z80) - ISSUE: No exception throwing found
```

### Proposed Error Model Enhancement

```cpp
namespace xasm {

enum class AssemblyError {
  UNSUPPORTED_INSTRUCTION,
  INVALID_ADDRESSING_MODE,
  OPERAND_OUT_OF_RANGE,
  UNDEFINED_SYMBOL,
  DUPLICATE_LABEL,
  INVALID_EXPRESSION,
  FILE_NOT_FOUND,
  SYNTAX_ERROR
};

class AssemblyException : public std::exception {
public:
  AssemblyException(AssemblyError code, 
                   const std::string& message,
                   size_t line_number = 0)
    : code_(code), message_(message), line_(line_number) {}
  
  const char* what() const noexcept override {
    return message_.c_str();
  }
  
  AssemblyError code() const { return code_; }
  size_t line() const { return line_; }

private:
  AssemblyError code_;
  std::string message_;
  size_t line_;
};

} // namespace xasm
```

**Benefits:**
- Error codes for programmatic handling
- Line number tracking for better error messages
- Internationalization support
- Easier error recovery

---

## Proposed Data-Driven Opcode Table

### Current Approach (Method per Instruction)

```cpp
// 75+ methods like this:
std::vector<uint8_t> Cpu6502::EncodeADC_Immediate(uint32_t operand) const {
  return {0x69, static_cast<uint8_t>(operand)};
}

std::vector<uint8_t> Cpu6502::EncodeADC_ZeroPage(uint32_t operand) const {
  return {0x65, static_cast<uint8_t>(operand)};
}
// ... repeat for every instruction/mode combination
```

### Proposed Data-Driven Approach

```cpp
struct OpcodeEntry {
  const char* mnemonic;
  AddressingMode mode;
  uint8_t opcode;
  uint8_t bytes;           // Total instruction length
  uint8_t cycles;          // Clock cycles (for simulation)
  
  // Validation
  bool requires_zero_page_operand() const {
    return mode == AddressingMode::ZeroPage ||
           mode == AddressingMode::ZeroPageX ||
           mode == AddressingMode::ZeroPageY;
  }
};

// In separate header (opcodes_6502.h)
namespace xasm {
namespace opcodes {
namespace m6502 {

constexpr OpcodeEntry OPCODE_TABLE[] = {
  // Mnemonic, Mode, Opcode, Bytes, Cycles
  {"ADC", AddressingMode::Immediate, 0x69, 2, 2},
  {"ADC", AddressingMode::ZeroPage,  0x65, 2, 3},
  {"ADC", AddressingMode::ZeroPageX, 0x75, 2, 4},
  {"ADC", AddressingMode::Absolute,  0x6D, 3, 4},
  {"ADC", AddressingMode::AbsoluteX, 0x7D, 3, 4},
  {"ADC", AddressingMode::AbsoluteY, 0x79, 3, 4},
  {"ADC", AddressingMode::IndirectX, 0x61, 2, 6},
  {"ADC", AddressingMode::IndirectY, 0x71, 2, 5},
  // ... rest of opcodes
};

} // namespace m6502
} // namespace opcodes
} // namespace xasm

// In cpu_6502.cpp
std::vector<uint8_t> Cpu6502::EncodeInstruction(
    const std::string& mnemonic,
    uint32_t operand,
    const std::string& operand_str) const {
  
  // Detect addressing mode from operand string
  AddressingMode mode = DetectAddressingMode(operand_str);
  
  // Look up opcode in table
  for (const auto& entry : opcodes::m6502::OPCODE_TABLE) {
    if (entry.mnemonic == mnemonic && entry.mode == mode) {
      // Validate operand range
      if (entry.requires_zero_page_operand() && operand > 0xFF) {
        throw std::out_of_range("Operand too large for zero page");
      }
      
      // Encode instruction
      std::vector<uint8_t> result = {entry.opcode};
      if (entry.bytes == 2) {
        result.push_back(static_cast<uint8_t>(operand));
      } else if (entry.bytes == 3) {
        result.push_back(static_cast<uint8_t>(operand & 0xFF));
        result.push_back(static_cast<uint8_t>(operand >> 8));
      }
      return result;
    }
  }
  
  throw std::invalid_argument("Unsupported instruction: " + mnemonic);
}
```

**Benefits:**
- Reduces 75 methods to 1 method + data table
- Easier to verify opcode correctness (compare table to datasheet)
- Can generate table from external data (CSV, JSON)
- Easier to add CPU variants (modify table, not code)
- Centralized opcode reference

---

## Summary

This data model document reveals:

1. **Strong Interface Design:** `CpuPlugin` is a well-designed Strategy pattern
2. **Missing Interface:** Syntax parsers lack common base (code duplication)
3. **Large Class Problem:** 68-97 methods per CPU class (SRP violation)
4. **Inconsistent Patterns:** Opcode organization varies across CPUs
5. **Error Handling Gap:** Z80 may not throw exceptions per contract

**Key Refactoring Opportunities:**
- Add `SyntaxParser` interface with `BaseSyntaxParser` implementation
- Refactor large CPU classes using data-driven opcode tables
- Standardize error handling across all plugins
- Extract helper classes (like 6502's branch handler) for other CPUs

---

**Document Status:** Complete
**Related Documents:**
- architecture.md (main review)
- ADR-001 through ADR-005 (proposed)

