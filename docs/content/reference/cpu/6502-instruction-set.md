---
sidebar_position: 1
---

# MOS 6502 Instruction Set Reference

**Processor:** MOS Technology 6502
**Architecture:** 8-bit
**Addressing:** 16-bit address bus (64KB)
**Released:** 1975

## Overview

The MOS 6502 is an 8-bit microprocessor with a 16-bit address space. It features a simple, elegant instruction set with 56 instructions and 13 addressing modes.

### Registers

- **A** - Accumulator (8-bit)
- **X** - X Index Register (8-bit)
- **Y** - Y Index Register (8-bit)
- **SP** - Stack Pointer (8-bit, page 1: $0100-$01FF)
- **PC** - Program Counter (16-bit)
- **P** - Processor Status Flags (8-bit)

### Status Flags (P Register)

| Bit | Flag | Name | Description |
|-----|------|------|-------------|
| 7 | N | Negative | Set if bit 7 of result is set |
| 6 | V | Overflow | Set on signed arithmetic overflow |
| 5 | - | (Unused) | Always 1 |
| 4 | B | Break | Set when BRK instruction executed |
| 3 | D | Decimal | Decimal mode flag (BCD arithmetic) |
| 2 | I | Interrupt Disable | Set to disable IRQ interrupts |
| 1 | Z | Zero | Set if result is zero |
| 0 | C | Carry | Carry/borrow flag |

## Addressing Modes

| Mode | Syntax | Example | Description |
|------|--------|---------|-------------|
| Implied | - | `TAX` | No operand needed |
| Accumulator | `A` | `LSR A` | Operates on accumulator |
| Immediate | `#$nn` | `LDA #$42` | Operand is constant value |
| Zero Page | `$nn` | `LDA $80` | Address in page 0 ($00-$FF) |
| Zero Page,X | `$nn,X` | `LDA $80,X` | Zero page address + X |
| Zero Page,Y | `$nn,Y` | `LDX $80,Y` | Zero page address + Y |
| Absolute | `$nnnn` | `LDA $2000` | Full 16-bit address |
| Absolute,X | `$nnnn,X` | `LDA $2000,X` | Absolute address + X |
| Absolute,Y | `$nnnn,Y` | `LDA $2000,Y` | Absolute address + Y |
| Indirect | `($nnnn)` | `JMP ($2000)` | Address stored at location |
| Indexed Indirect | `($nn,X)` | `LDA ($80,X)` | (Zero page + X) pointer |
| Indirect Indexed | `($nn),Y` | `LDA ($80),Y` | (Zero page pointer) + Y |
| Relative | `label` | `BNE loop` | PC-relative branch (-128 to +127) |

## Instruction Set by Category

### Load/Store Operations

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **LDA** | Load Accumulator | N Z | 2-3 | 2-5 |
| **LDX** | Load X Register | N Z | 2-3 | 2-4 |
| **LDY** | Load Y Register | N Z | 2-3 | 2-4 |
| **STA** | Store Accumulator | - | 2-3 | 3-5 |
| **STX** | Store X Register | - | 2-3 | 3-4 |
| **STY** | Store Y Register | - | 2-3 | 3-4 |

### Register Transfer

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **TAX** | Transfer A to X | N Z | 1 | 2 |
| **TAY** | Transfer A to Y | N Z | 1 | 2 |
| **TXA** | Transfer X to A | N Z | 1 | 2 |
| **TYA** | Transfer Y to A | N Z | 1 | 2 |
| **TSX** | Transfer SP to X | N Z | 1 | 2 |
| **TXS** | Transfer X to SP | - | 1 | 2 |

### Stack Operations

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **PHA** | Push Accumulator | - | 1 | 3 |
| **PHP** | Push Processor Status | - | 1 | 3 |
| **PLA** | Pull Accumulator | N Z | 1 | 4 |
| **PLP** | Pull Processor Status | All | 1 | 4 |

### Arithmetic Operations

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **ADC** | Add with Carry | N V Z C | 2-3 | 2-5 |
| **SBC** | Subtract with Carry | N V Z C | 2-3 | 2-5 |
| **INC** | Increment Memory | N Z | 2-3 | 5-6 |
| **INX** | Increment X | N Z | 1 | 2 |
| **INY** | Increment Y | N Z | 1 | 2 |
| **DEC** | Decrement Memory | N Z | 2-3 | 5-6 |
| **DEX** | Decrement X | N Z | 1 | 2 |
| **DEY** | Decrement Y | N Z | 1 | 2 |

### Logical Operations

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **AND** | Logical AND | N Z | 2-3 | 2-5 |
| **ORA** | Logical OR | N Z | 2-3 | 2-5 |
| **EOR** | Logical XOR | N Z | 2-3 | 2-5 |
| **BIT** | Bit Test | N V Z | 2-3 | 3-4 |

### Shift and Rotate

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **ASL** | Arithmetic Shift Left | N Z C | 1-3 | 2-6 |
| **LSR** | Logical Shift Right | N Z C | 1-3 | 2-6 |
| **ROL** | Rotate Left | N Z C | 1-3 | 2-6 |
| **ROR** | Rotate Right | N Z C | 1-3 | 2-6 |

### Comparison Operations

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **CMP** | Compare Accumulator | N Z C | 2-3 | 2-5 |
| **CPX** | Compare X Register | N Z C | 2-3 | 2-4 |
| **CPY** | Compare Y Register | N Z C | 2-3 | 2-4 |

### Branch Instructions

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **BCC** | Branch if Carry Clear | - | 2 | 2-4 |
| **BCS** | Branch if Carry Set | - | 2 | 2-4 |
| **BEQ** | Branch if Equal (Z=1) | - | 2 | 2-4 |
| **BNE** | Branch if Not Equal (Z=0) | - | 2 | 2-4 |
| **BMI** | Branch if Minus (N=1) | - | 2 | 2-4 |
| **BPL** | Branch if Plus (N=0) | - | 2 | 2-4 |
| **BVC** | Branch if Overflow Clear | - | 2 | 2-4 |
| **BVS** | Branch if Overflow Set | - | 2 | 2-4 |

### Jump and Subroutine

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **JMP** | Jump | - | 3 | 3-5 |
| **JSR** | Jump to Subroutine | - | 3 | 6 |
| **RTS** | Return from Subroutine | - | 1 | 6 |
| **RTI** | Return from Interrupt | All | 1 | 6 |

### Flag Operations

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **CLC** | Clear Carry Flag | C=0 | 1 | 2 |
| **CLD** | Clear Decimal Mode | D=0 | 1 | 2 |
| **CLI** | Clear Interrupt Disable | I=0 | 1 | 2 |
| **CLV** | Clear Overflow Flag | V=0 | 1 | 2 |
| **SEC** | Set Carry Flag | C=1 | 1 | 2 |
| **SED** | Set Decimal Mode | D=1 | 1 | 2 |
| **SEI** | Set Interrupt Disable | I=1 | 1 | 2 |

### System Operations

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **BRK** | Force Break | B=1, I=1 | 1 | 7 |
| **NOP** | No Operation | - | 1 | 2 |

## Instruction Details

### ADC - Add with Carry

**Operation:** A = A + M + C

**Description:** Adds the contents of memory and the carry flag to the accumulator.

**Addressing Modes:**
```asm
ADC #$42      ; Immediate
ADC $80       ; Zero Page
ADC $80,X     ; Zero Page,X
ADC $2000     ; Absolute
ADC $2000,X   ; Absolute,X
ADC $2000,Y   ; Absolute,Y
ADC ($80,X)   ; Indexed Indirect
ADC ($80),Y   ; Indirect Indexed
```

**Flags Affected:** N V Z C

**Notes:** In decimal mode (D=1), performs BCD addition.

### AND - Logical AND

**Operation:** A = A & M

**Description:** Performs bitwise AND between accumulator and memory.

**Addressing Modes:**
```asm
AND #$42      ; Immediate
AND $80       ; Zero Page
AND $80,X     ; Zero Page,X
AND $2000     ; Absolute
AND $2000,X   ; Absolute,X
AND $2000,Y   ; Absolute,Y
AND ($80,X)   ; Indexed Indirect
AND ($80),Y   ; Indirect Indexed
```

**Flags Affected:** N Z

### ASL - Arithmetic Shift Left

**Operation:** C ← [7][6][5][4][3][2][1][0] ← 0

**Description:** Shifts all bits left one position. Bit 0 is set to 0 and bit 7 is shifted into the carry flag.

**Addressing Modes:**
```asm
ASL A         ; Accumulator
ASL $80       ; Zero Page
ASL $80,X     ; Zero Page,X
ASL $2000     ; Absolute
ASL $2000,X   ; Absolute,X
```

**Flags Affected:** N Z C

### BCC - Branch if Carry Clear

**Operation:** Branch if C = 0

**Description:** If the carry flag is clear, adds the relative displacement to the program counter.

**Addressing Mode:**
```asm
BCC label     ; Relative (-128 to +127 bytes)
```

**Flags Affected:** None

**Cycles:** 2 if no branch, 3 if branch taken, 4 if branch crosses page boundary

### BIT - Bit Test

**Operation:** N = M[7], V = M[6], Z = !(A & M)

**Description:** Tests bits in memory with accumulator. Sets N and V from memory bits 7 and 6. Sets Z if (A & M) is zero.

**Addressing Modes:**
```asm
BIT $80       ; Zero Page
BIT $2000     ; Absolute
```

**Flags Affected:** N V Z

### BRK - Force Break

**Operation:** PC+2↓, [PC+1:PC]=[FFFE:FFFF]

**Description:** Forces an interrupt. Pushes PC+2 and status register to stack, then loads interrupt vector from $FFFE/$FFFF.

**Addressing Mode:**
```asm
BRK           ; Implied
```

**Flags Affected:** B I

**Notes:** Sets B flag to distinguish from hardware IRQ.

### JMP - Jump

**Operation:** PC = address

**Description:** Sets the program counter to the specified address.

**Addressing Modes:**
```asm
JMP $2000     ; Absolute
JMP ($2000)   ; Indirect
```

**Flags Affected:** None

**Notes:** Indirect mode has a page boundary bug - if address is $xxFF, high byte is fetched from $xx00 instead of $xx+1:00.

### JSR - Jump to Subroutine

**Operation:** PC+2↓, PC = address

**Description:** Pushes the address of the next instruction minus one to the stack, then jumps to the subroutine.

**Addressing Mode:**
```asm
JSR $2000     ; Absolute
```

**Flags Affected:** None

**Cycles:** 6

### LDA - Load Accumulator

**Operation:** A = M

**Description:** Loads a byte from memory into the accumulator.

**Addressing Modes:**
```asm
LDA #$42      ; Immediate
LDA $80       ; Zero Page
LDA $80,X     ; Zero Page,X
LDA $2000     ; Absolute
LDA $2000,X   ; Absolute,X
LDA $2000,Y   ; Absolute,Y
LDA ($80,X)   ; Indexed Indirect
LDA ($80),Y   ; Indirect Indexed
```

**Flags Affected:** N Z

### SBC - Subtract with Carry

**Operation:** A = A - M - !C

**Description:** Subtracts memory and the inverse of carry from the accumulator.

**Addressing Modes:**
```asm
SBC #$42      ; Immediate
SBC $80       ; Zero Page
SBC $80,X     ; Zero Page,X
SBC $2000     ; Absolute
SBC $2000,X   ; Absolute,X
SBC $2000,Y   ; Absolute,Y
SBC ($80,X)   ; Indexed Indirect
SBC ($80),Y   ; Indirect Indexed
```

**Flags Affected:** N V Z C

**Notes:** In decimal mode (D=1), performs BCD subtraction. Carry flag is set if no borrow occurred.

## Timing Notes

- Cycles listed are typical. Add 1 cycle for page boundary crossing on indexed modes.
- Add 1 cycle for branches taken, 2 cycles if branch crosses page boundary.
- Stack operations (PHA, PLA, PHP, PLP, JSR, RTS, RTI, BRK) are slower due to stack access.

## Bugs and Quirks

1. **JMP Indirect Page Boundary Bug**: If the low byte of the indirect address is $FF, the high byte is fetched from $xx00 instead of $(xx+1)00.

2. **Decimal Mode Flags**: In decimal mode, N, V, and Z flags may be invalid after ADC/SBC.

3. **ROR/ROL Timing**: These instructions take 1 extra cycle when operating on memory.

## References

- MOS Technology 6502 Programming Manual (1976)
- 6502 Assembly Language Programming by Lance Leventhal (1979)
- Western Design Center W65C02S Datasheet

## See Also

- [65C02 Instruction Set](./65c02-instruction-set.md) - Enhanced CMOS version
- [65816 Instruction Set](./65816-instruction-set.md) - 16-bit extension
