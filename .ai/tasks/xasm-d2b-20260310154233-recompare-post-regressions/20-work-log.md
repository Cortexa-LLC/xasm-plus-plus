# Work Log: Post-Regression-Fix Binary Comparison (xasm-d2b)

## 2026-03-10 — Agent Run

### Phase 1: Setup and Build

- Confirmed worktree at `/tmp/A2osX-335cd122` (commit 335cd122)
- Confirmed build dir at `/tmp/A2osX-335cd122-build/` (cmake configured for xasm++)
- xasm++ at HEAD commit d05fd40 (built, installed to /usr/local/bin/xasm++)
- Ran `make clean` then full rebuild with `--keep-going -j1`
- Build log saved to `/tmp/a2osx-build-d05fd40-full.log`

### Phase 2: Build Results

**Built successfully:** 131 targets  
**Failed to build:** 9 targets (make errors reported)

Failed targets:
- `bin/asm.65816` — MVN/MVP 65816 instructions
- `bin/asm.65R02` — BBR/BBS Rockwell instructions  
- `bin/fnt2fon` — Unterminated string
- `bin/nfsmount` — Not on stable disk (excluded from scope)
- `bin/tuitest` — Not on stable disk (excluded from scope)
- `bin/xmastree` — Unexpected character after expression
- `sbin/nfsd` — Not on stable disk (excluded from scope)
- `sys/pm.ramworks` — Unsupported instruction: TOO
- `sys/pm/pm.nsc` — Parse error: Unterminated string

### Phase 3: Regression Verification

Confirmed regressions from b850ca5 are fixed:
- **KERNEL:** Now assembles successfully (regression 1 fixed)
- **LIBBLKDEV:** Now assembles successfully (regression 1 fixed)
- **LIBBLKDEV.O:** Byte-for-byte identical to reference (regression 1 fixed)

### Phase 4: Binary Comparison

Used Python script at `/tmp/compare_d05fd40.py`  
Scope: 126 cmake targets that exist on the stable disk reference

**Results:**
- 2 identical (LIBBLKDEV.O, LIBGUI.O)
- 117 different (107 at 0x0008, 5 at 0x0000, 2 at 0x0001, 1 at 0x0010, 1 at 0x002f, 1 at 0x0089)
- 7 not built (all pre-existing, no new failures)
- 17 excluded from scope (not on stable disk)

### Phase 5: Key Finding

The DummyOrgAtom JMP regression is confirmed fixed:
- **b850ca5:** 89 files differing at 0x0001 (JMP instruction dropped)
- **d05fd40:** 2 files differing at 0x0001 (pre-existing, unrelated to DummyOrgAtom fix)
- The 107 files at 0x0008 matches xasm-9wz baseline exactly ✅

### Artifacts Created

- `.ai/tasks/xasm-d2b-20260310154233-recompare-post-regressions/comparison-report.md`
- `.ai/tasks/xasm-d2b-20260310154233-recompare-post-regressions/20-work-log.md` (this file)
- `/tmp/a2osx-build-d05fd40-full.log` — full build log
- `/tmp/compare_d05fd40.py` — comparison script
