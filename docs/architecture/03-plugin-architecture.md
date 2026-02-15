# Plugin Architecture Deep Dive

**Version:** 1.0  
**Date:** 2026-02-14  
**Status:** Reference Documentation

---

## Overview

The xasm++ plugin architecture is built on the **Strategy Pattern**, allowing runtime selection of CPU encoders, syntax parsers, and output formatters without modifying the core assembler engine.

---

## CPU Plugin System

### Interface Definition

```cpp
// include/xasm++/cpu/cpu_plugin.h

namespace xasm {

class CpuPlugin {
public:
  virtual ~CpuPlugin() = default;

  // Metadata queries
  virtual std::string GetCpuFamily() const = 0;
  virtual std::vector<std::string> GetSupportedVariants() const = 0;
  virtual bool HasOpcode(const std::string& mnemonic) const = 0;

  // Core encoding interface
  virtual std::vector<uint8_t> EncodeInstruction(
      const std::string& mnemonic,
      uint32_t operand,
      const std::string& operand_str) const = 0;

  // Special encoding (branches, multi-operand instructions)
  virtual bool RequiresSpecialEncoding(const std::string& mnemonic) const;
  virtual std::vector<uint8_t> EncodeInstructionSpecial(
      const std::string& mnemonic,
      const std::string& operand,
      uint16_t current_address) const;
};

} // namespace xasm
```

### Design Rationale

**Why three encoding parameters?**

1. `mnemonic` - Instruction name (e.g., "LDA", "JMP")
2. `operand` - Parsed numeric value (e.g., 0x1234 for "$1234")
3. `operand_str` - Original operand string (e.g., "$1234,X")

**Rationale:**
- `operand` provides the value for encoding
- `operand_str` needed to parse addressing modes (immediate, indexed, indirect)
- Separates value parsing from addressing mode detection

**Why `RequiresSpecialEncoding()`?**

Some instructions need context beyond standard operand:
- **Branch instructions:** Need current address for relative offset calculation
- **Multi-operand instructions:** e.g., MVN/MVP on 65816 (two operands)
- **Special parsing:** e.g., indexed indirect `($80,X)`

---

### Implementation Example: 6502

```cpp
// src/cpu/cpu_6502.cpp

class Cpu6502 : public CpuPlugin {
public:
  std::string GetCpuFamily() const override {
    return "6502";
  }

  std::vector<std::string> GetSupportedVariants() const override {
    return {"6502", "nmos6502"};
  }

  bool HasOpcode(const std::string& mnemonic) const override {
    std::string upper = ToUpper(mnemonic);
    return opcodes_.find(upper) != opcodes_.end();
  }

  std::vector<uint8_t> EncodeInstruction(
      const std::string& mnemonic,
      uint32_t operand,
      const std::string& operand_str) const override {
    
    // Step 1: Parse addressing mode from operand string
    AddressingMode mode = ParseAddressingMode(operand_str, mnemonic);
    
    // Step 2: Look up opcode for mnemonic + addressing mode
    uint8_t opcode = LookupOpcode(mnemonic, mode);
    if (opcode == 0xFF) {
      throw std::invalid_argument(
          "Unsupported addressing mode for " + mnemonic);
    }
    
    // Step 3: Encode bytes based on addressing mode
    return EncodeWithAddressingMode(opcode, mode, operand);
  }

private:
  // Addressing mode detection
  AddressingMode ParseAddressingMode(
      const std::string& operand_str,
      const std::string& mnemonic) const {
    
    if (operand_str.empty()) {
      // Check if accumulator mode (ASL A) or implied (INX)
      if (mnemonic == "ASL" || mnemonic == "LSR" || 
          mnemonic == "ROL" || mnemonic == "ROR") {
        return AddressingMode::Accumulator;
      }
      return AddressingMode::Implied;
    }
    
    if (operand_str[0] == '#') {
      return AddressingMode::Immediate;  // #$42
    }
    
    if (operand_str[0] == '(' && operand_str.back() == ')') {
      // Indirect modes: ($addr), ($addr,X), ($addr),Y
      if (operand_str.find(",X)") != std::string::npos) {
        return AddressingMode::IndexedIndirect;  // ($80,X)
      } else if (operand_str.find("),Y") != std::string::npos) {
        return AddressingMode::IndirectIndexed;  // ($80),Y
      } else {
        return AddressingMode::Indirect;  // ($1234)
      }
    }
    
    // Check for indexed addressing: ,X or ,Y
    if (operand_str.find(",X") != std::string::npos) {
      return (operand < 256) ? AddressingMode::ZeroPageX 
                             : AddressingMode::AbsoluteX;
    }
    
    if (operand_str.find(",Y") != std::string::npos) {
      return (operand < 256) ? AddressingMode::ZeroPageY 
                             : AddressingMode::AbsoluteY;
    }
    
    // Direct addressing (zero page or absolute)
    return (operand < 256) ? AddressingMode::ZeroPage 
                           : AddressingMode::Absolute;
  }

  // Opcode lookup table
  uint8_t LookupOpcode(const std::string& mnemonic, 
                       AddressingMode mode) const {
    std::string key = mnemonic + "_" + ToString(mode);
    auto it = opcode_table_.find(key);
    return (it != opcode_table_.end()) ? it->second : 0xFF;
  }

  // Encoding based on addressing mode
  std::vector<uint8_t> EncodeWithAddressingMode(
      uint8_t opcode,
      AddressingMode mode,
      uint32_t operand) const {
    
    switch (mode) {
      case AddressingMode::Implied:
      case AddressingMode::Accumulator:
        return {opcode};
      
      case AddressingMode::Immediate:
      case AddressingMode::ZeroPage:
      case AddressingMode::ZeroPageX:
      case AddressingMode::ZeroPageY:
        ValidateByteRange(operand);
        return {opcode, static_cast<uint8_t>(operand)};
      
      case AddressingMode::Absolute:
      case AddressingMode::AbsoluteX:
      case AddressingMode::AbsoluteY:
      case AddressingMode::Indirect:
        ValidateWordRange(operand);
        return {opcode, 
                static_cast<uint8_t>(operand & 0xFF),
                static_cast<uint8_t>((operand >> 8) & 0xFF)};
      
      case AddressingMode::IndexedIndirect:
      case AddressingMode::IndirectIndexed:
        ValidateByteRange(operand);
        return {opcode, static_cast<uint8_t>(operand)};
      
      default:
        throw std::invalid_argument("Unknown addressing mode");
    }
  }

  void ValidateByteRange(uint32_t value) const {
    if (value > 0xFF) {
      throw std::out_of_range("Operand exceeds byte range (0-255)");
    }
  }

  void ValidateWordRange(uint32_t value) const {
    if (value > 0xFFFF) {
      throw std::out_of_range("Operand exceeds word range (0-65535)");
    }
  }

  // Opcode table (initialized in constructor)
  std::unordered_map<std::string, uint8_t> opcode_table_;
};
```

---

### Branch Instruction Special Encoding

Branch instructions (BCC, BCS, BEQ, etc.) use relative addressing and require the current instruction address to calculate the offset.

```cpp
bool Cpu6502::RequiresSpecialEncoding(const std::string& mnemonic) const {
  static const std::unordered_set<std::string> branch_instructions = {
    "BCC", "BCS", "BEQ", "BMI", "BNE", "BPL", "BVC", "BVS"
  };
  return branch_instructions.count(ToUpper(mnemonic)) > 0;
}

std::vector<uint8_t> Cpu6502::EncodeInstructionSpecial(
    const std::string& mnemonic,
    const std::string& operand,
    uint16_t current_address) const {
  
  // Branch instructions need current address for relative offset
  if (RequiresSpecialEncoding(mnemonic)) {
    return branch_handler_.Encode(mnemonic, operand, current_address);
  }
  
  throw std::invalid_argument(
      "Special encoding not supported for: " + mnemonic);
}
```

**Branch Handler Implementation:**

```cpp
// cpu_6502_branch_handler.cpp

std::vector<uint8_t> Cpu6502BranchHandler::Encode(
    const std::string& mnemonic,
    const std::string& operand_str,
    uint16_t current_address) const {
  
  // Parse target address (symbol or literal)
  uint16_t target_address = ParseAddress(operand_str);
  
  // Calculate relative offset
  // Next instruction address = current + 2 (opcode + offset byte)
  int16_t offset = static_cast<int16_t>(target_address) 
                 - static_cast<int16_t>(current_address + 2);
  
  // Check if offset fits in 8-bit signed range (-128 to +127)
  if (offset < -128 || offset > 127) {
    throw std::out_of_range(
        "Branch target too far: " + std::to_string(offset) + " bytes");
  }
  
  uint8_t opcode = LookupBranchOpcode(mnemonic);
  return {opcode, static_cast<uint8_t>(offset & 0xFF)};
}
```

---

## CPU Plugin Family Hierarchy

### Inheritance for Code Reuse

```cpp
// Base class for 6502 family
class Cpu6502Base : public CpuPlugin {
protected:
  // Shared opcode table initialization
  virtual void InitializeOpcodes();
  
  // Common addressing mode parsing
  AddressingMode ParseAddressingMode(
      const std::string& operand_str,
      const std::string& mnemonic) const;
  
  // Common encoding logic
  std::vector<uint8_t> EncodeWithAddressingMode(
      uint8_t opcode,
      AddressingMode mode,
      uint32_t operand) const;
  
  std::unordered_map<std::string, uint8_t> opcode_table_;
};

// NMOS 6502 (original - 56 instructions)
class Cpu6502 : public Cpu6502Base {
public:
  std::string GetCpuFamily() const override { return "6502"; }
  
  std::vector<std::string> GetSupportedVariants() const override {
    return {"6502", "nmos6502"};
  }

protected:
  void InitializeOpcodes() override {
    Cpu6502Base::InitializeOpcodes();  // Base 56 instructions
    // No additional opcodes for NMOS 6502
  }
};

// CMOS 65C02 (adds 27 instructions)
class Cpu65C02 : public Cpu6502Base {
public:
  std::string GetCpuFamily() const override { return "65C02"; }
  
  std::vector<std::string> GetSupportedVariants() const override {
    return {"65c02", "cmos6502"};
  }

protected:
  void InitializeOpcodes() override {
    Cpu6502Base::InitializeOpcodes();  // Base 56 instructions
    
    // Add 65C02 extensions
    AddOpcode("BRA", AddressingMode::Relative, 0x80);      // Branch always
    AddOpcode("PHX", AddressingMode::Implied, 0xDA);       // Push X
    AddOpcode("PHY", AddressingMode::Implied, 0x5A);       // Push Y
    AddOpcode("PLX", AddressingMode::Implied, 0xFA);       // Pull X
    AddOpcode("PLY", AddressingMode::Implied, 0x7A);       // Pull Y
    AddOpcode("STZ", AddressingMode::ZeroPage, 0x64);      // Store zero
    AddOpcode("STZ", AddressingMode::ZeroPageX, 0x74);
    AddOpcode("STZ", AddressingMode::Absolute, 0x9C);
    AddOpcode("STZ", AddressingMode::AbsoluteX, 0x9E);
    AddOpcode("TRB", AddressingMode::ZeroPage, 0x14);      // Test and reset bits
    AddOpcode("TRB", AddressingMode::Absolute, 0x1C);
    AddOpcode("TSB", AddressingMode::ZeroPage, 0x04);      // Test and set bits
    AddOpcode("TSB", AddressingMode::Absolute, 0x0C);
    AddOpcode("BIT", AddressingMode::Immediate, 0x89);     // BIT immediate
    AddOpcode("BIT", AddressingMode::ZeroPageX, 0x34);
    AddOpcode("BIT", AddressingMode::AbsoluteX, 0x3C);
    // ... (rest of 65C02 extensions)
  }
};

// 65816 (16-bit variant - adds long addressing, 16-bit modes)
class Cpu65816 : public Cpu65C02 {
public:
  std::string GetCpuFamily() const override { return "65816"; }
  
  std::vector<std::string> GetSupportedVariants() const override {
    return {"65816", "65c816"};
  }

protected:
  void InitializeOpcodes() override {
    Cpu65C02::InitializeOpcodes();  // Base + 65C02 instructions
    
    // Add 65816 extensions
    AddOpcode("COP", AddressingMode::Immediate, 0x02);     // Co-processor
    AddOpcode("REP", AddressingMode::Immediate, 0xC2);     // Reset processor status
    AddOpcode("SEP", AddressingMode::Immediate, 0xE2);     // Set processor status
    AddOpcode("XBA", AddressingMode::Implied, 0xEB);       // Exchange B/A
    AddOpcode("XCE", AddressingMode::Implied, 0xFB);       // Exchange carry/emulation
    
    // Long addressing modes (24-bit addresses)
    AddOpcode("JML", AddressingMode::AbsoluteLong, 0x5C);  // Jump long
    AddOpcode("JML", AddressingMode::IndirectLong, 0xDC);
    AddOpcode("JSL", AddressingMode::AbsoluteLong, 0x22);  // Jump subroutine long
    // ... (rest of 65816 extensions)
  }

  // Override addressing mode parsing to handle long addressing
  AddressingMode ParseAddressingMode(
      const std::string& operand_str,
      const std::string& mnemonic) const override {
    
    // Check for long addressing (24-bit)
    if (operand_str.length() > 5 && operand_str[0] == '$') {
      // $123456 -> 24-bit address
      return AddressingMode::AbsoluteLong;
    }
    
    // Fall back to base parsing
    return Cpu6502Base::ParseAddressingMode(operand_str, mnemonic);
  }
};
```

**Code Reuse Metrics:**
- Cpu6502 → Cpu65C02: ~80% reuse (same base opcodes + extensions)
- Cpu65C02 → Cpu65816: ~70% reuse (adds new addressing modes)

---

## Syntax Plugin System (Proposed)

### Interface Definition (Proposed)

```cpp
// include/xasm++/syntax/syntax_parser.h (PROPOSED)

namespace xasm {

class SyntaxParser {
public:
  virtual ~SyntaxParser() = default;

  // Metadata
  virtual std::string GetName() const = 0;
  virtual std::string GetDescription() const = 0;

  // Core interface
  virtual ParsedLine ParseLine(const std::string& line) = 0;
  virtual bool IsDirective(const std::string& token) const = 0;
  
  // Factory method
  static std::unique_ptr<SyntaxParser> Create(SyntaxStyle style);
};

struct ParsedLine {
  std::string label;      // Optional label
  std::string mnemonic;   // Instruction or directive
  std::string operand;    // Operand string
  std::string comment;    // Optional comment
};

} // namespace xasm
```

### Template Method Pattern (Proposed)

```cpp
// Base implementation with common directive handling
class BaseSyntaxParser : public SyntaxParser {
public:
  ParsedLine ParseLine(const std::string& line) override {
    // Common parsing structure
    std::string trimmed = TrimWhitespace(line);
    
    // Extract comment (syntax-specific)
    std::string comment = ExtractComment(trimmed);
    
    // Extract label (syntax-specific)
    std::string label = ExtractLabel(trimmed);
    
    // Extract mnemonic and operand
    std::string mnemonic, operand;
    std::tie(mnemonic, operand) = SplitMnemonicOperand(trimmed);
    
    return ParsedLine{label, mnemonic, operand, comment};
  }

protected:
  // Template methods (override in derived classes)
  virtual std::string ExtractComment(std::string& line) = 0;
  virtual std::string ExtractLabel(std::string& line) = 0;
  
  // Common directive handling
  virtual void HandleORG(uint32_t address);
  virtual void HandleEQU(const std::string& symbol, uint32_t value);
  virtual void HandleDB(const std::vector<uint8_t>& bytes);
  virtual void HandleDW(const std::vector<uint16_t>& words);
  virtual void HandleDS(size_t count);
  virtual void HandleINCLUDE(const std::string& filename);
};

// Merlin syntax parser
class MerlinSyntaxParser : public BaseSyntaxParser {
public:
  std::string GetName() const override { return "Merlin"; }
  
  std::string GetDescription() const override {
    return "Apple IIgs Merlin assembler syntax";
  }

protected:
  std::string ExtractComment(std::string& line) override {
    // Merlin uses '*' or ';' for comments
    size_t pos = line.find_first_of("*;");
    if (pos == std::string::npos) return "";
    
    std::string comment = line.substr(pos + 1);
    line = line.substr(0, pos);
    return comment;
  }

  std::string ExtractLabel(std::string& line) override {
    // Merlin labels start at column 1, end with space or colon
    if (line.empty() || std::isspace(line[0])) return "";
    
    size_t pos = line.find_first_of(" :");
    if (pos == std::string::npos) {
      std::string label = line;
      line.clear();
      return label;
    }
    
    std::string label = line.substr(0, pos);
    line = line.substr(pos + 1);
    return label;
  }
};

// SCMASM syntax parser
class ScmasmSyntaxParser : public BaseSyntaxParser {
public:
  std::string GetName() const override { return "SCMASM"; }
  
  std::string GetDescription() const override {
    return "Apple II S-C Macro Assembler syntax";
  }

protected:
  std::string ExtractComment(std::string& line) override {
    // SCMASM uses ';' for comments
    size_t pos = line.find(';');
    if (pos == std::string::npos) return "";
    
    std::string comment = line.substr(pos + 1);
    line = line.substr(0, pos);
    return comment;
  }

  std::string ExtractLabel(std::string& line) override {
    // SCMASM labels start at column 1, end with space
    if (line.empty() || std::isspace(line[0])) return "";
    
    size_t pos = line.find(' ');
    if (pos == std::string::npos) {
      std::string label = line;
      line.clear();
      return label;
    }
    
    std::string label = line.substr(0, pos);
    line = line.substr(pos + 1);
    return label;
  }
};
```

**Benefits:**
- Eliminates ~40% duplication (directive handling)
- Consistent behavior across syntaxes
- Easy to add new syntax variants

---

## Output Plugin System

### Interface Definition

```cpp
// include/xasm++/output/output_plugin.h

namespace xasm {

class OutputPlugin {
public:
  virtual ~OutputPlugin() = default;

  // Metadata
  virtual std::string GetName() const = 0;
  virtual std::string GetFileExtension() const = 0;

  // Core interface
  virtual void Write(const std::string& filename,
                    const std::vector<Section>& sections,
                    const SymbolTable& symbols) = 0;
};

} // namespace xasm
```

### Implementation Examples

**Binary Output:**

```cpp
class BinaryOutputPlugin : public OutputPlugin {
public:
  std::string GetName() const override { return "Binary"; }
  std::string GetFileExtension() const override { return ".bin"; }

  void Write(const std::string& filename,
            const std::vector<Section>& sections,
            const SymbolTable& symbols) override {
    
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
      throw std::runtime_error("Failed to open output file");
    }
    
    for (const auto& section : sections) {
      for (const auto& atom : section.atoms) {
        if (atom->type == AtomType::Instruction) {
          auto inst = std::static_pointer_cast<InstructionAtom>(atom);
          out.write(reinterpret_cast<const char*>(inst->encoded_bytes.data()),
                   inst->encoded_bytes.size());
        } else if (atom->type == AtomType::Data) {
          auto data = std::static_pointer_cast<DataAtom>(atom);
          out.write(reinterpret_cast<const char*>(data->data.data()),
                   data->data.size());
        }
      }
    }
  }
};
```

**Listing Output:**

```cpp
class ListingOutputPlugin : public OutputPlugin {
public:
  std::string GetName() const override { return "Listing"; }
  std::string GetFileExtension() const override { return ".lst"; }

  void Write(const std::string& filename,
            const std::vector<Section>& sections,
            const SymbolTable& symbols) override {
    
    std::ofstream out(filename);
    if (!out) {
      throw std::runtime_error("Failed to open listing file");
    }
    
    // Header
    out << "xasm++ Listing File\n";
    out << "===================\n\n";
    
    // Symbol table
    out << "Symbol Table:\n";
    for (const auto& [name, value] : symbols.GetAllSymbols()) {
      out << std::hex << std::setfill('0') << std::setw(4) << value
          << "  " << name << "\n";
    }
    out << "\n";
    
    // Code listing
    for (const auto& section : sections) {
      out << "Section: " << section.name << "\n";
      out << "Origin: $" << std::hex << section.org << "\n\n";
      
      uint32_t address = section.org;
      for (const auto& atom : section.atoms) {
        // Address column
        out << std::hex << std::setfill('0') << std::setw(4) << address << "  ";
        
        // Bytes column
        if (atom->type == AtomType::Instruction) {
          auto inst = std::static_pointer_cast<InstructionAtom>(atom);
          for (uint8_t byte : inst->encoded_bytes) {
            out << std::hex << std::setfill('0') << std::setw(2) 
                << static_cast<int>(byte) << " ";
          }
          out << std::setfill(' ') << std::setw(12 - inst->encoded_bytes.size() * 3);
        }
        
        // Source column
        out << atom->source_line << "\n";
        
        address += atom->size;
      }
      out << "\n";
    }
  }
};
```

---

## Plugin Registration and Discovery (Future)

### Registry Pattern

```cpp
// include/xasm++/plugin_registry.h

template<typename PluginType>
class PluginRegistry {
public:
  using FactoryFunc = std::function<std::unique_ptr<PluginType>()>;

  static PluginRegistry& Instance() {
    static PluginRegistry instance;
    return instance;
  }

  void Register(const std::string& name, FactoryFunc factory) {
    factories_[name] = factory;
  }

  std::unique_ptr<PluginType> Create(const std::string& name) {
    auto it = factories_.find(name);
    if (it == factories_.end()) {
      throw std::runtime_error("Plugin not found: " + name);
    }
    return it->second();
  }

  std::vector<std::string> GetAvailablePlugins() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : factories_) {
      names.push_back(name);
    }
    return names;
  }

private:
  PluginRegistry() = default;
  std::unordered_map<std::string, FactoryFunc> factories_;
};

// Type aliases
using CpuRegistry = PluginRegistry<CpuPlugin>;
using SyntaxRegistry = PluginRegistry<SyntaxParser>;
using OutputRegistry = PluginRegistry<OutputPlugin>;
```

### Plugin Registration

```cpp
// src/cpu/cpu_6502.cpp

namespace {
  // Auto-registration at startup
  struct Cpu6502Registrar {
    Cpu6502Registrar() {
      CpuRegistry::Instance().Register("6502", 
          []() { return std::make_unique<Cpu6502>(); });
      CpuRegistry::Instance().Register("nmos6502",
          []() { return std::make_unique<Cpu6502>(); });
    }
  };
  
  static Cpu6502Registrar registrar;
}
```

### Plugin Usage

```cpp
// main.cpp

int main(int argc, char* argv[]) {
  // Parse command-line arguments
  std::string cpu_name = "6502";
  std::string syntax_name = "merlin";
  std::string output_format = "binary";
  
  // Create plugins via registry
  auto cpu = CpuRegistry::Instance().Create(cpu_name);
  auto syntax = SyntaxRegistry::Instance().Create(syntax_name);
  auto output = OutputRegistry::Instance().Create(output_format);
  
  // Assemble
  Assembler asm;
  asm.SetCpuPlugin(cpu.get());
  // ... (rest of assembly process)
  
  return 0;
}
```

---

## Testing Strategy for Plugins

### CPU Plugin Tests

```cpp
// tests/unit/test_cpu_6502.cpp

class Cpu6502Test : public ::testing::Test {
protected:
  Cpu6502 cpu_;
};

TEST_F(Cpu6502Test, EncodeImmediateAddressing) {
  auto bytes = cpu_.EncodeInstruction("LDA", 0x42, "#$42");
  EXPECT_EQ(bytes, std::vector<uint8_t>({0xA9, 0x42}));  // LDA #$42
}

TEST_F(Cpu6502Test, EncodeZeroPageAddressing) {
  auto bytes = cpu_.EncodeInstruction("LDA", 0x80, "$80");
  EXPECT_EQ(bytes, std::vector<uint8_t>({0xA5, 0x80}));  // LDA $80
}

TEST_F(Cpu6502Test, EncodeAbsoluteAddressing) {
  auto bytes = cpu_.EncodeInstruction("LDA", 0x1234, "$1234");
  EXPECT_EQ(bytes, std::vector<uint8_t>({0xAD, 0x34, 0x12}));  // LDA $1234 (little-endian)
}

TEST_F(Cpu6502Test, ThrowsOnInvalidOpcode) {
  EXPECT_THROW(
      cpu_.EncodeInstruction("INVALID", 0, ""),
      std::invalid_argument);
}

TEST_F(Cpu6502Test, ThrowsOnOperandOutOfRange) {
  EXPECT_THROW(
      cpu_.EncodeInstruction("LDA", 0x1234, "#$1234"),  // Immediate out of range
      std::out_of_range);
}
```

### Syntax Plugin Tests (Proposed)

```cpp
// tests/unit/test_merlin_syntax.cpp

class MerlinSyntaxTest : public ::testing::Test {
protected:
  MerlinSyntaxParser parser_;
};

TEST_F(MerlinSyntaxTest, ParseLabeledInstruction) {
  auto parsed = parser_.ParseLine("START LDA #$42");
  EXPECT_EQ(parsed.label, "START");
  EXPECT_EQ(parsed.mnemonic, "LDA");
  EXPECT_EQ(parsed.operand, "#$42");
}

TEST_F(MerlinSyntaxTest, ParseCommentLine) {
  auto parsed = parser_.ParseLine("* This is a comment");
  EXPECT_TRUE(parsed.mnemonic.empty());
  EXPECT_EQ(parsed.comment, " This is a comment");
}

TEST_F(MerlinSyntaxTest, ParseDirective) {
  auto parsed = parser_.ParseLine(" ORG $8000");
  EXPECT_EQ(parsed.mnemonic, "ORG");
  EXPECT_EQ(parsed.operand, "$8000");
}
```

---

## Performance Considerations

### Plugin Dispatch Overhead

**Cost of virtual function calls:**
- Modern CPUs: ~1-2 cycles overhead per call
- Negligible for assembler (not hot path)
- Benefits outweigh costs (extensibility, testability)

**Optimization:**
- CPU plugin encoding is called once per instruction per pass
- Typical program: 1000 instructions × 3 passes = 3000 calls
- At 1 nanosecond per call: 3 microseconds total (negligible)

### Memory Considerations

**Plugin storage:**
- CPU plugins: Stateless (opcode tables only)
- Syntax plugins: Minimal state (directive handlers)
- Output plugins: Stateless

**Recommendation:** Use singleton CPU plugins (shared across assemblies)

---

## Future Extensions

### Dynamic Plugin Loading (Phase 4+)

```cpp
// Load plugins from shared libraries (.so, .dll)
class DynamicPluginLoader {
public:
  void LoadPlugin(const std::string& library_path) {
    // dlopen() on Linux, LoadLibrary() on Windows
    void* handle = dlopen(library_path.c_str(), RTLD_LAZY);
    
    // Look up registration function
    typedef void (*RegisterFunc)(CpuRegistry&);
    auto register_func = reinterpret_cast<RegisterFunc>(
        dlsym(handle, "RegisterCpuPlugin"));
    
    // Call registration
    register_func(CpuRegistry::Instance());
  }
};
```

### Plugin Metadata and Discovery

```cpp
struct PluginMetadata {
  std::string name;
  std::string version;
  std::string author;
  std::string description;
  std::vector<std::string> dependencies;
};

class CpuPlugin {
  virtual PluginMetadata GetMetadata() const = 0;
};
```

---

**Document Version:** 1.0  
**Last Updated:** 2026-02-14  
**Related:** [Architecture README](README.md)
