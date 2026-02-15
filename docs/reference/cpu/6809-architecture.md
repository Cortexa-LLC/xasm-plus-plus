# Architecture Design: 6809 CPU Support with EDTASM+ Syntax

**Date:** 2026-02-03
**Architect:** Claude (Architect Role)
**Status:** Proposed
**Beads Task:** xasm++-7iy
**Priority:** P2 (High)

## Architecture Overview

This document defines the technical architecture for adding Motorola 6809 CPU support with EDTASM+ assembler syntax to xasm++. The design extends the existing CPU plugin and syntax parser frameworks while maintaining consistency with the current 6502/65816 implementation patterns.

**High-Level Architecture:**

```
┌─────────────────────────────────────────────────────────────────┐
│                          xasm++ Core                             │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────────┐    │
│  │  Assembler   │  │   Symbol     │  │    Expression      │    │
│  │   Engine     │  │   Table      │  │    Evaluator       │    │
│  └──────────────┘  └──────────────┘  └───────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
           │                    │                    │
           ▼                    ▼                    ▼
┌─────────────────────┐  ┌──────────────────────────────────────┐
│   CPU Plugin API    │  │     Syntax Parser API                │
├─────────────────────┤  ├──────────────────────────────────────┤
│ • Cpu6502           │  │ • MerlinSyntaxParser                 │
│ • Cpu65816          │  │ • ScmasmSyntaxParser                 │
│ • Cpu6809 (NEW) ────┼──┼─→ EdtasmSyntaxParser (NEW)           │
└─────────────────────┘  └──────────────────────────────────────┘
           │                                │
           ▼                                ▼
    ┌─────────────┐                ┌──────────────┐
    │  Opcode     │                │  Directive   │
    │  Encoding   │                │  Handlers    │
    └─────────────┘                └──────────────┘
```

**Integration Points:**
1. **CPU Plugin:** `Cpu6809` class implements instruction encoding for all 6809 opcodes
2. **Syntax Parser:** `EdtasmSyntaxParser` class implements EDTASM+ directive handling and parsing
3. **CLI Integration:** `--cpu 6809 --syntax edtasm` command-line flags
4. **Build System:** CMakeLists.txt updates to include new source files

## System Context

### Systems Involved

**Existing Components (Reused):**
- **Core Assembler:** Multi-pass assembly engine, expression evaluator, symbol table
- **Section Management:** Code/data organization
- **Output Plugins:** Binary, symbol, listing output
- **CLI Framework:** Command-line parsing (CLI11)
- **Build System:** CMake-based build

**New Components (To Be Created):**
- **Cpu6809 Plugin:** 6809 instruction encoding (partial stub exists)
- **EdtasmSyntaxParser:** EDTASM+ syntax parsing
- **6809 Test Suite:** Unit and integration tests

### External Dependencies

**Build-Time:**
- CLI11 (v2.4.2) - Command-line parsing
- GoogleTest (v1.14.0) - Testing framework
- CMake (≥3.20) - Build system

**Runtime:**
- C++20 standard library
- No external runtime dependencies

**Reference Materials:**
- vasm-ext EDTASM implementation: `~/Projects/Vintage/tools/vasm-ext/syntax/edtasm`
- 6809 instruction reference: `docs/reference/6809/instruction-set.md`
- EDTASM+ syntax reference: `docs/reference/6809/edtasm-syntax.md`

## Component Architecture

### Component 1: Cpu6809 Plugin

**Responsibility:** Encode 6809 assembly instructions into machine code bytes

**Location:** 
- Header: `include/xasm++/cpu/cpu_6809.h`
- Implementation: `src/cpu/m6809/cpu_6809.cpp`

**Current State:** Partial stub implementation exists with:
- Basic inherent instructions (NOP, RTS, CLRA, CLRB)
- Load/Store instructions (LDA, LDB, LDD, STA, STB, STD)
- Immediate, Direct, Extended addressing modes
- Direct page register management
- Big-endian byte order utilities

**Interfaces:**

```cpp
class Cpu6809 {
public:
    // Metadata
    std::string GetName() const;
    
    // Direct page management
    void SetDirectPage(uint8_t dp);
    uint8_t GetDirectPage() const;
    
    // Instruction encoding (59 base instructions)
    // Data Movement
    std::vector<uint8_t> EncodeLDA(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLDB(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLDD(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLDX(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLDY(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLDU(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLDS(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeSTA(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeSTB(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeSTD(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeSTX(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeSTY(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeSTU(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeSTS(uint32_t operand, AddressingMode6809 mode) const;
    
    // Arithmetic
    std::vector<uint8_t> EncodeADDA(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeADDB(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeADDD(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeSUBA(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeSUBB(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeSUBD(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeMUL() const;
    std::vector<uint8_t> EncodeDAA() const;
    std::vector<uint8_t> EncodeINC(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeDEC(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeNEG(uint32_t operand, AddressingMode6809 mode) const;
    
    // Logical
    std::vector<uint8_t> EncodeANDA(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeANDB(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeORA(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeORB(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeEORA(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeEORB(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeCOM(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeTST(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeCLR(uint32_t operand, AddressingMode6809 mode) const;
    
    // Shifts and Rotates
    std::vector<uint8_t> EncodeASL(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeASR(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLSL(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLSR(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeROL(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeROR(uint32_t operand, AddressingMode6809 mode) const;
    
    // Branches (short - 8-bit offset)
    std::vector<uint8_t> EncodeBRA(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBEQ(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBNE(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBCC(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBCS(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBPL(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBMI(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBVC(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBVS(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBGT(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBGE(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBLT(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBLE(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBHI(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBHS(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBLO(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBLS(uint32_t operand, AddressingMode6809 mode) const;
    
    // Long branches (16-bit offset, prefix $10)
    std::vector<uint8_t> EncodeLBRA(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLBEQ(uint32_t operand, AddressingMode6809 mode) const;
    // ... (all long branch variants)
    
    // Jumps and Subroutines
    std::vector<uint8_t> EncodeJMP(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeJSR(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeBSR(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeRTS() const;
    std::vector<uint8_t> EncodeRTI() const;
    
    // Stack Operations
    std::vector<uint8_t> EncodePSHS(uint8_t regmask) const;
    std::vector<uint8_t> EncodePSHU(uint8_t regmask) const;
    std::vector<uint8_t> EncodePULS(uint8_t regmask) const;
    std::vector<uint8_t> EncodePULU(uint8_t regmask) const;
    std::vector<uint8_t> EncodeLEAS(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLEAU(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLEAX(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeLEAY(uint32_t operand, AddressingMode6809 mode) const;
    
    // Register Operations
    std::vector<uint8_t> EncodeTFR(uint8_t src, uint8_t dst) const;
    std::vector<uint8_t> EncodeEXG(uint8_t src, uint8_t dst) const;
    std::vector<uint8_t> EncodeCMPA(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeCMPB(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeCMPD(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeCMPX(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeCMPY(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeCMPU(uint32_t operand, AddressingMode6809 mode) const;
    std::vector<uint8_t> EncodeCMPS(uint32_t operand, AddressingMode6809 mode) const;
    
    // Condition Code
    std::vector<uint8_t> EncodeANDCC(uint8_t mask) const;
    std::vector<uint8_t> EncodeORCC(uint8_t mask) const;
    std::vector<uint8_t> EncodeCWAI(uint8_t mask) const;
    std::vector<uint8_t> EncodeSYNC() const;
    
    // Special
    std::vector<uint8_t> EncodeNOP() const;
    std::vector<uint8_t> EncodeSWI() const;
    std::vector<uint8_t> EncodeSWI2() const;
    std::vector<uint8_t> EncodeSWI3() const;
    std::vector<uint8_t> EncodeSEX() const;
    std::vector<uint8_t> EncodeABX() const;
    
    // Size calculation
    size_t CalculateInstructionSize(AddressingMode6809 mode) const;

private:
    // Opcode table structures
    struct OpcodeTable {
        std::optional<uint8_t> immediate8;
        std::optional<uint8_t> immediate16;
        std::optional<uint8_t> direct;
        std::optional<uint8_t> extended;
        std::optional<uint8_t> indexed;
        std::optional<uint8_t> inherent;
        std::optional<uint8_t> relative8;
        std::optional<uint8_t> relative16;
    };
    
    // Generic encoding using table (reduce duplication)
    std::vector<uint8_t> EncodeWithTable(const OpcodeTable& table,
                                          uint32_t operand,
                                          AddressingMode6809 mode) const;
    
    // Indexed addressing post-byte encoding
    std::vector<uint8_t> EncodeIndexedPostByte(AddressingMode6809 mode,
                                                int32_t offset,
                                                uint8_t reg) const;
    
    // Byte order conversion
    std::vector<uint8_t> ToBigEndian(uint16_t value) const;
    
    // State
    uint8_t direct_page_ = 0x00;
};
```

**Dependencies:**
- `<cstdint>`, `<string>`, `<vector>`, `<optional>` (C++ standard library)
- No dependencies on other xasm++ components (self-contained plugin)

**Implementation Notes:**
- Big-endian byte order (MSB first) for 16-bit values
- Indexed addressing uses complex post-byte encoding (16 sub-modes)
- Multi-page opcode space: page 1 (normal), page 2 (prefix $10), page 3 (prefix $11)
- Direct page register defaults to $00 but configurable via SETDP directive

---

### Component 2: EdtasmSyntaxParser

**Responsibility:** Parse EDTASM+ assembly syntax and generate Atom stream

**Location:**
- Header: `include/xasm++/syntax/edtasm_syntax.h`
- Implementation: `src/syntax/edtasm/edtasm_syntax.cpp`

**Current State:** Does not exist - must be created

**Interfaces:**

```cpp
class EdtasmSyntaxParser {
public:
    EdtasmSyntaxParser();
    
    // Set CPU plugin for instruction encoding
    void SetCpu(Cpu6809* cpu);
    
    // Parse source into atoms and symbols
    void Parse(const std::string& source, 
               Section& section, 
               ConcreteSymbolTable& symbols);

private:
    // Line parsing
    std::string StripComments(const std::string& line);
    void ParseLine(const std::string& line, 
                   Section& section, 
                   ConcreteSymbolTable& symbols);
    
    // Label parsing
    std::string ParseLabel(const std::string& line, 
                          size_t& pos, 
                          Section& section,
                          ConcreteSymbolTable& symbols);
    
    // Directive handlers
    void HandleOrg(const std::string& operand, 
                   Section& section, 
                   ConcreteSymbolTable& symbols);
    void HandleEnd();
    void HandleEqu(const std::string& label, 
                   const std::string& operand,
                   ConcreteSymbolTable& symbols);
    void HandleSet(const std::string& label, 
                   const std::string& operand,
                   ConcreteSymbolTable& symbols);
    void HandleFCB(const std::string& operand, 
                   Section& section, 
                   ConcreteSymbolTable& symbols);
    void HandleFDB(const std::string& operand, 
                   Section& section, 
                   ConcreteSymbolTable& symbols);
    void HandleFCC(const std::string& operand, 
                   Section& section);
    void HandleRMB(const std::string& operand, 
                   Section& section, 
                   ConcreteSymbolTable& symbols);
    void HandleSetDP(const std::string& operand, 
                     ConcreteSymbolTable& symbols);
    void HandleNAM(const std::string& operand);
    void HandleTTL(const std::string& operand);
    void HandlePAGE();
    void HandleSPC(const std::string& operand);
    void HandleOPT(const std::string& operand);
    void HandleLIST();
    void HandleNOLIST();
    
    // Conditional assembly (P1)
    void HandleCOND(const std::string& operand, 
                    ConcreteSymbolTable& symbols);
    void HandleENDC();
    void HandleIFEQ(const std::string& operand, 
                    ConcreteSymbolTable& symbols);
    void HandleIFNE(const std::string& operand, 
                    ConcreteSymbolTable& symbols);
    void HandleIFGT(const std::string& operand, 
                    ConcreteSymbolTable& symbols);
    void HandleIFLT(const std::string& operand, 
                    ConcreteSymbolTable& symbols);
    void HandleIFGE(const std::string& operand, 
                    ConcreteSymbolTable& symbols);
    void HandleIFLE(const std::string& operand, 
                    ConcreteSymbolTable& symbols);
    void HandleELSE();
    void HandleENDIF();
    
    // INCLUDE directive (P1)
    void HandleINCLUDE(const std::string& operand, 
                       Section& section, 
                       ConcreteSymbolTable& symbols);
    
    // Macro system (P1)
    void HandleMACRO(const std::string& name);
    void HandleENDM();
    void ExpandMacro(const std::string& name, 
                     const std::string& operand,
                     Section& section, 
                     ConcreteSymbolTable& symbols);
    std::string SubstituteParameters(const std::string& line,
                                    const std::vector<std::string>& params);
    
    // OS-9 support (P2)
    void HandleMOD(const std::string& operand, 
                   Section& section, 
                   ConcreteSymbolTable& symbols);
    
    // Expression/number parsing
    uint32_t ParseNumber(const std::string& str);
    std::shared_ptr<Expression> ParseExpression(const std::string& str,
                                               ConcreteSymbolTable& symbols);
    
    // Instruction parsing
    void ParseInstruction(const std::string& mnemonic,
                         const std::string& operand,
                         Section& section,
                         ConcreteSymbolTable& symbols);
    
    // Addressing mode determination
    AddressingMode6809 DetermineAddressingMode(const std::string& operand);
    
    // Error formatting
    std::string FormatError(const std::string& message) const;

private:
    // State
    Cpu6809* cpu_ = nullptr;
    uint32_t current_address_ = 0;
    bool end_directive_seen_ = false;
    
    // Conditional assembly state
    struct ConditionalBlock {
        bool condition;
        bool in_else_block;
        bool should_emit;
    };
    std::vector<ConditionalBlock> conditional_stack_;
    
    // Macro state
    struct MacroDefinition {
        std::string name;
        std::vector<std::string> body;
        int param_count;
    };
    bool in_macro_definition_ = false;
    MacroDefinition current_macro_;
    std::unordered_map<std::string, MacroDefinition> macros_;
    int macro_expansion_depth_ = 0;
    
    // INCLUDE state
    std::vector<std::string> include_stack_;
    std::vector<std::string> include_paths_;
    
    // Listing control
    bool listing_enabled_ = true;
    
    // Source location (error reporting)
    std::string current_file_;
    int current_line_ = 0;
};
```

**Dependencies:**
- `Cpu6809` - For instruction encoding
- `Section` - For atom generation
- `ConcreteSymbolTable` - For symbol management
- `Expression` - For expression evaluation

**Implementation Notes:**
- Case-insensitive by default (configurable with `--nocase` flag)
- Comment handling: `*` in column 1, `;` inline
- Number formats: decimal, `$hex`, `%binary`, `'char'`
- FCC directive uses flexible delimiter (first non-whitespace after FCC)
- Macro parameters: `\\1`-`\\9`, local labels: `\\.label`, unique IDs: `\\@`

---

### Component 3: CLI Integration

**Responsibility:** Extend command-line interface to support 6809 CPU and EDTASM syntax

**Location:** `src/cli/command_line_options.cpp`

**Changes Required:**

```cpp
// Add 6809 to CPU enum
enum class CpuType {
    CPU_6502,
    CPU_65816,
    CPU_6809  // NEW
};

// Add EDTASM to syntax enum
enum class SyntaxType {
    SYNTAX_SIMPLE,
    SYNTAX_MERLIN,
    SYNTAX_SCMASM,
    SYNTAX_EDTASM  // NEW
};

// CLI flag registration
app.add_option("--cpu", cpu_type, "CPU type: 6502, 65816, 6809")
   ->transform(CLI::CheckedTransformer(cpu_map, CLI::ignore_case));

app.add_option("--syntax", syntax_type, "Syntax: simple, merlin, scmasm, edtasm")
   ->transform(CLI::CheckedTransformer(syntax_map, CLI::ignore_case));

// Validation
if (cpu_type == CpuType::CPU_6809 && syntax_type != SyntaxType::SYNTAX_EDTASM) {
    throw CLI::ValidationError("6809 CPU requires EDTASM syntax");
}
```

**Dependencies:**
- CLI11 library (existing)
- `Cpu6809` and `EdtasmSyntaxParser` classes

---

### Component 4: Build System Integration

**Responsibility:** Integrate new source files into CMake build

**Location:** `CMakeLists.txt`, `src/CMakeLists.txt`

**Changes Required:**

```cmake
# src/CMakeLists.txt
set(XASM_SOURCES
    # ... existing files ...
    
    # 6809 CPU Plugin
    cpu/m6809/cpu_6809.cpp
    
    # EDTASM Syntax Parser
    syntax/edtasm/edtasm_syntax.cpp
)

set(XASM_HEADERS
    # ... existing headers ...
    
    # 6809 CPU Plugin
    ${CMAKE_SOURCE_DIR}/include/xasm++/cpu/cpu_6809.h
    
    # EDTASM Syntax Parser
    ${CMAKE_SOURCE_DIR}/include/xasm++/syntax/edtasm_syntax.h
)
```

**Directory Structure:**
```
src/
├── cpu/
│   ├── m6502/
│   │   ├── cpu_6502.cpp
│   │   ├── cpu_65816.cpp
│   │   └── cpu_6502_branch_handler.cpp
│   └── m6809/
│       └── cpu_6809.cpp (NEW - stub exists)
├── syntax/
│   ├── merlin/
│   │   └── merlin_syntax.cpp
│   ├── scmasm/
│   │   └── scmasm_syntax.cpp
│   └── edtasm/ (NEW)
│       └── edtasm_syntax.cpp (NEW)
```

---

## Data Architecture

### Addressing Mode Enumeration

The 6809 uses a fundamentally different addressing model than the 6502:

```cpp
enum class AddressingMode6809 {
    // Basic modes
    Inherent,           // No operand (NOP, CLRA)
    Immediate8,         // 8-bit immediate (#$FF)
    Immediate16,        // 16-bit immediate (#$1234)
    Direct,             // Direct page (DP:$80)
    Extended,           // 16-bit absolute ($1234)
    
    // Indexed modes (16 sub-modes)
    IndexedZeroOffset,       // ,X ,Y ,U ,S
    Indexed5BitOffset,       // -16,X to +15,X
    Indexed8BitOffset,       // -128,X to +127,X
    Indexed16BitOffset,      // -32768,X to +32767,X
    IndexedAccumA,           // A,X
    IndexedAccumB,           // B,Y
    IndexedAccumD,           // D,U
    IndexedAutoInc1,         // ,X+
    IndexedAutoInc2,         // ,X++
    IndexedAutoDec1,         // ,-X
    IndexedAutoDec2,         // ,--X
    IndexedPCRelative8,      // label,PCR (8-bit)
    IndexedPCRelative16,     // label,PCR (16-bit)
    IndexedIndirect,         // [,X] [10,Y]
    IndexedExtendedIndirect, // [$1234]
    
    // Relative modes (branches)
    Relative8,          // Short branches (-128 to +127)
    Relative16,         // Long branches (-32768 to +32767)
};
```

**Key Design Decision:** Separate enum from 6502's `AddressingMode` to avoid confusion and enable type safety.

### Opcode Tables

**Structure:**
```cpp
struct OpcodeTable {
    std::optional<uint8_t> immediate8;
    std::optional<uint8_t> immediate16;
    std::optional<uint8_t> direct;
    std::optional<uint8_t> extended;
    std::optional<uint8_t> indexed;
    std::optional<uint8_t> inherent;
    std::optional<uint8_t> relative8;
    std::optional<uint8_t> relative16;
};

// Example: LDA instruction
static const OpcodeTable LDA_TABLE = {
    .immediate8 = 0x86,
    .direct = 0x96,
    .extended = 0xB6,
    .indexed = 0xA6,
    // No inherent, relative modes for LDA
};
```

**Rationale:** Reduces code duplication - one table per instruction, generic encoding function.

### Indexed Addressing Post-Byte Encoding

The 6809's indexed addressing is the most complex feature. The post-byte encodes:
- Index register selection (X, Y, U, S, PC)
- Addressing mode variant (16 sub-modes)
- Offset size (0, 5-bit, 8-bit, 16-bit)
- Indirection flag

**Post-Byte Format:**
```
Bit 7: Indirect flag (0=direct, 1=indirect)
Bit 6-5: Register selection (00=X, 01=Y, 10=U, 11=S)
Bit 4-0: Mode encoding (varies by sub-mode)
```

**Example Encodings:**
- `,X` (no offset): `10000100` (0x84)
- `10,X` (5-bit): `10001010` (0x8A) - offset in bits 4-0
- `100,X` (8-bit): `10001000` (0x88) + offset byte
- `1000,X` (16-bit): `10001001` (0x89) + 2 offset bytes
- `[,X]` (indirect): `10010100` (0x94)

**Implementation Strategy:**
1. Parse operand string to determine sub-mode
2. Calculate post-byte value based on sub-mode and register
3. Append offset bytes if needed
4. Return post-byte + offset bytes

---

## Integration Architecture

### Assembler Engine Integration

**Flow:**

```
User Source File (EDTASM+ syntax)
         │
         ▼
EdtasmSyntaxParser::Parse()
         │
         ├─→ Parse labels → ConcreteSymbolTable
         ├─→ Parse directives → HandleXXX()
         └─→ Parse instructions → ParseInstruction()
                    │
                    ▼
         DetermineAddressingMode()
                    │
                    ▼
         Cpu6809::EncodeXXX(operand, mode)
                    │
                    ▼
         std::vector<uint8_t> (machine code)
                    │
                    ▼
         Section::AddAtom(ByteAtom)
                    │
                    ▼
         Multi-Pass Assembly (resolve symbols)
                    │
                    ▼
         Binary Output
```

**Multi-Pass Resolution:**

The 6809 requires careful multi-pass handling for:
1. **Direct vs Extended addressing:** SETDP directive affects optimization
2. **Indexed offset sizes:** 5-bit, 8-bit, or 16-bit offset selection
3. **Branch distance:** Short (8-bit) vs long (16-bit) branch selection
4. **Symbol forward references:** Labels defined after use

**Pass 1:** Gather all symbols, determine approximate sizes
**Pass 2:** Resolve symbols, optimize addressing modes
**Pass 3:** Final encoding with all symbols resolved

### Directive Handling Strategy

**Core Directives (Phase 0 - P0):**
- `ORG` - Set origin address
- `END` - Mark end of assembly
- `EQU` - Define constant symbol (cannot redefine)
- `SET` - Define variable symbol (can redefine)
- `FCB` - Form constant byte(s)
- `FDB` - Form double byte(s) - **big-endian!**
- `FCC` - Form constant characters (flexible delimiter)
- `RMB` - Reserve memory bytes

**Advanced Directives (Phase 1 - P1):**
- `INCLUDE` - Include external file
- `MACRO`/`ENDM` - Macro definition
- `COND`/`ENDC` - Conditional assembly
- `IFEQ`, `IFNE`, `IFGT`, `IFLT`, `IFGE`, `IFLE` - Conditional variants
- `ELSE`, `ENDIF` - Conditional control
- `SETDP` - Set direct page for optimization
- `NAM`, `TTL`, `PAGE`, `SPC` - Listing control
- `OPT`, `LIST`, `NOLIST` - Options control

**OS-9 Directives (Phase 2 - P2):**
- `MOD` - OS-9 module header
- `OS9` - OS-9 system call macro

---

## Non-Functional Architecture

### Performance Architecture

**Target Performance:**
- Assembly speed: >10,000 lines/second
- Memory usage: <100MB for typical projects (<50K lines)
- Startup time: <100ms

**Optimization Strategies:**

1. **Opcode Table Lookup:** O(1) opcode selection using `std::optional<uint8_t>`
2. **String Interning:** Reuse common strings (label names, directive keywords)
3. **Expression Caching:** Cache evaluated expressions across passes
4. **Indexed Post-Byte Cache:** Memoize complex post-byte calculations

**Profiling Points:**
- Opcode encoding time
- Expression evaluation time
- Symbol table lookup time
- String parsing overhead

### Security Architecture

**Security Considerations:**

1. **Path Traversal Prevention:**
   - INCLUDE directive must sanitize file paths
   - Restrict to specified include directories
   - Prevent `../` escape sequences

2. **Macro Recursion Limits:**
   - Maximum macro expansion depth: 100 levels
   - Detect circular macro invocations
   - Prevent stack overflow attacks

3. **Expression Evaluation Safety:**
   - Bounded integer arithmetic (prevent overflow)
   - No arbitrary code execution
   - Safe division by zero handling

4. **Input Validation:**
   - Validate all numeric inputs (range checks)
   - Sanitize string inputs (buffer overflow prevention)
   - Limit source file size (<100MB)

### Scalability Architecture

**Scalability Strategies:**

1. **Multi-Pass Design:**
   - Linear O(n) complexity per pass
   - Maximum 3 passes for most programs
   - Symbol table: O(log n) lookup (std::map)

2. **Memory Management:**
   - Stream processing (don't load entire file in memory)
   - Incremental atom generation
   - Symbol table: compact representation

3. **Parallel Processing (Future):**
   - Instruction encoding is stateless (except DP register)
   - Potential for parallel encoding of independent sections
   - Thread-safe opcode tables

**Bottleneck Prevention:**
- Avoid repeated string allocations (use string_view)
- Preallocate vectors for known sizes
- Use move semantics for large data structures

---

## Technology Stack

### Programming Language

**Choice:** C++20

**Rationale:**
- Consistency with existing xasm++ codebase
- Strong type safety (enum class, optional)
- Zero-cost abstractions
- Excellent performance
- Cross-platform compatibility

**Alternatives Considered:**
- **Rust:** Would require rewriting entire project
- **C:** Lack of type safety, harder to maintain
- **Python:** Performance inadequate for assembler

### Build System

**Choice:** CMake (≥3.20)

**Rationale:**
- Already used by xasm++
- Cross-platform (Linux, macOS, Windows)
- Good IDE integration
- FetchContent for dependencies

### Testing Framework

**Choice:** GoogleTest (v1.14.0)

**Rationale:**
- Already integrated in xasm++
- Comprehensive feature set
- Good reporting and mocking
- Industry standard

### Dependency Management

**Choice:** CMake FetchContent

**Rationale:**
- No external package managers needed
- Versioned dependencies
- Reproducible builds
- Works on all platforms

---

## Architecture Decision Records (ADRs)

### ADR-001: Big-Endian Byte Order Handling

**Status:** Accepted
**Date:** 2026-02-03

**Context:**
The 6809 uses big-endian byte order (MSB first), while xasm++ was originally designed for the 6502 (little-endian, LSB first). We need a strategy for handling byte order differences.

**Decision:**
Create a `ToBigEndian(uint16_t value)` utility function in `Cpu6809` that returns `{high_byte, low_byte}`. Use this consistently for all 16-bit values (FDB, addresses, immediate16).

**Rationale:**
- **Encapsulation:** Byte order logic contained in CPU plugin
- **Type Safety:** Return `std::vector<uint8_t>` instead of manual bit manipulation
- **Maintainability:** Single function to change if needed
- **Clarity:** Explicit big-endian conversion in code

**Consequences:**
- **Positive:** Clean separation of byte order concerns
- **Positive:** Easy to test byte order correctness
- **Negative:** Slight overhead from vector allocation (negligible)

**Alternatives Considered:**
- **Inline bit shifts:** More error-prone, harder to maintain
- **Template metaprogramming:** Over-engineered for this case
- **Platform byte order detection:** Unnecessary - 6809 is always big-endian

---

### ADR-002: Direct Page Optimization Strategy

**Status:** Accepted
**Date:** 2026-02-03

**Context:**
The 6809's direct addressing mode can access any 256-byte page, controlled by the Direct Page (DP) register. The assembler needs to decide when to use direct (2 bytes) vs extended (3 bytes) addressing.

**Decision:**
1. Require explicit `SETDP` directive to inform assembler of DP value
2. Default DP = $00 if no SETDP directive
3. Use direct mode only when address matches DP:$xx pattern
4. Provide assembler warning if extended mode used when direct would work
5. No automatic DP tracking (too complex, error-prone)

**Rationale:**
- **Explicitness:** Programmer controls optimization explicitly
- **Correctness:** Avoids misassembling if DP changes at runtime
- **Compatibility:** Matches vasm-ext and LWASM behavior
- **Simplicity:** No need to track DP changes across program flow

**Consequences:**
- **Positive:** Predictable behavior, no surprises
- **Positive:** Binary output matches reference assemblers
- **Negative:** Programmer must remember to use SETDP
- **Negative:** No automatic optimization (but safer)

**Alternatives Considered:**
- **Automatic DP tracking:** Too complex, requires data flow analysis
- **Always use extended:** Wasteful, defeats purpose of direct mode
- **Command-line DP flag:** Less flexible than per-source SETDP

---

### ADR-003: Macro System Implementation Approach

**Status:** Accepted
**Date:** 2026-02-03

**Context:**
EDTASM+ has a powerful macro system with parameters (`\\1`-`\\9`), local labels (`\\.label`), and unique IDs (`\\@`). We need to decide how to implement macro expansion.

**Decision:**
1. **Text substitution approach:** Store macro body as vector of strings
2. **Parameter substitution:** Simple string replacement of `\\1` through `\\9`
3. **Local label mangling:** Append `_macro_name_invocation_count` to `\\.label`
4. **Unique ID:** Replace `\\@` with incrementing counter
5. **Recursion limit:** Maximum 100 levels of expansion
6. **Circular detection:** Track expansion stack, error on circular reference

**Rationale:**
- **Simplicity:** Text-based approach easier to implement and debug
- **Compatibility:** Matches original EDTASM+ behavior
- **Performance:** Adequate for typical macro usage patterns
- **Safety:** Recursion limits prevent stack overflow

**Consequences:**
- **Positive:** Easy to implement and test
- **Positive:** Compatible with existing EDTASM+ macros
- **Negative:** Repeated parsing of macro body (could cache AST)
- **Negative:** Error messages inside macros less clear

**Alternatives Considered:**
- **AST-based macros:** More complex, harder to maintain
- **Hygienic macros (Scheme-style):** Over-engineered for assembly
- **No macro support:** Users would need to use external preprocessor

---

### ADR-004: Indexed Addressing Post-Byte Encoding

**Status:** Accepted
**Date:** 2026-02-03

**Context:**
The 6809's indexed addressing has 16 sub-modes encoded in a post-byte. This is the most complex aspect of the 6809 architecture. We need a clear encoding strategy.

**Decision:**
1. **Parse operand string** to identify sub-mode (regex patterns)
2. **Build post-byte** based on sub-mode and register
3. **Append offset bytes** if needed (8-bit or 16-bit)
4. **Use lookup table** for post-byte calculation
5. **Separate function:** `EncodeIndexedPostByte(mode, offset, reg)`

**Parsing Strategy:**
```
Pattern                  Mode
-------                  ----
",X"                  → IndexedZeroOffset
"5,X"                 → Indexed5BitOffset (-16 to +15)
"100,X"               → Indexed8BitOffset (-128 to +127)
"1000,X"              → Indexed16BitOffset (-32768 to +32767)
"A,X"                 → IndexedAccumA
"B,Y"                 → IndexedAccumB
"D,U"                 → IndexedAccumD
",X+"                 → IndexedAutoInc1
",X++"                → IndexedAutoInc2
",-X"                 → IndexedAutoDec1
",--X"                → IndexedAutoDec2
"label,PCR"           → IndexedPCRelative (8 or 16 bit)
"[,X]"                → IndexedIndirect
"[$1234]"             → IndexedExtendedIndirect
```

**Post-Byte Encoding:**
```cpp
uint8_t EncodeIndexedPostByte(AddressingMode6809 mode, int32_t offset, uint8_t reg) {
    uint8_t post_byte = 0x80; // Indexed mode bit
    post_byte |= (reg & 0x03) << 5; // Register selection
    
    switch (mode) {
        case IndexedZeroOffset:       post_byte |= 0x04; break;
        case Indexed5BitOffset:       post_byte |= (offset & 0x1F); break;
        case Indexed8BitOffset:       post_byte |= 0x08; break;
        case Indexed16BitOffset:      post_byte |= 0x09; break;
        case IndexedAccumA:           post_byte |= 0x06; break;
        case IndexedAccumB:           post_byte |= 0x05; break;
        case IndexedAccumD:           post_byte |= 0x0B; break;
        case IndexedAutoInc1:         post_byte |= 0x00; break;
        case IndexedAutoInc2:         post_byte |= 0x01; break;
        case IndexedAutoDec1:         post_byte |= 0x02; break;
        case IndexedAutoDec2:         post_byte |= 0x03; break;
        case IndexedPCRelative8:      post_byte |= 0x0C; break;
        case IndexedPCRelative16:     post_byte |= 0x0D; break;
        case IndexedIndirect:         post_byte |= 0x10; break; // + indirect flag
        case IndexedExtendedIndirect: post_byte = 0x9F; break;
    }
    
    return post_byte;
}
```

**Rationale:**
- **Clarity:** Separate function easier to understand and test
- **Maintainability:** Indexed logic isolated from instruction encoding
- **Testability:** Can unit test post-byte generation independently
- **Reference:** Matches 6809 datasheet encoding exactly

**Consequences:**
- **Positive:** Clear separation of concerns
- **Positive:** Easy to verify against reference implementation
- **Negative:** Complex parsing logic (but unavoidable)

**Alternatives Considered:**
- **Inline encoding:** Would make instruction encoding unreadable
- **Macro-based encoding:** Less flexible, harder to debug
- **Runtime calculation:** Could cache, but complexity not warranted

---

## Related Documents

**Product Requirements:**
- [PRD: 6809 CPU Support](../../product/6809-cpu-support/prd.md)

**Reference Documentation:**
- [6809 Instruction Set Reference](../../reference/6809/instruction-set.md)
- [EDTASM+ Syntax Reference](../../reference/6809/edtasm-syntax.md)

**Implementation:**
- Task Packet: `.ai/tasks/2026-02-02_6809-cpu-support/`
- Beads Task: xasm++-7iy

**External References:**
- vasm-ext EDTASM module: `~/Projects/Vintage/tools/vasm-ext/syntax/edtasm/`
- Motorola 6809 Datasheet
- 6809 Assembly Language Programming (Lance Leventhal)
- Disk EDTASM Remastered (Tandy)

---

## Implementation Phasing

### Phase 2: Core Implementation (MVP)

**Goal:** Working 6809 assembler with basic functionality

**CPU Plugin Implementation:**
1. Complete all 59 base instructions (stub implementation extended)
2. Implement all addressing modes (immediate, direct, extended, indexed)
3. Indexed addressing post-byte encoding (all 16 sub-modes)
4. Big-endian byte order handling
5. Unit tests for each instruction and addressing mode

**Syntax Parser Implementation:**
1. Create `EdtasmSyntaxParser` class structure
2. Implement core directives (ORG, END, EQU, SET, FCB, FDB, FCC, RMB)
3. Implement comment parsing (`*` and `;`)
4. Implement number format parsing (`$`, `%`, `'`, decimal)
5. Implement instruction parsing and addressing mode determination
6. Integrate with `Cpu6809` for encoding
7. Unit tests for each directive and parser feature

**CLI Integration:**
1. Add `--cpu 6809` flag
2. Add `--syntax edtasm` flag
3. Validation: 6809 requires EDTASM syntax

**Build System:**
1. Add `src/cpu/m6809/cpu_6809.cpp` to build
2. Add `src/syntax/edtasm/edtasm_syntax.cpp` to build
3. Update include paths

**Acceptance Criteria:**
- All 59 instructions assemble correctly
- All addressing modes work
- Core directives functional
- Simple 6809 programs assemble
- Unit tests passing (>80% coverage)

---

### Phase 3: EDTASM+ Features (Post-MVP)

**Goal:** Feature parity with EDTASM+

**INCLUDE Directive:**
1. File path resolution (absolute, relative, include paths)
2. Recursive include support
3. Circular include detection
4. Include path search (`-I` flag)

**Macro System:**
1. MACRO/ENDM definition
2. Parameter substitution (`\\1`-`\\9`)
3. Local labels (`\\.label`) - mangle with invocation ID
4. Unique IDs (`\\@`) - incrementing counter
5. Macro expansion with proper symbol scoping
6. Recursion limit (100 levels)

**Conditional Assembly:**
1. COND/ENDC basic conditionals
2. IF/ELSE/ENDIF family (IFEQ, IFNE, IFGT, IFLT, IFGE, IFLE)
3. Nested conditional support
4. Expression evaluation for conditions

**Advanced Directives:**
1. SETDP (direct page optimization)
2. NAM, TTL (listing metadata)
3. PAGE, SPC (listing formatting)
4. OPT (listing options)
5. LIST/NOLIST (listing control)

**Acceptance Criteria:**
- INCLUDE works with complex projects
- Macros expand correctly with all features
- Conditional assembly tested
- Real EDTASM+ code assembles

---

### Phase 4: Validation and Polish (Quality)

**Goal:** Production-ready release

**Binary Compatibility:**
1. Test suite: 100+ test cases covering all features
2. Byte-by-byte comparison with vasm-ext output
3. Assemble real TRS-80 CoCo programs (3+ programs)
4. Regression tests

**Performance Optimization:**
1. Profile assembly speed
2. Optimize hot paths (opcode lookup, expression eval)
3. Memory usage optimization
4. Target: >10,000 lines/second

**Error Message Quality:**
1. Clear, actionable error messages
2. Context display (line number, source line)
3. Suggestions for common mistakes
4. Error message tests

**Documentation:**
1. User guide for 6809 support
2. EDTASM+ migration guide (from vasm, LWASM)
3. Examples and tutorials
4. API documentation (Doxygen)

**Acceptance Criteria:**
- 100% binary compatibility on test suite
- 3+ real programs assemble successfully
- Performance targets met
- Error messages clear and helpful
- Documentation complete

---

## Risks and Mitigations

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| **Indexed addressing complexity** | High | Medium | Reference Leventhal book, vasm-ext implementation, incremental testing |
| **EDTASM+ syntax edge cases** | Medium | Medium | Use vasm-ext as reference, test with real code, community feedback |
| **Binary incompatibility with vasm** | High | Low | Extensive test suite, byte-by-byte comparison, reference implementation |
| **Macro system complexity** | Medium | Medium | Phased implementation (start basic), recursion limits, careful testing |
| **Big-endian byte order bugs** | High | Low | Unit tests for byte order, ToBigEndian() function, careful review |
| **Direct page optimization errors** | Medium | Low | Explicit SETDP directive required, warnings for suboptimal code |
| **Performance below targets** | Low | Low | Profile early, optimize hot paths, use appropriate data structures |

---

## Validation Strategy

### Unit Testing

**CPU Plugin Tests:**
- Test each instruction with all supported addressing modes
- Test big-endian byte order conversion
- Test indexed post-byte encoding (all 16 sub-modes)
- Test direct page register management
- Test instruction size calculation

**Syntax Parser Tests:**
- Test each directive with valid/invalid inputs
- Test comment parsing
- Test number format parsing
- Test expression evaluation
- Test addressing mode determination
- Test macro expansion
- Test conditional assembly
- Test INCLUDE directive

**Integration Tests:**
- Assemble simple programs, verify binary output
- Test multi-pass symbol resolution
- Test forward references
- Test complex expressions
- Test nested macros and conditionals

### Binary Compatibility Validation

**Test Suite:**
1. Create 100+ test cases covering:
   - All instructions
   - All addressing modes
   - All directives
   - Complex expressions
   - Macros and conditionals
   - INCLUDE usage

2. Assemble with both xasm++ and vasm-ext
3. Compare binary output byte-by-byte
4. Report any differences

**Real-World Validation:**
1. Assemble existing TRS-80 CoCo programs
2. Compare output with reference assembler
3. Test on hardware or accurate emulator
4. Verify programs run correctly

### Performance Testing

**Benchmarks:**
- Small program (100 lines) - should be <10ms
- Medium program (1000 lines) - should be <100ms
- Large program (10000 lines) - should be <1s
- Very large program (50000 lines) - should be <5s

**Memory Profiling:**
- Track memory usage during assembly
- Verify no memory leaks
- Optimize if usage exceeds 100MB for typical projects

---

## Success Criteria

**Architecture Design Success:**
- ✓ Clear component boundaries defined
- ✓ Integration points specified
- ✓ Data models documented
- ✓ ADRs created for major decisions
- ✓ Technology choices justified
- ✓ Risks identified and mitigated
- ✓ Implementation phases defined
- ✓ Validation strategy documented

**Implementation Success (Future):**
- All 59 6809 instructions supported
- EDTASM+ syntax fully compatible
- Binary output matches vasm-ext (100%)
- Real TRS-80 CoCo code assembles (3+ programs)
- Performance targets met (>10,000 lines/sec)
- Unit test coverage >80%
- Documentation complete
- Community adoption (10+ GitHub stars)

---

## Next Steps

1. **Architect Review:** This architecture document
2. **User Approval:** Architecture design
3. **Engineer Handoff:** Begin Phase 2 implementation
4. **Create Task Packets:** Break down implementation into tasks
5. **Implement CPU Plugin:** Complete Cpu6809 class
6. **Implement Syntax Parser:** Create EdtasmSyntaxParser class
7. **Integration Testing:** Validate end-to-end
8. **Binary Compatibility:** Test against vasm-ext
9. **Documentation:** User guide and examples
10. **Release:** Version 0.2.0 with 6809 support

---

**Changelog:**

| Date | Version | Changes | Author |
|------|---------|---------|--------|
| 2026-02-03 | 1.0 | Initial architecture design | Claude (Architect) |

---

**Approval:**

- [ ] Architect (Author): Claude
- [ ] Project Maintainer: [Pending]
- [ ] Engineer (Reviewer): [Pending]

---

**End of Architecture Design Document**
