# Comparison Run 4: Post pending_label_ dummy fix + HasOpcode fix

## Task ID
See task_id above.

## Context

This is the 4th binary comparison run between A2osX files assembled by xasm++ (current main)
vs the reference STABLE.800.po extracted files.

## Previous Comparison (Run 3 = d05fd40)
- 2 identical, 117 different (107 at 0x0008 offset = ZP address), 7 not built

## Fixes Applied Since Run 3
1. `fix: pending_label_ in .DUMMY blocks must not emit LabelAtom` (0d281fd)
   - Root cause of the 107 ZP-address differences
2. `fix: accept unterminated strings in .CS/.CZ/.AS/.AT` (53f1fa5)
   - fnt2fon.s.txt line 811 now assembles (fnt2fon IS in stable reference)
3. `fix: HasOpcode() excludes 65816-only opcodes in 65c02 mode` (7531e43)
   - MVN/MVP/etc now correctly treated as labels in 65c02 mode

## Steps

1. Install updated xasm++: `cmake --install /Users/bryanw/Projects/Vintage/tools/xasm++/build --prefix /usr/local`
2. Build A2osX: `cd /Users/bryanw/Projects/Vintage/Apple/A2osX/build && make -k 2>&1`
   Note: Some files (nfsmount, tuitest, xmastree, nfsd, pm.ramworks) will still fail 
   (macro expansion not yet supported - these were in "not built" category previously)
3. Run comparison between:
   - Reference: /tmp/stable_extracted/ (files extracted from STABLE.800.po)
   - Built: /Users/bryanw/Projects/Vintage/Apple/A2osX/build/stage/
4. For each file in stable reference, compare byte-by-byte to xasm++ built version
5. Report: identical count, different count, not built count
6. For different files: show byte offset of first difference and whether it's at 0x0008 (ZP address) or elsewhere

## Output Format

Write a comparison-report.md to this task directory with:
- Summary: identical/different/not-built counts  
- List of identical files
- List of different files with first-diff-offset and brief analysis
- List of not-built files
