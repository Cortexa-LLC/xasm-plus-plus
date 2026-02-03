# Data Models: 6809 CPU Support

**Date:** 2026-02-03
**Architect:** Claude (Architect Role)
**Status:** Proposed
**Related:** [Architecture Design](architecture.md)

## Overview

This document defines the data models, structures, and encodings for the 6809 CPU plugin and EDTASM+ syntax parser. It covers instruction encoding, addressing modes, opcode tables, and data representations.

---

## 6809 Instruction Encoding

### Instruction Format

6809 instructions consist of:
1. **Opcode byte(s)** - 1 or 2 bytes (page 2/3 instructions have prefix)
2. **Post-byte** (optional) - Indexed addressing mode encoding
3. **Operand byte(s)** (optional) - 0-2 bytes depending on addressing mode

**General Format:**
```
[Prefix] [Opcode] [Post-byte] [Operand1] [Operand2]
   0-1      1         0-1        0-1        0-1
```

**Examples:**
```
NOP:           12                               (1 byte)
LDA #$42:      86 42                            (2 bytes)
LDA $1234:     B6 12 34                         (3 bytes)
LDA 10,X:      A6 8A                            (2 bytes - indexed)
LDA 1000,X:    A6 89 03 E8                      (4 bytes - indexed 16-bit)
SWI2:          10 3F                            (2 bytes - page 2)
```

---

## Addressing Modes

### Addressing Mode Enumeration

```cpp
enum class AddressingMode6809 {
    // Basic modes
    Inherent,           // No operand: NOP, CLRA, RTS
    Immediate8,         // 8-bit immediate: LDA #$42
    Immediate16,        // 16-bit immediate: LDD #$1234
    Direct,             // Direct page: LDA <$80
    Extended,           // 16-bit absolute: LDA $1234
    
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
    
    // Relative modes (branches only)
    Relative8,          // Short branches: BEQ label (-128 to +127)
    Relative16,         // Long branches: LBEQ label (-32768 to +32767)
};
```

### Addressing Mode Size Table

| Mode | Instruction Size | Notes |
|------|------------------|-------|
| Inherent | 1 byte | Opcode only (or 2 bytes for page 2/3) |
| Immediate8 | 2 bytes | Opcode + 8-bit value |
| Immediate16 | 3 bytes | Opcode + 16-bit value (big-endian) |
| Direct | 2 bytes | Opcode + 8-bit offset |
| Extended | 3 bytes | Opcode + 16-bit address (big-endian) |
| Relative8 | 2 bytes | Opcode + 8-bit signed offset |
| Relative16 | 3 bytes | Opcode + 16-bit signed offset (big-endian) |
| IndexedZeroOffset | 2 bytes | Opcode + post-byte |
| Indexed5BitOffset | 2 bytes | Opcode + post-byte (offset in post-byte) |
| Indexed8BitOffset | 3 bytes | Opcode + post-byte + 8-bit offset |
| Indexed16BitOffset | 4 bytes | Opcode + post-byte + 16-bit offset |
| IndexedAccumA/B/D | 2 bytes | Opcode + post-byte |
| IndexedAutoInc1/Dec1 | 2 bytes | Opcode + post-byte |
| IndexedAutoInc2/Dec2 | 2 bytes | Opcode + post-byte |
| IndexedPCRelative8 | 3 bytes | Opcode + post-byte + 8-bit offset |
| IndexedPCRelative16 | 4 bytes | Opcode + post-byte + 16-bit offset |
| IndexedIndirect | 2-4 bytes | Depends on sub-mode + indirect flag |
| IndexedExtendedIndirect | 5 bytes | Opcode + post-byte + 16-bit address |

---

## Indexed Addressing Post-Byte Encoding

The 6809's indexed addressing is the most complex feature. The post-byte encodes:
- Register selection (X, Y, U, S, PC)
- Addressing sub-mode (16 variants)
- Offset size (0, 5-bit, 8-bit, 16-bit)
- Indirection flag

### Post-Byte Format

```
Bit 7: Indirect flag (0 = direct, 1 = indirect)
Bit 6-5: Register selection
        00 = X
        01 = Y
        10 = U
        11 = S
Bit 4-0: Mode-specific encoding
```

### Register Encoding

| Register | Bits 6-5 |
|----------|----------|
| X | 00 |
| Y | 01 |
| U | 10 |
| S | 11 |
| PC | N/A (special handling for PC-relative) |

### Mode Encoding Table

| Mode | Post-Byte Format | Example | Encoding |
|------|------------------|---------|----------|
| **No Offset** | 1RR00100 | `,X` | 10000100 (0x84) |
| **5-bit Offset** | 0RRnnnnn | `10,X` | 00001010 (0x0A) |
| **8-bit Offset** | 1RR01000 + byte | `100,X` | 10001000 (0x88) + 0x64 |
| **16-bit Offset** | 1RR01001 + word | `1000,X` | 10001001 (0x89) + 0x03E8 |
| **Accumulator A** | 1RR00110 | `A,X` | 10000110 (0x86) |
| **Accumulator B** | 1RR00101 | `B,Y` | 01000101 (0x45) |
| **Accumulator D** | 1RR01011 | `D,U` | 10001011 (0x8B) |
| **Auto-increment +1** | 1RR00000 | `,X+` | 10000000 (0x80) |
| **Auto-increment +2** | 1RR00001 | `,X++` | 10000001 (0x81) |
| **Auto-decrement -1** | 1RR00010 | `,-X` | 10000010 (0x82) |
| **Auto-decrement -2** | 1RR00011 | `,--X` | 10000011 (0x83) |
| **PC-relative 8-bit** | 1RR01100 + byte | `label,PCR` | 10001100 (0x8C) + offset |
| **PC-relative 16-bit** | 1RR01101 + word | `label,PCR` | 10001101 (0x8D) + offset |
| **Indirect** | 1RR1xxxx | `[,X]` | 10010100 (0x94) |
| **Extended Indirect** | 10011111 + word | `[$1234]` | 10011111 (0x9F) + 0x1234 |

### 5-bit Offset Encoding

For offsets in the range **-16 to +15**, the offset is encoded directly in bits 4-0 of the post-byte:

```
Post-byte = 0RRnnnnn
  0 = 5-bit offset indicator
  RR = register (00=X, 01=Y, 10=U, 11=S)
  nnnnn = 5-bit two's complement offset
```

**Examples:**
- `0,X` → 00000000 (0x00)
- `1,X` → 00000001 (0x01)
- `15,X` → 00001111 (0x0F)
- `-1,X` → 00011111 (0x1F)
- `-16,X` → 00010000 (0x10)

**Note:** For offset = 0, use "no offset" mode (0x84) instead for efficiency.

### Indirect Flag

Setting bit 7 makes the addressing indirect:

```
Direct:    LDA  10,X    → A6 8A (post-byte = 10001010)
Indirect:  LDA  [10,X]  → A6 9A (post-byte = 10011010)
                              ^ bit 7 set
```

---

## Opcode Tables

### OpcodeTable Structure

```cpp
struct OpcodeTable {
    std::optional<uint8_t> immediate8;    // 8-bit immediate
    std::optional<uint8_t> immediate16;   // 16-bit immediate
    std::optional<uint8_t> direct;        // Direct page
    std::optional<uint8_t> extended;      // 16-bit absolute
    std::optional<uint8_t> indexed;       // Indexed (all variants)
    std::optional<uint8_t> inherent;      // No operand
    std::optional<uint8_t> relative8;     // 8-bit relative
    std::optional<uint8_t> relative16;    // 16-bit relative (prefix $10)
};
```

### Example Opcode Tables

#### LDA (Load Accumulator A)

```cpp
static const OpcodeTable LDA_TABLE = {
    .immediate8 = 0x86,
    .direct = 0x96,
    .indexed = 0xA6,
    .extended = 0xB6,
    // No immediate16, inherent, relative modes
};
```

**Encoding Examples:**
- `LDA #$42` → 86 42
- `LDA <$80` → 96 80
- `LDA ,X` → A6 84
- `LDA $1234` → B6 12 34

#### LDD (Load Accumulator D)

```cpp
static const OpcodeTable LDD_TABLE = {
    .immediate16 = 0xCC,
    .direct = 0xDC,
    .indexed = 0xEC,
    .extended = 0xFC,
    // No immediate8 (16-bit register), inherent, relative modes
};
```

**Encoding Examples:**
- `LDD #$1234` → CC 12 34 (big-endian!)
- `LDD <$80` → DC 80
- `LDD ,Y` → EC A4 (post-byte for ,Y)
- `LDD $1234` → FC 12 34

#### BEQ (Branch if Equal)

```cpp
static const OpcodeTable BEQ_TABLE = {
    .relative8 = 0x27,
    .relative16 = 0x1027,  // Page 2: prefix $10 + opcode $27
    // No immediate, direct, extended, indexed, inherent modes
};
```

**Encoding Examples:**
- `BEQ LABEL` (near) → 27 FE (offset = -2)
- `LBEQ LABEL` (far) → 10 27 12 34 (offset = $1234)

#### PSHS (Push to S Stack)

```cpp
static const OpcodeTable PSHS_TABLE = {
    .immediate8 = 0x34,  // Register mask is "immediate" operand
    // No other modes
};
```

**Encoding Examples:**
- `PSHS A,B` → 34 06 (regmask = 0x02 | 0x04)
- `PSHS X,Y,U` → 34 70 (regmask = 0x10 | 0x20 | 0x40)

---

## Byte Order (Endianness)

### Big-Endian Convention

The 6809 uses **big-endian** byte order (MSB first), unlike the 6502 (little-endian).

**16-bit Value Encoding:**
```cpp
uint16_t value = 0x1234;

// Big-endian (6809):
bytes = {0x12, 0x34}  // High byte first

// Little-endian (6502):
bytes = {0x34, 0x12}  // Low byte first
```

**Utility Function:**
```cpp
std::vector<uint8_t> ToBigEndian(uint16_t value) const {
    return {
        static_cast<uint8_t>((value >> 8) & 0xFF),  // High byte
        static_cast<uint8_t>(value & 0xFF)           // Low byte
    };
}
```

**Examples:**
- `LDD #$1234` → CC 12 34 (A=$12, B=$34)
- `LDX #$ABCD` → 8E AB CD (X=$ABCD)
- `FDB $5678` → 56 78 (word in memory)

---

## Register Set

### Register Encoding

| Register | Code | Size | Description |
|----------|------|------|-------------|
| D | 0x00 | 16-bit | Accumulator D (A:B concatenated) |
| X | 0x01 | 16-bit | Index Register X |
| Y | 0x02 | 16-bit | Index Register Y |
| U | 0x03 | 16-bit | User Stack Pointer |
| S | 0x04 | 16-bit | System Stack Pointer |
| PC | 0x05 | 16-bit | Program Counter |
| A | 0x08 | 8-bit | Accumulator A (high byte of D) |
| B | 0x09 | 8-bit | Accumulator B (low byte of D) |
| CC | 0x0A | 8-bit | Condition Code Register |
| DP | 0x0B | 8-bit | Direct Page Register |

**TFR/EXG Encoding:**

For `TFR src,dst` and `EXG src,dst`:
- Post-byte = (src_code << 4) | dst_code

**Examples:**
- `TFR D,X` → 1F 01 (D=0, X=1)
- `EXG A,B` → 1E 89 (A=8, B=9)

### Condition Code Register (CC)

```
Bit 7: E  - Entire state saved flag
Bit 6: F  - Fast interrupt mask
Bit 5: H  - Half carry (from bit 3)
Bit 4: I  - Interrupt mask
Bit 3: N  - Negative
Bit 2: Z  - Zero
Bit 1: V  - Overflow
Bit 0: C  - Carry
```

**ANDCC/ORCC Encoding:**
- `ANDCC #$FE` → 1C FE (clear carry)
- `ORCC #$50` → 1A 50 (set F and I flags)

### Stack Register Mask (PSHS/PULS/PSHU/PULU)

```
Bit 7: PC  - Program Counter
Bit 6: U/S - U for PSHS/PULS, S for PSHU/PULU
Bit 5: Y   - Index Register Y
Bit 4: X   - Index Register X
Bit 3: DP  - Direct Page Register
Bit 2: B   - Accumulator B
Bit 1: A   - Accumulator A
Bit 0: CC  - Condition Code Register
```

**Examples:**
- `PSHS A,B,X` → 34 16 (regmask = 0x02 | 0x04 | 0x10)
- `PULS CC,D,X,Y,U,PC` → 35 FF (all registers)

**Order of Push/Pull:**
- Push: PC, U/S, Y, X, DP, B, A, CC (left to right decreasing addresses)
- Pull: CC, A, B, DP, X, Y, U/S, PC (right to left increasing addresses)

---

## Direct Page Addressing

### Direct Page Register (DP)

The 6809 has a configurable Direct Page (DP) register that sets the high byte for direct addressing.

**Default:** DP = $00 (like 6502 zero page)

**Effective Address:**
```
EA = (DP << 8) | direct_offset
```

**Example:**
```asm
        SETDP   $10         ; Set DP to $10
        LDA     $80         ; Loads from $1080 (DP:$80), not $0080
```

**Direct vs Extended Optimization:**

The assembler must choose between direct (2 bytes) and extended (3 bytes) addressing:

```
DP = $20, operand = $2080:
  LDA <$80    → 96 80      (direct, 2 bytes)
  LDA $2080   → B6 20 80   (extended, 3 bytes)
  Both access the same address!
```

**SETDP Directive Requirement:**

The assembler needs explicit `SETDP` to optimize correctly:

```asm
        ORG     $2000
        SETDP   $20         ; Inform assembler: DP = $20

VAR1    RMB     1           ; $2000
VAR2    RMB     1           ; $2001

        LDA     VAR1        ; Assembler uses direct mode (96 00)
                            ; Without SETDP, would use extended (B6 20 00)
```

---

## Branch Offset Calculation

### Short Branches (8-bit offset)

**Range:** -128 to +127 bytes from the address *after* the branch instruction

**Calculation:**
```
offset = target_address - (branch_address + 2)
         ^^^^^^^^^^^^^^^^   ^^^^^^^^^^^^^^^^
         where to go        address after branch
```

**Example:**
```
Address  Code   Instruction
$1000    27 FE  BEQ $1000     ; Loop to self
                              ; offset = $1000 - ($1000 + 2) = -2 = $FE
$1002    ...

$1010    27 08  BEQ $101A     ; Branch forward
                              ; offset = $101A - ($1010 + 2) = +8
$1012    ...
$101A    ...    ; Target
```

### Long Branches (16-bit offset)

**Range:** -32768 to +32767 bytes

**Calculation:**
```
offset = target_address - (branch_address + 4)
         ^^^^^^^^^^^^^^^^   ^^^^^^^^^^^^^^^^
         where to go        address after long branch (4 bytes)
```

**Example:**
```
Address  Code         Instruction
$1000    10 27 12 30  LBEQ $2234   ; Long branch
                                   ; offset = $2234 - ($1000 + 4) = $1230
```

---

## Macro Data Model

### MacroDefinition Structure

```cpp
struct MacroDefinition {
    std::string name;                    // Macro name
    std::vector<std::string> body;       // Lines of macro body
    int param_count;                     // Number of parameters used
};
```

**Example:**
```cpp
MacroDefinition delay_macro = {
    .name = "DELAY",
    .body = {
        "        LDB     \\1",     // Parameter \\1
        ".LOOP   DECB",            // Local label .LOOP
        "        BNE     .LOOP"
    },
    .param_count = 1
};
```

### Macro Expansion

**Parameter Substitution:**
- `\\1` through `\\9` - Positional parameters
- `\\@` - Unique invocation ID (incrementing counter)
- `\\.label` - Local label (mangled with invocation ID)

**Example:**
```asm
DELAY   MACRO
        LDB     \1
\.LOOP  DECB
        BNE     \.LOOP
        ENDM

        DELAY   #10    ; Expands to:
                       ;   LDB #10
                       ;   DELAY_1.LOOP  DECB
                       ;   BNE DELAY_1.LOOP

        DELAY   #20    ; Expands to:
                       ;   LDB #20
                       ;   DELAY_2.LOOP  DECB
                       ;   BNE DELAY_2.LOOP
```

---

## Conditional Assembly Model

### ConditionalBlock Structure

```cpp
struct ConditionalBlock {
    bool condition;       // True if condition met
    bool in_else_block;   // True if currently in ELSE block
    bool should_emit;     // True if code should be emitted
};

std::vector<ConditionalBlock> conditional_stack_;  // Stack for nesting
```

**Example:**
```asm
DEBUG   EQU     1

        IFEQ    DEBUG          ; Condition: DEBUG == 0 (false)
        ; This code NOT assembled
        ELSE
        ; This code IS assembled (in else block)
        LDA     #$FF
        ENDIF
```

**Nesting:**
```asm
        IFNE    VAL1           ; Outer: VAL1 != 0
        LDA     #1
        IFNE    VAL2           ; Inner: VAL2 != 0
        LDA     #2
        ENDIF                  ; End inner
        ENDIF                  ; End outer
```

---

## Symbol Table Model

### Symbol Entry

```cpp
struct Symbol {
    std::string name;         // Symbol name
    uint32_t value;           // Symbol value (address or constant)
    bool is_label;            // True if label, false if EQU/SET
    bool is_constant;         // True if EQU, false if SET (can redefine)
    int section_id;           // Section where defined
};
```

**Examples:**
- Label: `START` → {name="START", value=0x6000, is_label=true}
- Constant: `BUFSIZE EQU 256` → {name="BUFSIZE", value=256, is_constant=true}
- Variable: `COUNT SET 0` → {name="COUNT", value=0, is_constant=false}

---

## Number Format Parsing

### Number Format Recognition

| Format | Prefix | Example | Value |
|--------|--------|---------|-------|
| Decimal | None | `255` | 255 |
| Hexadecimal | `$` | `$FF` | 255 |
| Binary | `%` | `%11111111` | 255 |
| Character | `'` | `'A'` | 65 (ASCII) |

### Parsing Logic

```cpp
uint32_t ParseNumber(const std::string& str) {
    if (str[0] == '$') {
        // Hexadecimal
        return std::stoul(str.substr(1), nullptr, 16);
    } else if (str[0] == '%') {
        // Binary
        return std::stoul(str.substr(1), nullptr, 2);
    } else if (str[0] == '\'' && str.size() == 3 && str[2] == '\'') {
        // Character constant
        return static_cast<uint8_t>(str[1]);
    } else {
        // Decimal
        return std::stoul(str, nullptr, 10);
    }
}
```

---

## Expression Evaluation Model

### Expression Tree

Expressions are parsed into AST (Abstract Syntax Tree) for evaluation:

```cpp
class Expression {
public:
    virtual uint32_t Evaluate(const SymbolTable& symbols) const = 0;
};

class LiteralExpression : public Expression {
    uint32_t value_;
public:
    uint32_t Evaluate(const SymbolTable&) const override { return value_; }
};

class SymbolExpression : public Expression {
    std::string symbol_;
public:
    uint32_t Evaluate(const SymbolTable& symbols) const override {
        return symbols.GetValue(symbol_);
    }
};

class BinaryExpression : public Expression {
    enum class Op { Add, Sub, Mul, Div, And, Or, Xor };
    std::shared_ptr<Expression> left_, right_;
    Op op_;
public:
    uint32_t Evaluate(const SymbolTable& symbols) const override {
        uint32_t l = left_->Evaluate(symbols);
        uint32_t r = right_->Evaluate(symbols);
        switch (op_) {
            case Op::Add: return l + r;
            case Op::Sub: return l - r;
            case Op::Mul: return l * r;
            case Op::Div: return l / r;
            // ...
        }
    }
};
```

**Example:**
```asm
RESULT  EQU     (BUFSIZE+10)*2

; Expression tree:
;        *
;       / \
;      +   2
;     / \
; BUFSIZE 10
```

---

## Related Documents

- [Architecture Design](architecture.md)
- [API Specification](api-spec.md)
- [6809 Instruction Set Reference](../../reference/6809/instruction-set.md)
- [EDTASM+ Syntax Reference](../../reference/6809/edtasm-syntax.md)

---

**Changelog:**

| Date | Version | Changes | Author |
|------|---------|---------|--------|
| 2026-02-03 | 1.0 | Initial data models document | Claude (Architect) |

---

**End of Data Models Document**
