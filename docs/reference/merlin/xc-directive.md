# XC Directive Implementation

## Overview

Implemented the `XC` directive to enable/disable 65C02 CPU mode switching in the xasm++ assembler. This directive allows assembly source files to toggle between 6502 and 65C02 instruction sets.

## Purpose

The XC directive is used in Merlin assembler source files (like GRAFIX.S and UNPACK.S) to:
- Enable 65C02-specific instructions (TSB, TRB, PHY, PLY, PHX, PLX, STZ, BRA)
- Toggle back to standard 6502 mode
- Allow mixed-mode assembly in a single source file

## Syntax

```assembly
xc           ; Enable 65C02 mode
xc on        ; Enable 65C02 mode (explicit)
xc off       ; Disable 65C02 mode (back to 6502)
```

The directive is case-insensitive: `XC`, `xc`, `Xc` all work.

## Implementation Details

### Files Modified

1. **src/core/cpu.h**
   - Added `CpuMode` enum (`MODE_6502`, `MODE_65C02`)
   - Added `cpu_mode` member variable
   - Added `SetCpuMode()` method
   - Added `GetCpuMode()` method
   - Made CPU6502 opcodes protected for subclass access

2. **src/core/cpu.cpp**
   - Implemented CPU mode tracking
   - Modified `EncodeInstruction()` to check CPU mode before using 65C02 instructions
   - Returns appropriate error if 65C02 instruction used in 6502 mode

3. **src/core/parser.h**
   - Added `cpu_mode_` member variable (defaults to MODE_6502)
   - Added CPU mode tracking in parser state

4. **src/core/parser.cpp**
   - Implemented `HandleXc()` directive handler
   - Parses operand: empty/`on` enables 65C02, `off` disables
   - Updates parser CPU mode state
   - Calls `cpu->SetCpuMode()` to synchronize CPU state

5. **src/core/assembler.cpp**
   - Updated branch instruction list to include BRA (65C02)
   - Added BRA opcode (0x80) to branch instruction handling

### 65C02 Instructions Supported

When XC mode is enabled, the following instructions become available:

| Mnemonic | Description | Zero Page | Absolute |
|----------|-------------|-----------|----------|
| TSB | Test and Set Bits | 0x04 | 0x0C |
| TRB | Test and Reset Bits | 0x14 | 0x1C |
| PHX | Push X Register | 0xDA | - |
| PLX | Pull X Register | 0xFA | - |
| PHY | Push Y Register | 0x5A | - |
| PLY | Pull Y Register | 0x7A | - |
| STZ | Store Zero | 0x64 | 0x9C |
| BRA | Branch Always | 0x80 | - |

## Testing

### Unit Tests

Created `tests/unit/test_xc_directive.cpp` with 10 comprehensive tests:

1. **XcEnablesCpu65C02Mode** - Verifies XC enables 65C02 mode
2. **XcOnEnablesCpu65C02Mode** - Verifies "XC ON" syntax
3. **XcOffDisablesCpu65C02Mode** - Verifies "XC OFF" disables mode
4. **XcCaseInsensitive** - Tests case insensitivity
5. **XcDoesNotGenerateAtoms** - Ensures no code generation
6. **XcMultipleToggles** - Tests repeated mode switching
7. **Enable65C02InstructionsTSB** - Tests TSB instruction
8. **Enable65C02InstructionsTRB** - Tests TRB instruction
9. **Enable65C02InstructionsPHY** - Tests PHY instruction
10. **GrafixSUsagePattern** - Tests real-world GRAFIX.S pattern

All tests pass: ✅ 11/11 (including MerlinSyntaxTest.XcDirective)

### Integration Test

Created `tests/integration/09_xc_directive.asm` demonstrating:
- Enabling 65C02 mode
- Using 65C02 instructions (TSB, TRB, PHY, PLY, PHX, PLX, STZ)
- Disabling 65C02 mode
- Using standard 6502 instructions
- Re-enabling 65C02 mode
- BRA (branch always) instruction

Binary output verified correct:
```
00000000  04 42 14 43 5a 7a da fa  64 44 a9 00 85 45 a2 ff  |.B.CZz..dD...E..|
00000010  86 46 0c 34 12 1c 35 12  80 03 ea ea ea 64 47     |.F.4..5......dG|
```

## Error Handling

The implementation properly handles:
- **Unknown instruction in 6502 mode**: Returns error if 65C02 instruction used without XC
- **Mode synchronization**: Parser and CPU states stay synchronized
- **Invalid operands**: Proper error for malformed XC directives

Example error:
```
Error: Unknown instruction 'TSB' at line 5
(Use 'xc' directive to enable 65C02 mode)
```

## Real-World Usage

This implementation enables assembly of:
- **GRAFIX.S** - Graphics routines using 65C02 instructions
- **UNPACK.S** - Unpacking routines with XC OFF
- Other Merlin-compatible source files

## Future Enhancements

Potential improvements:
1. Support for 65816 mode (XC with additional flag)
2. MX directive for 65816 M/X flags
3. Warning when XC OFF might cause issues
4. Optimization hints for 65C02-specific code

## References

- Merlin assembler documentation
- WDC 65C02 datasheet
- Apple II system software source code
- tests/integration/09_xc_directive.asm (example usage)

## Status

✅ **Complete** - All tests passing, ready for production use
✅ **Backward compatible** - Default mode is 6502
✅ **Well-tested** - 11 unit tests, integration test
✅ **Documented** - Code comments and this document
