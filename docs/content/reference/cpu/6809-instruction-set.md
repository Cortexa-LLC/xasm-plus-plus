---
sidebar_position: 2
---

# Motorola 6809 Instruction Set Reference

**Source:** 6809 Assembly Language Programming (Lance Leventhal)
**CPU:** Motorola 6809 8-bit Microprocessor

## Addressing Modes

The 6809 supports multiple addressing modes for flexible memory access:

### 1. Inherent Addressing
Instruction contains all necessary information. No operand bytes needed.

**Examples:**
- `ABX` - Add B to X
- `DAA` - Decimal Adjust Accumulator A
- `SWI` - Software Interrupt
- `ASRA` - Arithmetic Shift Right A
- `CLRB` - Clear B

### 2. Immediate Addressing
Operand is contained in bytes immediately following the opcode.

**Syntax:**
- 8-bit: `LDA #$FF` - Load immediate value into A
- 16-bit: `LDX #$1000` - Load immediate 16-bit value into X

**Assembler notation:** `#` prefix indicates immediate mode

### 3. Direct Addressing
Effective address formed by concatenating Direct Page Register (DP) with the byte following opcode.
Allows accessing 256 locations within any of 256 pages.

**Syntax:** `LDA $50` - Load from address DP:$50

**Benefits:**
- Faster than extended (fewer bytes, fewer cycles)
- Useful for zero-page-like access
- Requires SETDP directive to inform assembler of DP value

### 4. Extended Addressing
Full 16-bit address in two bytes following opcode.
Can access entire 64K address space.

**Syntax:** `LDA $1000` - Load from address $1000

### 5. Indexed Addressing
Most powerful addressing mode. Uses pointer registers (X, Y, U, S, PC) with various offset calculations.

**Variants:**
- **No offset:** `LDA ,X` - Load from address in X
- **5-bit offset:** `LDA 10,X` - Load from X+10 (0-31 range)
- **8-bit offset:** `LDA 100,X` - Load from X+100 (-128 to +127)
- **16-bit offset:** `LDA $1000,X` - Load from X+$1000
- **Accumulator offset:** `LDA A,X` - Load from X+A
- **Auto-increment:** `LDA ,X+` - Load from X, then X+=1
- **Auto-decrement:** `LDA ,-X` - X-=1, then load from X
- **Indirect:** `LDA [,X]` - Load from address pointed to by X
- **PC-relative:** `LDA LABEL,PCR` - Load relative to PC

### 6. Relative Addressing
Used for branch instructions. Offset is added to PC.

**Short branches:** 8-bit signed offset (-128 to +127)
**Long branches:** 16-bit signed offset

## Instruction Categories

### Data Movement
- `LDA`, `LDB`, `LDD` - Load accumulator(s)
- `LDX`, `LDY`, `LDU`, `LDS` - Load index/stack registers
- `STA`, `STB`, `STD` - Store accumulator(s)
- `STX`, `STY`, `STU`, `STS` - Store index/stack registers
- `TFR` - Transfer register to register
- `EXG` - Exchange registers
- `PSH`, `PUL` - Push/Pull registers to/from stack

### Arithmetic
- `ADD`, `ADC` - Add with/without carry
- `SUB`, `SBC` - Subtract with/without borrow
- `MUL` - Multiply (8x8=16 bit)
- `DAA` - Decimal adjust
- `INC`, `DEC` - Increment/Decrement
- `NEG` - Negate (two's complement)

### Logical
- `AND`, `OR`, `EOR` - Logical operations
- `COM` - Complement (one's complement)
- `TST` - Test (set flags without storing)
- `CLR` - Clear (set to zero)

### Shifts and Rotates
- `ASL`, `ASR` - Arithmetic shift left/right
- `LSL`, `LSR` - Logical shift left/right
- `ROL`, `ROR` - Rotate left/right through carry

### Branches
**Short branches (8-bit offset):**
- `BRA` - Branch always
- `BEQ`, `BNE` - Branch if equal/not equal
- `BCC`, `BCS` - Branch if carry clear/set
- `BPL`, `BMI` - Branch if plus/minus
- `BVC`, `BVS` - Branch if overflow clear/set
- `BGT`, `BGE`, `BLT`, `BLE` - Signed comparisons
- `BHI`, `BHS`, `BLO`, `BLS` - Unsigned comparisons

**Long branches (16-bit offset):**
- `LBRA`, `LBEQ`, `LBNE`, etc. - Long versions of short branches

### Jumps and Subroutines
- `JMP` - Jump to address
- `JSR` - Jump to subroutine
- `BSR` - Branch to subroutine (relative)
- `RTS` - Return from subroutine
- `RTI` - Return from interrupt

### Stack Operations
- `PSHS`, `PSHU` - Push to S or U stack
- `PULS`, `PULU` - Pull from S or U stack
- `LEAS`, `LEAU` - Load effective address into S or U

### Condition Code
- `ANDCC`, `ORCC` - Modify condition code register
- `CWAI` - Clear wait for interrupt
- `SYNC` - Synchronize with interrupt

### Special
- `NOP` - No operation
- `SWI`, `SWI2`, `SWI3` - Software interrupts
- `SEX` - Sign extend B into A

## Register Set

**Accumulators:**
- **A, B** - 8-bit accumulators (can be combined as D)
- **D** - 16-bit accumulator (A:B concatenated)

**Index Registers:**
- **X** - 16-bit index register
- **Y** - 16-bit index register

**Stack Pointers:**
- **U** - User stack pointer
- **S** - System stack pointer

**Program Counter:**
- **PC** - 16-bit program counter

**Special Registers:**
- **DP** - Direct Page register (8-bit)
- **CC** - Condition Code register (8-bit)

**Condition Code Flags:**
- **E** - Entire state saved (bit 7)
- **F** - Fast interrupt mask (bit 6)
- **H** - Half carry (bit 5)
- **I** - Interrupt mask (bit 4)
- **N** - Negative (bit 3)
- **Z** - Zero (bit 2)
- **V** - Overflow (bit 1)
- **C** - Carry (bit 0)

## Notes

1. **16-bit operations:** The 6809 has native 16-bit operations (unlike 6502)
2. **Two stacks:** Separate user (U) and system (S) stacks
3. **TFR/EXG:** Can transfer between any compatible register pairs
4. **MUL instruction:** Native 8x8→16 bit multiply
5. **Powerful indexing:** Most flexible indexed addressing of its era

## References

- [MC6809-MC6809E Microprocessor Programming Manual](https://www.maddes.net/m6809pm/sections.htm)
- [6809 Operation Guide](https://silicon-heaven.com/howel/parts/6809_operation.htm)
- [Motorola 6809 Programming Reference (Darren Atkinson)](https://colorcomputerarchive.com/repo/Documents/Books/Motorola%206809%20and%20Hitachi%206309%20Programming%20Reference%20(Darren%20Atkinson).pdf)
- 6809 Assembly Language Programming (Lance Leventhal) - Print Reference
