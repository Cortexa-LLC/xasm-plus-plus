---
sidebar_position: 2
---

# Quick Start

Get up and running with xasm++ in minutes! This guide shows you how to assemble your first program using the command-line interface.

## Your First Assembly Program

Let's create a simple "Hello World" program for the 6502 using Merlin syntax:

### 1. Create Your Source File

Create a file called `hello.asm`:

```asm
        ORG   $8000

START   LDA   #$48        ; 'H'
        STA   $0400
        LDA   #$45        ; 'E'
        STA   $0401
        LDA   #$4C        ; 'L'
        STA   $0402
        STA   $0403
        LDA   #$4F        ; 'O'
        STA   $0404
        RTS

        END
```

### 2. Assemble It

```bash
xasm++ --cpu 6502 --syntax merlin -o hello.bin hello.asm
```

### 3. Success!

You now have `hello.bin` containing the assembled machine code ready to run on a 6502 system!

## Command-Line Options

### Basic Usage

```bash
xasm++ --cpu <cpu> --syntax <syntax> -o <output> <input>
```

### Supported CPUs

| CPU | Description | Use Case |
|-----|-------------|----------|
| `6502` | MOS 6502 | Apple II, Commodore 64, NES |
| `65c02` | WDC 65C02 | Apple IIc, IIe enhanced |
| `65c02rock` | Rockwell 65C02 | Enhanced 65C02 variant |
| `65816` | WDC 65816 | Apple IIgs, SNES |
| `6809` | Motorola 6809 | TRS-80 Color Computer, Dragon |

### Supported Syntax Modes

| Syntax | CPUs | Description |
|--------|------|-------------|
| `merlin` | 6502/65C02/65816 | Merlin assembler syntax (Apple II) |
| `scmasm` | 6502/65C02/65816 | S-C Macro Assembler syntax |
| `edtasm` | 6809 | EDTASM+ syntax (TRS-80 CoCo) |

### Example Commands

**Apple II development (Merlin):**
```bash
xasm++ --cpu 6502 --syntax merlin -o prodos.sys bootloader.asm
```

**Commodore 64 (S-C Macro Assembler):**
```bash
xasm++ --cpu 6502 --syntax scmasm -o demo.prg demo.asm
```

**Apple IIgs (65816):**
```bash
xasm++ --cpu 65816 --syntax merlin -o toolbox.obj toolbox.asm
```

**TRS-80 Color Computer (6809):**
```bash
xasm++ --cpu 6809 --syntax edtasm -o game.bin game.asm
```

## Syntax Examples

### Merlin Syntax (6502/65816)

```asm
        ORG   $8000

INIT    LDA   #$00
        STA   COUNTER

LOOP    LDX   COUNTER
        LDA   DATA,X
        STA   SCREEN,X
        INC   COUNTER
        BNE   LOOP
        RTS

COUNTER DS    1
DATA    DFB   $48,$45,$4C,$4C,$4F
SCREEN  EQU   $0400

        END
```

### SCMASM Syntax (6502/65C02)

```asm
        .ORG  $8000

INIT:   LDA   #$00
        STA   COUNTER

LOOP:   LDX   COUNTER
        LDA   DATA,X
        STA   SCREEN,X
        INC   COUNTER
        BNE   LOOP
        RTS

COUNTER .BYTE 0
DATA    .BYTE $48,$45,$4C,$4C,$4F
SCREEN  =     $0400
```

### EDTASM Syntax (6809)

```asm
        ORG   $8000

INIT    LDA   #$00
        STA   COUNTER

LOOP    LDX   COUNTER
        LDA   DATA,X
        STA   SCREEN,X
        INC   COUNTER
        BNE   LOOP
        RTS

COUNTER RMB   1
DATA    FCB   $48,$45,$4C,$4C,$4F
SCREEN  EQU   $0400

        END
```

## Common Directives

### Merlin Syntax

| Directive | Description | Example |
|-----------|-------------|---------|
| `ORG` | Set origin address | `ORG $8000` |
| `EQU` | Define constant | `SCREEN EQU $0400` |
| `DFB` | Define byte(s) | `DFB $01,$02,$03` |
| `DW` | Define word | `DW $1234` |
| `ASC` | ASCII string | `ASC "HELLO"` |
| `DS` | Define storage | `DS 256` |
| `END` | End of source | `END` |

### SCMASM Syntax

| Directive | Description | Example |
|-----------|-------------|---------|
| `.ORG` | Set origin address | `.ORG $8000` |
| `=` | Define constant | `SCREEN = $0400` |
| `.BYTE` | Define byte(s) | `.BYTE $01,$02,$03` |
| `.WORD` | Define word | `.WORD $1234` |
| `.ASCII` | ASCII string | `.ASCII "HELLO"` |

### EDTASM Syntax (6809)

| Directive | Description | Example |
|-----------|-------------|---------|
| `ORG` | Set origin address | `ORG $8000` |
| `EQU` | Define constant | `SCREEN EQU $0400` |
| `FCB` | Form constant byte | `FCB $01,$02,$03` |
| `FDB` | Form double byte | `FDB $1234` |
| `FCC` | Form constant characters | `FCC "HELLO"` |
| `RMB` | Reserve memory bytes | `RMB 256` |
| `END` | End of source | `END` |

## Addressing Modes

### 6502/65C02/65816

```asm
        ; Implied
        NOP

        ; Accumulator
        ASL   A

        ; Immediate
        LDA   #$42

        ; Zero Page
        LDA   $80

        ; Zero Page,X
        LDA   $80,X

        ; Absolute
        LDA   $1234

        ; Absolute,X
        LDA   $1234,X

        ; Indirect
        JMP   ($1234)

        ; Indexed Indirect
        LDA   ($80,X)

        ; Indirect Indexed
        LDA   ($80),Y
```

### 6809

```asm
        ; Inherent
        NOP

        ; Immediate
        LDA   #$42

        ; Direct (zero page)
        LDA   $80

        ; Extended (absolute)
        LDA   $1234

        ; Indexed
        LDA   ,X          ; Zero offset
        LDA   5,X         ; 5-bit offset
        LDA   $80,X       ; 8-bit offset
        LDA   $1234,X     ; 16-bit offset
        LDA   A,X         ; Accumulator offset
        LDA   [,X]        ; Indirect
```

## Troubleshooting

### Common Errors

**"Unknown instruction"**
- Check CPU and syntax mode match
- Verify instruction exists for selected CPU
- Check spelling and capitalization

**"Undefined symbol"**
- Ensure labels are defined before use (or multi-pass can resolve)
- Check label spelling matches reference

**"Invalid addressing mode"**
- Not all instructions support all addressing modes
- Check CPU reference manual

### Getting Help

```bash
xasm++ --help
```

## Next Steps

- **[Installation](./installation.md)** - Build from source
- **[Architecture Overview](../architecture/overview.md)** - Understand the design
- **[Developer Documentation](../api/overview.md)** - Integrate xasm++ into your tools

## Real-World Example

Here's a complete Apple II boot sector program:

```asm
        ORG   $800

BOOT    LDA   #$00
        STA   $C050       ; Graphics mode off
        STA   $C052       ; Full screen
        STA   $C054       ; Page 1

        LDX   #$00
CLEAR   LDA   #$A0        ; Space character
        STA   $0400,X
        STA   $0500,X
        STA   $0600,X
        STA   $0700,X
        INX
        BNE   CLEAR

        LDY   #$00
PRINT   LDA   MESSAGE,Y
        BEQ   DONE
        ORA   #$80        ; Set high bit
        STA   $0400,Y
        INY
        BNE   PRINT

DONE    JMP   DONE        ; Infinite loop

MESSAGE ASC   "XASM++ RULES!"
        DFB   $00

        END
```

Assemble with:
```bash
xasm++ --cpu 6502 --syntax merlin -o boot.bin boot.asm
```
