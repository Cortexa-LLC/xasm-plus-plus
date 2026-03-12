# Work Log - Binary Comparison Run 4

## Session: 2026-03-10

### Task
Run binary comparison after:
- 0d281fd: fix pending_label_ in .DUMMY blocks
- 53f1fa5: fix unterminated strings in SCMASM .CS/.CZ/.AS/.AT
- 7531e43: fix HasOpcode() mode-aware (excludes 65816-only opcodes in 65c02/6502 mode)

### Steps Completed

1. **Installed latest xasm++** (7531e43) via `cmake --install build --prefix /usr/local`
   - Confirmed: `-- Up-to-date: /usr/local/bin/xasm++`

2. **Built A2osX** with `make -k` (keep-going on errors)
   - Result: 136 targets built (vs 126 in run 3)
   - Expected failures: BIN/ADM, BIN/BASIC, etc. (unsupported macros)

3. **Ran binary comparison** against `/tmp/stable_extracted/` (from STABLE.800.po)
   - Script: `/tmp/compare_run4.py`
   - Results saved: `/tmp/compare_run4_results.json`

4. **Analyzed results** and compared against run 3 (d05fd40)

### Results

**Run 4 (7531e43) vs Run 3 (d05fd40):**

| Metric | Run 3 | Run 4 | Change |
|--------|-------|-------|--------|
| Identical | 2 | 3 | +1 ✅ |
| Different | 113 | 119 | +6 |
| Not built | 17 | 10 | -7 ✅ |
| Regressions | - | 0 | none ✅ |

**Key improvements:**
- A2OSX.LOGO: now identical (was not_built in run 3)
- SYS/KERNEL, LIB/LIBBLKDEV, LIB/LIBTUI.O: now build (pending_label_ fix)
- BIN/FNT2FON: now builds (unterminated string fix)
- BIN/ASM.65816, BIN/ASM.65R02: now build (HasOpcode fix)

### Key Finding: BIN Layout Version

The pending_label_ fix in .DUMMY blocks changed the binary layout significantly.
In run 3, the bug caused `.DA #2` (BIN Layout Version 2) to incorrectly produce
byte 0x01 at offset 5, which happened to match the stable binary (which was built
from an older version of the source using `.DA #1`). After the fix, `.DA #2` correctly
produces 0x02, which matches the source code but not the stable.

This explains why 39 files moved from first_diff=0x0001 to 0x0005 - they now correctly
implement BIN Layout Version 2 per the current source code.

### Known Remaining Issues

1. **S.PS.F.EVENT undefined** (20 files): Symbol missing from INC/a2osx.i - pre-existing issue
2. **Code size differences** (45 files): Minor size differences in assembled code vs stable
3. **10 files not built**: Unsupported macros (>PSTR, PR#3) and missing source

### Status
✅ Complete
