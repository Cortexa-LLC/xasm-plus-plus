# API Specification: 6809 CPU Support

**Date:** 2026-02-03
**Architect:** Claude (Architect Role)
**Status:** Proposed
**Related:** [Architecture Design](architecture.md)

## API Overview

**Purpose:** Define the programming interfaces for the 6809 CPU plugin and EDTASM+ syntax parser.

**Scope:** Internal APIs for xasm++ components, not public end-user APIs.

**Version:** 1.0

**Authentication:** N/A (internal APIs)

---

## Component APIs

### 1. Cpu6809 Plugin API

**Purpose:** Encode 6809 assembly instructions into machine code

**Location:** `include/xasm++/cpu/cpu_6809.h`

---

#### 1.1 Metadata Methods

##### GetName()

**Purpose:** Get the CPU plugin name

**Signature:**
```cpp
std::string GetName() const;
```

**Parameters:** None

**Returns:**
- `std::string` - "6809"

**Example:**
```cpp
Cpu6809 cpu;
std::cout << cpu.GetName(); // Prints: "6809"
```

---

#### 1.2 Direct Page Management

##### SetDirectPage()

**Purpose:** Set the Direct Page (DP) register value

**Signature:**
```cpp
void SetDirectPage(uint8_t dp);
```

**Parameters:**
- `dp` (uint8_t, required): Direct page value (0x00-0xFF)

**Returns:** void

**Side Effects:** Updates internal `direct_page_` state

**Example:**
```cpp
Cpu6809 cpu;
cpu.SetDirectPage(0x20); // DP = $20, direct addressing accesses $20xx
```

##### GetDirectPage()

**Purpose:** Get the current Direct Page register value

**Signature:**
```cpp
uint8_t GetDirectPage() const;
```

**Parameters:** None

**Returns:**
- `uint8_t` - Current DP value (0x00-0xFF)

**Example:**
```cpp
Cpu6809 cpu;
cpu.SetDirectPage(0x10);
uint8_t dp = cpu.GetDirectPage(); // Returns: 0x10
```

---

#### 1.3 Instruction Encoding Methods

##### EncodeLDA()

**Purpose:** Encode LDA (Load Accumulator A) instruction

**Signature:**
```cpp
std::vector<uint8_t> EncodeLDA(uint32_t operand, AddressingMode6809 mode) const;
```

**Parameters:**
- `operand` (uint32_t, required): Address or immediate value
- `mode` (AddressingMode6809, required): Addressing mode

**Returns:**
- `std::vector<uint8_t>` - Encoded instruction bytes

**Supported Modes:**
- `Immediate8` - LDA #$42
- `Direct` - LDA <$80
- `Extended` - LDA $1234
- `Indexed*` - LDA ,X (all indexed variants)

**Example:**
```cpp
Cpu6809 cpu;

// LDA #$42 (immediate)
auto bytes = cpu.EncodeLDA(0x42, AddressingMode6809::Immediate8);
// Returns: {0x86, 0x42}

// LDA $1234 (extended)
auto bytes = cpu.EncodeLDA(0x1234, AddressingMode6809::Extended);
// Returns: {0xB6, 0x12, 0x34}  // Big-endian!
```

**Error Handling:**
- Invalid mode: Returns empty vector `{}`
- Out-of-range operand: Truncates to valid range

---

##### EncodeLDB()

**Purpose:** Encode LDB (Load Accumulator B) instruction

**Signature:**
```cpp
std::vector<uint8_t> EncodeLDB(uint32_t operand, AddressingMode6809 mode) const;
```

**Parameters:** Same as EncodeLDA()

**Returns:** `std::vector<uint8_t>` - Encoded instruction bytes

**Supported Modes:** Same as LDA

**Example:**
```cpp
Cpu6809 cpu;

// LDB #$FF
auto bytes = cpu.EncodeLDB(0xFF, AddressingMode6809::Immediate8);
// Returns: {0xC6, 0xFF}
```

---

##### EncodeLDD()

**Purpose:** Encode LDD (Load Accumulator D) instruction

**Signature:**
```cpp
std::vector<uint8_t> EncodeLDD(uint32_t operand, AddressingMode6809 mode) const;
```

**Parameters:**
- `operand` (uint32_t, required): Address or 16-bit immediate value
- `mode` (AddressingMode6809, required): Addressing mode

**Returns:** `std::vector<uint8_t>` - Encoded instruction bytes

**Supported Modes:**
- `Immediate16` - LDD #$1234
- `Direct` - LDD <$80
- `Extended` - LDD $1234
- `Indexed*` - LDD ,X

**Example:**
```cpp
Cpu6809 cpu;

// LDD #$1234 (16-bit immediate)
auto bytes = cpu.EncodeLDD(0x1234, AddressingMode6809::Immediate16);
// Returns: {0xCC, 0x12, 0x34}  // Big-endian: A=$12, B=$34
```

**Note:** D is the 16-bit concatenation of A (high byte) and B (low byte).

---

##### EncodeSTA()

**Purpose:** Encode STA (Store Accumulator A) instruction

**Signature:**
```cpp
std::vector<uint8_t> EncodeSTA(uint32_t operand, AddressingMode6809 mode) const;
```

**Parameters:**
- `operand` (uint32_t, required): Address to store to
- `mode` (AddressingMode6809, required): Addressing mode

**Returns:** `std::vector<uint8_t>` - Encoded instruction bytes

**Supported Modes:**
- `Direct` - STA <$80
- `Extended` - STA $1234
- `Indexed*` - STA ,X

**Note:** STA does not support Immediate mode (cannot store to a constant).

**Example:**
```cpp
Cpu6809 cpu;

// STA $1234
auto bytes = cpu.EncodeSTA(0x1234, AddressingMode6809::Extended);
// Returns: {0xB7, 0x12, 0x34}
```

---

##### EncodeSTB(), EncodeSTD()

**Purpose:** Store Accumulator B or D

**Signatures:**
```cpp
std::vector<uint8_t> EncodeSTB(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeSTD(uint32_t operand, AddressingMode6809 mode) const;
```

**Parameters/Returns:** Same as EncodeSTA()

---

##### EncodeLDX(), EncodeLDY(), EncodeLDU(), EncodeLDS()

**Purpose:** Load index or stack pointer registers

**Signatures:**
```cpp
std::vector<uint8_t> EncodeLDX(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeLDY(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeLDU(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeLDS(uint32_t operand, AddressingMode6809 mode) const;
```

**Parameters:**
- `operand` (uint32_t, required): Address or 16-bit immediate value
- `mode` (AddressingMode6809, required): Addressing mode

**Supported Modes:**
- `Immediate16` - LDX #$1234
- `Direct` - LDX <$80
- `Extended` - LDX $1234
- `Indexed*` - LDX ,Y

**Example:**
```cpp
Cpu6809 cpu;

// LDX #$1000
auto bytes = cpu.EncodeLDX(0x1000, AddressingMode6809::Immediate16);
// Returns: {0x8E, 0x10, 0x00}  // Big-endian
```

---

##### EncodeSTX(), EncodeSTY(), EncodeSTU(), EncodeSTS()

**Purpose:** Store index or stack pointer registers

**Signatures:**
```cpp
std::vector<uint8_t> EncodeSTX(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeSTY(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeSTU(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeSTS(uint32_t operand, AddressingMode6809 mode) const;
```

**Parameters/Returns:** Same as EncodeLDX()

---

##### Arithmetic Instructions

**Signatures:**
```cpp
std::vector<uint8_t> EncodeADDA(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeADDB(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeADDD(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeSUBA(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeSUBB(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeSUBD(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeMUL() const;  // Inherent
std::vector<uint8_t> EncodeDAA() const;  // Inherent
std::vector<uint8_t> EncodeINC(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeDEC(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeNEG(uint32_t operand, AddressingMode6809 mode) const;
```

**Example:**
```cpp
Cpu6809 cpu;

// ADDA #10
auto bytes = cpu.EncodeADDA(10, AddressingMode6809::Immediate8);
// Returns: {0x8B, 0x0A}

// MUL (A * B → D)
auto bytes = cpu.EncodeMUL();
// Returns: {0x3D}
```

---

##### Branch Instructions

**Signatures:**
```cpp
// Short branches (8-bit offset)
std::vector<uint8_t> EncodeBRA(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeBEQ(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeBNE(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeBCC(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeBCS(uint32_t operand, AddressingMode6809 mode) const;
// ... (all branch variants)

// Long branches (16-bit offset, prefix $10)
std::vector<uint8_t> EncodeLBRA(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeLBEQ(uint32_t operand, AddressingMode6809 mode) const;
// ... (all long branch variants)
```

**Parameters:**
- `operand` (uint32_t, required): Target address
- `mode` (AddressingMode6809, required): Must be Relative8 or Relative16

**Example:**
```cpp
Cpu6809 cpu;

// BEQ LABEL (short branch, 8-bit offset)
// Assume current PC = $1000, target = $1010, offset = $0E
auto bytes = cpu.EncodeBEQ(0x0E, AddressingMode6809::Relative8);
// Returns: {0x27, 0x0E}

// LBEQ LABEL (long branch, 16-bit offset)
// Assume offset = $1234
auto bytes = cpu.EncodeLBEQ(0x1234, AddressingMode6809::Relative16);
// Returns: {0x10, 0x27, 0x12, 0x34}  // Prefix $10 + BEQ opcode + offset
```

**Note:** Offset calculation is the responsibility of the syntax parser/assembler, not the CPU plugin.

---

##### Jump and Subroutine Instructions

**Signatures:**
```cpp
std::vector<uint8_t> EncodeJMP(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeJSR(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeBSR(uint32_t operand, AddressingMode6809 mode) const;
std::vector<uint8_t> EncodeRTS() const;  // Inherent
std::vector<uint8_t> EncodeRTI() const;  // Inherent
```

**Example:**
```cpp
Cpu6809 cpu;

// JMP $1234 (extended)
auto bytes = cpu.EncodeJMP(0x1234, AddressingMode6809::Extended);
// Returns: {0x7E, 0x12, 0x34}

// JSR $1234 (extended)
auto bytes = cpu.EncodeJSR(0x1234, AddressingMode6809::Extended);
// Returns: {0xBD, 0x12, 0x34}

// RTS
auto bytes = cpu.EncodeRTS();
// Returns: {0x39}
```

---

##### Stack Operations

**Signatures:**
```cpp
std::vector<uint8_t> EncodePSHS(uint8_t regmask) const;
std::vector<uint8_t> EncodePSHU(uint8_t regmask) const;
std::vector<uint8_t> EncodePULS(uint8_t regmask) const;
std::vector<uint8_t> EncodePULU(uint8_t regmask) const;
```

**Parameters:**
- `regmask` (uint8_t, required): Register mask (bit field)

**Register Mask Bits:**
- Bit 0: CC (Condition Code)
- Bit 1: A (Accumulator A)
- Bit 2: B (Accumulator B)
- Bit 3: DP (Direct Page)
- Bit 4: X (Index Register X)
- Bit 5: Y (Index Register Y)
- Bit 6: U or S (depending on instruction)
- Bit 7: PC (Program Counter)

**Example:**
```cpp
Cpu6809 cpu;

// PSHS A,B,X (push A, B, and X to S stack)
uint8_t regmask = 0x02 | 0x04 | 0x10; // A | B | X
auto bytes = cpu.EncodePSHS(regmask);
// Returns: {0x34, 0x16}  // PSHS opcode + regmask
```

---

##### Transfer and Exchange

**Signatures:**
```cpp
std::vector<uint8_t> EncodeTFR(uint8_t src, uint8_t dst) const;
std::vector<uint8_t> EncodeEXG(uint8_t src, uint8_t dst) const;
```

**Parameters:**
- `src` (uint8_t, required): Source register code
- `dst` (uint8_t, required): Destination register code

**Register Codes:**
- 0x00: D (A:B)
- 0x01: X
- 0x02: Y
- 0x03: U
- 0x04: S
- 0x05: PC
- 0x08: A
- 0x09: B
- 0x0A: CC
- 0x0B: DP

**Example:**
```cpp
Cpu6809 cpu;

// TFR D,X (transfer D to X)
auto bytes = cpu.EncodeTFR(0x00, 0x01);
// Returns: {0x1F, 0x01}  // TFR opcode + post-byte (D=0, X=1)
```

---

##### CalculateInstructionSize()

**Purpose:** Calculate the size of an encoded instruction

**Signature:**
```cpp
size_t CalculateInstructionSize(AddressingMode6809 mode) const;
```

**Parameters:**
- `mode` (AddressingMode6809, required): Addressing mode

**Returns:**
- `size_t` - Instruction size in bytes (1-5)

**Example:**
```cpp
Cpu6809 cpu;

size_t size = cpu.CalculateInstructionSize(AddressingMode6809::Immediate8);
// Returns: 2 (opcode + 8-bit operand)

size_t size = cpu.CalculateInstructionSize(AddressingMode6809::Extended);
// Returns: 3 (opcode + 16-bit address)
```

---

### 2. EdtasmSyntaxParser API

**Purpose:** Parse EDTASM+ assembly syntax and generate Atom stream

**Location:** `include/xasm++/syntax/edtasm_syntax.h`

---

#### 2.1 Constructor and Setup

##### EdtasmSyntaxParser()

**Purpose:** Construct parser instance

**Signature:**
```cpp
EdtasmSyntaxParser();
```

**Example:**
```cpp
EdtasmSyntaxParser parser;
```

---

##### SetCpu()

**Purpose:** Set CPU plugin for instruction encoding

**Signature:**
```cpp
void SetCpu(Cpu6809* cpu);
```

**Parameters:**
- `cpu` (Cpu6809*, required): Pointer to CPU plugin (must remain valid during parsing)

**Example:**
```cpp
EdtasmSyntaxParser parser;
Cpu6809 cpu;
parser.SetCpu(&cpu);
```

---

#### 2.2 Parsing Methods

##### Parse()

**Purpose:** Parse EDTASM+ source code and populate section/symbols

**Signature:**
```cpp
void Parse(const std::string& source, 
           Section& section, 
           ConcreteSymbolTable& symbols);
```

**Parameters:**
- `source` (const std::string&, required): Assembly source code (multi-line)
- `section` (Section&, required): Section to populate with atoms
- `symbols` (ConcreteSymbolTable&, required): Symbol table to populate with labels

**Returns:** void

**Side Effects:**
- Populates `section` with `ByteAtom`, `LabelAtom`, etc.
- Populates `symbols` with label definitions
- May throw `std::runtime_error` on parse errors

**Example:**
```cpp
EdtasmSyntaxParser parser;
Cpu6809 cpu;
parser.SetCpu(&cpu);

Section section("CODE", 0x01, 0x6000);
ConcreteSymbolTable symbols;

std::string source = R"(
        ORG     $6000
START   LDA     #$42
        STA     $1234
LOOP    BRA     LOOP
        END     START
)";

parser.Parse(source, section, symbols);

// Now section contains atoms and symbols contains labels
```

**Error Handling:**
- Syntax error: Throws `std::runtime_error` with line number and context
- Undefined label: Throws error (unless forward reference, resolved in later pass)
- Invalid opcode: Throws error with suggestion if possible

---

### 3. AddressingMode6809 Enumeration

**Purpose:** Define addressing modes for 6809 CPU

**Location:** `include/xasm++/cpu/cpu_6809.h`

**Values:**

```cpp
enum class AddressingMode6809 {
    Inherent,                    // No operand
    Immediate8,                  // 8-bit immediate
    Immediate16,                 // 16-bit immediate
    Direct,                      // Direct page
    Extended,                    // 16-bit absolute
    
    // Indexed modes (16 sub-modes)
    IndexedZeroOffset,           // ,X ,Y ,U ,S
    Indexed5BitOffset,           // -16,X to +15,X
    Indexed8BitOffset,           // -128,X to +127,X
    Indexed16BitOffset,          // -32768,X to +32767,X
    IndexedAccumA,               // A,X
    IndexedAccumB,               // B,Y
    IndexedAccumD,               // D,U
    IndexedAutoInc1,             // ,X+
    IndexedAutoInc2,             // ,X++
    IndexedAutoDec1,             // ,-X
    IndexedAutoDec2,             // ,--X
    IndexedPCRelative8,          // label,PCR (8-bit)
    IndexedPCRelative16,         // label,PCR (16-bit)
    IndexedIndirect,             // [,X] [10,Y]
    IndexedExtendedIndirect,     // [$1234]
    
    // Relative modes
    Relative8,                   // Short branches
    Relative16,                  // Long branches
};
```

**Usage:**
```cpp
AddressingMode6809 mode = AddressingMode6809::Immediate8;
auto bytes = cpu.EncodeLDA(0x42, mode);
```

---

## Data Models

### OpcodeTable Structure

**Purpose:** Map addressing modes to opcodes for an instruction

**Definition:**
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
```

**Example:**
```cpp
// LDA instruction opcode table
static const OpcodeTable LDA_TABLE = {
    .immediate8 = 0x86,
    .direct = 0x96,
    .extended = 0xB6,
    .indexed = 0xA6,
    // No inherent or relative modes for LDA
};
```

---

### MacroDefinition Structure

**Purpose:** Store macro definition for expansion

**Definition:**
```cpp
struct MacroDefinition {
    std::string name;                    // Macro name
    std::vector<std::string> body;       // Lines of macro body
    int param_count;                     // Number of parameters
};
```

**Example:**
```cpp
MacroDefinition macro = {
    .name = "DELAY",
    .body = {"        LDB     \\1", ".LOOP   DECB", "        BNE     .LOOP"},
    .param_count = 1
};
```

---

### ConditionalBlock Structure

**Purpose:** Track conditional assembly state

**Definition:**
```cpp
struct ConditionalBlock {
    bool condition;       // True if condition is met
    bool in_else_block;   // True if in ELSE block
    bool should_emit;     // True if code should be emitted
};
```

---

## Error Handling

### Error Codes

The APIs use C++ exceptions for error handling:

- `std::runtime_error` - General errors (syntax errors, undefined labels)
- `std::invalid_argument` - Invalid parameters
- `std::out_of_range` - Numeric values out of range

### Error Message Format

```
Error: [message]
  at [file]:[line]
  [source line]
  [^ marker at error position]

Suggestion: [hint if applicable]
```

**Example:**
```
Error: Undefined label 'UNKNWN'
  at test.asm:10
  10:         BRA     UNKNWN
                      ^
Suggestion: Did you mean 'UNKNOWN'?
```

---

## Versioning

**API Version:** 1.0
**Stability:** Unstable (pre-1.0 release)

**Breaking Changes Policy:**
- Major version bump for breaking changes
- Minor version bump for new features (backward compatible)
- Patch version bump for bug fixes

**Deprecation Policy:**
- Deprecated methods marked with `[[deprecated]]` attribute
- Removed after 2 minor version releases

---

## Performance Characteristics

### Time Complexity

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Encode instruction | O(1) | Constant time opcode lookup |
| Parse line | O(n) | Linear in line length |
| Symbol lookup | O(log n) | Balanced tree (std::map) |
| Macro expansion | O(m * k) | m = macro body size, k = expansion count |

### Space Complexity

| Data Structure | Complexity | Notes |
|----------------|------------|-------|
| Opcode tables | O(1) | Static tables |
| Symbol table | O(n) | n = number of symbols |
| Macro definitions | O(m) | m = total macro body size |
| Section atoms | O(a) | a = number of atoms |

---

## Thread Safety

**Current Status:** Not thread-safe

**Reasoning:** Single-threaded assembly is adequate for typical use cases

**Future Consideration:** Thread-safe encoding (stateless CPU plugin) for parallel assembly

**Thread-Safe Components:**
- Opcode table lookups (const static data)
- ToBigEndian() utility (pure function)

**Non-Thread-Safe Components:**
- Direct page register state (`direct_page_`)
- Parser state (`current_address_`, `conditional_stack_`, etc.)
- Symbol table mutations

---

## Examples

### Complete Assembly Example

```cpp
#include "xasm++/cpu/cpu_6809.h"
#include "xasm++/syntax/edtasm_syntax.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"

int main() {
    // Create components
    Cpu6809 cpu;
    EdtasmSyntaxParser parser;
    parser.SetCpu(&cpu);
    
    Section section("CODE", 0x01, 0x6000);
    ConcreteSymbolTable symbols;
    
    // Assembly source
    std::string source = R"(
* Simple 6809 program
        ORG     $6000
        
START   LDX     #$1000      ; Load X with $1000
        LDA     #$FF        ; Load A with $FF
LOOP    STA     ,X+         ; Store A to [X], increment X
        CMPX    #$2000      ; Compare X to $2000
        BNE     LOOP        ; Loop if not equal
        RTS                 ; Return
        
        END     START
    )";
    
    // Parse
    try {
        parser.Parse(source, section, symbols);
        
        // Output binary
        auto& atoms = section.GetAtoms();
        for (const auto& atom : atoms) {
            if (auto byte_atom = dynamic_cast<ByteAtom*>(atom.get())) {
                std::cout << std::hex << (int)byte_atom->GetValue() << " ";
            }
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Assembly error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

---

## Related Documents

- [Architecture Design](architecture.md)
- [Data Models](data-models.md)
- [6809 Instruction Set Reference](../../reference/6809/instruction-set.md)
- [EDTASM+ Syntax Reference](../../reference/6809/edtasm-syntax.md)

---

**Changelog:**

| Date | Version | Changes | Author |
|------|---------|---------|--------|
| 2026-02-03 | 1.0 | Initial API specification | Claude (Architect) |

---

**End of API Specification**
