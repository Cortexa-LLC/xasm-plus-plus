---
sidebar_position: 1
---

# Merlin Assembler Syntax Reference

**Target Systems:** Apple II series (II+, IIe, IIc, IIgs)
**CPU Support:** 6502, 65C02, 65802, 65816
**Versions Covered:** Merlin 8, Merlin 16, Merlin Pro
**Purpose:** Complete syntax reference for xasm++ Merlin compatibility

---

## Table of Contents

- [Overview](#overview)
- [Version Differences](#version-differences)
- [Source Line Format](#source-line-format)
- [Number Formats](#number-formats)
- [Expressions](#expressions)
- [Addressing Modes](#addressing-modes)
- [Pseudo-Opcodes Reference](#pseudo-opcodes-reference)
- [String Handling](#string-handling)
- [Label Rules](#label-rules)
- [Macro System](#macro-system)
- [Conditional Assembly](#conditional-assembly)
- [65816 Features](#65816-features)
- [Quick Reference Tables](#quick-reference-tables)

---

## Overview

Merlin is a professional macro assembler for the 6502 family, originally developed by Glen Bredon for Apple II computers. It provides comprehensive support for:

- **6502** - Standard 6502 instruction set
- **65C02** - CMOS enhancements (via XC directive)
- **65802/65816** - 16-bit extensions (Merlin Pro)
- **Macros** - Parameter substitution and expansion
- **Conditional Assembly** - DO/ELSE/FIN blocks
- **Relocatable Code** - REL/ENT/EXT support
- **50+ Pseudo-Opcodes** - Extensive directive set

**Key Philosophy:** Simple, powerful, and compatible with Apple II development conventions.

---

## Version Differences

### Merlin 8 (Original)

**Features:**
- 6502 instruction set only
- 8-character label limit
- Basic pseudo-opcodes (ORG, EQU, DFB, DA, ASC, HEX, etc.)
- Macro support with ]1-]9 parameters
- DOS 3.3 file operations

**Limitations:**
- No 65C02 support
- DDB byte order varies

### Merlin 16 (Enhanced)

**Enhancements over Merlin 8:**
- 65C02 support via XC directive
- 16-character labels
- Additional string directives (STR, REV, DCI)
- Relocatable code (REL, ENT, EXT, USE)
- ProDOS pathname support
- Consistent DDB byte order (big-endian)

### Merlin Pro (Professional)

**Enhancements over Merlin 16:**
- Full 65816 support (16-bit mode)
- MX directive for register size tracking
- Long addressing modes
- Enhanced macro capabilities
- Source-level debugger integration
- ProDOS 16 support

**Recommended:** Use Merlin Pro features as baseline for maximum compatibility.

---

## Source Line Format

### Line Structure

```
[label] [opcode] [operand] [;comment]
```

**Fields:**
1. **Label** (optional): Starts in column 1, alphanumeric + `_`, ends with whitespace or `:`
2. **Opcode** (required): CPU instruction or pseudo-opcode
3. **Operand** (optional): Expression, addressing mode specifier, or directive parameter
4. **Comment** (optional): Text following `;` or `*` in column 1

### Examples

```asm
START    LDA  #$00        ;Initialize accumulator
         STA  $C000       ;Store to hardware
LOOP     INC  COUNT       ;Increment counter
         BNE  LOOP        ;Branch if not zero
*
* Block comment using asterisk in column 1
*
         RTS              ;Return
```

### Blank Lines

Blank lines are allowed and ignored. Use for readability.

### Column Rules

- **Column 1:** Reserved for labels or `*` comment
- **Whitespace:** Tabs or spaces separate fields (typically one tab)
- **Case:** Instructions are case-insensitive; labels are case-sensitive in some contexts

---

## Number Formats

### Decimal (Default)

Numbers without prefix are decimal:

```asm
    LDA  #100        ;Decimal 100
    DFB  255         ;Decimal 255
```

### Hexadecimal ($)

Prefix with dollar sign:

```asm
    LDA  #$FF        ;Hex FF (255 decimal)
    STA  $C000       ;Address $C000
    DFB  $20,$30,$40 ;Multiple hex bytes
```

### Binary (%)

Prefix with percent sign:

```asm
    LDA  #%10101010  ;Binary 170
    AND  #%11110000  ;Binary mask
```

### ASCII Character (')

Single character in single quotes:

```asm
    LDA  #'A         ;ASCII 'A' (65)
    CMP  #'Z         ;ASCII 'Z' (90)
```

**High-bit variants:**
- `'A` - Normal ASCII (0x41)
- `"A` - With high bit set (0xC1) - Note: Some variants

---

## Expressions

### Operators

**Precedence** (highest to lowest):

| Level | Operators | Description |
|-------|-----------|-------------|
| 4 | `< > ^` | Low byte, high byte, bank byte (unary) |
| 3 | `* /` | Multiply, divide |
| 2 | `+ -` | Add, subtract |
| 1 | `& . !` | Bitwise AND, OR, XOR |

**Parentheses** `()` override precedence.

### Byte Extraction Operators

- `<expression` - Low byte (bits 0-7)
- `>expression` - High byte (bits 8-15)
- `^expression` - Bank byte (bits 16-23, 65816 only)

```asm
ADDR     EQU  $1234
LOBYTE   EQU  <ADDR      ;Result: $34
HIBYTE   EQU  >ADDR      ;Result: $12
```

### Bitwise Operators

- `&` - Bitwise AND
- `.` - Bitwise OR (period)
- `!` - Bitwise XOR

```asm
MASK1    EQU  $F0
MASK2    EQU  $0F
BOTH     EQU  MASK1&MASK2    ;Result: $00
EITHER   EQU  MASK1.MASK2    ;Result: $FF
```

### Special Symbols

- `*` - Current program counter value
- `]N` - Macro parameter (N = 1-9)

```asm
HERE     EQU  *           ;Save current address
OFFSET   EQU  TARGET-*    ;Distance to target
         BNE  *+5         ;Branch forward 5 bytes
```

### Evaluation Rules

- **Left-to-right** within same precedence (unlike algebraic)
- Example: `10+5*2` = `20` (not 20 as in algebra)
- Use parentheses: `10+(5*2)` = `20`

---

## Addressing Modes

### Mode Summary Table

| Mode | Syntax | Example | Bytes | Notes |
|------|--------|---------|-------|-------|
| **Implied** | `opc` | `NOP` | 1 | No operand |
| **Accumulator** | `opc A` | `ASL A` | 1 | Optional A |
| **Immediate** | `#expr` | `LDA #$20` | 2 | Literal value |
| **Absolute** | `expr` | `LDA $1000` | 3 | 16-bit address |
| **Absolute,X** | `expr,X` | `LDA $1000,X` | 3 | Indexed by X |
| **Absolute,Y** | `expr,Y` | `LDA $1000,Y` | 3 | Indexed by Y |
| **Zero Page** | `expr` | `LDA $20` | 2 | Auto if < $100 |
| **Zero Page,X** | `expr,X` | `LDA $20,X` | 2 | ZP indexed X |
| **Zero Page,Y** | `expr,Y` | `LDX $20,Y` | 2 | ZP indexed Y |
| **Indirect** | `(expr)` | `JMP ($1000)` | 3 | Pointer jump |
| **(Indirect,X)** | `(expr,X)` | `LDA ($20,X)` | 2 | Pre-indexed |
| **(Indirect),Y** | `(expr),Y` | `LDA ($20),Y` | 2 | Post-indexed |
| **Relative** | `label` | `BNE LOOP` | 2 | Auto-calc offset |

### 65C02 Additional Modes

| Mode | Syntax | Example | Notes |
|------|--------|---------|-------|
| **(Zero Page)** | `(expr)` | `LDA ($20)` | ZP indirect |
| **(Absolute,X)** | `(expr,X)` | `JMP ($1000,X)` | Indexed indirect JMP |

### 65816 Long Addressing

| Mode | Syntax | Example | Notes |
|------|--------|---------|-------|
| **Absolute Long** | `expr` | `LDA $010000` | 24-bit address |
| **Absolute Long,X** | `expr,X` | `LDA $010000,X` | 24-bit + X |
| **[Direct Indirect]** | `[expr]` | `LDA [$20]` | 24-bit pointer |
| **[Direct],Y** | `[expr],Y` | `LDA [$20],Y` | 24-bit ptr + Y |
| **Stack Relative** | `expr,S` | `LDA $02,S` | Stack offset |
| **(SR,S),Y** | `(expr,S),Y` | `LDA ($02,S),Y` | SR indirect + Y |

### Force Addressing Mode

**Prefix operators** override automatic mode selection:

- `<` - Force zero page (2 bytes)
- `>` - Force absolute (3 bytes)
- `>>` - Force long (4 bytes, 65816)

```asm
VALUE    EQU  $05
         LDA  VALUE       ;Auto: zero page (2 bytes)
         LDA  >VALUE      ;Force: absolute (3 bytes)
         LDA  >>VALUE     ;Force: long (4 bytes, 65816)
```

---

## Pseudo-Opcodes Reference

### Assembly Control

#### ORG - Set Origin

**Syntax:** `ORG expression`

Set assembly address:

```asm
         ORG  $8000       ;Assemble at $8000
```

Can be used multiple times to create non-contiguous sections.

#### END - End Assembly

**Syntax:** `END`

Mark end of source file (optional in most contexts).

### Symbol Definition

#### EQU - Equate Symbol

**Syntax:** `label EQU expression`

Define constant symbol:

```asm
MAXVAL   EQU  255         ;Define constant
IOPORT   EQU  $C000       ;I/O address
OFFSET   EQU  END-START   ;Computed value
```

Cannot be redefined. Use `=` for variables.

#### = - Variable Assignment

**Syntax:** `label = expression`

Define or reassign variable:

```asm
COUNTER  =    0           ;Initialize
COUNTER  =    COUNTER+1   ;Increment
```

Can be redefined, unlike EQU.

### Data Generation

#### DFB - Define Byte

**Syntax:** `DFB byte[,byte...]`

Generate byte values:

```asm
         DFB  $20              ;Single byte
         DFB  $20,$30,$40      ;Multiple bytes
         DFB  <ADDR,>ADDR      ;Low/high bytes
```

**Aliases:** DB, FCB (some variants)

#### DA - Define Address (Word)

**Syntax:** `DA address[,address...]`

Generate 16-bit addresses (little-endian):

```asm
         DA   $1000            ;Stores $00,$10
         DA   LABEL1,LABEL2    ;Multiple addresses
         DA   *                ;Current PC
```

**Aliases:** DW (some variants)

#### DDB - Define Double Byte

**Syntax:** `DDB value[,value...]`

Generate 16-bit values:

```asm
         DDB  $1234            ;Big-endian: $12,$34
```

**Note:** Byte order is big-endian in Merlin 16/Pro, may vary in Merlin 8.

#### HEX - Hexadecimal Bytes

**Syntax:** `HEX hex-digits`

Compact hex notation (no $ prefix):

```asm
         HEX  20304050         ;4 bytes: $20,$30,$40,$50
         HEX  FF,00            ;With commas
         HEX  A0A1A2           ;3 bytes
```

**Rules:**
- Even number of hex digits
- No `$` prefix
- Case insensitive

#### ASC - ASCII String

**Syntax:** `ASC "string"` or `ASC 'string'`

Generate ASCII string:

```asm
         ASC  "HELLO WORLD"    ;Plain ASCII
         ASC  'Text'           ;Single quotes OK
```

**Embedded hex:**
```asm
         ASC  "Line 1"8D       ;String + CR byte
```

#### DCI - Define Character Inverted

**Syntax:** `DCI "string"`

Last character has high bit inverted:

```asm
         DCI  "HELLO"          ;HELL-O (O = $CF not $4F)
```

Use case: String termination marker.

#### INV - Inverse String

**Syntax:** `INV "string"`

All characters have high bit set:

```asm
         INV  "FLASH"          ;All bytes $80-$FF range
```

#### REV - Reverse String

**Syntax:** `REV "string"`

String stored in reverse order:

```asm
         REV  "HELLO"          ;Stores "OLLEH"
```

#### STR - Pascal String

**Syntax:** `STR "string"`

Length-prefixed string:

```asm
         STR  "HELLO"          ;Stores: $05,"HELLO"
```

Length byte = string length (max 255).

#### DS - Define Storage

**Syntax:** `DS size[,fill]`

Reserve bytes:

```asm
         DS   100              ;100 bytes (unfilled)
         DS   256,$FF          ;256 bytes of $FF
         DS   \                ;Align to page boundary
```

**Special:** `DS \` advances to next page boundary.

### Listing Control

#### LST - Listing Control

**Syntax:** `LST [ON|OFF|RTN]`

```asm
         LST  OFF              ;Disable listing
         LST  ON               ;Enable listing
         LST  RTN              ;Suppress macro expansion
```

#### PAG - Page Eject

**Syntax:** `PAG`

Force new page in listing.

#### TTL - Title

**Syntax:** `TTL "title"`

Set listing page title:

```asm
         TTL  "Main Program"
```

#### SKP - Skip Lines

**Syntax:** `SKP n`

Insert n blank lines in listing:

```asm
         SKP  3                ;3 blank lines
```

### Conditional Assembly

#### DO - Begin Conditional

**Syntax:** `DO expression`

Assemble if expression ≠ 0:

```asm
DEBUG    EQU  1
         DO   DEBUG            ;Assemble if DEBUG set
         JSR  TRACE
         FIN
```

#### ELSE - Conditional Else

**Syntax:** `ELSE`

Alternate block for DO:

```asm
         DO   VERSION-2
         JSR  NEWCODE          ;If VERSION=2
         ELSE
         JSR  OLDCODE          ;If VERSION≠2
         FIN
```

#### FIN - End Conditional

**Syntax:** `FIN`

End DO/ELSE block.

### Macro Directives

#### MAC - Begin Macro

**Syntax:** `label MAC`

Define macro:

```asm
MOVE     MAC                   ;Define MOVE macro
         LDA  ]1               ;Parameter 1
         STA  ]2               ;Parameter 2
         EOM                   ;End macro
```

**Note:** Some variants use `.MA` instead of `MAC`.

#### EOM - End Macro

**Syntax:** `EOM` or `\<\<\<`

End macro definition.

#### PMC - Print Macro

**Syntax:** `PMC [ON|OFF]`

Control macro expansion in listing:

```asm
         PMC  ON               ;Show expansion
         PMC  OFF              ;Hide expansion
```

### Relocatable Code

#### REL - Relocatable

**Syntax:** `REL`

Begin relocatable code section:

```asm
         REL                   ;Start relocatable
         LDA  BUFFER           ;Uses relocatable addr
```

#### ENT - Entry Point

**Syntax:** `ENT symbol[,symbol...]`

Export symbols for linker:

```asm
         ENT  MAIN,INIT        ;Export these
```

#### EXT - External Reference

**Syntax:** `EXT symbol[,symbol...]`

Import symbols from other modules:

```asm
         EXT  PRINT,GETLN      ;Import these
```

#### USE - Use Object File

**Syntax:** `USE filename`

Link with object file.

### File Operations

#### PUT - Include File

**Syntax:** `PUT filename`

Include source file:

```asm
         PUT  MACROS           ;Include macros
         PUT  LIBRARY          ;Include library
```

#### SAV - Save Object

**Syntax:** `SAV filename`

Specify output file:

```asm
         SAV  MYPROG           ;Save as MYPROG
```

### CPU Selection

#### XC - CPU Mode

**Syntax:** `XC [ON|OFF]`

Enable 65C02 instructions:

```asm
         XC                    ;Enable 65C02
         STZ  $00              ;Now legal
         XC   OFF              ;Back to 6502
```

**Merlin Pro:** `XC 65816` for 65816 mode.

#### MX - Register Size (65816)

**Syntax:** `MX %mb`

Set 65816 register size flags:

```asm
         MX   %00              ;16-bit A, 16-bit X/Y
         MX   %10              ;8-bit A, 16-bit X/Y
         MX   %11              ;8-bit A, 8-bit X/Y
```

**Bits:**
- Bit 1 (m): M flag (0=16-bit A, 1=8-bit A)
- Bit 0 (b): X flag (0=16-bit X/Y, 1=8-bit X/Y)

**Critical:** MX doesn't generate code, only tells assembler immediate operand size.

---

## String Handling

### Delimiter Rules

Strings use matching delimiters:

```asm
         ASC  "Text in double quotes"
         ASC  'Text in single quotes'
         ASC  /Text with slashes/
```

### High Bit Control

**Normal ASCII** (high bit clear):
```asm
         ASC  "HELLO"          ;$48,$45,$4C,$4C,$4F
```

**Inverse/Flash** (high bit set):
```asm
         INV  "HELLO"          ;$C8,$C5,$CC,$CC,$CF
```

**Last character marked:**
```asm
         DCI  "HELLO"          ;$48,$45,$4C,$4C,$CF
```

### Embedded Hex

Insert raw hex in strings:

```asm
         ASC  "Line 1"8D       ;String + CR
         ASC  "Text"00         ;Null-terminated
```

**Format:** Two hex digits (no $ prefix) between string segments.

### Control Characters

Special sequences for control codes:

```asm
         ASC  "Press ^M"       ;^M = RETURN ($0D)
         ASC  "ESC^["          ;^[ = ESC ($1B)
```

**Supported:** `^A` through `^Z`, `^[`, `^]`, `^\`, `^^`, `^_`

---

## Label Rules

### Valid Labels

**First character:** Letter (A-Z, a-z)
**Subsequent:** Letters, digits, underscore
**Length:** 8 chars (Merlin 8), 16 chars (Merlin 16/Pro)
**Terminator:** Whitespace or colon (optional)

```asm
START              ;Basic label
LOOP1              ;With digit
MY_ROUTINE         ;With underscore
Label:             ;With colon
```

### Invalid Labels

```asm
1LABEL             ;Cannot start with digit
MY-LABEL           ;Hyphen not allowed
VERYLONGLABELNAME  ;Too long (>16 chars)
LDA                ;Cannot match mnemonic
ORG                ;Cannot match pseudo-op
```

### Local Labels

**Format:** `:NAME` or `]NAME` (Merlin Pro)

```asm
ROUTINE  LDA  #$00
:LOOP    STA  $C000,X         ;Local label
         INX
         BNE  :LOOP           ;Reference local
         RTS

NEXT     LDA  #$01
:LOOP    STA  $C001,X         ;Different :LOOP
         INX
         BNE  :LOOP
         RTS
```

**Scope:** Reset at next global label.

### Anonymous Labels

**Format:** `^` (forward) or `-` (backward)

```asm
         LDA  #$00
^        STA  $C000,X         ;Define forward label
         INX
         BNE  ^               ;Branch to it
```

**Note:** Not all Merlin versions support anonymous labels.

---

## Macro System

### Macro Definition

```asm
MACRONAME MAC                 ;Begin definition
         ; ... macro body ...
         ; Use ]1-]9 for parameters
         EOM                   ;End definition
```

### Parameter Substitution

Parameters referenced as `]1` through `]9`:

```asm
STORE    MAC                   ;Define STORE macro
         LDA  ]1               ;First parameter
         STA  ]2               ;Second parameter
         EOM

; Invocation:
         STORE #$42,BUFFER     ;Generates: LDA #$42; STA BUFFER
```

### Macro Label Parameter

`]0` contains the macro invocation label (if any):

```asm
WAIT     MAC
]0.LOOP  DEX                   ;Use ]0 as label prefix
         BNE  ]0.LOOP
         EOM

; Invocation:
WAIT1    WAIT                  ;Generates WAIT1.LOOP label
WAIT2    WAIT                  ;Generates WAIT2.LOOP label
```

### Nested Macros

Macros can invoke other macros:

```asm
INNER    MAC
         LDA  ]1
         EOM

OUTER    MAC
         INNER ]1              ;Call INNER
         STA  ]2
         EOM

         OUTER $10,$20         ;Expands both
```

### Listing Control

```asm
         PMC  OFF              ;Hide macro expansion
         MYMACRO ARG1,ARG2     ;Only show invocation
         PMC  ON               ;Resume expansion display
```

---

## Conditional Assembly

### Basic Structure

```asm
         DO   condition
         ; ... code if condition ≠ 0 ...
         FIN
```

### With ELSE

```asm
         DO   condition
         ; ... code if condition ≠ 0 ...
         ELSE
         ; ... code if condition = 0 ...
         FIN
```

### Nesting

Conditionals can be nested:

```asm
         DO   PLATFORM-1
         DO   DEBUG
         JSR  APPLE_DEBUG
         FIN
         FIN
```

### Common Patterns

**Feature flags:**
```asm
DEBUG    EQU  1
         DO   DEBUG
         JSR  TRACE
         FIN
```

**Version selection:**
```asm
VERSION  EQU  2
         DO   VERSION-2
         JSR  V2CODE
         ELSE
         JSR  V1CODE
         FIN
```

**Platform targeting:**
```asm
APPLE2E  EQU  1
         DO   APPLE2E
         LDA  $C000,X          ;//e specific
         ELSE
         LDA  $C000            ;Standard
         FIN
```

---

## 65816 Features

### Enabling 65816 Mode

```asm
         XC   65816            ;Enable 65816 (Merlin Pro)
```

### Register Size Tracking

**MX Directive:** Tell assembler current register sizes

```asm
         REP  #$30             ;16-bit A, X, Y (in code)
         MX   %00              ;Tell assembler
         LDA  #$1234           ;Now 16-bit immediate
         LDX  #$5678           ;16-bit immediate

         SEP  #$20             ;8-bit A (in code)
         MX   %10              ;Tell assembler
         LDA  #$12             ;Now 8-bit immediate
```

**Critical:** MX directive doesn't generate code - programmer must manually track REP/SEP.

### Long Addressing

**24-bit addresses:**
```asm
         LDA  $123456          ;Load from bank $12, addr $3456
         LDA  $010000,X        ;Bank 1 with X offset
```

**Long indirect:**
```asm
         LDA  [$20]            ;24-bit pointer at DP $20
         LDA  [$20],Y          ;Plus Y offset
```

**Stack relative:**
```asm
         LDA  $02,S            ;Load from stack offset
         LDA  ($02,S),Y        ;Stack relative indirect
```

### Additional 65816 Instructions

**Stack operations:**
```asm
         PEA  $1234            ;Push 16-bit literal
         PER  LABEL            ;Push effective address
         PHB                   ;Push data bank
         PHD                   ;Push direct page
         PHK                   ;Push program bank
```

**Long jumps:**
```asm
         JML  $020000          ;Jump long
         JSL  $010000          ;Call long subroutine
         RTL                   ;Return long
```

**Register transfers:**
```asm
         TCD                   ;C to D
         TCS                   ;C to S
         TDC                   ;D to C
         TSC                   ;S to C
         TXY                   ;X to Y
         TYX                   ;Y to X
         XBA                   ;Exchange B and A
         XCE                   ;Exchange C and E
```

**Block moves:**
```asm
         MVN  $12,$34          ;Move next (increment)
         MVP  $12,$34          ;Move previous (decrement)
```

**Processor control:**
```asm
         REP  #$30             ;Reset status bits
         SEP  #$20             ;Set status bits
         COP                   ;Coprocessor
         WDM                   ;Reserved
```

---

## Quick Reference Tables

### Pseudo-Opcode Summary

| Directive | Purpose | Example |
|-----------|---------|---------|
| **Assembly Control** |
| ORG | Set origin | `ORG $8000` |
| END | End assembly | `END` |
| **Symbol Definition** |
| EQU | Define constant | `MAXVAL EQU 255` |
| = | Define variable | `COUNT = 0` |
| **Data Generation** |
| DFB/DB | Define byte(s) | `DFB $20,$30` |
| DA/DW | Define address | `DA ROUTINE` |
| DDB | Define word (BE) | `DDB $1234` |
| HEX | Hex bytes | `HEX 010203` |
| ASC | ASCII string | `ASC "TEXT"` |
| DCI | String, last inv | `DCI "TEXT"` |
| INV | Inverse string | `INV "TEXT"` |
| REV | Reverse string | `REV "TEXT"` |
| STR | Pascal string | `STR "TEXT"` |
| DS | Reserve space | `DS 256` |
| **Listing** |
| LST | Listing control | `LST OFF` |
| PAG | Page eject | `PAG` |
| TTL | Set title | `TTL "Title"` |
| SKP | Skip lines | `SKP 3` |
| **Conditional** |
| DO | Begin conditional | `DO DEBUG` |
| ELSE | Alternate block | `ELSE` |
| FIN | End conditional | `FIN` |
| **Macros** |
| MAC | Begin macro | `MOVE MAC` |
| EOM/\<\<\< | End macro | `EOM` |
| PMC | Print macro | `PMC OFF` |
| **Relocatable** |
| REL | Relocatable mode | `REL` |
| ENT | Entry point | `ENT MAIN` |
| EXT | External ref | `EXT PRINT` |
| USE | Use object | `USE LIBRARY` |
| **Files** |
| PUT | Include file | `PUT MACROS` |
| SAV | Save object | `SAV OUTPUT` |
| **CPU** |
| XC | CPU mode | `XC 65816` |
| MX | Register size | `MX %00` |

### Number Format Quick Reference

| Format | Syntax | Example | Value |
|--------|--------|---------|-------|
| Decimal | `nnn` | `255` | 255 |
| Hexadecimal | `$hh` | `$FF` | 255 |
| Binary | `%bbbb` | `%11111111` | 255 |
| ASCII | `'c` | `'A` | 65 |

### Operator Precedence

| Level | Operators | Example |
|-------|-----------|---------|
| 4 (high) | `< > ^` | `<$1234` = `$34` |
| 3 | `* /` | `10*5` = `50` |
| 2 | `+ -` | `10+5` = `15` |
| 1 (low) | `& . !` | `$F0&$0F` = `$00` |

### Addressing Mode Byte Counts

| Mode | Bytes | Example |
|------|-------|---------|
| Implied | 1 | `NOP` |
| Accumulator | 1 | `ASL A` |
| Immediate | 2 | `LDA #$20` |
| Zero Page | 2 | `LDA $20` |
| Zero Page,X/Y | 2 | `LDA $20,X` |
| Absolute | 3 | `LDA $1000` |
| Absolute,X/Y | 3 | `LDA $1000,X` |
| (Indirect,X) | 2 | `LDA ($20,X)` |
| (Indirect),Y | 2 | `LDA ($20),Y` |
| Indirect | 3 | `JMP ($1000)` |
| Relative | 2 | `BNE LOOP` |
| **65816 Long** |
| Absolute Long | 4 | `LDA $123456` |
| Long,X | 4 | `LDA $123456,X` |
| [Direct] | 2 | `LDA [$20]` |
| [Direct],Y | 2 | `LDA [$20],Y` |

---

## Implementation Notes for xasm++

### Critical Parsing Rules

1. **Expression Evaluation:** Left-to-right within same precedence level
2. **Addressing Mode Selection:** Auto-detect ZP vs absolute based on value
3. **String Delimiters:** Support multiple delimiter types
4. **Macro Expansion:** Parameter substitution with ]0-]9
5. **Conditional Nesting:** Track DO/ELSE/FIN depth

### Two-Pass Assembly

**Pass 1:** Collect symbols, resolve forward references
**Pass 2:** Generate code, check all references resolved

### Version-Specific Features

- **Merlin 8:** 8-char labels, DDB byte order caution
- **Merlin 16:** 16-char labels, relocatable code, consistent DDB
- **Merlin Pro:** 65816 support, MX directive, long addressing

### Testing Recommendations

1. Test with original Merlin source files (Prince of Persia, etc.)
2. Verify all addressing modes with real code
3. Test macro expansion with nested invocations
4. Verify MX directive interaction with immediate values
5. Test string handling (ASC, DCI, INV, REV, STR)
6. Verify expression evaluation order
7. Test conditional assembly with nesting

---

## References

- **Merlin 8/16 Manual** - Glen Bredon, Roger Wagner Publishing
- **Merlin Pro Manual** - Professional edition documentation
- **65816 Programming Manual** - WDC 65816 reference
- **Apple II Reference Manual** - Apple Computer, Inc.

---

**Document Version:** 1.0
**Created:** 2026-01-29
**For:** xasm++ Merlin compatibility implementation
**Maintainer:** xasm++ development team
