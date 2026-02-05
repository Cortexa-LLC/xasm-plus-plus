---
id: 65c02-instruction-set
sidebar_position: 2
---

# WDC 65C02 Instruction Set Reference

**Processor:** Western Design Center 65C02
**Architecture:** 8-bit CMOS
**Addressing:** 16-bit address bus (64KB)
**Released:** 1983

## Overview

The 65C02 is a CMOS version of the 6502 with bug fixes, additional instructions, and new addressing modes. It maintains backward compatibility with the 6502 while adding significant improvements.

### Key Improvements Over 6502

1. **New Instructions**: 8 new opcodes (BRA, PHX, PHY, PLX, PLY, STZ, TRB, TSB)
2. **New Addressing Modes**: (ZP) indirect and (ZP),Y indexed
3. **Bug Fixes**: JMP indirect page boundary bug fixed
4. **Timing Changes**: RMW instructions take 1 fewer cycle
5. **Undefined Opcodes**: All behave as NOPs instead of crashing

### Registers

Same as 6502:
- **A** - Accumulator (8-bit)
- **X** - X Index Register (8-bit)
- **Y** - Y Index Register (8-bit)
- **SP** - Stack Pointer (8-bit, page 1: $0100-$01FF)
- **PC** - Program Counter (16-bit)
- **P** - Processor Status Flags (8-bit)

## New Instructions (65C02 Only)

### BRA - Branch Always

**Operation:** PC = PC + offset

**Description:** Unconditional relative branch (always taken).

**Addressing Mode:**
```asm
BRA label     ; Relative (-128 to +127 bytes)
```

**Flags Affected:** None

**Bytes:** 2 | **Cycles:** 3-4

**Notes:** Always adds 1 cycle for branch taken, plus 1 more if crossing page boundary.

### PHX - Push X Register

**Operation:** X↓

**Description:** Pushes the X register onto the stack.

**Addressing Mode:**
```asm
PHX           ; Implied
```

**Flags Affected:** None

**Bytes:** 1 | **Cycles:** 3

### PHY - Push Y Register

**Operation:** Y↓

**Description:** Pushes the Y register onto the stack.

**Addressing Mode:**
```asm
PHY           ; Implied
```

**Flags Affected:** None

**Bytes:** 1 | **Cycles:** 3

### PLX - Pull X Register

**Operation:** X↑

**Description:** Pulls a byte from the stack into the X register.

**Addressing Mode:**
```asm
PLX           ; Implied
```

**Flags Affected:** N Z

**Bytes:** 1 | **Cycles:** 4

### PLY - Pull Y Register

**Operation:** Y↑

**Description:** Pulls a byte from the stack into the Y register.

**Addressing Mode:**
```asm
PLY           ; Implied
```

**Flags Affected:** N Z

**Bytes:** 1 | **Cycles:** 4

### STZ - Store Zero

**Operation:** M = 0

**Description:** Stores zero to memory (more efficient than LDA #0 / STA).

**Addressing Modes:**
```asm
STZ $80       ; Zero Page
STZ $80,X     ; Zero Page,X
STZ $2000     ; Absolute
STZ $2000,X   ; Absolute,X
```

**Flags Affected:** None

**Bytes:** 2-3 | **Cycles:** 3-5

### TRB - Test and Reset Bits

**Operation:** Z = !(A & M), M = M & !A

**Description:** Tests bits (sets Z if A & M == 0), then clears bits in memory where accumulator bits are set.

**Addressing Modes:**
```asm
TRB $80       ; Zero Page
TRB $2000     ; Absolute
```

**Flags Affected:** Z

**Bytes:** 2-3 | **Cycles:** 5-6

**Example:**
```asm
LDA #$0F      ; Mask for low nibble
TRB $80       ; Test and clear low nibble of $80
```

### TSB - Test and Set Bits

**Operation:** Z = !(A & M), M = M | A

**Description:** Tests bits (sets Z if A & M == 0), then sets bits in memory where accumulator bits are set.

**Addressing Modes:**
```asm
TSB $80       ; Zero Page
TSB $2000     ; Absolute
```

**Flags Affected:** Z

**Bytes:** 2-3 | **Cycles:** 5-6

**Example:**
```asm
LDA #$80      ; Bit 7 mask
TSB $2000     ; Test and set bit 7
```

## New Addressing Modes (65C02 Only)

### Zero Page Indirect: (ZP)

Available for: ADC, AND, CMP, EOR, LDA, ORA, SBC, STA

**Syntax:**
```asm
LDA ($80)     ; Load from address stored at $80/$81
STA ($90)     ; Store to address stored at $90/$91
```

**Description:** Uses a zero page address as a pointer to the actual data location. More efficient than indexed indirect when X is not needed.

**Cycles:** Typically 1 fewer than ($nn,X) equivalent

### Absolute Indexed Indirect: (nnnn,X)

Available for: JMP only

**Syntax:**
```asm
JMP ($2000,X) ; Jump to address at $2000+X
```

**Description:** Enhanced JMP indirect with X index. Useful for jump tables.

**Bytes:** 3 | **Cycles:** 6

## Bug Fixes

### JMP Indirect Page Boundary

**6502 Bug:**
```asm
JMP ($10FF)   ; Would fetch high byte from $1000 instead of $1100
```

**65C02 Fix:**
```asm
JMP ($10FF)   ; Correctly fetches high byte from $1100
```

The 65C02 properly handles page crossings in JMP indirect mode.

### RMW (Read-Modify-Write) Instructions

**6502:** ASL, LSR, ROL, ROR memory operations took 6 cycles (zero page) or 7 cycles (absolute)

**65C02:** Same operations take 5 cycles (zero page) or 6 cycles (absolute)

**Instructions affected:**
- ASL memory
- DEC memory
- INC memory
- LSR memory
- ROL memory
- ROR memory

## Complete Instruction Set

### All 6502 Instructions

The 65C02 supports all 56 legal 6502 instructions. See [6502 Instruction Set](./6502-instruction-set.md) for details.

### Additional 65C02 Instructions

| Mnemonic | Description | Flags | Bytes | Cycles |
|----------|-------------|-------|-------|--------|
| **BRA** | Branch Always | - | 2 | 3-4 |
| **PHX** | Push X Register | - | 1 | 3 |
| **PHY** | Push Y Register | - | 1 | 3 |
| **PLX** | Pull X Register | N Z | 1 | 4 |
| **PLY** | Pull Y Register | N Z | 1 | 4 |
| **STZ** | Store Zero | - | 2-3 | 3-5 |
| **TRB** | Test and Reset Bits | Z | 2-3 | 5-6 |
| **TSB** | Test and Set Bits | Z | 2-3 | 5-6 |

## Enhanced Instructions (Modified from 6502)

### BIT - Bit Test (Enhanced)

**6502 Modes:**
```asm
BIT $80       ; Zero Page
BIT $2000     ; Absolute
```

**65C02 Additional Modes:**
```asm
BIT #$42      ; Immediate (doesn't affect N/V)
BIT $80,X     ; Zero Page,X
BIT $2000,X   ; Absolute,X
```

**Notes:** Immediate mode only affects Z flag (N and V unchanged).

## Undefined Opcodes

**6502:** Undefined opcodes could crash or have unpredictable behavior.

**65C02:** All undefined opcodes are treated as single-byte NOPs (no operation). This makes the 65C02 more robust and easier to debug.

## Decimal Mode Changes

The 65C02 fixes several decimal mode issues:

1. **Flags Valid:** N, V, Z flags are now valid after ADC/SBC in decimal mode
2. **Timing:** Decimal mode no longer adds extra cycles
3. **Behavior:** More predictable BCD arithmetic

## Code Examples

### Using New Stack Instructions

```asm
; Save registers (more efficient than 6502)
        PHX              ; Push X
        PHY              ; Push Y

        ; ... do work ...

        PLY              ; Pull Y
        PLX              ; Pull X
        RTS
```

### Using STZ

```asm
; 6502 way (4 bytes, 5 cycles)
        LDA #0
        STA $2000
        STA $2001

; 65C02 way (6 bytes, 8 cycles, but clearer)
        STZ $2000        ; More readable
        STZ $2001
```

### Using BRA for Loops

```asm
; 6502 way (use dummy branch)
LOOP    ; ... code ...
        CLC
        BCC LOOP         ; Always branches

; 65C02 way (clearer intent)
LOOP    ; ... code ...
        BRA LOOP         ; Unconditional branch
```

### Using Zero Page Indirect

```asm
; 6502 way
        LDX #0
        LDA ($80,X)      ; 6 cycles

; 65C02 way
        LDA ($80)        ; 5 cycles, clearer
```

### Using TRB/TSB for Bit Manipulation

```asm
; Clear bits 0-3 in memory
        LDA #$0F
        TRB $2000        ; Clear low nibble

; Set bit 7 (enable flag)
        LDA #$80
        TSB $2000        ; Set bit 7

; Test if any bits are set
        LDA #$FF
        TRB $2000        ; Z=1 if $2000 was zero
```

## Timing Summary

| Operation Type | 6502 Cycles | 65C02 Cycles | Improvement |
|---------------|-------------|--------------|-------------|
| RMW Zero Page | 5 | 5 | Same |
| RMW Absolute | 6 | 6 | Same |
| JMP Indirect | 5 | 6 | +1 (but fixes bug) |
| Decimal ADC/SBC | +1 | +0 | -1 cycle |

## Compatibility Notes

### Forward Compatibility

All 6502 code runs correctly on 65C02 with these exceptions:

1. **Undefined Opcodes:** Code relying on undefined opcode behavior will break
2. **Decimal Mode:** Code depending on invalid flags in decimal mode may behave differently
3. **Timing:** Code with cycle-exact timing may need adjustment

### Backward Compatibility

65C02 code using new instructions will not run on 6502.

## References

- WDC W65C02S Datasheet
- Rockwell R65C02 Datasheet
- Programming the 65816 by David Eyes and Ron Lichty
- 6502 Assembly Language Programming by Lance Leventhal

## See Also

- [6502 Instruction Set](./6502-instruction-set.md) - Base instruction set
- [65C02 Rockwell Instruction Set](./65c02-rockwell-instruction-set.md) - Rockwell variant with additional bit instructions
- [65816 Instruction Set](./65816-instruction-set.md) - 16-bit extension
