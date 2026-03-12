# Task Contract: CPU Plugin EncodeInstruction() Interface

**Beads Task:** xasm++-7q2

## Objective

Complete the CPU plugin architecture refactor by implementing the polymorphic `EncodeInstruction()` interface for both Cpu6502 and Cpu6809, eliminating the need for casting and enabling proper multi-CPU support.

## Background

Current state:
- CpuPlugin base interface exists with `EncodeInstruction()` pure virtual method
- CPU constants defined (`cpu::CPU_6502`, `cpu::CPU_6809`, etc.)
- CLI accepts `--cpu 6809` and `--syntax edtasm`
- Assembler uses `CpuPlugin*` but has 6502-specific `instruction_handlers_`
- Cpu6809 has instruction-specific methods (`EncodeLDA()`, `EncodeSTA()`, etc.)
- Both CPU headers have `EncodeInstruction()` declared but not implemented

Problem:
- Assembler's `EncodeInstructions()` method uses 6502-specific instruction handlers with casting
- 6809 instructions cannot be encoded through the assembler
- `--cpu 6809 --syntax edtasm` produces 6502 opcodes instead of 6809 opcodes

## Requirements

### 1. Implement Cpu6809::EncodeInstruction()

**File:** `src/cpu/m6809/cpu_6809.cpp`

**Functionality:**
- Parse `operand_str` to determine AddressingMode6809
- Map mnemonic to appropriate Encode* method
- Return encoded bytes or throw on unsupported instruction/mode

**Addressing Mode Detection (from operand_str):**
- `#value` → Immediate8 or Immediate16
- `<address` → Direct
- `address` → Extended
- `,X` → IndexedZeroOffset (X register)
- `offset,X` → Indexed5BitOffset / Indexed8BitOffset / Indexed16BitOffset
- `,X+` → IndexedAutoInc1
- `,--X` → IndexedAutoDec2
- etc. (see AddressingMode6809 enum for full list)

**Instruction Dispatch:**
- Create dispatch table mapping mnemonics to Encode* methods
- Handle all 59 fundamental 6809 instructions
- Throw `std::invalid_argument` for unsupported instructions/modes

### 2. Implement Cpu6502::EncodeInstruction()

**File:** `src/cpu/m6502/cpu_6502.cpp`

**Functionality:**
- Move `instruction_handlers_` from Assembler into Cpu6502
- Parse `operand_str` to determine AddressingMode (6502-style)
- Dispatch to appropriate encoding logic
- Return encoded bytes or throw on unsupported instruction/mode

**Addressing Mode Detection (from operand_str):**
- `#value` → Immediate
- `address` → ZeroPage or Absolute (based on value < 256)
- `address,X` → ZeroPageX or AbsoluteX
- `address,Y` → ZeroPageY or AbsoluteY
- `(address,X)` → IndirectX
- `(address),Y` → IndirectY
- `(address)` → Indirect (JMP only)

**Instruction Dispatch:**
- Reuse existing instruction handler logic from Assembler
- Support all 6502/65C02/65816 instructions based on CPU mode
- Handle branch relaxation for 65C02/65816
- Handle MVN/MVP for 65816

### 3. Update Assembler::EncodeInstructions()

**File:** `src/core/assembler.cpp`

**Changes:**
- Replace instruction_handlers_ table usage with `cpu_->EncodeInstruction()` call
- Remove static_cast<Cpu6502*> workarounds
- Keep special handling for 65816 branches (BRA, BEQ, etc.) if needed
- Preserve error handling and reporting

**Before:**
```cpp
Cpu6502* cpu6502 = static_cast<Cpu6502*>(cpu_);
inst->encoded_bytes = it->second(cpu6502, value, mode);
```

**After:**
```cpp
inst->encoded_bytes = cpu_->EncodeInstruction(mnemonic, value, operand_str);
```

### 4. Remove instruction_handlers_ from Assembler

**Files:**
- `include/xasm++/assembler.h` - Remove InstructionHandler typedef and instruction_handlers_ member
- `src/core/assembler.cpp` - Remove InitializeInstructionHandlers() and RegisterXXXInstructions() methods

## Acceptance Criteria

### Functional Requirements

- [ ] `Cpu6809::EncodeInstruction()` implemented and working
- [ ] `Cpu6502::EncodeInstruction()` implemented and working
- [ ] Assembler uses `cpu_->EncodeInstruction()` instead of instruction_handlers_
- [ ] No casting from CpuPlugin* to Cpu6502* or Cpu6809* in assembler
- [ ] `--cpu 6809 --syntax edtasm` produces correct 6809 opcodes

### Test Requirements

- [ ] All existing unit tests pass (6502, 65C02, 65816)
- [ ] All existing 6809 unit tests pass (indexed addressing, long branches)
- [ ] EDTASM parser tests pass with 6809
- [ ] End-to-end test: assemble 6809 program with EDTASM syntax
- [ ] Verify binary output matches expected 6809 opcodes

### Quality Requirements

- [ ] Zero compiler warnings
- [ ] Clean build
- [ ] Proper error messages for unsupported instructions/modes
- [ ] Code follows existing patterns and style

## Test Cases

### 6809 Basic Instructions
```asm
* Test 6809 instruction encoding
        ORG     $1000
        LDA     #42         ; 86 2A
        STA     $2000       ; B7 20 00
        LDX     #$1234      ; 8E 12 34
        STX     ,X          ; AF 84
        RTS                 ; 39
        END
```

**Expected binary:** `86 2A B7 20 00 8E 12 34 AF 84 39`

### 6502 Existing Functionality
```asm
; Test 6502 still works
        ORG $8000
        LDA #$42
        STA $2000
        RTS
```

**Expected binary:** `A9 42 8D 00 20 60`

## Dependencies

**Blocked by:** None (all prerequisites complete)

**Blocks:** None

## Estimated Effort

**Total:** 4-6 hours

Breakdown:
- Cpu6809::EncodeInstruction(): 1-2 hours
- Cpu6502::EncodeInstruction(): 2-3 hours
- Assembler refactor: 30 minutes
- Testing and debugging: 1 hour

## Technical Notes

### Addressing Mode Parsing

Both CPUs need operand parsers. Key differences:

**6502:**
- Little-endian byte order
- Zero page vs absolute determined by value size
- Indirect modes use parentheses: `(addr,X)`, `(addr),Y`

**6809:**
- Big-endian byte order
- Direct page configurable (not fixed to $00)
- Indexed modes use comma: `,X`, `5,X`, `[10,Y]`
- Post-byte encoding for indexed modes (complex)

### Error Handling

Both implementations should throw:
- `std::invalid_argument` - Unsupported instruction or addressing mode
- `std::out_of_range` - Operand value out of range

Assembler will catch these and convert to AssemblerError objects with source locations.

## References

**Architecture:**
- `include/xasm++/cpu/cpu_plugin.h` - Base interface
- `.ai/tasks/2026-02-03_6809-indexed-addressing/30-results.md` - 6809 indexed modes reference
- `.ai/tasks/2026-02-03_6809-edtasm-parser/30-results.md` - EDTASM parser implementation

**Testing:**
- `tests/unit/test_cpu6502.cpp` - 6502 test patterns
- `tests/unit/test_cpu6809.cpp` - 6809 test patterns
- `tests/unit/test_edtasm_syntax.cpp` - EDTASM parser tests
