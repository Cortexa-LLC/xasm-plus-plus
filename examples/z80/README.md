# Z80 Assembly Examples for xasm++

This directory contains example Z80 assembly programs demonstrating the EDTASM-M80++ parser features.

## Overview

All examples use the EDTASM-M80++ syntax, which is compatible with:
- EDTASM (TRS-80 Editor Assembler)
- M80 (Microsoft Z80 Macro Assembler)
- ZMAC (Unix/Linux Z80 assembler)
- Z80ASM (various Z80 assembler variants)

## Example Programs

### 1. hello_cpm.asm
**CP/M Hello World**

Simple CP/M program that prints "Hello, World!" to the console.

**Assemble:**
```bash
xasm++ --cpu z80 --syntax edtasm_m80_plusplus hello_cpm.asm -o hello.com
```

**Run:**
```
A>hello
Hello, World!
```

**Features Demonstrated:**
- ORG directive for CP/M TPA ($0100)
- System equates (BDOS function calls)
- String data with CR/LF
- CP/M program structure

---

### 2. interrupt_handler.asm
**Z80 Interrupt Handler**

Demonstrates interrupt mode 1 (IM 1) with a timer-based interrupt service routine.

**Assemble:**
```bash
xasm++ --cpu z80 --syntax edtasm_m80_plusplus interrupt_handler.asm -o int.bin
```

**Features Demonstrated:**
- Interrupt vector at $0038 (IM 1)
- Interrupt service routine (ISR)
- Register preservation in ISR
- 16-bit counter increment
- RETI instruction
- Stack management

**Systems:**
- TRS-80 Model I/III/4
- ZX Spectrum
- MSX computers
- Any Z80 system supporting IM 1

---

### 3. data_tables.asm
**Data Tables and Lookup**

Comprehensive example showing various data table techniques.

**Assemble:**
```bash
xasm++ --cpu z80 --syntax edtasm_m80_plusplus data_tables.asm -o tables.bin
```

**Features Demonstrated:**
- Jump tables for command dispatch
- Lookup tables (sine table example)
- String pointer tables
- Mixed data structures (records)
- Array indexing techniques
- Bounds checking
- Indirect jumps via tables

**Techniques:**
1. **Jump Table Dispatch** - Fast command routing
2. **Sine Lookup** - Mathematical function tables
3. **String Tables** - Efficient string storage
4. **Structured Data** - Multi-field records

---

## Building All Examples

```bash
# From xasm++ root directory
cd examples/z80

# Hello World (CP/M)
../../build/bin/xasm++ --cpu z80 --syntax edtasm_m80_plusplus hello_cpm.asm -o hello.com

# Interrupt Handler
../../build/bin/xasm++ --cpu z80 --syntax edtasm_m80_plusplus interrupt_handler.asm -o int.bin

# Data Tables
../../build/bin/xasm++ --cpu z80 --syntax edtasm_m80_plusplus data_tables.asm -o tables.bin
```

---

## Syntax Features Used

### Number Formats
```assembly
LD    A,$FF               ; Hex with $ prefix
LD    A,0FFH              ; Hex with H suffix (must start with digit)
LD    A,%11111111         ; Binary with % prefix
LD    A,11111111B         ; Binary with B suffix
LD    A,255               ; Decimal
```

### Data Directives
```assembly
DB    'Text',13,10,0      ; Define bytes
DW    $1234,$5678         ; Define words (little-endian)
DS    256                 ; Define space (reserve memory)
```

### Labels
```assembly
GLOBAL_LABEL:             ; Global label
.local_label:             ; Local label (Z80ASM style)
1:                        ; Numeric local (ZMAC style)
```

### Expressions
```assembly
VALUE   EQU   (END-START)/2    ; Expression in EQU
        LD    A,(BUFFER+5)     ; Expression in operand
```

---

## CP/M Notes

### Running CP/M Programs

The `hello_cpm.asm` example generates a `.COM` file suitable for CP/M systems.

**Memory Layout:**
- $0000-$00FF: Zero page and system area
- $0100-$FFFF: Transient Program Area (TPA)

**BDOS Functions:**
- 2 - Console output (character)
- 9 - Write string ($ terminated)

**Return to CP/M:**
- `RET` instruction returns to CP/M CCP
- Exit code in A register (0 = success)

---

## TRS-80 Notes

### Memory Map (Model I/III)
```
$0000-$2FFF: ROM (12KB BASIC)
$3000-$37FF: Video RAM (1KB)
$4000-$7FFF: User RAM (16KB)
$8000-$FFFF: Expansion RAM
```

### Interrupt Modes
- **IM 0:** 8080-compatible mode
- **IM 1:** Fixed vector at $0038 (RST 38H)
- **IM 2:** Vectored interrupts via I register table

---

## Common Patterns

### Loop with Counter
```assembly
        LD    B,10               ; Counter
LOOP:   ; ... do something ...
        DJNZ  LOOP               ; Decrement B and loop if not zero
```

### 16-bit Increment
```assembly
        LD    HL,(COUNTER)       ; Load 16-bit counter
        INC   HL                 ; Increment
        LD    (COUNTER),HL       ; Store back
```

### Jump Table
```assembly
JUMP_TABLE:
        DW    HANDLER0
        DW    HANDLER1
        DW    HANDLER2

DISPATCH:
        ADD   A,A                ; A = A * 2 (word size)
        LD    HL,JUMP_TABLE
        ADD   A,L
        LD    L,A
        JR    NC,1f
        INC   H
1:      LD    E,(HL)
        INC   HL
        LD    D,(HL)
        EX    DE,HL
        JP    (HL)
```

---

## Further Reading

- [EDTASM-M80++ Parser Reference](../../docs/reference/z80/edtasm-m80-plusplus-parser.md)
- [Z80 Architecture Reference](../../docs/reference/cpu/z80-architecture.md)
- [xasm++ User Guide](../../docs/user-guide.md)

---

## License

These examples are part of the xasm++ project and are provided as educational material.
Feel free to use, modify, and learn from them.

---

**Last Updated:** 2026-02-10
**xasm++ Version:** 1.0
