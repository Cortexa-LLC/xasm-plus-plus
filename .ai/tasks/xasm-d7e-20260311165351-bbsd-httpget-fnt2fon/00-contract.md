# Contract: Investigate bbsd(-16B), httpget(-14B), fnt2fon(-13B) deltas

## Task ID
xasm-d7e

## Problem
Three binaries are larger than stable:
- `sbin/bbsd`: 1191 bytes built vs 1175 stable (delta -16B)
- `bin/httpget`: built larger by 14B vs stable
- `bin/fnt2fon`: built larger by 13B vs stable

## Files
Sources in `/tmp/A2osX-335cd122/`:
- `SBIN/BBSD.S.txt`
- `BIN/HTTPGET.S.txt`  
- `BIN/FNT2FON.S.txt` (or similar name — check directory)

Built: `/tmp/A2osX-335cd122-build/stage/`
Stable: `/tmp/stable_extracted/`
Include path: `/tmp/A2osX-335cd122/INC/`

## Background
xasm++ is a modern C++ cross-assembler for SCMASM syntax (Apple II / 6502).
Bug patterns seen in other A2osX files:
1. Branch relaxation: BRA/BCC/etc. expanded to JMP (+3 bytes each)
2. EquateAtom in .DUMMY: `LABEL .EQ *` re-evaluated on wrong PC
3. Macro arg parsing: comma in inline comment treated as arg separator (+1-4 bytes)

## Investigation Method (repeat for each file)
```bash
# Generate listing
/tmp/xasm_new --cpu 65c02 --syntax scmasm \
  /tmp/A2osX-335cd122/SBIN/BBSD.S.txt \
  -o /tmp/bbsd.bin --list > /tmp/bbsd.lst 2>&1

# Find first diff
python3 -c "
s=open('/tmp/stable_extracted/SBIN/BBSD.dump','rb').read()[16:]
b=open('/tmp/A2osX-335cd122-build/stage/sbin/bbsd','rb').read()[16:]
for i,(a,c) in enumerate(zip(s,b)):
    if a!=c:
        print(f'First diff at code offset {i} (vaddr \${0x2000+i:04X}): stable=\${a:02X} built=\${c:02X}')
        break
"
```

## Acceptance Criteria
- [ ] Root cause identified for each of the 3 files
- [ ] If assembler bug: fix implemented, all 1828 tests pass
- [ ] `cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build` exits 0
- [ ] `ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build` exits 0
- [ ] Findings documented in `.ai/docs/a2osx-bug-tracker.md`

## Key Assembler Files
- `src/syntax/scmasm_syntax.cpp`
- `src/syntax/scmasm_directive_handlers.cpp`
- `src/core/assembler.cpp`
- `src/cpu/cpu_6502.cpp`

## Related Projects
Related Projects: /Users/bryanw/Projects/Vintage/Apple/A2osX
