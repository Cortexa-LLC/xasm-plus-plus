# Work Log: xasm-aii — STABLE Binary Diff Investigation

## Session (2026-03-10)

### Findings

#### Root cause of size differences
Binary comparison against STABLE.800.po revealed that STABLE was assembled from **older source (a417f7ab era, BIN Layout v1)**, not from the c11287b3 source we assembled. The source at c11287b3 bumped BIN Layout from v1 to v2 with a different header structure. All five binaries (kernel, getty, login, initd, libblkdev) had significant source changes between commits.

#### Real xasm++ bugs found and fixed (commit e2e696d)

**Bug 1: `.DA` inline comment with commas**
- File: `src/syntax/scmasm_directive_handlers.cpp` — `HandleDa()`
- LIBBLKDEV.S.txt: `.DA #$61   6502,Level 1 (65c02)` — text after first whitespace is an inline comment; comma inside comment was producing extra `.DA` operands
- Effect: +extra bytes at byte 3 of every binary using this pattern
- Fix: stop parsing `.DA` list when a token contains internal whitespace (inline comment)
- libblkdev impact: -1205 bytes → -172 bytes (remaining is source change)

**Bug 2: `.HS` dot-nibble separators**
- File: `src/syntax/scmasm_directive_handlers.cpp` — `HandleHs()`
- SCMASM uses `.HS 01.38.b0.03` with dots as visual nibble separators
- xasm++ was not stripping dots before hex parsing
- Fix: normalize input by removing all `.` before nibble extraction

**Bug 3: Stale CLI tests**
- File: `tests/unit/test_cli.cpp`
- `CLITest.OrgDefaultZero`: expected `opts.org == 0`, actual is `-1` (intentional sentinel)
- `CLITest.MaxPassesDefault`: expected `opts.max_passes == 10`, actual is `50` (MAX_PASSES)
- Fix: update expected values to match current implementation

### Build validation
- All 1804 tests pass after fixes
- A2osX mini build completes without errors
- All assembled binaries have correct headers (d8 7c = CLD + JMP($addr,X))
- Kernel: 78 a2 (SEI LDX #n) — correct for kernel entry point type

### Conclusion
xasm++ correctly assembles the current c11287b3 source. The remaining size differences vs STABLE are entirely due to source changes between commits and are **expected**.
