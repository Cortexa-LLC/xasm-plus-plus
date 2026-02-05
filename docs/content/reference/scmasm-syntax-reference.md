---
sidebar_position: 2
---

# S-C Macro Assembler (SCMASM) Syntax Reference

**Target Platform:** Apple II series (//e, //c, II+, IIe)
**CPU Support:** 6502, Sweet-16, 65C02, 65R02, 65802, 65816
**Versions Covered:** 2.0, 3.0, 3.1
**Purpose:** Complete syntax reference for xasm++ SCMASM compatibility

---

## Table of Contents

- [Version History](#version-history)
- [Overview](#overview)
- [Quick Reference](#quick-reference)
- [Source Line Format](#source-line-format)
- [Line Numbers](#line-numbers)
- [Labels](#labels)
- [Number Formats](#number-formats)
- [String Directives](#string-directives)
- [Data Definition](#data-definition)
- [Expressions](#expressions)
- [Addressing Modes](#addressing-modes)
- [Directives Reference](#directives-reference)
- [Macro System](#macro-system)
- [Conditional Assembly](#conditional-assembly)
- [CPU Modes](#cpu-modes)
- [Comparison with Merlin](#comparison-with-merlin)
- [Implementation Notes](#implementation-notes)

---

## Version History

### Version 2.0 (Original Release)

**Core Features:**
- Standard 6502/65C02/65R02/65816 support
- `.OP` directive for CPU selection
- Macro support with `\0`-`\9` parameters
- Conditional assembly (`.DO`/`.ELSE`/`.FIN`)
- String directives (`.AS`, `.AT`)
- Line numbering (0-65535)
- Local labels (`.N` numeric)
- 32-bit expression evaluation
- Boolean operators (`&`, `|`, `~`)

**Data Directives:**
- `.DA` - Define address/data (8/16/24/32-bit)
- `.HS` - Hexadecimal string
- `.BS` - Block storage
- `.DFB` - Define byte

**String Semantics:**
- Delimiter choice controls high bit

### Version 3.0 (Major Enhancement)

**New Features:**
- **Indirect indexed addressing:** `[exp]` and `[exp],Y` syntax
- **Underscores in symbols:** `MY_LABEL` now valid
- **Label-only lines:** Semicolon terminates opcode search
- **Macro parameter delimiters:** Improved bracket/quote handling
- **`.LIST XOFF`:** Suppress extra listing lines
- **`.AZ` directive:** Null-terminated ASCII strings
- **Mixed hex/ASCII:** `.AS`, `.AT`, `.AZ` support embedded hex
- **`.HS` commas:** Comma-separated hex bytes allowed
- **`.DA` string support:** `$$` prefix for ASCII strings
- **`.EQ` 24/32-bit:** Full value preservation (no truncation)
- **`.TF` TSYS support:** Generate Apple II SYS files

**Compatibility:**
- Backward compatible with 2.0 syntax
- Enhanced parameter substitution
- Better error reporting

### Version 3.1 (Current)

**Refinements:**
- Enhanced error messages
- Improved macro expansion
- Additional listing options
- Expression evaluation improvements

---

## Overview

The S-C Macro Assembler (SCASM) is a professional macro assembler developed by S-C Software Corporation (Bob Sander-Cederlof). It provides comprehensive support for:

- **6502** - Standard 6502 instruction set (default)
- **Sweet-16** - Apple's 16-bit interpreter
- **65C02** - CMOS 6502 with enhanced instructions
- **65R02** - Rockwell 65C02 with bit manipulation
- **65802/65816** - Full 16-bit processors

**Key Philosophy:** BASIC-style interface with professional assembler features.

**Unique Features:**
- BASIC-style line numbering (optional)
- Local labels (`.N` forward-reference only)
- 32-bit expression evaluation
- Flexible string delimiter semantics
- Memory protection
- Target file generation

---

## Quick Reference

### Common Directives

| Directive | Purpose | Example |
|-----------|---------|---------|
| `.OR` | Set origin | `.OR $0800` |
| `.EQ` | Define symbol | `BUFSIZE .EQ 256` |
| `.SE` | Set variable | `COUNT .SE 0` |
| `.DA` | Define address | `.DA START,END` |
| `.HS` | Hex bytes | `.HS 01,02,03` |
| `.AS` | ASCII string | `.AS /HELLO/` |
| `.AT` | ASCII terminated | `.AT /HELLO/` |
| `.AZ` | ASCII null-term | `.AZ /HELLO/` |
| `.BS` | Block storage | `.BS 256,$00` |
| `.MA` | Define macro | `.MA MACRONAME` |
| `.EM` | End macro | `.EM` |
| `.DO` | Conditional | `.DO DEBUG` |
| `.FI` | End conditional | `.FI` |
| `.OP` | CPU mode | `.OP 65C02` |
| `.TF` | Target file | `.TF OUTPUT` |

### String Delimiter Semantics (CRITICAL)

**Version 2.0+ Behavior:**

Delimiter ASCII value controls high bit setting:

| Delimiter | ASCII Value | High Bit | Usage |
|-----------|-------------|----------|-------|
| `!` | 0x21 | SET | Inverse video |
| `"` | 0x22 | SET | Standard |
| `#` | 0x23 | SET | Inverse |
| `$` | 0x24 | SET | Inverse |
| `%` | 0x25 | SET | Inverse |
| `&` | 0x26 | SET | Inverse |
| `'` | 0x27 | CLEAR | Normal |
| `/` | 0x2F | CLEAR | Normal (common) |
| `:` | 0x3A | CLEAR | Normal |

**Rule:** Delimiter ASCII < 0x27 (`'`) → high bit SET, ≥ 0x27 → high bit CLEAR

**Examples:**
```asm
    .AS "HELLO"     ; High bit SET: C8 C5 CC CC CF
    .AS 'HELLO'     ; High bit CLEAR: 48 45 4C 4C 4F
    .AS /HELLO/     ; High bit CLEAR: 48 45 4C 4C 4F
```

### Addressing Mode Quick Reference

| Mode | Syntax | Example |
|------|--------|---------|
| Immediate | `#expr` | `LDA #$20` |
| Zero Page | `expr` | `LDA $20` |
| Absolute | `expr` | `LDA $2000` |
| Indexed | `expr,X` | `LDA $2000,X` |
| Indirect | `(expr)` | `JMP ($2000)` |
| Indexed Indirect | `(expr,X)` | `LDA ($20,X)` |
| Indirect Indexed | `(expr),Y` | `LDA ($20),Y` |
| **Indirect Indexed Alt** | `[expr]` | `LDA [PTR]` (v3.0+) |
| **Indirect Indexed Alt** | `[expr],Y` | `LDA [PTR],Y` (v3.0+) |

---

## Source Line Format

### General Structure

```
[line_number] [label] [opcode] [operand] [;comment]
```

**Components:**
1. **Line Number** (optional): 0-65535, auto-sorted
2. **Label** (optional): Symbol definition
3. **Opcode**: CPU instruction or directive (`.` prefix for directives)
4. **Operand**: Expression or parameters
5. **Comment**: Text after `;` or `*` in column 1

### Examples

```asm
1000 START    LDA #$00        ; Initialize accumulator
1010          STA $C000       ; Store to hardware
1020 *--------------------------------
1030          RTS             ; Return
```

### Version 3.0 Label-Only Lines

Semicolon allows label without opcode:

```asm
1000 ROUTINE ;Entry point here
```

---

## Line Numbers

### Characteristics

- **Range:** 0 to 65535
- **Optional:** Not required
- **Auto-sorting:** Lines sorted by number
- **Display:** 4 digits (< 10000), 5 digits (≥ 10000)

### Examples

```asm
0100          .OR $0800
1000 START    LDA #$00
9999          RTS
10000         NOP
65535 END     BRK
```

### Editing Commands

**RENUMBER:** Renumber all lines
**COPY:** Copy line ranges
**EDIT:** Edit specific line

---

## Labels

### Standard Labels

- First character: Letter (A-Z, a-z)
- Subsequent: Letters, digits, underscore (v3.0+)
- Length: Implementation-dependent (typically 16 chars)
- Cannot be redefined (use `.SE` for variables)

```asm
1000 START    LDA #$00
1010 COUNT    .DA $00
1020 MY_VAR   .EQ 255        ; Underscore OK (v3.0+)
```

### Local Labels (Numeric)

**Format:** `.N` where N = 0-9

**Important:** Forward-reference ONLY. Redefined each occurrence.

```asm
1000 LOOP     LDA TABLE,X
1010          BEQ .1          ; Forward reference
1020          JSR PROCESS
1030 .1       INX             ; Defines .1
1040          CPX #10
1050          BNE LOOP
1060          RTS
1070 .1       NOP             ; Redefines .1 (previous inaccessible)
```

**Use Case:** Avoid label conflicts in repeated code patterns.

---

## Number Formats

### Decimal

Standard decimal numbers:

```asm
    LDA  #100
    .DA  32767
```

### Hexadecimal

Prefix with `$`:

```asm
    LDA  #$FF
    STA  $C000
    .DA  $1234
```

### Binary

Prefix with `%`, optional `.` separators:

```asm
    LDA  #%11001100
    AND  #%0111.1111        ; With separators
    .DA  %10000001
```

### ASCII Character

**Low bit zero** (normal): Use `'`

```asm
    LDA  #'A                ; $41
    CMP  #'Z                ; $5A
```

**High bit set**: Use `"`

```asm
    LDA  #"A                ; $C1
    CMP  #"Z                ; $DA
```

Trailing quote optional: `#'X` or `#'X'` both valid.

---

## String Directives

### String Delimiter Semantics (CRITICAL)

**The delimiter character's ASCII value determines high bit behavior.**

**Rule:** ASCII < 0x27 → high bit SET, ASCII ≥ 0x27 → high bit CLEAR

### .AS - ASCII String

**Syntax:** `.AS delimstringdelim`

High bit controlled by delimiter:

```asm
    .AS "HELLO"             ; High bit SET (0x22 < 0x27)
                            ; Output: C8 C5 CC CC CF

    .AS 'HELLO'             ; High bit CLEAR (0x27)
                            ; Output: 48 45 4C 4C 4F

    .AS /WORLD/             ; High bit CLEAR (0x2F > 0x27)
                            ; Output: 57 4F 52 4C 44
```

**Common Delimiters:**
- `"` - High bit SET (inverse)
- `'` - High bit CLEAR (normal)
- `/` - High bit CLEAR (normal)
- `|` - High bit CLEAR
- `*` - High bit CLEAR

**Mixed Hex/ASCII (v3.0+):**

```asm
    .AS "TEXT"0D            ; "TEXT" + CR
    .AS "LINE1"0D0A"LINE2"  ; Two lines with CRLF
```

### .AT - ASCII String (Terminated)

**Syntax:** `.AT delimstringdelim`

Like `.AS`, but **last character has high bit set**:

```asm
    .AT 'HELLO'             ; 48 45 4C 4C CF (last char high)
```

**Use Case:** Apple II text routines using high bit as terminator.

### .AZ - ASCII String (Null-Terminated)

**Syntax:** `.AZ delimstringdelim` (v3.0+)

Appends null byte:

```asm
    .AZ "HELLO"             ; C8 C5 CC CC CF 00
    .AZ /PATH/              ; 50 41 54 48 00
```

### Delimiter Rules

- Any non-alphanumeric character
- Same delimiter at start and end
- Common: `/`, `|`, `*`, `#`, `@`, `'`, `"`

---

## Data Definition

### .DA - Define Address/Data

Generates 8/16/24/32-bit values:

**Syntax Operators:**

| Operator | Bits | Description |
|----------|------|-------------|
| `#expr` | 8 | Low byte |
| `/expr` | 8 | Second byte (bits 8-15) |
| `expr` | 16 | Low 16 bits (little-endian) |
| `<expr` | 24 | Low 24 bits |
| `>expr` | 32 | Full 32 bits |

**Examples:**

```asm
    .DA #$12345678          ; 78 (low byte)
    .DA /$12345678          ; 56 (2nd byte)
    .DA $12345678           ; 78 56 (16-bit)
    .DA <$12345678          ; 78 56 34 (24-bit)
    .DA >$12345678          ; 78 56 34 12 (32-bit)
```

**String Support (v3.0+):**

```asm
    .DA $$"HELLO"           ; String as addresses
```

### .HS - Hexadecimal String

Raw hex bytes:

```asm
    .HS 01 02 03 04
    .HS A9 00 8D 00 C0      ; LDA #$00 : STA $C000
```

**With commas (v3.0+):**

```asm
    .HS 01,02,03,04
    .HS A9,00,8D,00,C0
```

### .BS - Block Storage

Reserve bytes with optional fill:

```asm
    .BS 256                 ; 256 bytes (default $00)
    .BS 100,$FF             ; 100 bytes filled with $FF
```

**Range:** 0-32767 bytes

### .DFB - Define Byte

Alias for byte definition:

```asm
    .DFB $42                ; Single byte
    .DFB 1,2,3,4            ; Multiple bytes
```

---

## Expressions

### Operators

**Precedence** (highest to lowest):

1. Parentheses `()`
2. Multiplication `*`, Division `/`
3. Addition `+`, Subtraction `-`
4. Boolean AND `&`
5. Boolean OR `|`, XOR `~`

### Arithmetic

```asm
    LDA  #100+50            ; Addition
    STA  VALUE*2            ; Multiplication
    .DA  END-START          ; Subtraction
```

### Boolean/Logical (v2.0+)

```asm
MASK1   .EQ  $F0
MASK2   .EQ  $0F

BOTH    .EQ  MASK1&MASK2    ; AND: $00
EITHER  .EQ  MASK1|MASK2    ; OR:  $FF
TOGGLE  .EQ  MASK1~MASK2    ; XOR: $FF
```

### Byte Extraction

```asm
VALUE   .EQ  $12345678

    .DA  #VALUE             ; Low byte: 78
    .DA  /VALUE             ; 2nd byte: 56
    .DA  ~VALUE             ; 3rd byte: 34
    .DA  VALUE/256          ; Shift right 8 bits
    .DA  VALUE/65536        ; Shift right 16 bits
```

### Special Symbols

- `*` - Current program counter
- `\N` - Macro parameter (N = 0-9)

---

## Addressing Modes

### Standard 6502 Modes

| Mode | Syntax | Example | Bytes |
|------|--------|---------|-------|
| Implied | `opc` | `RTS` | 1 |
| Accumulator | `opc A` | `INC A` | 1 |
| Immediate | `#expr` | `LDA #$00` | 2 |
| Zero Page | `expr` | `LDA $80` | 2 |
| Zero Page,X | `expr,X` | `LDA $80,X` | 2 |
| Zero Page,Y | `expr,Y` | `LDX $80,Y` | 2 |
| Absolute | `expr` | `LDA $C000` | 3 |
| Absolute,X | `expr,X` | `LDA $C000,X` | 3 |
| Absolute,Y | `expr,Y` | `LDA $C000,Y` | 3 |
| Indirect | `(expr)` | `JMP ($FFFC)` | 3 |
| Indexed Indirect | `(expr,X)` | `LDA ($80,X)` | 2 |
| Indirect Indexed | `(expr),Y` | `LDA ($80),Y` | 2 |

### Version 3.0 Alternative Syntax

**Square bracket notation** for indirect indexed:

```asm
    LDA  [PTR]              ; Equivalent to LDA ($PTR),Y with Y=0
    STA  [PTR],Y            ; Same as STA ($PTR),Y
```

**Advantages:**
- Clearer notation
- Compatible with Orca, APW, Merlin-816

### Force Addressing Mode

| Prefix | Mode | Bytes | Example |
|--------|------|-------|---------|
| (none) | Auto | Varies | `LDA VALUE` |
| `<` | Zero Page | 2 | `LDA <VALUE` |
| `>` | Absolute | 3 | `LDA >VALUE` |
| `>>` | Long | 4 | `LDA >>VALUE` (65816) |

**Examples:**

```asm
VALUE   .EQ  $05

    LDA  VALUE              ; Auto: zero page (2 bytes)
    LDA  >VALUE             ; Force: absolute (3 bytes)
    LDA  >>VALUE            ; Force: long (4 bytes, 65816)
```

---

## Directives Reference

### Assembly Control

#### .OR - Set Origin

**Syntax:** `.OR expression`

Set assembly address:

```asm
    .OR  $0800              ; Start at $0800
    .OR  *+$100             ; Advance 256 bytes
```

#### .LIST - Listing Control

**Syntax:** `.LIST [option]`

**Options:**
- `ON` - Enable listing
- `OFF` - Disable listing
- `CON` - Show conditional blocks
- `COFF` - Hide conditional blocks
- `XOFF` - Suppress extra lines (v3.0+)

```asm
    .LIST OFF               ; Disable listing
    .LIST ON                ; Enable listing
    .LIST XOFF              ; Suppress extra (v3.0+)
```

### Symbol Definition

#### .EQ - Equate Symbol

**Syntax:** `label .EQ expression`

Define constant:

```asm
BUFSIZE .EQ  256
MAXVAL  .EQ  $FFFF
OFFSET  .EQ  START+$10
```

**Version 3.0+:** Preserves full 24/32-bit values (no truncation).

#### .SE - Set Equate (Variable)

**Syntax:** `label .SE expression`

Redefinable symbol:

```asm
COUNTER .SE  0
COUNTER .SE  COUNTER+1      ; Increment
```

### Macro Directives

#### .MA - Define Macro

**Syntax:** `.MA [MACRONAME]`

Begin macro definition:

```asm
    .MA MOVE
    LDA \0
    STA \1
    .EM
```

Parameters: `\0` through `\9` (10 parameters max).

#### .EM - End Macro

**Syntax:** `.EM`

End macro definition.

**Invocation:**

```asm
    MOVE SOURCE,DEST        ; Generates: LDA SOURCE; STA DEST
```

**Version 3.0+ Enhancements:**
- Improved bracket/quote handling
- Better parameter delimiter behavior

### Conditional Assembly

#### .DO - Begin Conditional

**Syntax:** `.DO expression`

Assemble if expression ≠ 0:

```asm
DEBUG   .EQ  1
    .DO  DEBUG
    JSR  TRACE
    .FIN
```

#### .ELSE - Conditional Else

**Syntax:** `.ELSE`

Alternate block:

```asm
    .DO  VERSION-2
    JSR  NEWCODE
    .ELSE
    JSR  OLDCODE
    .FIN
```

#### .FIN - End Conditional

**Syntax:** `.FIN`

End conditional block.

**Nesting:** Up to 63 levels.

### Loop Directive

#### .LU - Loop

**Syntax:** `.LU count`

Repeat lines:

```asm
INDEX   .SE  0
    .LU  8
    .DA  INDEX*256
INDEX   .SE  INDEX+1
    .ENDU
```

#### .ENDU - End Loop

**Syntax:** `.ENDU`

End loop block.

### Phase Assembly

#### .PH - Phase

**Syntax:** `.PH address`

Assemble for execution at different address:

```asm
    .PH  $9000              ; Code runs at $9000
PATCH   LDA  #DATA          ; But stored elsewhere
    .EP
```

#### .EP - End Phase

**Syntax:** `.EP`

End phase section.

#### .DUMMY - Dummy Section

**Syntax:** `.DUMMY`

Define symbols without generating code:

```asm
    .DUMMY
    .OR  $0000
ZPTR    .BS  2
COUNT   .BS  1
    .ED
```

#### .ED - End Dummy

**Syntax:** `.ED`

End dummy section.

### File Operations

#### .IN - Include File

**Syntax:** `.IN filename`

Include source file:

```asm
    .IN  MACROS
    .IN  LIBRARY
```

#### .TF - Target File

**Syntax:** `.TF filename[,start,length[,type]]`

Generate binary output:

```asm
    .TF  OUTPUT
    .TF  PROGRAM,2000,1000
    .TF  SYSTEM,6000,2000,TSYS      ; SYS file (v3.0+)
```

**Types (v3.0+):**
- Default: Binary dump
- `TSYS`: Apple II SYS file

---

## Macro System

### Parameter Substitution

Parameters referenced as `\0` through `\9`:

```asm
STORE   .MA
    LDA  \0                 ; First parameter
    STA  \1                 ; Second parameter
    .EM

; Invocation:
    STORE #$42,BUFFER       ; Generates: LDA #$42; STA BUFFER
```

### Macro Label Generation

Use `\0` with label context:

```asm
WAIT    .MA
LOOP\0  DEX
    BNE  LOOP\0
    .EM

; Invocation:
WAIT1   WAIT                ; Generates LOOPWAIT1 label
WAIT2   WAIT                ; Generates LOOPWAIT2 label
```

### Nested Macros

Macros can invoke other macros:

```asm
INNER   .MA
    LDA  \0
    .EM

OUTER   .MA
    INNER \0
    STA  \1
    .EM

    OUTER $10,$20           ; Expands both macros
```

### Delimiter Improvements (v3.0+)

**Square brackets:** Preserve literal text

```asm
MESSAGE .MA
    .AS  [\0]               ; Preserves \0 literally
    .EM

    MESSAGE "HELLO WORLD"   ; Works with spaces
```

---

## Conditional Assembly

### Basic Structure

```asm
    .DO  condition
    ; ... code if condition ≠ 0 ...
    .FIN
```

### With ELSE

```asm
    .DO  condition
    ; ... code if condition ≠ 0 ...
    .ELSE
    ; ... code if condition = 0 ...
    .FIN
```

### Nesting

Up to 63 levels:

```asm
    .DO  PLATFORM-1
    .DO  DEBUG
    JSR  APPLE_DEBUG
    .FIN
    .FIN
```

---

## CPU Modes

### .OP Directive

**Syntax:** `.OP cpu_type`

Select instruction set:

```asm
    .OP  6502               ; Standard 6502 (default)
    .OP  SW16               ; Sweet-16
    .OP  65C02              ; CMOS 65C02
    .OP  65R02              ; Rockwell 65C02
    .OP  65802              ; 65802
    .OP  65816              ; 65816
```

### Detection Algorithm

Scans for key characters:
- No key chars → 6502
- Contains "S" → Sweet-16
- Contains "C" → 65C02
- Contains "R" → 65R02
- Contains "8" → 65802/65816

### Mode Inheritance

Each mode includes predecessors:
- **65C02:** 6502 + enhancements
- **65R02:** 6502 + 65C02 + bit ops
- **65816:** 6502 + 65C02 + 16-bit

### 65C02 Instructions

**New opcodes:**

```asm
BRA  label              ; Branch Always
PHX                     ; Push X
PHY                     ; Push Y
PLX                     ; Pull X
PLY                     ; Pull Y
STZ  addr               ; Store Zero
TRB  addr               ; Test and Reset Bits
TSB  addr               ; Test and Set Bits
```

**Enhanced addressing:**

```asm
BIT  #val               ; Immediate
INC                     ; Accumulator
DEC                     ; Accumulator
JMP  (abs,X)            ; Indexed indirect
LDA  (zp)               ; ZP indirect
```

### 65R02 Rockwell Instructions

**Bit manipulation:**

```asm
SMB  bit,zp             ; Set Memory Bit (0-7)
RMB  bit,zp             ; Reset Memory Bit (0-7)
BBR  bit,zp,label       ; Branch if Bit Reset
BBS  bit,zp,label       ; Branch if Bit Set
```

### 65816 Instructions

**Stack operations:**

```asm
PEA  val16              ; Push effective address
PEI  val8               ; Push indirect
PER  label              ; Push PC-relative
PHB                     ; Push data bank
PHD                     ; Push direct page
PHK                     ; Push program bank
```

**Long addressing:**

```asm
LDA  $123456            ; 24-bit address
JSL  $010000            ; Jump long
JML  $020000            ; Jump long
RTL                     ; Return long
```

**16-bit immediate:**

```asm
    LDA  ##$1234        ; 16-bit immediate
```

**Note:** Double delimiter (`##`) indicates 16-bit.

---

## Comparison with Merlin

### Syntax Similarities

Both SCASM and Merlin share Apple II heritage:

| Feature | SCASM | Merlin |
|---------|-------|--------|
| Hex prefix | `$` | `$` |
| Binary prefix | `%` | `%` |
| Comment | `;` | `*`, `;` |
| Symbol def | `.EQ` | `EQU` |
| Data | `.DA` | `DA` |
| String | `.AS` | `ASC` |

### Major Differences

#### 1. Directive Syntax

| Purpose | SCASM | Merlin |
|---------|-------|--------|
| Origin | `.OR` | `ORG` |
| Hex string | `.HS` | `HEX` |
| ASCII | `.AS` | `ASC` |
| Macro begin | `.MA` | `MAC` |
| Macro end | `.EM` | `EOM` or `\<\<\<` |
| Conditional | `.DO`/`.FIN` | `DO`/`FIN` |

**Key:** SCASM uses `.` prefix for directives.

#### 2. String Handling

**SCASM:** Delimiter ASCII value controls high bit

```asm
.AS "TEXT"      ; High bit SET (0x22 < 0x27)
.AS 'TEXT'      ; High bit CLEAR (0x27)
```

**Merlin:** Directive variant controls high bit

```asm
ASC "TEXT"      ; High bit CLEAR
INV "TEXT"      ; High bit SET
DCI "TEXT"      ; Last char high bit
```

#### 3. Local Labels

**SCASM:** `.N` numeric, forward-reference only

```asm
.1  INX
    BNE .1      ; Cannot reference backward
```

**Merlin:** `:NAME` alphanumeric, bidirectional

```asm
:LOOP INX
    BNE :LOOP   ; Backward reference OK
```

#### 4. Macro Parameters

**SCASM:** `\0` through `\9`

```asm
.MA MOVE
    LDA \0
    STA \1
.EM
```

**Merlin:** `]1` through `]9`

```asm
MOVE MAC
    LDA ]1
    STA ]2
    EOM
```

#### 5. Line Numbers

**SCASM:** Supported (BASIC-style)

```asm
1000 START LDA #$00
1010       STA $00
```

**Merlin:** Not supported

### Migration Guide

**From Merlin to SCASM:**

1. **Add dot prefixes:**
   - `ORG` → `.OR`
   - `HEX` → `.HS`
   - `ASC` → `.AS` or `.AT`

2. **Update macro syntax:**
   - `MAC` → `.MA`
   - `EOM` or `\<\<\<` → `.EM`
   - `]param` → `\param`

3. **Convert string directives:**
   - `ASC "text"` → `.AS 'text'` (high bit clear)
   - `INV "text"` → `.AS "text"` (high bit set)
   - `DCI "text"` → `.AT 'text'` (last char high)

4. **Convert local labels:**
   - `:LABEL` → Use unique names or numeric `.N`

5. **Update conditionals:**
   - `DO` → `.DO`
   - `FIN` → `.FIN`
   - `ELSE` → `.ELSE`

**Example Conversion:**

**Merlin:**
```asm
         ORG  $2000
START    LDA  #$00
         STA  BUFFER
:LOOP    INC  BUFFER
         BNE  :LOOP
         ASC  "HELLO"
```

**SCASM:**
```asm
         .OR  $2000
START    LDA  #$00
         STA  BUFFER
.1       INC  BUFFER
         BNE  .1
         .AS  'HELLO'
```

---

## Implementation Notes

### For xasm++ Implementers

#### Critical Features

1. **Line number handling:** Optional, auto-sort
2. **Dot prefix directives:** All directives start with `.`
3. **Local labels:** `.N` numeric, forward-ref only
4. **String delimiter:** Check ASCII < 0x27 for high bit
5. **32-bit expressions:** Full 32-bit arithmetic
6. **Macro parameters:** `\0` through `\9`
7. **CPU selection:** `.OP` directive parsing

#### String Delimiter Algorithm

```c
bool high_bit_set(char delimiter) {
    return (delimiter < 0x27);  // ASCII apostrophe
}

uint8_t process_char(char c, char delimiter) {
    if (high_bit_set(delimiter))
        return c | 0x80;
    else
        return c & 0x7F;
}
```

#### Version-Specific Features

**v2.0 Baseline:**
- Standard directives
- Basic string handling
- Delimiter high bit rule
- Macro support
- Conditional assembly

**v3.0 Additions:**
- Underscores in symbols
- `[exp]` addressing syntax
- `.AZ` directive
- Mixed hex/ASCII in strings
- Comma-separated `.HS`
- `.DA` with `$$` strings
- Full 24/32-bit `.EQ` values
- `.TF` TSYS support
- Label-only lines with `;`

#### Testing Recommendations

1. Test delimiter semantics thoroughly
2. Verify local label scoping
3. Test 32-bit expression evaluation
4. Verify macro parameter substitution
5. Test `.DA` byte extraction operators
6. Verify CPU mode instruction sets
7. Test mixed hex/ASCII strings (v3.0+)
8. Validate `[exp]` syntax (v3.0+)

---

## Appendix: Complete Directive List

### Alphabetical Reference

| Directive | Category | Purpose | Version |
|-----------|----------|---------|---------|
| `.AS` | Data | ASCII string | 2.0 |
| `.AT` | Data | ASCII terminated | 2.0 |
| `.AZ` | Data | ASCII null-term | 3.0+ |
| `.BS` | Data | Block storage | 2.0 |
| `.DA` | Data | Define address | 2.0 |
| `.DFB` | Data | Define byte | 2.0 |
| `.DO` | Control | Conditional begin | 2.0 |
| `.DUMMY` | Phase | Dummy section | 2.0 |
| `.ED` | Phase | End dummy | 2.0 |
| `.ELSE` | Control | Conditional else | 2.0 |
| `.EM` | Macro | End macro | 2.0 |
| `.ENDU` | Loop | End loop | 2.0 |
| `.EP` | Phase | End phase | 2.0 |
| `.EQ` | Symbol | Define symbol | 2.0 |
| `.FIN` | Control | End conditional | 2.0 |
| `.HS` | Data | Hex string | 2.0 |
| `.IN` | File | Include file | 2.0 |
| `.LIST` | Control | Listing control | 2.0 |
| `.LU` | Loop | Begin loop | 2.0 |
| `.MA` | Macro | Define macro | 2.0 |
| `.OP` | CPU | Select CPU | 2.0 |
| `.OR` | Control | Set origin | 2.0 |
| `.PH` | Phase | Begin phase | 2.0 |
| `.SE` | Symbol | Set variable | 2.0 |
| `.TF` | File | Target file | 2.0 |

---

## References

- **S-C Macro Assembler 2.0 Manual** - Bob Sander-Cederlof, S-C Software Corporation
- **S-C Macro Assembler 3.0 Enhancements** - Technical notes
- **A2osX Project** - Modern SCASM usage examples
- **vasm SCASM Syntax Module** - Reference implementation

---

**Document Version:** 1.0
**Created:** 2026-01-29
**For:** xasm++ SCASM compatibility implementation
**Maintainer:** xasm++ development team
