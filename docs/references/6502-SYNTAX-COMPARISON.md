# 6502 Assembler Syntax Comparison

**Document:** 6502 Syntax Variants for Apple II and Retro Systems
**Version:** 1.0
**Date:** 2026-01-24
**Sources:** sourcerer formatters, vasm-ext documentation

---

## Overview

This document compares the syntax differences between major 6502 assemblers used in vintage computing, particularly for Apple II development. Understanding these differences is critical for xasm++ to support multiple syntax modes.

**Target Syntaxes:**
1. **Merlin** - Apple IIgs assembler (most common for Apple II development)
2. **SCMASM** - S-C Macro Assembler (Apple II, line-numbered)
3. **EDTASM** - EDTASM+ (TRS-80 Color Computer, 6809 focus but influenced 6502 tools)

---

## Quick Reference Table

| Feature | Merlin | SCMASM | xasm++ (Current) | Notes |
|---------|--------|--------|------------------|-------|
| **Line Numbers** | No | Yes (1000, 1010...) | No | SCMASM uses BASIC-style line numbering |
| **Comment Char** | `;` | `*` | N/A | SCMASM uses `*` at start of line or after code |
| **Label Position** | Column 1 | Column 1 or after line number | N/A | Labels flush left or indented |
| **Directive Prefix** | None | `.` (dot) | N/A | SCMASM uses `.EQ`, `.OR`, `.DA`, etc. |
| **Equate Directive** | `EQU` | `.EQ` | N/A | Define constants |
| **Origin Directive** | `ORG` | `.OR` | N/A | Set assembly address |
| **Data Byte** | `DB`, `DFB` | `.DA` | N/A | Define byte data |
| **Data Word** | `DW`, `DA` | `.DA` (16-bit) | N/A | Define word data |
| **ASCII String** | `ASC` | `.AS` | N/A | Define ASCII string |
| **Conditional** | `DO`/`ELSE`/`FIN` | `.DO`/`.ELSE`/`.FIN` | N/A | Conditional assembly |
| **Macro Definition** | `MAC`/`<<<` | `.MA`/`.EM` | N/A | Macro support |
| **Include File** | `PUT` | `.IN` | N/A | Include external file |
| **Hex Prefix** | `$` | `$` | `$` | Hex notation |
| **Binary Prefix** | `%` | `%` | N/A | Binary notation |
| **Decimal** | No prefix | No prefix | No prefix | Decimal numbers |
| **CPU Mode** | `MX` directive | N/A | N/A | 65816 register width control |

---

## Addressing Mode Syntax (Universal)

All three syntaxes use **identical addressing mode syntax** for 6502 instructions:

```assembly
; Implied
INX

; Accumulator
ASL A

; Immediate
LDA #$42

; Zero Page
LDA $80

; Zero Page,X
LDA $80,X

; Zero Page,Y
LDX $80,Y

; Absolute
JMP $1234

; Absolute,X
LDA $1234,X

; Absolute,Y
LDA $1234,Y

; Indirect (JMP only)
JMP ($1234)

; Indexed Indirect
LDA ($80,X)

; Indirect Indexed
LDA ($80),Y

; Relative (branches)
BNE loop
```

**Status:** ✅ xasm++ supports all of these (Phase 2.4 complete)

---

## Merlin Syntax

### Overview

Merlin is the most popular assembler for Apple II development, particularly for Apple IIgs programming with 65816 support.

**Versions:**
- **Merlin 8** - Original 8-bit 6502/65C02 assembler
- **Merlin 16** - Enhanced for 65816 (Apple IIgs)
- **Merlin 16+** - Modern version with extended features
- **Merlin 32** - Modern cross-assembler for macOS/Linux/Windows

### Basic Structure

```assembly
; Comment starts with semicolon

LABEL    EQU   $1234              ; Define constant
         ORG   $8000              ; Set origin

START    LDA   #$00               ; Instruction with label
         STA   ZERO_PAGE          ; Reference to zero page

LOOP     LDX   #$FF               ; Local loop
         DEX                      ; No operand
         BNE   LOOP               ; Branch to label

         RTS                      ; Return
```

### Column Layout

```
Column:  0         9    14                            40
         |         |    |                             |
LABEL    OPCODE    OPERAND                            ; Comment
```

**Rules:**
- Labels start at column 1 (no leading whitespace)
- Opcodes at column 9
- Operands at column 14
- Comments at column 40+ (after `;`)

### Directives

**Constants:**
```assembly
ZERO     EQU   $00                ; Define constant (no dot prefix)
MAX      EQU   $FF
```

**Origin:**
```assembly
         ORG   $8000              ; Set assembly address
```

**Data Definition:**
```assembly
         DFB   $01,$02,$03        ; Define bytes (Data Form Byte)
         DB    $01,$02,$03        ; Define bytes (alternate)
         DA    $1234,$5678        ; Define words (16-bit, little-endian)
         DW    $1234,$5678        ; Define words (alternate)
         ASC   "HELLO"            ; ASCII string (no terminator)
         DCI   "TEXT"             ; ASCII with inverted last char
         FLS   "FILENAME"         ; Flashing text
         REV   "REVERSE"          ; Reverse video text
         HEX   0102030405         ; Hex bytes (no spaces)
```

**Conditional Assembly:**
```assembly
         DO    APPLE_IIE          ; If APPLE_IIE is non-zero
         LDA   #$00
         ELSE                     ; Otherwise
         LDA   #$FF
         FIN                      ; End conditional
```

**Macros:**
```assembly
PRINT    MAC                     ; Define macro
         LDA   #]1               ; Parameter ]1
         JSR   COUT
         <<<                     ; End macro

         PRINT $48               ; Use macro
```

**Include:**
```assembly
         PUT   MACROS.S           ; Include external file
```

### 65816 Extensions (Merlin 16+)

**Register Width Control:**
```assembly
         MX    %00                ; Both A and X/Y are 16-bit
         MX    %01                ; A is 16-bit, X/Y are 8-bit
         MX    %10                ; A is 8-bit, X/Y are 16-bit
         MX    %11                ; Both A and X/Y are 8-bit
```

**Long Addressing:**
```assembly
         LDA   >LABEL             ; Force 16-bit absolute
         LDA   <LABEL             ; Force 8-bit zero page
         LDA   LABEL:BANK         ; Bank byte notation
```

**65816 Opcodes:**
```assembly
         PHB                      ; Push Data Bank
         PLB                      ; Pull Data Bank
         PHK                      ; Push Program Bank
         MVN   $00,$01            ; Block move negative
         MVP   $00,$01            ; Block move positive
```

### Comments

```assembly
; This is a full-line comment

         LDA   #$00               ; This is an end-of-line comment

*        This is NOT valid in Merlin (use semicolon)
```

### Label Rules

**Valid:**
```assembly
LABEL    NOP                      ; Standard label
_LABEL   NOP                      ; Leading underscore OK
LABEL123 NOP                      ; Digits OK (not first char)
```

**Invalid:**
```assembly
123LABEL NOP                      ; Cannot start with digit
LABEL-1  NOP                      ; Hyphen not allowed
```

**Local Labels (Merlin 16+):**
```assembly
MAIN     LDA   #$00
:LOOP    DEX                      ; Local label (colon prefix)
         BNE   :LOOP              ; Reference local label
```

---

## SCMASM Syntax

### Overview

S-C Macro Assembler (SCMASM) is a traditional Apple II assembler with BASIC-style line numbering. It was popular in the 1980s and influenced many Apple II assemblers.

**Versions:**
- **SCMASM 3.0** - Original version
- **SCMASM 3.1** - Enhanced version (most common reference)

### Basic Structure

```assembly
1000 * Comment starts with asterisk
1010 *
1020 ZERO     .EQ   $00                      Zero constant
1030 MAX      .EQ   $FF                      Max value
1040 *
1050          .OR   $8000                    Set origin
1060 *
1070 START    LDA   #$00                     Load zero
1080          STA   ZERO                     Store
1090 *
1100 LOOP     LDX   #$FF                     Local loop
1110          DEX                            Decrement
1120          BNE   LOOP                     Branch
1130 *
1140          RTS                            Return
```

### Line Numbering

**Format:**
- Start: 1000
- Increment: 10
- Range: 1000-99990

**Rules:**
- Line numbers are required
- Must be at start of line
- Followed by space or `*` for comment
- Can be renumbered automatically

### Column Layout

```
Column:  0    5         14   19                            45
         |    |         |    |                             |
1000          LABEL     OPCODE OPERAND                     Comment
```

**Rules:**
- Line number at column 0
- Labels after line number (or column 14)
- Opcodes at column 19
- Operands after opcode
- Comments anywhere after code (no special character needed)

### Directives

**Constants (dot prefix):**
```assembly
1000 ZERO     .EQ   $00                      Define constant
1010 MAX      .EQ   $FF
```

**Origin (dot prefix):**
```assembly
1000          .OR   $8000                    Set assembly address
```

**Data Definition (dot prefix):**
```assembly
1000          .DA   $01,$02,$03              Define bytes
1010          .DA   #$1234,#$5678            Define words (use # prefix)
1020          .AS   "HELLO"                  ASCII string
1030          .HS   0102030405               Hex bytes
```

**Conditional Assembly (dot prefix):**
```assembly
1000          .DO   APPLE_IIE                If APPLE_IIE is non-zero
1010          LDA   #$00
1020          .ELSE                          Otherwise
1030          LDA   #$FF
1040          .FIN                           End conditional
```

**Macros (dot prefix):**
```assembly
1000          .MA   PRINT                    Define macro
1010          LDA   ]1                       Parameter ]1
1020          JSR   COUT
1030          .EM                            End macro
1040 *
1050          >PRINT $48                     Use macro (> prefix)
```

**Include (dot prefix):**
```assembly
1000          .IN   MACROS                   Include external file
```

### Comments

```assembly
1000 * This is a full-line comment
1010 *
1020          LDA   #$00                     This is an end-of-line comment
1030 *        Note: No semicolon, just asterisk
```

### Label Rules

**Valid:**
```assembly
1000 LABEL    NOP                            Standard label
1010 _LABEL   NOP                            Leading underscore OK
1020 LABEL123 NOP                            Digits OK (not first char)
```

**Invalid:**
```assembly
1000 123LABEL NOP                            Cannot start with digit
```

---

## 65C02 Support Requirements

### New Opcodes (65C02 vs 6502)

The 65C02 adds new opcodes and addressing modes:

**New Instructions:**
```assembly
BRA   label              ; Branch Always (relative)
PHX                      ; Push X
PLX                      ; Pull X
PHY                      ; Push Y
PLY                      ; Pull Y
STZ   $80                ; Store Zero (ZeroPage, ZeroPageX, Absolute, AbsoluteX)
TRB   $80                ; Test and Reset Bits
TSB   $80                ; Test and Set Bits
```

**Enhanced Addressing Modes:**
```assembly
LDA   ($80)              ; Indirect (no ,Y) - NEW for 65C02
JMP   ($1234,X)          ; Indexed Indirect JMP - NEW for 65C02
BIT   #$80               ; Immediate BIT - NEW for 65C02
BIT   $80,X              ; ZeroPageX BIT - NEW for 65C02
BIT   $1234,X            ; AbsoluteX BIT - NEW for 65C02
```

**Rockwell/WDC Extensions:**
```assembly
BBR0  $80,label          ; Branch on Bit Reset
BBS0  $80,label          ; Branch on Bit Set
RMB0  $80                ; Reset Memory Bit
SMB0  $80                ; Set Memory Bit
```

**Detection Strategy:**
- Use CPU mode flag: `--cpu 6502`, `--cpu 65c02`, `--cpu 65c02-rockwell`
- Enable/disable opcodes based on mode
- Warn on 65C02-specific opcodes when in 6502 mode

---

## 65816 Support Requirements

### Overview

The 65816 (WDC 65C816) is a 16-bit extension of the 6502, used in the Apple IIgs and Super Nintendo.

**Key Features:**
- 16-bit accumulator and index registers (switchable to 8-bit)
- 24-bit addressing (16MB address space)
- New opcodes and addressing modes
- Backward compatible with 6502/65C02

### Register Width Control

**MX Directive (Merlin):**
```assembly
         MX    %00                ; A and X/Y are 16-bit
         MX    %01                ; A is 16-bit, X/Y are 8-bit
         MX    %10                ; A is 8-bit, X/Y are 16-bit
         MX    %11                ; A is 8-bit, X/Y are 8-bit (6502 mode)
```

**Effect on Code Generation:**
```assembly
         MX    %00                ; 16-bit mode
         LDA   #$1234             ; 3-byte immediate (opcode + 2 bytes)

         MX    %11                ; 8-bit mode
         LDA   #$12               ; 2-byte immediate (opcode + 1 byte)
```

**Status Register Bits:**
- `m` bit (bit 5): Accumulator register size (0=16-bit, 1=8-bit)
- `x` bit (bit 4): Index register size (0=16-bit, 1=8-bit)

### New Addressing Modes

**Long Addressing (24-bit):**
```assembly
         LDA   $123456            ; Absolute Long
         LDA   [$80]              ; Indirect Long
         LDA   [$80],Y            ; Indirect Long Indexed
```

**Stack Relative:**
```assembly
         LDA   $03,S              ; Stack Relative
         LDA   ($03,S),Y          ; Stack Relative Indirect Indexed
```

**Block Move:**
```assembly
         MVN   srcbank,dstbank    ; Block Move Negative
         MVP   srcbank,dstbank    ; Block Move Positive
```

### New Instructions

**Data Bank Operations:**
```assembly
PHB                       ; Push Data Bank
PLB                       ; Pull Data Bank
PHK                       ; Push Program Bank
```

**Direct Page Operations:**
```assembly
PHD                       ; Push Direct Page
PLD                       ; Pull Direct Page
TCD                       ; Transfer C to Direct Page
TDC                       ; Transfer Direct Page to C
```

**16-bit Operations:**
```assembly
PEA   #$1234              ; Push Effective Address
PEI   ($80)               ; Push Effective Indirect Address
PER   label               ; Push Effective PC Relative Address
```

**Long Jumps:**
```assembly
JML   $123456             ; Jump Long
JSL   $123456             ; Jump to Subroutine Long
RTL                       ; Return from Subroutine Long
```

**Other New Opcodes:**
```assembly
COP   #$00                ; Coprocessor
WDM   #$00                ; Reserved (WDC)
XBA                       ; Exchange B and A accumulators
XCE                       ; Exchange Carry and Emulation bits
```

### Syntax Variations

**Force Addressing Mode:**
```assembly
         LDA   >LABEL             ; Force 16-bit absolute
         LDA   <LABEL             ; Force 8-bit direct page (zero page)
         LDA   LABEL:BANK         ; Specify bank byte
```

### Implementation Requirements

**For xasm++:**
1. CPU mode flags: `6502`, `65c02`, `65c02-rockwell`, `65816`
2. Track `m` and `x` flags during assembly (for immediate operand size)
3. Support `MX` directive to set register widths
4. Support long addressing modes (24-bit)
5. Support new 65816 opcodes
6. Warn/error on mode-specific opcodes when in wrong mode

---

## Key Differences Summary

### Structural Differences

| Aspect | Merlin | SCMASM |
|--------|--------|--------|
| Line numbers | No | Yes (1000, 1010...) |
| Comment char | `;` | `*` |
| Directive prefix | None | `.` (dot) |
| Equate | `EQU` | `.EQ` |
| Origin | `ORG` | `.OR` |
| Data byte | `DFB`, `DB` | `.DA` |
| Conditional | `DO`/`FIN` | `.DO`/`.FIN` |
| Macro | `MAC`/`<<<` | `.MA`/`.EM` |
| Include | `PUT` | `.IN` |

### Similarities

Both syntaxes share:
- ✅ Identical 6502 addressing mode syntax
- ✅ `$` for hex, `%` for binary
- ✅ Labels at column 1
- ✅ Similar column-based formatting
- ✅ Macro support
- ✅ Conditional assembly

### xasm++ Implementation Priority

**Phase 1: Core 6502 (DONE)**
- ✅ All addressing modes
- ✅ All 56 legal opcodes
- ✅ Multi-pass assembly
- ✅ Label resolution

**Phase 2: Syntax Support (Future)**
- [ ] Merlin syntax mode
- [ ] SCMASM syntax mode
- [ ] Directive parsing (`.EQ`, `EQU`, `.OR`, `ORG`, etc.)
- [ ] Data directives (`.DA`, `DFB`, etc.)
- [ ] Conditional assembly (`.DO`/`.FIN`)

**Phase 3: Advanced Features**
- [ ] Macro support
- [ ] Include files
- [ ] 65C02 mode and opcodes
- [ ] 65816 mode and opcodes
- [ ] MX directive for register width tracking

---

## References

### Source Materials

1. **sourcerer** - Multi-CPU disassembler with Merlin/SCMASM formatters
   - `/Users/bryanw/Projects/Vintage/tools/sourcerer/src/output/merlin_formatter.cpp`
   - `/Users/bryanw/Projects/Vintage/tools/sourcerer/src/output/scmasm_formatter.cpp`

2. **vasm-ext** - Extended vasm assembler (syntax modules)
   - SCMASM syntax module documentation
   - Merlin syntax module documentation

3. **Merlin Documentation**
   - Merlin 8/16 manual
   - Merlin 32 documentation

4. **SCMASM Documentation**
   - S-C Macro Assembler 3.1 manual

5. **65816 Programming Manual**
   - Western Design Center 65816 datasheet
   - Apple IIgs Toolbox Reference

### Related Documents

- `MERLIN-REFERENCE.md` - Detailed Merlin syntax guide
- `SCMASM-REFERENCE.md` - Detailed SCMASM syntax guide
- `6502-OPCODES.md` - Complete 6502 opcode reference
- `65C02-EXTENSIONS.md` - 65C02 additions and differences
- `65816-REFERENCE.md` - 65816 architecture and opcodes

---

**Document Version:** 1.0
**Last Updated:** 2026-01-24
**Status:** Complete

