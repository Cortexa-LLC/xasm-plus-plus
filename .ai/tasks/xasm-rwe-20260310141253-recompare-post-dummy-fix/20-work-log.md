# Work Log: Post-.DUMMY Fix Binary Comparison (xasm-rwe)

## 2026-03-10 — Agent Run

### Phase 1: Setup and Rebuild

- Verified worktree exists at `/tmp/A2osX-335cd122`
- Confirmed xasm++ is at commit b850ca5 (HEAD)
- Built xasm++ from source: `make -j$(sysctl -n hw.ncpu)` in build/
- Installed new binary to `/usr/local/bin/xasm++` (sha: 82c8e9d...)
- Found existing build dir at `/tmp/A2osX-335cd122-build/`
- Ran `make clean` then full rebuild with `--keep-going` flag

### Phase 2: Critical Discovery — Two Regressions

**Regression 1: SYS/KERNEL build fails**
- Error: `kernel.s.dirent:516: Undefined symbol: K.FCLOSE`  
- Root cause: The star-label fix (e0b4425/f2a45a6) processes `*K.CloseDir .EQ K.FClose`
  as a symbol definition, but `K.FClose` is defined in a later `.INB` file (stdio)
- This is a NEW regression vs b7b739c (kernel built successfully before)

**Regression 2: LIB/LIBBLKDEV build fails**
- Error: `inc/io.d2.i:34: Unexpected character after expression: N`
- Root cause: Same star-label fix processes `*IO.D2.ReadSect .EQ $Cn5C` where `n` is
  a slot-placeholder, not a hex digit
- This is also NEW vs b7b739c

**Regression 3: .DUMMY before .1 forward-ref breaks jmp (.1,x)**
- Symptoms: 89 files now differ at offset 0x0001 (JMP instruction dropped)
- Root cause: b850ca5 DummyOrgAtom insertion before forward-reference label
  causes the `jmp (.1,x)` to be silently dropped during code emission
- Reproduced with minimal test case (see report)
- This is the MAIN regression from b850ca5 itself

### Phase 3: Comparison Results

Full comparison run after rebuild:
- Identical: 2 (same as prior run)
- Different: 113 (was 116, minor improvement in count)
- Not Built: 17 (was 15, +2 regressions)

Note: 89 "different" files now differ at 0x0001 instead of 0x0008
meaning the first difference is now earlier in the file (the JMP header byte)

### Phase 4: Test Case Development

Minimal test cases developed:
1. `.DUMMY .OR <literal>` before forward ref → JMP drops (confirmed regression)
2. `.DUMMY .OR <symbol>` before forward ref → JMP drops (same bug)
3. `.DUMMY .OR <symbol>` AFTER .1 label → JMP works fine (correct)

### Summary

- b850ca5 (.DUMMY/.OR fix) did NOT improve binary comparison results
- It introduced 3 regressions: kernel failure, libblkdev failure, JMP-drop bug
- The prior run (b7b739c) produced better results for most files
- Two recommended fixes detailed in comparison-report.md

### Files Created
- `.ai/tasks/xasm-rwe-20260310141253-recompare-post-dummy-fix/comparison-report.md`
- `.ai/tasks/xasm-rwe-20260310141253-recompare-post-dummy-fix/20-work-log.md`
- `/tmp/comparison_results_v2/results_v2.json`
- `/tmp/run_compare_v2.py`
