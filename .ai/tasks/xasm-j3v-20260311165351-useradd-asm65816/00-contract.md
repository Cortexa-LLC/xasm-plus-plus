# Contract: Investigate useradd(-48B) and asm.65816(-9B) deltas

## Task ID
xasm-j3v

## Problem
- `bin/useradd`: built 48 bytes larger than stable (1582 vs 1534)
- `bin/asm.65816`: built 9 bytes larger than stable

## Files
Sources in `/tmp/A2osX-335cd122/`:
- `BIN/USERADD.S.txt`
- `BIN/ASM.65816.S.txt` (or check directory for exact name)

Built: `/tmp/A2osX-335cd122-build/stage/`
Stable: `/tmp/stable_extracted/`
Include path: `/tmp/A2osX-335cd122/INC/`

## Background
xasm++ is a modern C++ cross-assembler for SCMASM syntax (Apple II / 6502).

useradd at -48B: unusually large delta. Could be:
- Many branch relaxations
- `.DUMMY` equate bug (Bug 12) not fully fixed — check for remaining instances
- Accumulation of macro arg parsing issues

asm.65816 at -9B: three 3-byte expansions, or other issue. Note: this file is the
65816 CPU module for xasm's built-in assembler tool — it assembles for 65816 targets
from within the 65C02 A2osX environment. May use 65816-specific directives.

## Investigation Method
```bash
# useradd
/tmp/xasm_new --cpu 65c02 --syntax scmasm \
  /tmp/A2osX-335cd122/BIN/USERADD.S.txt \
  -o /tmp/useradd.bin --list > /tmp/useradd.lst 2>&1

python3 -c "
s=open('/tmp/stable_extracted/BIN/USERADD.dump','rb').read()[16:]
b=open('/tmp/A2osX-335cd122-build/stage/bin/useradd','rb').read()[16:]
diffs=[(i,a,c) for i,(a,c) in enumerate(zip(s,b)) if a!=c]
print(f'First diff at code offset {diffs[0][0]} (vaddr \${0x2000+diffs[0][0]:04X})')
print(f'Total differing bytes: {len(diffs)}')
print(f'Size diff: {len(b)-len(s):+d}')
"
```

## Acceptance Criteria
- [ ] Root cause identified for each file
- [ ] If assembler bug: fix implemented, all 1828 tests pass
- [ ] `cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build` exits 0
- [ ] `ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build` exits 0
- [ ] Findings in `.ai/docs/a2osx-bug-tracker.md`

## Related Projects
Related Projects: /Users/bryanw/Projects/Vintage/Apple/A2osX

## Knowledge Graph Instructions

**At task start, query both KGs before reading any source files:**

```
# Current project (xasm++)
mcp__kg__get_preflight_context({task: "investigate useradd -48B asm.65816 -9B delta"})
mcp__kg__search_knowledge({query: "useradd branch relaxation investigation"})
mcp__kg__search_knowledge({query: "asm.65816 size difference"})

# Related project (A2osX)
mcp__kg__get_preflight_context({task: "USERADD ASM.65816 source structure", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
mcp__kg__search_knowledge({query: "USERADD DUMMY section macros", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
mcp__kg__search_knowledge({query: "ASM 65816 assembler tool source", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
```

**Write every significant finding to the KG immediately** (prefix with [INVESTIGATION]).
**Write a [COMPLETION] summary before TaskComplete.**
