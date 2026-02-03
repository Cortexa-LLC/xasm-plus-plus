# Zilog Z80 Architecture Reference

**Document:** Z80 CPU Architecture and Assembler Syntax
**Version:** 1.0
**Date:** 2026-01-24
**Sources:** Zilog Z80 datasheet, vasm Z80 module, sourcerer Z80 planning

---

## Overview

The Zilog Z80 is an 8-bit microprocessor designed as an enhanced, software-compatible successor to the Intel 8080. It features a powerful instruction set, multiple addressing modes, and extensive interrupt handling capabilities, making it one of the most popular 8-bit CPUs in history.

**Key Features:**
- 8-bit data bus, 16-bit address bus (64KB address space)
- Software compatible with Intel 8080
- Two register sets (main and alternate)
- 4KB dynamic RAM refresh circuitry
- Powerful block move/search/compare instructions
- Three interrupt modes
- Sophisticated bit manipulation

**Systems:**
- TRS-80 Model I, III, 4
- Sinclair ZX Spectrum
- Amstrad CPC
- MSX computers
- Sega Master System, Game Gear
- Nintendo Game Boy (modified Z80)
- Numerous arcade games (Pac-Man, Galaga, etc.)

---

## Registers

### 8-bit Registers

**Main Register Set:**
- **A** - Accumulator (primary arithmetic register)
- **F** - Flags register (status flags)
- **B, C** - General purpose (often used as 16-bit BC)
- **D, E** - General purpose (often used as 16-bit DE)
- **H, L** - General purpose (often used as 16-bit HL for addressing)

**Alternate Register Set:**
- **A', F'** - Alternate accumulator and flags
- **B', C'** - Alternate BC
- **D', E'** - Alternate DE
- **H', L'** - Alternate HL

**Note:** Register sets can be swapped with `EX` or `EXX` instructions for fast context switching.

### 16-bit Registers

**AF** - Accumulator and Flags (A:F)
**BC** - General purpose pair
**DE** - General purpose pair
**HL** - General purpose pair (primary memory pointer)

**Special Registers:**
- **IX** - 16-bit index register X
- **IY** - 16-bit index register Y
- **SP** - Stack Pointer
- **PC** - Program Counter

**Other:**
- **I** - Interrupt Vector register
- **R** - Memory Refresh register

### Flags Register (F)

```
Bit 7  6  5  4  3  2  1  0
    S  Z  -  H  -  P/V N  C

S = Sign (bit 7 of result)
Z = Zero
H = Half Carry (bit 3 carry, for BCD)
P/V = Parity/Overflow
N = Add/Subtract (for BCD adjust)
C = Carry
```

**Notes:**
- Bit 5 and 3 are undocumented and vary by instruction
- P/V is Parity for logical ops, Overflow for arithmetic

---

## Addressing Modes

The Z80 has multiple addressing modes, some similar to 6502 but with different syntax.

### 1. Implicit (Implied)

**Description:** No operand; operation implied by opcode.

**Examples:**
```assembly
NOP                       ; No operation
HALT                      ; Halt CPU
EI                        ; Enable interrupts
DI                        ; Disable interrupts
RET                       ; Return from subroutine
RETI                      ; Return from interrupt
RETN                      ; Return from NMI
EXX                       ; Exchange register pairs
DAA                       ; Decimal adjust accumulator
CPL                       ; Complement accumulator
NEG                       ; Negate accumulator
CCF                       ; Complement carry flag
SCF                       ; Set carry flag
```

### 2. Register

**Description:** Operation on registers.

**8-bit:**
```assembly
LD    A,B                 ; Load A from B
LD    C,E                 ; Load C from E
ADD   A,C                 ; Add C to A
SUB   B                   ; Subtract B from A
AND   E                   ; AND A with E
OR    H                   ; OR A with H
XOR   L                   ; XOR A with L
CP    D                   ; Compare A with D
INC   A                   ; Increment A
DEC   B                   ; Decrement B
```

**16-bit:**
```assembly
LD    BC,DE               ; Load BC from DE
ADD   HL,BC               ; Add BC to HL
ADC   HL,DE               ; Add DE to HL with carry
SBC   HL,BC               ; Subtract BC from HL with borrow
INC   HL                  ; Increment HL
DEC   DE                  ; Decrement DE
```

### 3. Immediate

**Description:** Operand is a constant value.

**8-bit:**
```assembly
LD    A,42H               ; Load A with $42
LD    B,0FFH              ; Load B with $FF
ADD   A,10H               ; Add $10 to A
SUB   5                   ; Subtract 5 from A
AND   11110000B           ; AND A with binary
OR    00001111B           ; OR A with binary
XOR   0FFH                ; XOR A with $FF
CP    0                   ; Compare A with 0
```

**16-bit:**
```assembly
LD    BC,1234H            ; Load BC with $1234
LD    DE,8000H            ; Load DE with $8000
LD    HL,0FFFFH           ; Load HL with $FFFF
LD    SP,8000H            ; Load SP with $8000
LD    IX,4000H            ; Load IX with $4000
LD    IY,6000H            ; Load IY with $6000
```

### 4. Direct (Absolute)

**Description:** 16-bit absolute memory address.

**8-bit load/store:**
```assembly
LD    A,(1234H)           ; Load A from memory at $1234
LD    (8000H),A           ; Store A to memory at $8000
```

**16-bit load/store:**
```assembly
LD    BC,(1234H)          ; Load BC from memory at $1234 (little-endian)
LD    (8000H),HL          ; Store HL to memory at $8000
LD    (9000H),IX          ; Store IX to memory at $9000
```

### 5. Indirect Register

**Description:** Memory address in a register pair.

**HL indirect:**
```assembly
LD    A,(HL)              ; Load A from address in HL
LD    (HL),B              ; Store B to address in HL
INC   (HL)                ; Increment memory at HL
DEC   (HL)                ; Decrement memory at HL
AND   (HL)                ; AND A with memory at HL
OR    (HL)                ; OR A with memory at HL
```

**BC/DE indirect (limited):**
```assembly
LD    A,(BC)              ; Load A from address in BC
LD    A,(DE)              ; Load A from address in DE
LD    (BC),A              ; Store A to address in BC
LD    (DE),A              ; Store A to address in DE
```

**SP indirect:**
```assembly
LD    HL,(SP)             ; Load HL from stack (peek)
```

### 6. Indexed (IX/IY + displacement)

**Description:** Index register plus signed 8-bit displacement.

**Syntax:** `(IX+d)` or `(IY+d)` where d = -128 to +127

**Examples:**
```assembly
LD    A,(IX+0)            ; Load A from IX+0
LD    B,(IX+10)           ; Load B from IX+10
LD    (IX+20),C           ; Store C to IX+20
LD    (IY-5),A            ; Store A to IY-5

INC   (IX+0)              ; Increment memory at IX+0
DEC   (IY+10)             ; Decrement memory at IY+10

ADD   A,(IX+5)            ; Add memory at IX+5 to A
SUB   (IY+10)             ; Subtract memory at IY+10 from A
AND   (IX+0)              ; AND A with memory at IX+0
OR    (IY+20)             ; OR A with memory at IY+20
```

**Use cases:**
- Accessing structure fields: `(IX+FIELD_OFFSET)`
- Array access with base pointer
- Local variables on stack: `(IX-offset)`

### 7. Relative (Branches Only)

**Description:** Signed 8-bit offset from PC.

**Syntax:** `JR offset` or `JR cc,offset`

**Examples:**
```assembly
JR    LOOP                ; Jump relative (unconditional)
JR    Z,DONE              ; Jump relative if zero
JR    NZ,LOOP             ; Jump relative if not zero
JR    C,ERROR             ; Jump relative if carry
JR    NC,OK               ; Jump relative if no carry
DJNZ  LOOP                ; Decrement B and jump if not zero
```

**Range:** -126 to +129 bytes from current instruction

### 8. Bit Position (Bit Instructions)

**Description:** Bit number (0-7) and register/memory location.

**Syntax:** `BIT b,r` where b = bit number, r = register or (HL)/(IX+d)/(IY+d)

**Examples:**
```assembly
BIT   0,A                 ; Test bit 0 of A
BIT   7,B                 ; Test bit 7 of B
SET   3,C                 ; Set bit 3 of C
RES   5,D                 ; Reset bit 5 of D

BIT   0,(HL)              ; Test bit 0 of memory at HL
SET   7,(HL)              ; Set bit 7 of memory at HL
RES   4,(IX+10)           ; Reset bit 4 of memory at IX+10
SET   2,(IY-5)            ; Set bit 2 of memory at IY-5
```

---

## Instruction Set

The Z80 has approximately 252 opcodes (including undocumented instructions).

### Data Movement (8-bit)

```assembly
LD    A,B                 ; Load A from B
LD    A,42H               ; Load A with immediate
LD    A,(HL)              ; Load A from memory at HL
LD    A,(1234H)           ; Load A from memory at $1234
LD    A,(BC)              ; Load A from memory at BC
LD    A,(DE)              ; Load A from memory at DE

LD    (HL),A              ; Store A to memory at HL
LD    (1234H),A           ; Store A to memory at $1234
LD    (BC),A              ; Store A to memory at BC
LD    (DE),A              ; Store A to memory at DE

LD    A,I                 ; Load A from I register
LD    A,R                 ; Load A from R register
LD    I,A                 ; Load I from A
LD    R,A                 ; Load R from A
```

### Data Movement (16-bit)

```assembly
LD    BC,1234H            ; Load BC with immediate
LD    DE,8000H            ; Load DE with immediate
LD    HL,0FFFFH           ; Load HL with immediate
LD    SP,8000H            ; Load SP with immediate
LD    IX,4000H            ; Load IX with immediate
LD    IY,6000H            ; Load IY with immediate

LD    BC,(1234H)          ; Load BC from memory
LD    DE,(1234H)          ; Load DE from memory
LD    HL,(1234H)          ; Load HL from memory
LD    SP,(1234H)          ; Load SP from memory
LD    IX,(1234H)          ; Load IX from memory
LD    IY,(1234H)          ; Load IY from memory

LD    (1234H),BC          ; Store BC to memory
LD    (1234H),DE          ; Store DE to memory
LD    (1234H),HL          ; Store HL to memory
LD    (1234H),SP          ; Store SP to memory
LD    (1234H),IX          ; Store IX to memory
LD    (1234H),IY          ; Store IY to memory

LD    SP,HL               ; Load SP from HL
LD    SP,IX               ; Load SP from IX
LD    SP,IY               ; Load SP from IY
```

### Arithmetic (8-bit)

```assembly
ADD   A,B                 ; Add B to A
ADD   A,42H               ; Add immediate to A
ADD   A,(HL)              ; Add memory at HL to A
ADD   A,(IX+10)           ; Add memory at IX+10 to A

ADC   A,C                 ; Add C to A with carry
SUB   B                   ; Subtract B from A
SBC   A,D                 ; Subtract D from A with borrow

INC   A                   ; Increment A
INC   B                   ; Increment B
INC   (HL)                ; Increment memory at HL
INC   (IX+5)              ; Increment memory at IX+5

DEC   A                   ; Decrement A
DEC   C                   ; Decrement C
DEC   (HL)                ; Decrement memory at HL
DEC   (IY-3)              ; Decrement memory at IY-3

NEG                       ; Negate A (two's complement)
```

### Arithmetic (16-bit)

```assembly
ADD   HL,BC               ; Add BC to HL
ADD   HL,DE               ; Add DE to HL
ADD   HL,HL               ; Double HL
ADD   HL,SP               ; Add SP to HL

ADC   HL,BC               ; Add BC to HL with carry
ADC   HL,DE               ; Add DE to HL with carry
ADC   HL,HL               ; Add HL to itself with carry
ADC   HL,SP               ; Add SP to HL with carry

SBC   HL,BC               ; Subtract BC from HL with borrow
SBC   HL,DE               ; Subtract DE from HL with borrow

INC   BC                  ; Increment BC
INC   DE                  ; Increment DE
INC   HL                  ; Increment HL
INC   SP                  ; Increment SP
INC   IX                  ; Increment IX
INC   IY                  ; Increment IY

DEC   BC                  ; Decrement BC
DEC   DE                  ; Decrement DE
DEC   HL                  ; Decrement HL
DEC   SP                  ; Decrement SP
DEC   IX                  ; Decrement IX
DEC   IY                  ; Decrement IY
```

### Logical Operations

```assembly
AND   A                   ; AND A with A (test A, clear carry)
AND   B                   ; AND A with B
AND   42H                 ; AND A with immediate
AND   (HL)                ; AND A with memory at HL
AND   (IX+10)             ; AND A with memory at IX+10

OR    B                   ; OR A with B
OR    0FFH                ; OR A with immediate
OR    (HL)                ; OR A with memory at HL

XOR   B                   ; XOR A with B
XOR   A                   ; Clear A (common idiom)
XOR   (HL)                ; XOR A with memory at HL

CPL                       ; Complement A (one's complement)
NEG                       ; Negate A (two's complement)
```

### Shift and Rotate

**Rotate Accumulator:**
```assembly
RLCA                      ; Rotate A left circular
RRCA                      ; Rotate A right circular
RLA                       ; Rotate A left through carry
RRA                       ; Rotate A right through carry
```

**Rotate/Shift General:**
```assembly
RLC   B                   ; Rotate B left circular
RLC   (HL)                ; Rotate memory at HL left circular
RLC   (IX+5)              ; Rotate memory at IX+5 left circular

RRC   C                   ; Rotate C right circular
RL    D                   ; Rotate D left through carry
RR    E                   ; Rotate E right through carry

SLA   A                   ; Shift A left arithmetic
SRA   B                   ; Shift B right arithmetic
SRL   C                   ; Shift C right logical

RLD                       ; Rotate left digit (A and (HL))
RRD                       ; Rotate right digit (A and (HL))
```

### Bit Manipulation

```assembly
BIT   0,A                 ; Test bit 0 of A
BIT   7,B                 ; Test bit 7 of B
BIT   3,(HL)              ; Test bit 3 of memory at HL
BIT   5,(IX+10)           ; Test bit 5 of memory at IX+10

SET   0,A                 ; Set bit 0 of A
SET   7,C                 ; Set bit 7 of C
SET   4,(HL)              ; Set bit 4 of memory at HL
SET   2,(IY-5)            ; Set bit 2 of memory at IY-5

RES   3,B                 ; Reset bit 3 of B
RES   6,D                 ; Reset bit 6 of D
RES   1,(HL)              ; Reset bit 1 of memory at HL
RES   7,(IX+0)            ; Reset bit 7 of memory at IX+0
```

### Compare

```assembly
CP    A                   ; Compare A with A (test for zero)
CP    B                   ; Compare A with B
CP    42H                 ; Compare A with immediate
CP    (HL)                ; Compare A with memory at HL
CP    (IX+10)             ; Compare A with memory at IX+10

CPI                       ; Compare A with (HL), increment HL, decrement BC
CPIR                      ; Repeat CPI until BC=0 or match found
CPD                       ; Compare A with (HL), decrement HL, decrement BC
CPDR                      ; Repeat CPD until BC=0 or match found
```

### Jump Instructions

**Absolute:**
```assembly
JP    1234H               ; Jump to $1234
JP    Z,DONE              ; Jump if zero
JP    NZ,LOOP             ; Jump if not zero
JP    C,ERROR             ; Jump if carry
JP    NC,OK               ; Jump if no carry
JP    PE,EVEN             ; Jump if parity even
JP    PO,ODD              ; Jump if parity odd
JP    M,NEGATIVE          ; Jump if minus (sign flag set)
JP    P,POSITIVE          ; Jump if plus (sign flag clear)

JP    (HL)                ; Jump to address in HL
JP    (IX)                ; Jump to address in IX
JP    (IY)                ; Jump to address in IY
```

**Relative:**
```assembly
JR    LOOP                ; Jump relative (unconditional)
JR    Z,DONE              ; Jump relative if zero
JR    NZ,LOOP             ; Jump relative if not zero
JR    C,ERROR             ; Jump relative if carry
JR    NC,OK               ; Jump relative if no carry

DJNZ  LOOP                ; Decrement B and jump if not zero
```

### Call and Return

```assembly
CALL  SUBR                ; Call subroutine
CALL  Z,SUBR              ; Call if zero
CALL  NZ,SUBR             ; Call if not zero
CALL  C,ERROR_HANDLER     ; Call if carry
CALL  NC,OK_HANDLER       ; Call if no carry

RET                       ; Return from subroutine
RET   Z                   ; Return if zero
RET   NZ                  ; Return if not zero
RET   C                   ; Return if carry
RET   NC                  ; Return if no carry

RETI                      ; Return from interrupt
RETN                      ; Return from NMI
```

### Stack Operations

```assembly
PUSH  BC                  ; Push BC onto stack
PUSH  DE                  ; Push DE onto stack
PUSH  HL                  ; Push HL onto stack
PUSH  AF                  ; Push AF onto stack
PUSH  IX                  ; Push IX onto stack
PUSH  IY                  ; Push IY onto stack

POP   BC                  ; Pop BC from stack
POP   DE                  ; Pop DE from stack
POP   HL                  ; Pop HL from stack
POP   AF                  ; Pop AF from stack
POP   IX                  ; Pop IX from stack
POP   IY                  ; Pop IY from stack
```

### Exchange

```assembly
EX    DE,HL               ; Exchange DE and HL
EX    AF,AF'              ; Exchange AF and AF'
EX    (SP),HL             ; Exchange (SP) and HL
EX    (SP),IX             ; Exchange (SP) and IX
EX    (SP),IY             ; Exchange (SP) and IY

EXX                       ; Exchange BC/DE/HL with BC'/DE'/HL'
```

### Block Operations

**Load:**
```assembly
LDI                       ; Load (DE) from (HL), increment HL, DE, decrement BC
LDIR                      ; Repeat LDI until BC=0 (block copy)
LDD                       ; Load (DE) from (HL), decrement HL, DE, decrement BC
LDDR                      ; Repeat LDD until BC=0 (block copy backward)
```

**Compare:**
```assembly
CPI                       ; Compare A with (HL), increment HL, decrement BC
CPIR                      ; Repeat CPI until BC=0 or match found
CPD                       ; Compare A with (HL), decrement HL, decrement BC
CPDR                      ; Repeat CPD until BC=0 or match found
```

**Input:**
```assembly
INI                       ; Input from (C) to (HL), increment HL, decrement B
INIR                      ; Repeat INI until B=0
IND                       ; Input from (C) to (HL), decrement HL, decrement B
INDR                      ; Repeat IND until B=0
```

**Output:**
```assembly
OUTI                      ; Output (HL) to (C), increment HL, decrement B
OTIR                      ; Repeat OUTI until B=0
OUTD                      ; Output (HL) to (C), decrement HL, decrement B
OTDR                      ; Repeat OUTD until B=0
```

### I/O Operations

```assembly
IN    A,(42H)             ; Input from port $42 to A
IN    A,(C)               ; Input from port C to A
IN    B,(C)               ; Input from port C to B

OUT   (42H),A             ; Output A to port $42
OUT   (C),A               ; Output A to port C
OUT   (C),B               ; Output B to port C
```

### Interrupt and Control

```assembly
EI                        ; Enable interrupts
DI                        ; Disable interrupts
IM    0                   ; Set interrupt mode 0 (8080 compatible)
IM    1                   ; Set interrupt mode 1 (RST 38H)
IM    2                   ; Set interrupt mode 2 (vector table)

HALT                      ; Halt CPU until interrupt
NOP                       ; No operation

RST   00H                 ; Restart at $0000
RST   08H                 ; Restart at $0008
RST   10H                 ; Restart at $0010
RST   18H                 ; Restart at $0018
RST   20H                 ; Restart at $0020
RST   28H                 ; Restart at $0028
RST   30H                 ; Restart at $0030
RST   38H                 ; Restart at $0038
```

### Miscellaneous

```assembly
DAA                       ; Decimal adjust accumulator (for BCD)
CPL                       ; Complement A
NEG                       ; Negate A (two's complement)
CCF                       ; Complement carry flag
SCF                       ; Set carry flag
```

---

## Assembler Syntax

### Typical Z80 Assembler Syntax

Z80 assemblers vary significantly in syntax. Common variants include:

**Zilog/Microsoft Style:**
```assembly
; Comment with semicolon
        ORG   8000H              ; Origin directive

START:  LD    A,42H              ; Load immediate
        LD    (1234H),A          ; Store to memory
        CALL  SUBR               ; Call subroutine
        RET                      ; Return

SUBR:   LD    HL,TABLE           ; Load address
        LD    BC,10              ; Load count
        LDIR                     ; Block copy
        RET

TABLE:  DB    1,2,3,4,5          ; Define bytes
        DW    1234H,5678H        ; Define words
```

**Common Directives:**
```assembly
        ORG   address            ; Set origin
LABEL   EQU   value              ; Define constant
        DB    byte,byte,...      ; Define bytes
        DW    word,word,...      ; Define words (little-endian)
        DS    count              ; Define space (reserve bytes)
        DEFB  byte,byte,...      ; Define bytes (alternate)
        DEFW  word,word,...      ; Define words (alternate)
        DEFM  "string"           ; Define message (string)
        DEFS  count              ; Define space (alternate)
```

**Numeric Formats:**
```assembly
        LD    A,42               ; Decimal
        LD    A,2AH              ; Hex (suffix H)
        LD    A,0x2A             ; Hex (prefix 0x)
        LD    A,$2A              ; Hex (prefix $)
        LD    A,42                ; Octal (prefix 0)
        LD    A,00101010B        ; Binary (suffix B)
        LD    A,%00101010        ; Binary (prefix %)
```

---

## TRS-80 Model I/III/4 Specifics

### Memory Map (TRS-80 Model I/III)

```
$0000-$2FFF: ROM (12KB BASIC)
$3000-$37FF: Video RAM (1KB, 64x16 characters)
$3800-$38FF: Reserved
$3900-$3BFF: Keyboard memory map
$3C00-$3FFF: I/O ports
$4000-$7FFF: User RAM (16KB standard, expandable)
$8000-$FFFF: Additional RAM (with expansion)
```

### I/O Ports (TRS-80)

```
$FF: Keyboard input
$FE: Printer output
$EC-$EF: Disk controller (floppy)
```

---

## Implementation Requirements for xasm++

### Phase 1: Core Z80 Support

**Minimum implementation:**
- [ ] All standard Z80 opcodes (~252 total)
- [ ] All addressing modes (register, immediate, absolute, indexed, bit, etc.)
- [ ] Little-endian byte order (LSB first, like 6502)
- [ ] IX/IY indexed addressing with displacement
- [ ] Multi-pass assembly

### Phase 2: Z80 Assembler Syntax

**Syntax support:**
- [ ] Standard Z80 directive parsing (`ORG`, `EQU`, `DB`, `DW`, `DS`)
- [ ] Hex notation variants (`$`, `0x`, `H` suffix)
- [ ] Binary notation (`%`, `B` suffix)
- [ ] Label definitions and references

### Phase 3: Advanced Features

**Enhanced capabilities:**
- [ ] Undocumented Z80 instructions (optional)
- [ ] Block operation support (LDIR, CPIR, etc.)
- [ ] TRS-80 specific equates and symbols
- [ ] Binary output formats (`.CMD`, `.BIN`, `.ROM`)

---

## References

### Source Materials

1. **Zilog Z80 CPU User Manual** - Official datasheet
   - Complete instruction set reference
   - Timing diagrams
   - Interrupt modes

2. **sourcerer** - Z80 CPU planning
   - `/Users/bryanw/Projects/Vintage/tools/sourcerer/src/cpu/z80/`
   - (Placeholder for future implementation)

3. **vasm** - Z80 assembler module
   - Reference for syntax and opcodes

### Online Resources

- [Z80.info](http://z80.info/) - Comprehensive Z80 resources
- [Z80 CPU User Manual](https://www.zilog.com/docs/z80/um0080.pdf) - Official Zilog documentation
- [ClrHome Z80 Tables](http://clrhome.org/table/) - Complete opcode reference
- [TRS-80 Model I/III](http://www.trs-80.org/) - TRS-80 community and documentation
- [WorldOfSpectrum](https://worldofspectrum.org/) - ZX Spectrum resources

---

**Document Version:** 1.0
**Last Updated:** 2026-01-24
**Status:** Complete

