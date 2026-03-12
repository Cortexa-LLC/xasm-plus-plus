# Branch Relaxation - Quick Reference

## What Changed?
Branch instructions now automatically relax when the target is out of range.

## Before This Fix
```
BEQ far_label    ; Target > 127 bytes away
ERROR: Branch target out of range
```

## After This Fix
```
BEQ far_label    ; Target > 127 bytes away
SUCCESS: Automatically relaxed to 5 bytes
```

## How to Use
No changes needed! Just write your assembly code normally:

```asm
        BEQ target
        ; ... many instructions ...
target: RTS
```

The assembler will automatically:
- Use 2-byte encoding if target is within ±127 bytes
- Use 5-byte relaxed encoding if target is beyond ±127 bytes

## Technical Details

### Short Branch (2 bytes)
```
Opcode: Branch instruction opcode (e.g., F0 for BEQ)
Offset: Signed 8-bit offset (-128 to +127)
Example: F0 0A = BEQ +10
```

### Long Branch (5 bytes) - Relaxed
```
Byte 1: Complementary branch opcode (e.g., D0 for BNE instead of BEQ)
Byte 2: Offset +3 (skip over JMP instruction)
Byte 3: JMP opcode (4C)
Bytes 4-5: Target address (little-endian)
Example: D0 03 4C 00 12 = BNE *+5; JMP $1200
```

## Branch Instruction Support
All 6502 branch instructions are supported:
- `BEQ` - Branch if Equal
- `BNE` - Branch if Not Equal
- `BCC` - Branch if Carry Clear
- `BCS` - Branch if Carry Set
- `BMI` - Branch if Minus
- `BPL` - Branch if Plus
- `BVC` - Branch if Overflow Clear
- `BVS` - Branch if Overflow Set

## Testing
Run branch-specific tests:
```bash
./build/tests/unit/test_assembler --gtest_filter="*Branch*"
```

Run full test suite:
```bash
cmake --build build --target test
```

Verify the fix:
```bash
./.ai/tasks/2026-01-26_branch-relaxation-integration/verify_fix.sh
```

## Files Modified
- `src/core/assembler.cpp` - Branch encoding logic
- `tests/unit/test_assembler.cpp` - Added 3 tests

## Impact
- ✅ Prince of Persia files can now assemble
- ✅ No manual workarounds needed for long branches
- ✅ Existing code continues to work unchanged
- ✅ Zero performance impact

## Questions?
See the full work log at: `.ai/tasks/2026-01-26_branch-relaxation-integration/20-work-log.md`
