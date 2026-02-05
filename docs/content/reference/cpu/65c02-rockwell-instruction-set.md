---
id: 65c02-rockwell-instruction-set
sidebar_position: 3
---

# Rockwell 65C02 Instruction Set Reference

**Processor:** Rockwell R65C02
**Architecture:** 8-bit CMOS
**Addressing:** 16-bit address bus (64KB)
**Released:** 1983

## Overview

The Rockwell 65C02 is a variant of the WDC 65C02 with additional bit manipulation instructions. It includes all WDC 65C02 enhancements plus 16 new instructions for efficient bit testing and manipulation.

### Key Features

1. **All WDC 65C02 Features**: Includes all improvements from WDC 65C02
2. **Bit Manipulation**: RMB/SMB instructions for setting/clearing individual bits
3. **Bit Branch Instructions**: BBR/BBS for testing and branching on individual bits
4. **Zero Page Only**: Bit instructions only work with zero page addresses

## Rockwell-Specific Instructions

### RMB - Reset Memory Bit

**Operation:** M[bit] = 0

**Description:** Clears a specific bit (0-7) in a zero page location.

**Addressing Mode:**
```asm
RMB0 $80      ; Clear bit 0 of $80
RMB1 $80      ; Clear bit 1 of $80
RMB2 $80      ; Clear bit 2 of $80
RMB3 $80      ; Clear bit 3 of $80
RMB4 $80      ; Clear bit 4 of $80
RMB5 $80      ; Clear bit 5 of $80
RMB6 $80      ; Clear bit 6 of $80
RMB7 $80      ; Clear bit 7 of $80
```

**Flags Affected:** None

**Bytes:** 2 | **Cycles:** 5

**Example:**
```asm
RMB7 $2000    ; ERROR - must be zero page
RMB7 $80      ; OK - clear bit 7 of $80
```

### SMB - Set Memory Bit

**Operation:** M[bit] = 1

**Description:** Sets a specific bit (0-7) in a zero page location.

**Addressing Mode:**
```asm
SMB0 $80      ; Set bit 0 of $80
SMB1 $80      ; Set bit 1 of $80
SMB2 $80      ; Set bit 2 of $80
SMB3 $80      ; Set bit 3 of $80
SMB4 $80      ; Set bit 4 of $80
SMB5 $80      ; Set bit 5 of $80
SMB6 $80      ; Set bit 6 of $80
SMB7 $80      ; Set bit 7 of $80
```

**Flags Affected:** None

**Bytes:** 2 | **Cycles:** 5

**Example:**
```asm
SMB0 $90      ; Set bit 0 of $90
```

### BBR - Branch on Bit Reset

**Operation:** Branch if M[bit] = 0

**Description:** Tests a specific bit (0-7) in a zero page location and branches if the bit is clear (0).

**Addressing Mode:**
```asm
BBR0 $80,label ; Branch if bit 0 of $80 is clear
BBR1 $80,label ; Branch if bit 1 of $80 is clear
BBR2 $80,label ; Branch if bit 2 of $80 is clear
BBR3 $80,label ; Branch if bit 3 of $80 is clear
BBR4 $80,label ; Branch if bit 4 of $80 is clear
BBR5 $80,label ; Branch if bit 5 of $80 is clear
BBR6 $80,label ; Branch if bit 6 of $80 is clear
BBR7 $80,label ; Branch if bit 7 of $80 is clear
```

**Flags Affected:** None

**Bytes:** 3 | **Cycles:** 5-7

**Timing:** 5 cycles if no branch, 6 cycles if branch taken, 7 cycles if branch crosses page boundary

**Example:**
```asm
WAIT    BBR7 $80,WAIT  ; Wait until bit 7 of $80 is set
```

### BBS - Branch on Bit Set

**Operation:** Branch if M[bit] = 1

**Description:** Tests a specific bit (0-7) in a zero page location and branches if the bit is set (1).

**Addressing Mode:**
```asm
BBS0 $80,label ; Branch if bit 0 of $80 is set
BBS1 $80,label ; Branch if bit 1 of $80 is set
BBS2 $80,label ; Branch if bit 2 of $80 is set
BBS3 $80,label ; Branch if bit 3 of $80 is set
BBS4 $80,label ; Branch if bit 4 of $80 is set
BBS5 $80,label ; Branch if bit 5 of $80 is set
BBS6 $80,label ; Branch if bit 6 of $80 is set
BBS7 $80,label ; Branch if bit 7 of $80 is set
```

**Flags Affected:** None

**Bytes:** 3 | **Cycles:** 5-7

**Timing:** 5 cycles if no branch, 6 cycles if branch taken, 7 cycles if branch crosses page boundary

**Example:**
```asm
        BBS0 $90,READY ; Branch if bit 0 of $90 is set
```

## Rockwell Instruction Summary

| Instruction | Variants | Description | Flags | Bytes | Cycles |
|-------------|----------|-------------|-------|-------|--------|
| **RMB** | RMB0-RMB7 | Reset Memory Bit | - | 2 | 5 |
| **SMB** | SMB0-SMB7 | Set Memory Bit | - | 2 | 5 |
| **BBR** | BBR0-BBR7 | Branch on Bit Reset | - | 3 | 5-7 |
| **BBS** | BBS0-BBS7 | Branch on Bit Set | - | 3 | 5-7 |

**Total:** 32 new instructions (8 variants × 4 instruction types)

## Complete Instruction Set

### WDC 65C02 Base

The Rockwell 65C02 includes all 69 instructions from the WDC 65C02:
- All 56 original 6502 instructions
- 8 new WDC 65C02 instructions (BRA, PHX, PHY, PLX, PLY, STZ, TRB, TSB)
- Enhanced addressing modes for BIT

See [65C02 Instruction Set](./65c02-instruction-set.md) for details.

### Rockwell Extensions

Plus 32 additional bit manipulation instructions (RMB0-7, SMB0-7, BBR0-7, BBS0-7).

**Total Instruction Count:** 101 instructions

## Code Examples

### Setting/Clearing Hardware Flags

```asm
; Enable interrupt (set bit 7 of control register)
        SMB7 $D000     ; Set enable bit

; Disable interrupt (clear bit 7)
        RMB7 $D000     ; Clear enable bit
```

### Polling Hardware Status

```asm
; Wait for data ready (bit 0)
WAIT    BBR0 $D001,WAIT ; Loop until bit 0 set

        LDA $D000       ; Read data
```

### Multiple Flag Manipulation

```asm
; Configure device: bits 7,5,3 set, others clear
        LDA #0
        STA $80         ; Clear all bits

        SMB7 $80        ; Set bit 7
        SMB5 $80        ; Set bit 5
        SMB3 $80        ; Set bit 3

        LDA $80         ; Load configured value
        STA $D000       ; Write to device
```

### Bit-Banged Serial Output

```asm
; Send byte in A via bit 0 of $80
SEND    LDX #8          ; 8 bits to send
LOOP    LSR A           ; Shift bit into carry
        BCS SET1        ; Branch if bit is 1
        RMB0 $80        ; Bit is 0
        BRA NEXT
SET1    SMB0 $80        ; Bit is 1
NEXT    DEX
        BNE LOOP
        RTS
```

### State Machine Implementation

```asm
; State flags in $F0 (bits 0-2 for state 0-7)
STATE0  BBS0 $F0,STATE1
        ; Handle state 0
        RTS

STATE1  BBS1 $F0,STATE2
        ; Handle state 1
        RTS

STATE2  BBS2 $F0,STATE3
        ; Handle state 2
        RTS
```

## Comparison: WDC vs Rockwell

### Bit Setting/Clearing

**WDC 65C02:**
```asm
; Clear bit 7
        LDA $80
        AND #$7F       ; 3 bytes, 7 cycles
        STA $80

; Set bit 7
        LDA $80
        ORA #$80       ; 3 bytes, 7 cycles
        STA $80
```

**Rockwell 65C02:**
```asm
; Clear bit 7
        RMB7 $80       ; 2 bytes, 5 cycles

; Set bit 7
        SMB7 $80       ; 2 bytes, 5 cycles
```

**Savings:** 1 byte and 2 cycles per operation

### Bit Testing with Branch

**WDC 65C02:**
```asm
; Wait for bit 7 set
WAIT    LDA $80
        AND #$80       ; 5 bytes, ~13 cycles per loop
        BEQ WAIT
```

**Rockwell 65C02:**
```asm
; Wait for bit 7 set
WAIT    BBR7 $80,WAIT  ; 3 bytes, ~6 cycles per loop
```

**Savings:** 2 bytes and ~7 cycles per loop iteration

## Hardware Applications

The Rockwell bit instructions are particularly useful for:

1. **I/O Port Control**: Direct bit manipulation of hardware registers
2. **Status Polling**: Efficient waiting on hardware status bits
3. **Protocol Implementation**: Bit-level communication protocols
4. **Flag Management**: Compact flag storage and testing
5. **Real-Time Systems**: Fast, deterministic bit operations

## Compatibility Notes

### Forward Compatibility

- All WDC 65C02 code runs on Rockwell 65C02
- Rockwell-specific code (RMB/SMB/BBR/BBS) will not run on WDC 65C02

### Backward Compatibility

- All 6502 code runs on Rockwell 65C02 (with same exceptions as WDC 65C02)
- Rockwell code will not run on 6502 or standard WDC 65C02

### Detection at Runtime

```asm
; Test for Rockwell extensions
        SMB0 $00        ; Try to set bit 0
        LDA $00
        AND #$01        ; Test if bit 0 is set
        BEQ NOT_ROCK    ; Not Rockwell if bit not set
        ; Is Rockwell 65C02
```

## Processor Variants

Different manufacturers produced 65C02 variants:

| Manufacturer | Bit Instructions | Notes |
|--------------|------------------|-------|
| **WDC** | No | Standard 65C02 |
| **Rockwell** | Yes (RMB/SMB/BBR/BBS) | Most common with bit ops |
| **Synertek** | Yes | Same as Rockwell |
| **GTE** | Yes | Same as Rockwell |

## References

- Rockwell R65C02 Datasheet
- R65C02 Assembly Language Programming Manual
- WDC W65C02S Datasheet (for base 65C02 features)
- Programming the 65816 by David Eyes and Ron Lichty

## See Also

- [6502 Instruction Set](./6502-instruction-set.md) - Base instruction set
- [65C02 Instruction Set](./65c02-instruction-set.md) - WDC variant without bit instructions
- [65816 Instruction Set](./65816-instruction-set.md) - 16-bit extension
