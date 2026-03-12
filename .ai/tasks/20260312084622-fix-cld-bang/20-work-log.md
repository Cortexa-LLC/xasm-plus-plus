# Work Log

**Task ID:** fix-cld-bang
**Started:** 2026-03-12 08:56
**Status:** Completed

---

## Work Sessions

### Session 1: 2026-03-12 08:56

#### Objectives for This Session
```
✓ Understand the issue with cld! instruction suffix
✓ Locate the code where mnemonic lookup happens
✓ Implement fix to strip trailing ! from mnemonics
✓ Add regression test for the fix
✓ Verify all tests pass
```

#### Work Completed
```
✓ Added code to strip trailing '!' from mnemonic in assembler.cpp before table lookup
✓ Added defensive stripping in CPU plugin methods (EncodeInstruction, GetInstructionSize, etc.)
✓ Added regression test Cpu6502Test.CLD_WithBangSuffix
✓ Verified fix with manual test case (cld! assembles to 0xD8)
✓ All 1865 tests pass
```

**Files Modified:**
- `src/core/assembler.cpp` - Strip trailing '!' from mnemonic before CPU encoding (line ~524)
- `src/cpu/cpu_6502.cpp` - Added defensive stripping in multiple methods:
  - `GetInstructionSize()` - Strip '!' before size calculation
  - `EncodeInstruction()` - Strip '!' before encoding and use clean_mnemonic throughout
  - `RequiresSpecialEncoding()` - Strip '!' before checking special encoding
  - `EncodeInstructionSpecial()` - Strip '!' before special encoding
  - `HasOpcode()` - Strip '!' before opcode validation
- `tests/unit/test_cpu6502.cpp` - Added Test 94b: CLD_WithBangSuffix regression test

**Tests Added:**
- `Cpu6502Test.CLD_WithBangSuffix` - Verifies that "CLD!" encodes the same as "CLD" (opcode 0xD8)

**Manual Verification:**
```
$ cat test_cld_bang.s
start:
    cld!
    nop

$ ./build-test/bin/xasm++ --cpu 6502 test_cld_bang.s -o test_cld_bang.bin
Assembly successful: test_cld_bang.bin

$ hexdump -C test_cld_bang.bin
00000000  d8 ea                                             |..|
           ↑  ↑
          CLD NOP
```

#### Implementation Details

**Problem:**
The '!' suffix in mnemonics like `cld!` was not being stripped before the mnemonic lookup in the opcode table, causing assembly failures for pppssc.drv.

**Solution:**
Added mnemonic suffix stripping at multiple layers:

1. **Primary fix (assembler.cpp):** Strip trailing '!' right after extracting mnemonic from instruction but before passing to CPU plugin
2. **Defensive fixes (cpu_6502.cpp):** Also strip '!' in CPU methods to handle direct calls

**Why both layers?**
- Assembler layer: Handles normal assembly flow
- CPU layer: Handles unit tests that call CPU methods directly and provides defense-in-depth

**Pattern used:**
```cpp
// Strip trailing '!' from mnemonic before processing
std::string clean_mnemonic = mnemonic;
if (!clean_mnemonic.empty() && clean_mnemonic.back() == '!') {
  clean_mnemonic.pop_back();
}
```

Then replaced all references to `mnemonic` parameter with `clean_mnemonic` in function body.

#### Test Results
```
All tests passed: 1865/1865 (100%)
Build warnings: 0
Manual test: ✓ (cld! → 0xD8)
Regression test: ✓ (CLD_WithBangSuffix passes)
```

---

## Blockers
None

---

## Next Steps
- Task complete
- Ready for code review
