# Investigate: Code size differences (108 files, diff@0x0008+)

## Task ID
xasm-7sz

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/xasm-7sz-20260310180803-investigate-code-size/

## Context

**CORRECT BASELINE**: The comparison must use 335cd122 A2osX source (NOT the xasm++_stable branch).
- **STABLE reference** (from STABLE.800.po image): `/tmp/stable_extracted/`
- **xasm++ output** (built from 335cd122 source): `/tmp/A2osX-335cd122-build/stage/`

A2osX source used for xasm++ builds: `/tmp/A2osX-335cd122/`
INC files: `/tmp/A2osX-335cd122/INC/`

Correct comparison results (335cd122 source):
- 2 identical
- 151 different (108 differ at 0x0008+, i.e., code section differs after matching 8-byte header)
- 60 not built

NO rebuild is needed — use files already on disk.

## Key fact: Why xasm++_stable branch is WRONG comparison baseline
The xasm++_stable branch has `A2osX.GP = $D400`, while 335cd122 has `A2osX.GP = $0140`.
These are constants from INC/A2osX.I.txt used in nearly every binary. Using the wrong branch
produces fundamentally different binaries.

## Task: Deep diff analysis to find root cause

### Step 1: ACC — comparing stable vs 335cd122 xasm++ build
```bash
xxd /tmp/stable_extracted/BIN/ACC.dump | head -40
xxd /tmp/A2osX-335cd122-build/stage/bin/acc | head -40
diff <(xxd /tmp/stable_extracted/BIN/ACC.dump) <(xxd /tmp/A2osX-335cd122-build/stage/bin/acc)
```
Source: `/tmp/A2osX-335cd122/BIN/ACC.S.txt`

Sizes to check: `wc -c /tmp/stable_extracted/BIN/ACC.dump /tmp/A2osX-335cd122-build/stage/bin/acc`

### Step 2: LIB/LIBTUI.O — same size (219 bytes), only 1 byte differs
```bash
diff <(xxd /tmp/stable_extracted/LIB/LIBTUI.O.dump) <(xxd /tmp/A2osX-335cd122-build/stage/lib/libtui.o)
```
Source: `/tmp/A2osX-335cd122/LIB/LIBTUI.O.txt`

### Step 3: Pick a small BIN/ file and find exact divergence
Look at a small binary (e.g., ACOS, ARC) where first_diff is close to 0x0008:
```bash
wc -c /tmp/stable_extracted/BIN/ACOS.dump /tmp/A2osX-335cd122-build/stage/bin/acos
diff <(xxd /tmp/stable_extracted/BIN/ACOS.dump) <(xxd /tmp/A2osX-335cd122-build/stage/bin/acos) | head -30
```
Source: `/tmp/A2osX-335cd122/BIN/ACOS.S.txt`

### Step 4: Hypothesis testing
After analyzing the diffs, for each diff location:
1. What are the differing bytes? Decode as 6502 instructions.
2. What source line produces that code? (use binary header offsets to back-calculate)
3. Is this a:
   - Zero-page vs absolute addressing choice (ZP=2 bytes, ABS=3 bytes)?
   - Branch relaxation difference (BRA/BCC encoding)?
   - Macro expansion difference?
   - Constant/symbol value difference?
   - Different instruction encoding?
4. If ZP vs ABS: does the symbol actually fit in ZP ($00-$FF)? What does xasm++ emit vs SCMASM?

## A2osX source location
335cd122 source: `/tmp/A2osX-335cd122/`
INC files: `/tmp/A2osX-335cd122/INC/`
xasm++ binary: `/usr/local/bin/xasm++` (or build at `/Users/bryanw/Projects/Vintage/tools/xasm++/build/xasm++`)

## Deliverable
Write investigation-report.md to this task directory with:
1. For each analyzed file: exact diff bytes, decoded instruction, source line, root cause
2. Pattern summary: are all diffs the same root cause?
3. Specific xasm++ source file/function to fix (if a bug is found)
