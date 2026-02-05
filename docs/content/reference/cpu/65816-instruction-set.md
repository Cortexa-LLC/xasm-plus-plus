---
sidebar_position: 4
---

# WDC 65816 Instruction Set Reference

**Processor:** Western Design Center 65C816
**Architecture:** 16-bit (with 8-bit compatibility mode)
**Addressing:** 24-bit address bus (16MB)
**Released:** 1984

## Overview

The 65816 is a 16-bit extension of the 65C02 with full backward compatibility. It features 24-bit addressing, 16-bit registers, and additional instructions while maintaining the ability to run 6502/65C02 code.

### Key Features

1. **16-bit Operation**: Accumulator and index registers can operate in 16-bit mode
2. **24-bit Addressing**: Full 16MB address space via Data Bank Register
3. **Backward Compatible**: Runs all 6502/65C02 code in emulation mode
4. **New Instructions**: Block moves, stack operations, and more
5. **Additional Registers**: Program Bank, Data Bank, Direct Page
6. **Two Modes**: Native mode (16-bit) and Emulation mode (6502 compatible)

## Registers

### 8-bit Mode (Compatible with 6502/65C02)

- **A** - Accumulator (8-bit) or **C** (16-bit with B as high byte)
- **B** - Accumulator High Byte (8-bit, separate in native mode)
- **X** - X Index Register (8/16-bit)
- **Y** - Y Index Register (8/16-bit)
- **S** - Stack Pointer (16-bit in native mode, 8-bit in emulation)
- **PC** - Program Counter (16-bit)
- **P** - Processor Status Flags (8-bit)

### Additional 65816 Registers

- **PBR** - Program Bank Register (8-bit, high byte of PC)
- **DBR** - Data Bank Register (8-bit, default bank for data access)
- **D** - Direct Page Register (16-bit, replaces fixed $00 page)

### Combined Registers

- **C** - 16-bit Accumulator (A + B)
- **X** - 8 or 16-bit depending on X flag
- **Y** - 8 or 16-bit depending on X flag
- **S** - Stack Pointer (16-bit in native mode)

## Status Flags (P Register)

| Bit | Flag | Name | Description |
|-----|------|------|-------------|
| 7 | N | Negative | Set if bit 7 (or 15 in 16-bit) of result is set |
| 6 | V | Overflow | Set on signed arithmetic overflow |
| 5 | M | Memory/Accumulator | 0=16-bit A, 1=8-bit A (native mode) |
| 4 | X/B | Index/Break | 0=16-bit X/Y, 1=8-bit X/Y (native) / Break (emulation) |
| 3 | D | Decimal | Decimal mode flag (BCD arithmetic) |
| 2 | I | Interrupt Disable | Set to disable IRQ interrupts |
| 1 | Z | Zero | Set if result is zero |
| 0 | C | Carry | Carry/borrow flag |

## Operating Modes

### Emulation Mode (E=1)

- **Compatibility**: Behaves like 65C02
- **Registers**: 8-bit A, 8-bit X/Y, 8-bit stack pointer (page 1)
- **Addressing**: 16-bit addresses only
- **Reset State**: Processor starts in emulation mode
- **Purpose**: Run unmodified 6502/65C02 code

### Native Mode (E=0)

- **Enhanced**: Full 65816 capabilities
- **Registers**: Configurable 8 or 16-bit A, X, Y
- **Stack**: 16-bit stack pointer (anywhere in bank 0)
- **Addressing**: 24-bit addresses available
- **Purpose**: New 65816 code

### Mode Switching

```asm
; Enter native mode from emulation
        CLC
        XCE              ; Exchange carry with emulation flag

; Enter emulation mode from native
        SEC
        XCE              ; Exchange carry with emulation flag
```

## New 65816 Instructions

### Block Move Instructions

#### MVN - Move Block Negative

**Operation:** Move (X)→(Y), decrement addresses, repeat

**Description:** Moves a block of memory from source to destination, processing from high to low addresses.

**Syntax:**
```asm
MVN srcbank,destbank  ; Move A+1 bytes
```

**Registers:**
- **A**: Number of bytes to move - 1
- **X**: Source address (low 16 bits)
- **Y**: Destination address (low 16 bits)
- **Operand byte 1**: Destination bank
- **Operand byte 2**: Source bank

**Flags Affected:** None

**Bytes:** 3 | **Cycles:** 7 per byte

**Example:**
```asm
        LDA #$0FFF       ; Move 4096 bytes
        LDX #$1000       ; Source: $011000
        LDY #$2000       ; Dest: $022000
        MVN $02,$01      ; Move from bank $01 to $02
```

#### MVP - Move Block Positive

**Operation:** Move (X)→(Y), increment addresses, repeat

**Description:** Moves a block of memory from source to destination, processing from low to high addresses.

**Syntax:**
```asm
MVP srcbank,destbank  ; Move A+1 bytes
```

**Registers:** Same as MVN

**Flags Affected:** None

**Bytes:** 3 | **Cycles:** 7 per byte

**Example:**
```asm
        LDA #$03FF       ; Move 1024 bytes
        LDX #$3000       ; Source: $013000
        LDY #$4000       ; Dest: $024000
        MVP $02,$01      ; Move from bank $01 to $02
```

### Stack Operations

#### PEA - Push Effective Absolute Address

**Operation:** Push 16-bit value

**Description:** Pushes a 16-bit immediate value onto the stack.

**Syntax:**
```asm
PEA $1234        ; Push $1234 onto stack
```

**Flags Affected:** None

**Bytes:** 3 | **Cycles:** 5

#### PEI - Push Effective Indirect Address

**Operation:** Push (D+offset)

**Description:** Pushes the 16-bit value stored at direct page offset onto the stack.

**Syntax:**
```asm
PEI ($12)        ; Push value at (D+$12)
```

**Flags Affected:** None

**Bytes:** 2 | **Cycles:** 6

#### PER - Push Effective PC Relative Address

**Operation:** Push PC+offset

**Description:** Calculates PC+offset and pushes the result onto the stack.

**Syntax:**
```asm
PER label        ; Push address of label
```

**Flags Affected:** None

**Bytes:** 3 | **Cycles:** 6

#### PHB - Push Data Bank Register

**Operation:** DBR↓

**Description:** Pushes the 8-bit Data Bank Register onto the stack.

**Syntax:**
```asm
PHB              ; Save data bank
```

**Flags Affected:** None

**Bytes:** 1 | **Cycles:** 3

#### PHD - Push Direct Page Register

**Operation:** D↓

**Description:** Pushes the 16-bit Direct Page Register onto the stack.

**Syntax:**
```asm
PHD              ; Save direct page
```

**Flags Affected:** None

**Bytes:** 1 | **Cycles:** 4

#### PHK - Push Program Bank Register

**Operation:** PBR↓

**Description:** Pushes the 8-bit Program Bank Register onto the stack.

**Syntax:**
```asm
PHK              ; Save program bank
```

**Flags Affected:** None

**Bytes:** 1 | **Cycles:** 3

#### PLB - Pull Data Bank Register

**Operation:** DBR↑

**Description:** Pulls a byte from the stack into the Data Bank Register.

**Syntax:**
```asm
PLB              ; Restore data bank
```

**Flags Affected:** N Z

**Bytes:** 1 | **Cycles:** 4

#### PLD - Pull Direct Page Register

**Operation:** D↑

**Description:** Pulls 16 bits from the stack into the Direct Page Register.

**Syntax:**
```asm
PLD              ; Restore direct page
```

**Flags Affected:** N Z

**Bytes:** 1 | **Cycles:** 5

### Long Addressing Instructions

Long addressing mode uses 24-bit addresses (bank + 16-bit address).

#### JML - Jump Long

**Operation:** PC = address[15:0], PBR = address[23:16]

**Description:** Jumps to a 24-bit address (can cross banks).

**Syntax:**
```asm
JML $012000      ; Jump to bank $01, address $2000
JML [$2000]      ; Jump to address stored at $2000 (24-bit)
```

**Flags Affected:** None

**Bytes:** 4 (absolute) or 3 (indirect) | **Cycles:** 4 or 6

#### JSL - Jump to Subroutine Long

**Operation:** PBR↓, PC+3↓, PC = address[15:0], PBR = address[23:16]

**Description:** Calls a subroutine in any bank. Pushes return address (24-bit) onto stack.

**Syntax:**
```asm
JSL $012000      ; Call routine in bank $01
```

**Flags Affected:** None

**Bytes:** 4 | **Cycles:** 8

#### RTL - Return from Subroutine Long

**Operation:** PC↑, PBR↑, PC = PC + 1

**Description:** Returns from a long subroutine call (pulls 24-bit address).

**Syntax:**
```asm
RTL              ; Return from JSL
```

**Flags Affected:** None

**Bytes:** 1 | **Cycles:** 6

### Register Operations

#### REP - Reset Processor Status Bits

**Operation:** P = P & !operand

**Description:** Clears specified flags in the processor status register.

**Syntax:**
```asm
REP #$30         ; Clear M and X flags (16-bit mode)
REP #$20         ; Clear M flag (16-bit accumulator)
REP #$10         ; Clear X flag (16-bit index registers)
```

**Flags Affected:** Any flags in operand mask

**Bytes:** 2 | **Cycles:** 3

**Common Uses:**
```asm
REP #$30         ; 16-bit A, X, Y
REP #$20         ; 16-bit A only
REP #$10         ; 16-bit X, Y only
```

#### SEP - Set Processor Status Bits

**Operation:** P = P | operand

**Description:** Sets specified flags in the processor status register.

**Syntax:**
```asm
SEP #$30         ; Set M and X flags (8-bit mode)
SEP #$20         ; Set M flag (8-bit accumulator)
SEP #$10         ; Set X flag (8-bit index registers)
```

**Flags Affected:** Any flags in operand mask

**Bytes:** 2 | **Cycles:** 3

**Common Uses:**
```asm
SEP #$30         ; 8-bit A, X, Y
SEP #$20         ; 8-bit A only
SEP #$10         ; 8-bit X, Y only
```

#### TCD - Transfer C to Direct Page

**Operation:** D = C

**Description:** Transfers 16-bit accumulator C to Direct Page Register.

**Syntax:**
```asm
TCD              ; D = C
```

**Flags Affected:** N Z

**Bytes:** 1 | **Cycles:** 2

#### TCS - Transfer C to Stack Pointer

**Operation:** S = C

**Description:** Transfers 16-bit accumulator C to Stack Pointer.

**Syntax:**
```asm
TCS              ; S = C
```

**Flags Affected:** None (native mode)

**Bytes:** 1 | **Cycles:** 2

#### TDC - Transfer Direct Page to C

**Operation:** C = D

**Description:** Transfers Direct Page Register to 16-bit accumulator C.

**Syntax:**
```asm
TDC              ; C = D
```

**Flags Affected:** N Z

**Bytes:** 1 | **Cycles:** 2

#### TSC - Transfer Stack Pointer to C

**Operation:** C = S

**Description:** Transfers Stack Pointer to 16-bit accumulator C.

**Syntax:**
```asm
TSC              ; C = S
```

**Flags Affected:** N Z

**Bytes:** 1 | **Cycles:** 2

#### TXY - Transfer X to Y

**Operation:** Y = X

**Description:** Transfers X register to Y register (respects 8/16-bit mode).

**Syntax:**
```asm
TXY              ; Y = X
```

**Flags Affected:** N Z

**Bytes:** 1 | **Cycles:** 2

#### TYX - Transfer Y to X

**Operation:** X = Y

**Description:** Transfers Y register to X register (respects 8/16-bit mode).

**Syntax:**
```asm
TYX              ; X = Y
```

**Flags Affected:** N Z

**Bytes:** 1 | **Cycles:** 2

#### XBA - Exchange B and A

**Operation:** Swap A ↔ B

**Description:** Exchanges the high and low bytes of the 16-bit accumulator.

**Syntax:**
```asm
XBA              ; Swap high/low bytes
```

**Flags Affected:** N Z (based on new A value)

**Bytes:** 1 | **Cycles:** 3

**Example:**
```asm
LDA #$1234       ; C = $1234 (A=$34, B=$12)
XBA              ; C = $3412 (A=$12, B=$34)
```

#### XCE - Exchange Carry with Emulation

**Operation:** Swap C ↔ E

**Description:** Exchanges the Carry flag with the Emulation flag, switching between native and emulation modes.

**Syntax:**
```asm
CLC
XCE              ; Enter native mode (E=0)

SEC
XCE              ; Enter emulation mode (E=1)
```

**Flags Affected:** C E

**Bytes:** 1 | **Cycles:** 2

### Miscellaneous

#### BRL - Branch Long

**Operation:** PC = PC + offset (16-bit)

**Description:** Relative branch with 16-bit displacement (-32768 to +32767).

**Syntax:**
```asm
BRL faraway      ; Can branch anywhere in current bank
```

**Flags Affected:** None

**Bytes:** 3 | **Cycles:** 4

#### COP - Co-Processor

**Operation:** Trigger COP vector

**Description:** Software interrupt for co-processor communication.

**Syntax:**
```asm
COP #$12         ; Co-processor instruction
```

**Flags Affected:** D=0, I=1 (native), B=1 (emulation)

**Bytes:** 2 | **Cycles:** 7-8

#### STP - Stop Processor

**Operation:** Halt until reset

**Description:** Stops the processor until RESET is asserted.

**Syntax:**
```asm
STP              ; Low power mode
```

**Flags Affected:** None

**Bytes:** 1 | **Cycles:** 3 (then waits)

#### WAI - Wait for Interrupt

**Operation:** Wait until IRQ or NMI

**Description:** Puts the processor in low-power mode until an interrupt occurs.

**Syntax:**
```asm
WAI              ; Wait for interrupt
```

**Flags Affected:** None

**Bytes:** 1 | **Cycles:** 3 (then waits)

#### WDM - Reserved

**Operation:** No operation (reserved for future use)

**Description:** Reserved opcode for future expansion. Acts as 2-byte NOP.

**Syntax:**
```asm
WDM #$42         ; Reserved instruction
```

**Flags Affected:** None

**Bytes:** 2 | **Cycles:** 2

## New Addressing Modes

### Stack Relative

Access data relative to stack pointer.

**Syntax:**
```asm
LDA $03,S        ; Load from S+3
STA $05,S        ; Store to S+5
```

**Available for:** ADC, AND, CMP, EOR, LDA, ORA, SBC, STA

### Stack Relative Indirect Indexed

Access via pointer on stack, indexed by Y.

**Syntax:**
```asm
LDA ($03,S),Y    ; Load from (S+3)+Y
STA ($05,S),Y    ; Store to (S+5)+Y
```

### Long Addressing

24-bit absolute addressing.

**Syntax:**
```asm
LDA $012000      ; Load from bank $01, addr $2000
STA $028000      ; Store to bank $02, addr $8000
```

### Long Indexed

24-bit addressing with X index.

**Syntax:**
```asm
LDA $012000,X    ; Load from bank $01, addr $2000+X
```

### Direct Page Indirect Long

24-bit pointer in direct page.

**Syntax:**
```asm
LDA [$12]        ; Load from 24-bit address at D+$12
```

### Direct Page Indirect Long Indexed

24-bit pointer in direct page, indexed by Y.

**Syntax:**
```asm
LDA [$12],Y      ; Load from (24-bit address at D+$12)+Y
```

## Code Examples

### Mode Switching

```asm
; Start in emulation mode (power-on state)
        CLC
        XCE              ; Switch to native mode

        REP #$30         ; 16-bit A, X, Y
        ; Now in native 16-bit mode

        ; Later, switch back to emulation
        SEP #$30         ; 8-bit A, X, Y (prepare for emulation)
        SEC
        XCE              ; Switch to emulation mode
```

### 16-bit Operations

```asm
        REP #$30         ; 16-bit mode

        LDA #$1234       ; Load 16-bit value
        STA $2000        ; Store 16-bit (stores $34, $35)

        LDX #$0100       ; Load 16-bit index
        LDA $2000,X      ; Indexed access

        SEP #$30         ; Back to 8-bit
```

### Block Copy

```asm
; Copy 4096 bytes from $018000 to $028000
        REP #$20         ; 16-bit accumulator
        LDA #$0FFF       ; Byte count - 1
        LDX #$8000       ; Source offset
        LDY #$8000       ; Dest offset
        SEP #$20         ; Back to 8-bit
        MVN $02,$01      ; Move from bank $01 to $02
```

### Stack Frame Access

```asm
; Function with local variables
FUNC    PHD              ; Save direct page
        TSC              ; Get stack pointer
        SEC
        SBC #$0010       ; Reserve 16 bytes
        TCS              ; Set new stack top
        TCD              ; Direct page = stack frame

        ; Access locals via direct page
        LDA ($02)        ; Local variable at offset 2
        STA ($04)        ; Local variable at offset 4

        ; Restore and return
        PLD              ; Restore direct page
        RTS
```

### Long Subroutine Call

```asm
; In bank $00
        JSL $012000      ; Call routine in bank $01

; In bank $01
ROUTINE LDA #$42         ; Do work
        RTL              ; Return to bank $00
```

## Performance Notes

### 16-bit vs 8-bit Operations

- **16-bit loads/stores**: +1 cycle
- **16-bit arithmetic**: +1 cycle
- **16-bit index**: +1 cycle per memory access

**Trade-off:** Speed vs. capability

### Block Move Performance

MVN/MVP: 7 cycles per byte (very efficient for large moves)

### Direct Page

- **Direct page at $0000**: Same speed as 6502 zero page
- **Direct page not at $0000**: +1 cycle per direct page access

## Compatibility

### Running 6502/65C02 Code

1. Power on (starts in emulation mode)
2. All 6502/65C02 code runs unmodified
3. Stack in page 1 ($0100-$01FF)
4. 8-bit A, X, Y
5. 16-bit addressing only

### Detecting 65816

```asm
; Test for 65816
        SEC
        XCE              ; Try to enter emulation
        BCS IS_65816     ; Carry set if was in native mode
        ; Is 6502/65C02
IS_65816:
        ; Is 65816
```

## References

- WDC W65C816S Datasheet
- Programming the 65816 by David Eyes and Ron Lichty
- 65816/65802 Assembly Language Programming by Michael Fischer
- Apple IIgs Hardware Reference Manual

## See Also

- [6502 Instruction Set](./6502-instruction-set.md) - Base instruction set
- [65C02 Instruction Set](./65c02-instruction-set.md) - 8-bit CMOS version
