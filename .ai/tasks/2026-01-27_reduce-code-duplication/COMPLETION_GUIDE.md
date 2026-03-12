# Completion Guide - Code Duplication Refactoring

## Current Status (2026-01-27)

**Progress:** 7/57 instructions refactored (12% complete)
**Tests:** All 236 tests passing ✓
**Build:** Zero warnings ✓

## What's Been Done

### Infrastructure Created ✓
1. **OpcodeTable struct** (cpu_6502.h)
   - 19 fields for all addressing modes
   - Uses std::optional for modes not supported by instruction

2. **EncodeWithTable() helper** (cpu_6502.cpp)
   - Generic encoding function (182 lines)
   - Eliminates switch statement duplication
   - Handles CPU mode restrictions (6502/65C02/65816)
   - Correctly encodes 1/2/3 byte operands

### Instructions Refactored ✓
1. LDA - Load Accumulator (including 65816 modes)
2. STA - Store Accumulator
3. ADC - Add with Carry
4. SBC - Subtract with Carry
5. AND - Logical AND
6. ORA - Logical OR
7. EOR - Exclusive OR

All these now use opcode tables and call `EncodeWithTable()`.

## What Remains

### Remaining Instructions (50)

**Load/Store Instructions (4):**
- [ ] LDX - Load X Register
- [ ] LDY - Load Y Register
- [ ] STX - Store X Register
- [ ] STY - Store Y Register

**Compare Instructions (3):**
- [ ] CMP - Compare Accumulator
- [ ] CPX - Compare X Register
- [ ] CPY - Compare Y Register

**Branch Instructions (8):**
- [ ] BEQ - Branch if Equal
- [ ] BNE - Branch if Not Equal
- [ ] BCC - Branch if Carry Clear
- [ ] BCS - Branch if Carry Set
- [ ] BMI - Branch if Minus
- [ ] BPL - Branch if Plus
- [ ] BVC - Branch if Overflow Clear
- [ ] BVS - Branch if Overflow Set

**Inc/Dec Memory Instructions (2):**
- [ ] INC - Increment Memory
- [ ] DEC - Decrement Memory

**Shift/Rotate Instructions (4):**
- [ ] ASL - Arithmetic Shift Left
- [ ] LSR - Logical Shift Right
- [ ] ROL - Rotate Left
- [ ] ROR - Rotate Right

**Special Instructions:**
- [ ] BIT - Bit Test
- [ ] JMP - Jump (special case: has indirect mode)
- [ ] JSR - Jump to Subroutine

**Plus 31 More 65C02/65816 Instructions** (not in original 6502):
- STZ, TRB, TSB, BRA (65C02)
- PHX, PLX, PHY, PLY (65C02 stack)
- PHB, PLB, PHK, PHD, PLD (65816 bank/direct)
- TCD, TDC, TCS, TSC (65816 transfers)
- JML, JSL, RTL (65816 long)
- PEA, PEI, PER, MVN, MVP, COP, WDM, XBA, XCE (65816 misc)

## Refactoring Pattern

For each instruction, replace this pattern:

```cpp
// OLD (duplicated 50 times)
std::vector<uint8_t> Cpu6502::EncodeXXX(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::XXX_IMM);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;
        // ... 10-20 more cases ...
    }

    return bytes;
}
```

With this pattern:

```cpp
// NEW (table-driven, no duplication)
std::vector<uint8_t> Cpu6502::EncodeXXX(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable XXX_TABLE = {
        .immediate = Opcodes::XXX_IMM,
        .zero_page = Opcodes::XXX_ZP,
        // ... other modes or std::nullopt ...
    };

    return EncodeWithTable(XXX_TABLE, operand, mode);
}
```

## Step-by-Step Process

For each remaining instruction:

1. **Identify addressing modes** - Check Opcodes namespace (opcodes_6502.h)
2. **Create opcode table** - Map modes to opcodes
3. **Replace switch statement** - Use `EncodeWithTable()`
4. **Build and test** - Verify no regressions
5. **Commit** - One instruction or logical group at a time

Example for LDX:

```cpp
std::vector<uint8_t> Cpu6502::EncodeLDX(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable LDX_TABLE = {
        .immediate = Opcodes::LDX_IMM,
        .zero_page = Opcodes::LDX_ZP,
        .zero_page_x = std::nullopt,          // LDX doesn't support ZP,X
        .zero_page_y = Opcodes::LDX_ZPY,      // But does support ZP,Y!
        .absolute = Opcodes::LDX_ABS,
        .absolute_x = std::nullopt,
        .absolute_y = Opcodes::LDX_ABY,
        // ... rest std::nullopt ...
    };

    return EncodeWithTable(LDX_TABLE, operand, mode);
}
```

## Testing Strategy

After each refactoring:

```bash
# Build
cmake --build build

# Run tests
./build/tests/unit/test_cpu6502

# Should see:
# [  PASSED  ] 236 tests.
```

## Success Criteria

- [ ] All 57 instructions refactored
- [ ] Duplication reduced from 95% to < 10%
- [ ] All 236 tests passing
- [ ] Zero compiler warnings
- [ ] Build succeeds

## Files to Modify

- `src/cpu/m6502/cpu_6502.cpp` - Replace Encode methods (lines vary)
- `include/xasm++/cpu/opcodes_6502.h` - Reference (no changes needed)

## Estimated Completion

- **Time per instruction:** 2-5 minutes
- **50 instructions remaining:** ~2-4 hours
- **Testing/verification:** 30 minutes

## Notes

- Branch instructions (BEQ, BNE, etc.) are simpler - only have Relative mode
- JMP is special - has Absolute and Indirect modes
- Transfer/stack instructions (TAX, PHA, etc.) are trivial - Implied mode only (no operand)
- The helper function `EncodeWithTable()` already handles all complexity

## References

- Task contract: `.ai/tasks/2026-01-27_reduce-code-duplication/00-contract.md`
- Opcode constants: `include/xasm++/cpu/opcodes_6502.h`
- Test suite: `tests/unit/test_cpu6502.cpp`
