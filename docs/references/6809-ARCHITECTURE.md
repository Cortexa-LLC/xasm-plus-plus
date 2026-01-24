# Motorola 6809 Architecture Reference

**Document:** 6809 CPU Architecture and Assembler Syntax
**Version:** 1.0
**Date:** 2026-01-24
**Sources:** Lance A. Leventhal, sourcerer 6809 implementation, EDTASM+ documentation

---

## Overview

The Motorola 6809 is an 8-bit microprocessor designed as an advanced successor to the 6800. It features a sophisticated instruction set, multiple addressing modes, and powerful 16-bit operations, making it one of the most capable 8-bit CPUs ever designed.

**Key Features:**
- 8-bit data bus, 16-bit address bus (64KB address space)
- Two 8-bit accumulators (A and B) that can combine into 16-bit accumulator (D)
- Two 16-bit index registers (X and Y)
- Two stack pointers (S for hardware stack, U for user stack)
- Advanced addressing modes (13 modes)
- Position-independent code support
- Fast interrupt handling
- Multiply instruction (8x8=16 bit)

**Systems:**
- TRS-80 Color Computer (CoCo)
- Dragon 32/64
- Vectrex arcade system
- Various industrial control systems

---

## Registers

### 8-bit Accumulators

**A (Accumulator A)** - 8-bit primary accumulator
**B (Accumulator B)** - 8-bit secondary accumulator
**D (Double Accumulator)** - 16-bit concatenation of A:B
- High byte: A
- Low byte: B
- Can be used for 16-bit operations

### 16-bit Index Registers

**X (Index Register X)** - 16-bit index register
**Y (Index Register Y)** - 16-bit index register

**Features:**
- Full 16-bit arithmetic
- Auto-increment/decrement
- Offset indexing
- Position-independent code support

### Stack Pointers

**S (System Stack Pointer)** - 16-bit hardware stack pointer
- Used for interrupts, subroutine calls
- Auto-decrements on push, auto-increments on pull

**U (User Stack Pointer)** - 16-bit user stack pointer
- General-purpose stack
- Same push/pull capabilities as S
- Enables dual-stack architectures

### Other Registers

**PC (Program Counter)** - 16-bit program counter
**DP (Direct Page Register)** - 8-bit direct page base address
- Used with direct addressing mode
- Extends direct addressing to any 256-byte page in memory

**CC (Condition Code Register)** - 8-bit flags
```
Bit 7  6  5  4  3  2  1  0
    E  F  H  I  N  Z  V  C

E = Entire state saved (set by interrupt)
F = FIRQ mask (1 = masked)
H = Half carry (for BCD operations)
I = IRQ mask (1 = masked)
N = Negative
Z = Zero
V = Overflow
C = Carry
```

---

## Addressing Modes

The 6809 has 13 addressing modes, more than the 6502, with sophisticated indexing capabilities.

### 1. Inherent (Implied)

**Description:** No operand required; operation implied by opcode.

**Examples:**
```assembly
NOP                       ; No operation
CLRA                      ; Clear accumulator A
ASLA                      ; Arithmetic shift left A
RTS                       ; Return from subroutine
SEX                       ; Sign extend B into A (D = signed B)
DAA                       ; Decimal adjust A
MUL                       ; Multiply A * B -> D
ABX                       ; Add B to X
```

### 2. Immediate

**Description:** Operand is a constant value (8-bit or 16-bit).

**8-bit Immediate:**
```assembly
LDA   #$42                ; Load A with $42
CMPA  #$FF                ; Compare A with $FF
ANDA  #%11110000          ; AND A with binary 11110000
```

**16-bit Immediate:**
```assembly
LDD   #$1234              ; Load D with $1234
LDX   #$8000              ; Load X with $8000
CMPD  #$0000              ; Compare D with $0000
```

### 3. Direct (Direct Page)

**Description:** 8-bit address offset within direct page (DP * 256).

**Syntax:** `<address` or just `address` (assembler auto-detects)

**Examples:**
```assembly
LDA   <$80                ; Load A from DP:$80
STA   <$40                ; Store A to DP:$40
INC   <$FF                ; Increment DP:$FF
```

**Notes:**
- Faster and smaller than extended addressing
- DP register defaults to $00 (zero page)
- Can be changed with `TFR` or `SETDP` directive

### 4. Extended (Absolute)

**Description:** 16-bit absolute address.

**Syntax:** `address` or `>address` (force extended)

**Examples:**
```assembly
LDA   $1234               ; Load A from $1234
STA   $8000               ; Store A to $8000
JMP   $C000               ; Jump to $C000
```

### 5. Indexed

The 6809's indexed addressing is the most sophisticated of any 8-bit CPU, with many sub-modes.

#### 5a. Zero Offset Indexed

**Syntax:** `,R` where R = X, Y, U, or S

```assembly
LDA   ,X                  ; Load A from address in X
STB   ,Y                  ; Store B to address in Y
LDD   ,U                  ; Load D from address in U
```

#### 5b. Constant Offset Indexed

**Syntax:** `n,R` where n = 5-bit, 8-bit, or 16-bit offset

**5-bit offset:** -16 to +15 (signed)
```assembly
LDA   5,X                 ; Load A from X+5
LDB   -10,Y               ; Load B from Y-10
```

**8-bit offset:** -128 to +127 (signed)
```assembly
LDA   $80,X               ; Load A from X+$80 (force 8-bit)
STB   -50,Y               ; Store B to Y-50
```

**16-bit offset:** -32768 to +32767 (signed)
```assembly
LDA   $1234,X             ; Load A from X+$1234
LDD   -1000,Y             ; Load D from Y-1000
```

#### 5c. Accumulator Offset Indexed

**Syntax:** `A,R`, `B,R`, or `D,R`

```assembly
LDA   A,X                 ; Load A from X+A (signed)
LDB   B,Y                 ; Load B from Y+B (signed)
LDD   D,U                 ; Load D from U+D (signed)
```

**Note:** Offset is treated as signed 8-bit (A or B) or signed 16-bit (D)

#### 5d. Auto-Increment/Decrement Indexed

**Post-increment:** `,R+` or `,R++`
**Pre-decrement:** `,-R` or `,--R`

```assembly
LDA   ,X+                 ; Load A from X, then X++
LDB   ,X++                ; Load B from X, then X += 2
STA   ,-Y                 ; Y--, then store A to Y
STD   ,--U                ; U -= 2, then store D to U
```

**Use cases:**
- Stepping through byte arrays (,X+)
- Stepping through word arrays (,X++)
- Stack operations (,--S to push, ,S+ to pop)

#### 5e. Indirect Indexed

**Syntax:** `[,R]` or `[n,R]`

**Indirect:**
```assembly
LDA   [,X]                ; Load A from address stored at X
JMP   [,Y]                ; Jump to address stored at Y
```

**Indirect with offset:**
```assembly
LDA   [10,X]              ; Load A from address at X+10
JSR   [$100,Y]            ; Call subroutine at address in Y+$100
```

#### 5f. PC Relative Indexed

**Syntax:** `n,PCR` or `label,PCR`

```assembly
LDA   10,PCR              ; Load A from PC+10
JSR   SUBR,PCR            ; Call subroutine (position-independent)
LDX   TABLE,PCR           ; Load X from table (PIC)
```

**Features:**
- Position-independent code
- Relocatable code
- No absolute addresses needed

#### 5g. Extended Indirect

**Syntax:** `[address]`

```assembly
LDA   [$1234]             ; Load A from address stored at $1234
JMP   [$FFFE]             ; Jump to address at $FFFE (IRQ vector)
```

### 6. Relative

**Description:** Signed 8-bit or 16-bit offset from PC (branches only).

**Short branches:** 8-bit offset (-128 to +127 bytes)
```assembly
BEQ   LOOP                ; Branch if equal (8-bit)
BNE   DONE                ; Branch if not equal (8-bit)
BRA   START               ; Branch always (8-bit)
```

**Long branches:** 16-bit offset (-32768 to +32767 bytes)
```assembly
LBEQ  FAR_LOOP            ; Long branch if equal (16-bit)
LBNE  FAR_DONE            ; Long branch if not equal (16-bit)
LBRA  FAR_START           ; Long branch always (16-bit)
```

---

## Instruction Set

The 6809 has approximately 59 fundamental instruction types (not including addressing mode variants).

### Data Movement

**8-bit:**
```assembly
LDA   #$42                ; Load A
LDB   $80                 ; Load B
STA   $1234               ; Store A
STB   ,X+                 ; Store B with post-increment
```

**16-bit:**
```assembly
LDD   #$1234              ; Load D (A:B)
LDX   $8000               ; Load X
LDY   ,U                  ; Load Y
LDS   #$8000              ; Load S (system stack)
LDU   #$0800              ; Load U (user stack)

STD   $1234               ; Store D
STX   ,X                  ; Store X
STY   10,Y                ; Store Y
STS   <$80                ; Store S
STU   ,--U                ; Store U (push onto user stack)
```

**Transfer between registers:**
```assembly
TFR   A,B                 ; Transfer A to B
TFR   X,Y                 ; Transfer X to Y
TFR   S,U                 ; Transfer S to U
EXG   A,B                 ; Exchange A and B
EXG   X,Y                 ; Exchange X and Y
```

**Special moves:**
```assembly
SEX                       ; Sign extend B into A (D = signed B)
```

### Arithmetic (8-bit)

```assembly
ADDA  #$10                ; Add to A
ADCA  $80                 ; Add with carry to A
SUBA  ,X                  ; Subtract from A
SBCA  #$01                ; Subtract with borrow from A
INCA                      ; Increment A
DECA                      ; Decrement A
NEGA                      ; Negate A (two's complement)

ADDB  #$20                ; Add to B
ADCB  ,Y                  ; Add with carry to B
SUBB  $1234               ; Subtract from B
SBCB  #$01                ; Subtract with borrow from B
INCB                      ; Increment B
DECB                      ; Decrement B
NEGB                      ; Negate B
```

### Arithmetic (16-bit)

```assembly
ADDD  #$1234              ; Add to D
SUBD  #$1000              ; Subtract from D

LEAX  10,X                ; Load effective address X+10 into X
LEAY  ,X                  ; Load effective address X into Y
LEAS  -10,S               ; Adjust S by -10 (allocate stack)
LEAU  10,U                ; Adjust U by +10

ABX                       ; Add B to X (X += B, unsigned)
MUL                       ; Multiply A * B -> D (unsigned)
```

### Logical Operations

```assembly
ANDA  #%11110000          ; AND A
ANDB  $80                 ; AND B
ANDCC #$FE                ; AND condition codes (clear carry)

ORA   #%00001111          ; OR A
ORB   ,X                  ; OR B
ORCC  #$50                ; OR condition codes (set I and F masks)

EORA  #$FF                ; XOR A
EORB  $1234               ; XOR B

COMA                      ; Complement A (one's complement)
COMB                      ; Complement B
```

### Shift and Rotate

```assembly
ASLA                      ; Arithmetic shift left A
ASLB                      ; Arithmetic shift left B
ASL   $80                 ; Arithmetic shift left memory

ASRA                      ; Arithmetic shift right A
ASRB                      ; Arithmetic shift right B
ASR   ,X                  ; Arithmetic shift right memory

LSLA                      ; Logical shift left A (same as ASLA)
LSLB                      ; Logical shift left B
LSL   $1234               ; Logical shift left memory

LSRA                      ; Logical shift right A
LSRB                      ; Logical shift right B
LSR   ,Y                  ; Logical shift right memory

ROLA                      ; Rotate left through carry A
ROLB                      ; Rotate left through carry B
ROL   <$40                ; Rotate left through carry memory

RORA                      ; Rotate right through carry A
RORB                      ; Rotate right through carry B
ROR   10,X                ; Rotate right through carry memory
```

### Test and Compare

```assembly
TSTA                      ; Test A (set flags, don't modify)
TSTB                      ; Test B
TST   $80                 ; Test memory

CMPA  #$42                ; Compare A
CMPB  ,X                  ; Compare B
CMPD  #$1234              ; Compare D
CMPX  $8000               ; Compare X
CMPY  ,Y                  ; Compare Y
CMPS  #STACK_END          ; Compare S
CMPU  #USER_STACK         ; Compare U

BITA  #%00000001          ; Bit test A
BITB  $80                 ; Bit test B
```

### Branch Instructions

**Simple branches (8-bit offset):**
```assembly
BRA   LABEL               ; Branch always
BRN   LABEL               ; Branch never (NOP with timing)

BEQ   LABEL               ; Branch if equal (Z=1)
BNE   LABEL               ; Branch if not equal (Z=0)

BCS   LABEL               ; Branch if carry set (C=1)
BCC   LABEL               ; Branch if carry clear (C=0)
BHS   LABEL               ; Branch if higher or same (unsigned, C=0)
BLO   LABEL               ; Branch if lower (unsigned, C=1)

BMI   LABEL               ; Branch if minus (N=1)
BPL   LABEL               ; Branch if plus (N=0)

BVS   LABEL               ; Branch if overflow set (V=1)
BVC   LABEL               ; Branch if overflow clear (V=0)

BHI   LABEL               ; Branch if higher (unsigned, C=0 and Z=0)
BLS   LABEL               ; Branch if lower or same (unsigned, C=1 or Z=1)

BGE   LABEL               ; Branch if greater or equal (signed, N⊕V=0)
BLT   LABEL               ; Branch if less than (signed, N⊕V=1)
BGT   LABEL               ; Branch if greater than (signed, Z=0 and N⊕V=0)
BLE   LABEL               ; Branch if less or equal (signed, Z=1 or N⊕V=1)
```

**Long branches (16-bit offset):**
```assembly
LBRA  FAR_LABEL           ; Long branch always
LBRN  FAR_LABEL           ; Long branch never

LBEQ  FAR_LABEL           ; Long branch if equal
LBNE  FAR_LABEL           ; Long branch if not equal

; ... (all short branches have long equivalents with LB prefix)
```

**Branch on bit (EDTASM+ extension):**
```assembly
BRCLR $80,#%00000001,LABEL ; Branch if bit clear
BRSET $80,#%00000001,LABEL ; Branch if bit set
```

### Jump and Subroutine

```assembly
JMP   $8000               ; Jump absolute
JMP   [,X]                ; Jump indirect
JMP   LABEL,PCR           ; Jump PC-relative (PIC)

JSR   SUBR                ; Jump to subroutine
JSR   [TABLE,Y]           ; Jump to subroutine indirect
JSR   ROUTINE,PCR         ; Jump to subroutine PC-relative (PIC)

BSR   LOCAL_SUBR          ; Branch to subroutine (8-bit relative)
LBSR  FAR_SUBR            ; Long branch to subroutine (16-bit relative)

RTS                       ; Return from subroutine
```

### Stack Operations

**Push/Pull on System Stack (S):**
```assembly
PSHS  A                   ; Push A onto S
PSHS  B                   ; Push B onto S
PSHS  D                   ; Push D (A:B) onto S
PSHS  X                   ; Push X onto S
PSHS  Y                   ; Push Y onto S
PSHS  U                   ; Push U onto S
PSHS  PC                  ; Push PC onto S
PSHS  CC                  ; Push CC onto S
PSHS  A,B,X,Y,U,PC,CC     ; Push multiple registers

PULS  A                   ; Pull A from S
PULS  B                   ; Pull B from S
PULS  X,Y                 ; Pull X and Y from S
PULS  PC                  ; Pull PC from S (return)
PULS  CC                  ; Pull CC from S
PULS  A,B,X,Y,U,PC,CC     ; Pull multiple registers
```

**Push/Pull on User Stack (U):**
```assembly
PSHU  A                   ; Push A onto U
PSHU  B                   ; Push B onto U
PSHU  D                   ; Push D onto U
PSHU  X,Y,S               ; Push X, Y, S onto U

PULU  A                   ; Pull A from U
PULU  B                   ; Pull B from U
PULU  X,Y                 ; Pull X and Y from U
```

**Notes:**
- Multiple registers pushed in specific order: CC, A, B, DP, X, Y, U/S, PC
- Pulled in reverse order
- Common idiom: `PSHS A,B,X,Y` to save context

### Interrupt Handling

```assembly
SWI                       ; Software interrupt (vector at $FFFA)
SWI2                      ; Software interrupt 2 (vector at $FFF4)
SWI3                      ; Software interrupt 3 (vector at $FFF2)
CWAI  #$00                ; Clear CC bits and wait for interrupt
SYNC                      ; Synchronize to interrupt

RTI                       ; Return from interrupt
```

**Interrupt Vectors (6809):**
```
$FFF0-$FFF1: Reserved
$FFF2-$FFF3: SWI3
$FFF4-$FFF5: SWI2
$FFF6-$FFF7: FIRQ (Fast Interrupt Request)
$FFF8-$FFF9: IRQ (Interrupt Request)
$FFFA-$FFFB: SWI
$FFFC-$FFFD: NMI (Non-Maskable Interrupt)
$FFFE-$FFFF: RESET
```

### Miscellaneous

```assembly
NOP                       ; No operation
DAA                       ; Decimal adjust A (BCD arithmetic)
SEX                       ; Sign extend B into A
CWAI  #$00                ; Clear and wait for interrupt
SYNC                      ; Synchronize to interrupt
```

---

## EDTASM+ Syntax

### Overview

EDTASM+ is the standard assembler for the TRS-80 Color Computer (CoCo), designed specifically for 6809 programming.

**Features:**
- Semicolon comments
- No line numbers (unlike SCMASM)
- Directives without dot prefix
- Conditional assembly
- Macro support

### Basic Structure

```assembly
; Comment starts with semicolon

ZERO     EQU   $00               ; Define constant
MAX      EQU   $FF

         ORG   $0E00             ; Set origin (typical CoCo user RAM)

START    LDD   #$0000            ; Instruction with label
         STD   ZERO_PAGE         ; Reference to direct page

LOOP     LDX   #$FF              ; Local loop
         LEAX  -1,X              ; Decrement X
         BNE   LOOP              ; Branch to label

         RTS                     ; Return
```

### Column Layout

```
Column:  0         9    15                           40
         |         |    |                            |
LABEL    OPCODE    OPERAND                           ; Comment
```

### Directives

**Constants:**
```assembly
ZERO     EQU   $00               ; Define constant
STACK    EQU   $8000
```

**Origin:**
```assembly
         ORG   $0E00             ; Set assembly address
```

**Data Definition:**
```assembly
         FCB   $01,$02,$03       ; Form Constant Byte
         FDB   $1234,$5678       ; Form Double Byte (16-bit, big-endian!)
         FCC   "HELLO"           ; Form Constant Characters
         FCS   "END"             ; Form Constant String (last char OR $80)
         FCN   "NULL"            ; Form Constant with Null (add $00)
```

**Note:** 6809 uses **big-endian** byte order (MSB first), unlike 6502 (little-endian)

**Storage Reservation:**
```assembly
BUFFER   RMB   256              ; Reserve Memory Bytes
TABLE    RMB   $100
```

**Direct Page:**
```assembly
         SETDP $00               ; Set direct page to $00 (zero page)
         SETDP $20               ; Set direct page to $20 (DP = $2000-$20FF)
```

**Conditional Assembly:**
```assembly
COCO3    EQU   1                ; Define flag

         IFEQ  COCO3            ; If COCO3 = 0
         LDA   #$01
         ENDC                   ; End conditional

         IFNE  COCO3            ; If COCO3 ≠ 0
         LDA   #$02
         ENDC
```

**Macros:**
```assembly
PRINT    MACRO                  ; Define macro
         LDA   \1               ; Parameter \1
         JSR   PUTCHR
         ENDM                   ; End macro

         PRINT #$48             ; Use macro
```

**Include:**
```assembly
         INCLUDE MACROS.INC     ; Include external file
```

### Comments

```assembly
; This is a full-line comment

         LDA   #$00              ; This is an end-of-line comment

* This is NOT valid in EDTASM+ (use semicolon)
```

---

## TRS-80 Color Computer Specifics

### Memory Map

**Color Computer 1/2:**
```
$0000-$001F: Direct page variables
$0020-$007F: System variables
$0080-$009F: System variables (extended)
$00A0-$00FF: System stack
$0400-$05FF: Text screen (32x16 characters)
$0600-$1FFF: BASIC program area
$2000-$7FFF: User RAM (depends on configuration)
$8000-$FEFF: ROM (Extended Color BASIC)
$FF00-$FF5F: SAM (memory controller)
$FF60-$FFBF: Reserved
$FFC0-$FFDF: Cartridge ROM
$FFE0-$FFFF: Interrupt vectors
```

**Color Computer 3:**
```
$0000-$7FFF: User RAM (128KB configurable with memory management)
$8000-$FEFF: ROM or RAM-banked
$FF00-$FFFF: I/O and vectors
```

### I/O Addresses

**PIA (6821 Peripheral Interface Adapter):**
```
$FF00-$FF03: PIA0 (keyboard, joystick, cassette, sound)
$FF20-$FF23: PIA1 (printer, serial, cartridge)
```

**GIME (Graphics Interrupt Memory Enhancement) - CoCo 3:**
```
$FF90-$FFBF: GIME registers (video, interrupts, MMU)
```

---

## Implementation Requirements for xasm++

### Phase 1: Core 6809 Support

**Minimum implementation:**
- [ ] All fundamental 6809 opcodes (~59 types)
- [ ] All 13 addressing modes
- [ ] Big-endian byte order (MSB first, unlike 6502)
- [ ] Direct page register tracking
- [ ] Multi-pass assembly for forward references

### Phase 2: EDTASM+ Syntax

**Syntax support:**
- [ ] EDTASM+ directive parsing (`ORG`, `EQU`, `FCB`, `FDB`, `FCC`, etc.)
- [ ] Semicolon comments
- [ ] `SETDP` directive for direct page management
- [ ] Conditional assembly (`IFEQ`, `IFNE`, `ENDC`)
- [ ] Macro support (`MACRO`, `ENDM`)

### Phase 3: Advanced Features

**Enhanced capabilities:**
- [ ] Position-independent code (PCR addressing)
- [ ] Dual-stack management (S and U)
- [ ] CoCo-specific equates and symbols
- [ ] Binary output formats (`.BIN`, `.CCC`, `.ROM`)

---

## References

### Source Materials

1. **Lance A. Leventhal** - "6809 Assembly Language Programming", Osborne/McGraw-Hill
   - Located: `docs/references/Pages from 6809 Assembly Language Programming (Lance Leventhal).pdf`

2. **sourcerer** - 6809 CPU implementation
   - `/Users/bryanw/Projects/Vintage/tools/sourcerer/src/cpu/m6809/`
   - EDTASM+ formatter implementation

3. **EDTASM+ Manual** - Radio Shack/Tandy Color Computer documentation

4. **Motorola 6809 Datasheet** - MC6809 Technical Reference

### Online Resources

- [6809.org.uk](http://www.6809.org.uk/) - 6809 community and resources
- [Color Computer Archive](https://colorcomputerarchive.com/) - CoCo documentation
- [CoCo SDC](http://cocosdc.blogspot.com/) - Hardware and software resources
- [6809 Instruction Set](http://www.6809.org.uk/dragon/illegal-opcodes.html)

---

**Document Version:** 1.0
**Last Updated:** 2026-01-24
**Status:** Complete

