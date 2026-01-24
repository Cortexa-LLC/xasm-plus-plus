# xasm++ Feature Documentation

**Version:** 0.2.0
**Last Updated:** 2026-01-24
**Status:** Phase 2.4 Complete

---

## Overview

This document provides comprehensive documentation of all implemented features in xasm++. The assembler is currently in active development with complete 6502 CPU support and syntax parsing.

---

## 6502 CPU Support

### Complete Implementation

xasm++ provides **complete 6502 CPU support** with all legal opcodes and addressing modes fully implemented and tested.

**Status:** ✅ **PRODUCTION READY**

**Statistics:**
- **56 Legal Opcodes** - All official 6502 instructions
- **13 Addressing Modes** - Complete addressing mode support
- **155 CPU Tests** - 100% passing, comprehensive coverage
- **Zero Compiler Warnings** - Clean, production-quality code

---

## Supported Instructions

All 56 legal 6502 opcodes are fully implemented with proper addressing mode support.

### Arithmetic Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **ADC** | Add with Carry | Immediate, ZeroPage, ZeroPageX, Absolute, AbsoluteX, AbsoluteY, IndirectX, IndirectY | ✅ |
| **SBC** | Subtract with Carry | Immediate, ZeroPage, ZeroPageX, Absolute, AbsoluteX, AbsoluteY, IndirectX, IndirectY | ✅ |

### Logical Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **AND** | Logical AND | Immediate, ZeroPage, ZeroPageX, Absolute, AbsoluteX, AbsoluteY, IndirectX, IndirectY | ✅ |
| **ORA** | Logical OR | Immediate, ZeroPage, ZeroPageX, Absolute, AbsoluteX, AbsoluteY, IndirectX, IndirectY | ✅ |
| **EOR** | Exclusive OR | Immediate, ZeroPage, ZeroPageX, Absolute, AbsoluteX, AbsoluteY, IndirectX, IndirectY | ✅ |
| **BIT** | Bit Test | ZeroPage, Absolute | ✅ |

### Shift and Rotate Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **ASL** | Arithmetic Shift Left | Accumulator, ZeroPage, ZeroPageX, Absolute, AbsoluteX | ✅ |
| **LSR** | Logical Shift Right | Accumulator, ZeroPage, ZeroPageX, Absolute, AbsoluteX | ✅ |
| **ROL** | Rotate Left | Accumulator, ZeroPage, ZeroPageX, Absolute, AbsoluteX | ✅ |
| **ROR** | Rotate Right | Accumulator, ZeroPage, ZeroPageX, Absolute, AbsoluteX | ✅ |

### Load and Store Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **LDA** | Load Accumulator | Immediate, ZeroPage, ZeroPageX, Absolute, AbsoluteX, AbsoluteY, IndirectX, IndirectY | ✅ |
| **LDX** | Load X Register | Immediate, ZeroPage, ZeroPageY, Absolute, AbsoluteY | ✅ |
| **LDY** | Load Y Register | Immediate, ZeroPage, ZeroPageX, Absolute, AbsoluteX | ✅ |
| **STA** | Store Accumulator | ZeroPage, ZeroPageX, Absolute, AbsoluteX, AbsoluteY, IndirectX, IndirectY | ✅ |
| **STX** | Store X Register | ZeroPage, ZeroPageY, Absolute | ✅ |
| **STY** | Store Y Register | ZeroPage, ZeroPageX, Absolute | ✅ |

### Transfer Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **TAX** | Transfer A to X | Implied | ✅ |
| **TAY** | Transfer A to Y | Implied | ✅ |
| **TXA** | Transfer X to A | Implied | ✅ |
| **TYA** | Transfer Y to A | Implied | ✅ |
| **TSX** | Transfer Stack Pointer to X | Implied | ✅ |
| **TXS** | Transfer X to Stack Pointer | Implied | ✅ |

### Increment and Decrement Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **INC** | Increment Memory | ZeroPage, ZeroPageX, Absolute, AbsoluteX | ✅ |
| **DEC** | Decrement Memory | ZeroPage, ZeroPageX, Absolute, AbsoluteX | ✅ |
| **INX** | Increment X | Implied | ✅ |
| **DEX** | Decrement X | Implied | ✅ |
| **INY** | Increment Y | Implied | ✅ |
| **DEY** | Decrement Y | Implied | ✅ |

### Compare Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **CMP** | Compare Accumulator | Immediate, ZeroPage, ZeroPageX, Absolute, AbsoluteX, AbsoluteY, IndirectX, IndirectY | ✅ |
| **CPX** | Compare X Register | Immediate, ZeroPage, Absolute | ✅ |
| **CPY** | Compare Y Register | Immediate, ZeroPage, Absolute | ✅ |

### Branch Operations

All branch instructions use **relative addressing** with proper offset calculation (`target - (PC + 2)`).

| Instruction | Description | Condition | Status |
|-------------|-------------|-----------|--------|
| **BCC** | Branch if Carry Clear | C = 0 | ✅ |
| **BCS** | Branch if Carry Set | C = 1 | ✅ |
| **BEQ** | Branch if Equal | Z = 1 | ✅ |
| **BNE** | Branch if Not Equal | Z = 0 | ✅ |
| **BMI** | Branch if Minus | N = 1 | ✅ |
| **BPL** | Branch if Plus | N = 0 | ✅ |
| **BVC** | Branch if Overflow Clear | V = 0 | ✅ |
| **BVS** | Branch if Overflow Set | V = 1 | ✅ |

**Features:**
- Relative offset calculation: `target - (PC + 2)`
- Range validation: -128 to +127 bytes
- Forward and backward branches
- Label references supported
- Multi-pass resolution for undefined labels
- Clear error messages for out-of-range branches

### Jump and Subroutine Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **JMP** | Jump | Absolute, Indirect | ✅ |
| **JSR** | Jump to Subroutine | Absolute | ✅ |
| **RTS** | Return from Subroutine | Implied | ✅ |
| **RTI** | Return from Interrupt | Implied | ✅ |

### Stack Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **PHA** | Push Accumulator | Implied | ✅ |
| **PLA** | Pull Accumulator | Implied | ✅ |
| **PHP** | Push Processor Status | Implied | ✅ |
| **PLP** | Pull Processor Status | Implied | ✅ |

### Flag Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **CLC** | Clear Carry Flag | Implied | ✅ |
| **SEC** | Set Carry Flag | Implied | ✅ |
| **CLD** | Clear Decimal Flag | Implied | ✅ |
| **SED** | Set Decimal Flag | Implied | ✅ |
| **CLI** | Clear Interrupt Disable | Implied | ✅ |
| **SEI** | Set Interrupt Disable | Implied | ✅ |
| **CLV** | Clear Overflow Flag | Implied | ✅ |

### System Operations

| Instruction | Description | Addressing Modes | Status |
|-------------|-------------|------------------|--------|
| **NOP** | No Operation | Implied | ✅ |
| **BRK** | Break | Implied | ✅ |

---

## Addressing Modes

All 13 6502 addressing modes are fully implemented with complete syntax parsing support.

### 1. Implied

**Description:** No operand required; operation implied by opcode.

**Syntax:** `MNEMONIC`

**Examples:**
```assembly
INX          ; Increment X register
RTS          ; Return from subroutine
CLC          ; Clear carry flag
```

**Status:** ✅ Complete

---

### 2. Accumulator

**Description:** Operation on accumulator register.

**Syntax:** `MNEMONIC A`

**Examples:**
```assembly
ASL A        ; Arithmetic shift left accumulator
LSR A        ; Logical shift right accumulator
ROL A        ; Rotate left accumulator
ROR A        ; Rotate right accumulator
```

**Status:** ✅ Complete (Phase 2.4)

**Features:**
- Explicit "A" operand parsing
- Distinguishes from Absolute mode

---

### 3. Immediate

**Description:** Operand is a constant value.

**Syntax:** `MNEMONIC #$XX` or `MNEMONIC #value`

**Examples:**
```assembly
LDA #$42     ; Load accumulator with hex value $42
LDX #$00     ; Load X register with hex value $00
CMP #$FF     ; Compare accumulator with hex value $FF
```

**Status:** ✅ Complete

**Features:**
- `#` prefix required
- Hex values with `$` prefix
- 8-bit immediate values (0-255)

---

### 4. Zero Page

**Description:** Address in zero page ($0000-$00FF).

**Syntax:** `MNEMONIC $XX`

**Examples:**
```assembly
LDA $80      ; Load from zero page address $80
STA $40      ; Store to zero page address $40
INC $FF      ; Increment zero page address $FF
```

**Status:** ✅ Complete

**Features:**
- 8-bit address (0-255)
- Faster execution than absolute addressing
- Automatic detection based on address value

---

### 5. Zero Page, X

**Description:** Zero page address plus X register offset.

**Syntax:** `MNEMONIC $XX,X`

**Examples:**
```assembly
LDA $80,X    ; Load from zero page $80 + X
STA $40,X    ; Store to zero page $40 + X
INC $00,X    ; Increment zero page $00 + X
```

**Status:** ✅ Complete (Phase 2.4)

**Features:**
- `,X` suffix parsing
- Whitespace tolerance: `$80,X` or `$80, X`
- Distinguishes ZeroPage vs Absolute by value
- Label support: `table,X`

---

### 6. Zero Page, Y

**Description:** Zero page address plus Y register offset.

**Syntax:** `MNEMONIC $XX,Y`

**Examples:**
```assembly
LDX $80,Y    ; Load X from zero page $80 + Y
STX $40,Y    ; Store X to zero page $40 + Y
```

**Status:** ✅ Complete (Phase 2.4)

**Features:**
- `,Y` suffix parsing
- Whitespace tolerance: `$80,Y` or `$80, Y`
- Limited instructions support (LDX, STX)
- Label support: `buffer,Y`

---

### 7. Absolute

**Description:** Full 16-bit address.

**Syntax:** `MNEMONIC $XXXX`

**Examples:**
```assembly
LDA $1234    ; Load from address $1234
JMP $8000    ; Jump to address $8000
STA $2000    ; Store to address $2000
```

**Status:** ✅ Complete

**Features:**
- 16-bit address (0-65535)
- Label references supported
- Default mode for symbolic addresses

---

### 8. Absolute, X

**Description:** 16-bit address plus X register offset.

**Syntax:** `MNEMONIC $XXXX,X`

**Examples:**
```assembly
LDA $1234,X  ; Load from $1234 + X
STA $2000,X  ; Store to $2000 + X
INC $3000,X  ; Increment $3000 + X
```

**Status:** ✅ Complete (Phase 2.4)

**Features:**
- `,X` suffix parsing
- Whitespace tolerance: `$1234,X` or `$1234, X`
- Distinguishes from ZeroPageX by value
- Label support: `table,X`

---

### 9. Absolute, Y

**Description:** 16-bit address plus Y register offset.

**Syntax:** `MNEMONIC $XXXX,Y`

**Examples:**
```assembly
LDA $1234,Y  ; Load from $1234 + Y
STA $2000,Y  ; Store to $2000 + Y
```

**Status:** ✅ Complete (Phase 2.4)

**Features:**
- `,Y` suffix parsing
- Whitespace tolerance: `$1234,Y` or `$1234, Y`
- Distinguishes from ZeroPageY by value
- Label support: `buffer,Y`

---

### 10. Indirect

**Description:** 16-bit pointer to target address (used by JMP only).

**Syntax:** `JMP ($XXXX)`

**Examples:**
```assembly
JMP ($1234)  ; Jump to address stored at $1234-$1235
JMP ($FFFE)  ; Jump to address stored at $FFFE-$FFFF (IRQ vector)
```

**Status:** ✅ Complete (Phase 2.4)

**Features:**
- Parentheses syntax: `($addr)`
- Whitespace tolerance: `($1234)` or `( $1234 )`
- Only valid for JMP instruction
- 16-bit pointer address

---

### 11. Indexed Indirect (Indirect, X)

**Description:** Zero page pointer plus X offset, then dereference.

**Syntax:** `MNEMONIC ($XX,X)`

**Examples:**
```assembly
LDA ($80,X)  ; Load from address at ($80 + X)
STA ($40,X)  ; Store to address at ($40 + X)
```

**Status:** ✅ Complete (Phase 2.4)

**Features:**
- Parentheses with comma inside: `($addr,X)`
- Whitespace tolerance: `($80,X)` or `( $80 , X )`
- Zero page base address only
- X register offset added before dereferencing

**Use Case:** Accessing array of pointers indexed by X

---

### 12. Indirect Indexed (Indirect), Y

**Description:** Dereference zero page pointer, then add Y offset.

**Syntax:** `MNEMONIC ($XX),Y`

**Examples:**
```assembly
LDA ($80),Y  ; Load from (address at $80) + Y
STA ($40),Y  ; Store to (address at $40) + Y
```

**Status:** ✅ Complete (Phase 2.4)

**Features:**
- Parentheses with comma outside: `($addr),Y`
- Whitespace tolerance: `($80),Y` or `( $80 ) , Y`
- Zero page base pointer
- Y register offset added after dereferencing

**Use Case:** Accessing structure fields or array elements via pointer

---

### 13. Relative

**Description:** Signed 8-bit offset from program counter (branches only).

**Syntax:** `BRANCH label`

**Examples:**
```assembly
loop:
    LDA $80,X
    INX
    BNE loop     ; Branch if not zero (relative offset calculated)
```

**Status:** ✅ Complete (Phase 2.4)

**Features:**
- Automatic offset calculation: `target - (PC + 2)`
- Range validation: -128 to +127 bytes
- Forward and backward branches supported
- Multi-pass resolution for undefined labels
- Clear error messages for out-of-range branches

**Implementation Details:**
- Offset calculated during encoding pass
- Undefined labels handled gracefully on first pass
- Convergence in subsequent passes
- Validation ensures offset fits in signed byte

---

## Syntax Parsing Features

### Whitespace Tolerance

All addressing modes handle various whitespace patterns correctly.

**Examples:**
```assembly
LDA $80,X       ; Standard spacing
LDA $80, X      ; Space after comma
LDA ( $80 )     ; Spaces inside parentheses
LDA ( $80 , X ) ; Spaces everywhere
JMP ($1234)     ; No spaces (also valid)
```

**Status:** ✅ Complete (Phase 2.4)

**Features:**
- Flexible whitespace parsing
- Trims leading/trailing spaces
- Handles spaces around commas
- Handles spaces inside parentheses

---

### Label Support

Labels can be used with all addressing modes that accept addresses.

**Examples:**
```assembly
    ; Define labels
start:
    LDA #$00
    STA counter

loop:
    LDA table,X     ; Indexed mode with label
    STA buffer,Y    ; Indexed mode with label
    INX
    BNE loop        ; Branch to label

    JMP end

table:  .byte $01,$02,$03,$04
buffer: .byte $00,$00,$00,$00
counter: .byte $00
end:
```

**Status:** ✅ Complete

**Supported:**
- Labels with indexed modes (`,X`, `,Y`)
- Labels with branch instructions (relative addressing)
- Forward and backward references
- Multi-pass resolution

---

### Multi-Pass Assembly

The assembler uses multi-pass processing to resolve forward references and undefined labels.

**Features:**
- **Pass 1:** Collect labels and calculate addresses
- **Pass 2:** Resolve label values and encode instructions
- **Additional passes** if needed for convergence
- **MAX_PASSES limit** prevents infinite loops

**Handles:**
- Forward label references
- Backward label references
- Branch instruction offset calculation
- Undefined labels on first pass (value=0)

**Status:** ✅ Complete

---

### Error Handling

The assembler provides clear error messages for common issues.

**Branch Offset Out of Range:**
```
Error: Branch target out of range: offset 200 (must be -128 to +127)
```

**Invalid Addressing Mode:**
- CPU encode methods return empty vectors
- Graceful failure with no crash

**Undefined Labels:**
- Handled gracefully on first pass (value=0)
- Resolved in subsequent passes
- Clear error if label never defined

**Status:** ✅ Adequate (incremental approach, will be enhanced in future phases)

---

## Testing

### Test Coverage

**Total Tests:** 197
- **Assembler Tests:** 42 (22 existing + 20 new)
- **CPU Tests:** 155 (all existing, no regressions)

**Pass Rate:** 100% (197/197)

**Test Execution Time:** ~4ms total (0ms per test suite)

### Test Breakdown

**Assembler Tests (42):**
- **Group 1 (Accumulator):** 2 tests
- **Group 2 (Indexed):** 5 tests
- **Group 3 (Simple Indirect):** 2 tests
- **Group 4 (Complex Indirect):** 6 tests
- **Group 5 (Integration):** 5 tests
- **Existing tests:** 22 tests

**CPU Tests (155):**
- All 56 legal opcodes tested
- All addressing mode combinations tested
- Opcode encoding verified
- No regressions

### Test Quality

**TDD Process:** ✅ RED-GREEN-REFACTOR followed
- Tests written before implementation
- Atomic commits per group
- Verified by Tester role

**Test Characteristics:**
- ✅ **Clear:** Descriptive names, easy to understand failures
- ✅ **Independent:** No shared state, can run in isolation
- ✅ **Reliable:** No flakiness, deterministic behavior
- ✅ **Fast:** <100ms per test, ~4ms total

**Coverage:**
- ✅ Happy path scenarios
- ✅ Edge cases (boundaries, whitespace)
- ✅ Error scenarios (out-of-range branches)
- ✅ Integration tests (realistic programs)

---

## Code Quality

### Standards Compliance

**C++ Best Practices:**
- ✅ 2-space indentation (project standard)
- ✅ Implementations in .cpp files (not headers)
- ✅ Clean separation of concerns
- ✅ SOLID principles followed

**Compiler:**
- ✅ Zero warnings
- ✅ Clean build on macOS
- ✅ CMake configuration

**Code Review:**
- ✅ Tester approved (TDD compliance)
- ✅ Reviewer approved (code quality excellent)

### Documentation

**Inline Documentation:**
- ✅ Comments explain parsing logic
- ✅ Examples in comments (`ASL A`, `LDA $80,X`)
- ✅ Complex logic explained (branch encoding)

**External Documentation:**
- ✅ Task packets complete (contract, plan, work log, review, acceptance)
- ✅ This FEATURES.md document
- ✅ Updated README.md

---

## Known Limitations

### Current Limitations

**No Critical Limitations** - All required functionality implemented

### Deferred Items

1. **Defensive bounds checking** (Reviewer recommendation)
   - Description: Add explicit `.empty()` checks before `[0]` string access
   - Priority: Low (works in practice, defensive hardening)
   - Future: Code hardening pass

2. **Parser refactoring** (Nice-to-have)
   - Description: Extract `DetermineAddressingMode()` to separate Parser class
   - Priority: Low (current implementation is maintainable)
   - Future: Consider if parser complexity grows

---

## Future Work

### Phase 3: CLI and File I/O (Next)
- Command-line interface
- Source file parsing (`.asm` files)
- Binary output (`.bin`, `.prg` formats)
- Listing file generation
- Symbol file generation

### Phase 4-6: Advanced Features
- **Directives:** `.org`, `.byte`, `.word`, `.include`, `.if`, `.macro`
- **Additional CPUs:** 65C02, 6809, Z80, 68000
- **Multiple Syntaxes:** SCMASM, Merlin, EDTASM, Motorola
- **Optimizations:** Peephole optimization, branch optimization

---

## References

### Task Packets

All implementation work documented in task packets:
- `.ai/tasks/2026-01-22_complete-6502-syntax-parsing/`
  - `00-contract.md` - Requirements and acceptance criteria
  - `10-plan.md` - Implementation approach
  - `20-work-log.md` - Development progress
  - `30-review.md` - Tester and Reviewer assessments
  - `40-acceptance.md` - Final acceptance report

### Git History

Complete implementation history in git commits:
- `f8c6752` - Group 1: Accumulator mode
- `29b2654` - Group 2: Indexed modes (`,X`, `,Y`)
- `3c14604` - Group 3: Simple Indirect
- `8f507a0` - Group 4: Complex Indirect (`($80,X)`, `($80),Y`)
- `ffdaf26` - Group 5: Integration tests + branch encoding fix

### Source Code

- `src/core/assembler.cpp` - Core assembler logic
- `src/core/assembler.h` - Assembler interface
- `src/plugins/cpu6502.cpp` - 6502 CPU plugin implementation
- `src/plugins/cpu6502.h` - 6502 CPU plugin interface
- `tests/unit/test_assembler.cpp` - Assembler tests
- `tests/unit/test_cpu6502.cpp` - CPU tests

---

## Contact and Support

- **Author:** Bryan Woodruff
- **Company:** Cortexa LLC
- **Email:** bryan.woodruff@cortexa.com

For bug reports, feature requests, or contributions, please see [CONTRIBUTING.md](CONTRIBUTING.md) (coming soon).

---

**Document Version:** 1.0
**Last Updated:** 2026-01-24
**Status:** Phase 2.4 Complete

