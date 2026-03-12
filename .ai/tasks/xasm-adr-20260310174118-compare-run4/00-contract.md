# Binary Comparison Run 4

## Task ID
xasm-adr

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/xasm-adr-20260310170000-compare-run4/

## What To Do

### 1. Install latest xasm++
```bash
cmake --install /Users/bryanw/Projects/Vintage/tools/xasm++/build --prefix /usr/local
```

### 2. Build A2osX (keep going on errors - some files use unsupported macros)
```bash
cd /Users/bryanw/Projects/Vintage/Apple/A2osX/build && make -k 2>&1
```
Expected failures (pre-existing, NOT in stable reference): nfsmount, tuitest, xmastree, nfsd, pm.ramworks

### 3. Run binary comparison

Reference files: /tmp/stable_extracted/ (extracted from STABLE.800.po)
Built files: /Users/bryanw/Projects/Vintage/Apple/A2osX/build/stage/

The stable_extracted directory has subdirs: BIN/, SBIN/, SYS/, DRV/, LIB/, A2OSX.SYSTEM.dump, etc.
Each file is named like FILENAME.dump and contains the raw binary.

The built files in stage/ are named like bin/filename, sbin/filename, sys/pm, etc.

Write a Python script to:
- Walk /tmp/stable_extracted/ recursively
- For each .dump file, determine the corresponding built path in stage/
  (e.g. BIN/ASM.dump → stage/bin/asm, SBIN/GETTY.dump → stage/sbin/getty)
- Compare byte-by-byte
- Categorize: identical, different (with first-diff offset), not-built

### 4. Write comparison-report.md

Write to: /Users/bryanw/Projects/Vintage/tools/xasm++/.ai/tasks/xasm-adr-TIMESTAMP-compare-run4/comparison-report.md

Include:
- Summary: identical/different/not-built counts vs previous (run 3: 2 identical, 117 different, 7 not built)  
- List of identical files
- List of different files with first-diff offset and byte values
- List of not-built/missing files
- Key findings / root causes of remaining differences

## Previous Fixes Applied
- pending_label_ in .DUMMY blocks now correctly gets ZP address (was root cause of 107 files)
- HasOpcode() now excludes 65816-only opcodes in 65c02 mode  
- Unterminated strings in .CS/.CZ/.AS accepted (SCMASM compatible)
- Plus earlier fixes for .DUMMY/.OR, star-labels, etc.

## Acceptance Criteria
comparison-report.md written with accurate counts and analysis
