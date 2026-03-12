# Contract: Investigate and fix lib/libgui -132B delta

## Task ID
xasm-i6h

## Problem
lib/libgui assembled by xasm++ is **132 bytes larger** than the STABLE.800.po reference.
This is the largest remaining negative-delta (built > stable), indicating multiple branch
relaxations or other size-inflating assembler bugs in the 10KB GUI library.

## Files
- Source: `/tmp/A2osX-335cd122/LIB/LIBGUI.S.txt`
- Built: `/tmp/A2osX-335cd122-build/stage/lib/libgui` (10912 bytes)
- Stable: `/tmp/stable_extracted/LIB/LIBGUI.dump` (10780 bytes)
- Include path: `/tmp/A2osX-335cd122/INC/`

## Background
xasm++ is a modern C++ cross-assembler for SCMASM syntax (Apple II / 6502).
We are making it produce byte-identical output to the original SCMASM assembler using
A2osX commit 335cd122 source. The stable reference was assembled from a417f7ab era source
with the original SCMASM assembler (binary in STABLE.800.po).

Known bug patterns to look for:
1. Branch relaxation: BRA/BCC/etc. expanded to JMP (+3 bytes each) when they should be short
2. EquateAtom in .DUMMY: `LABEL .EQ *` inside `.DUMMY` sections -- `LABEL .EQ *` should NOT
   be re-evaluated on subsequent passes (fixed in recent commits for most cases, but may
   have remaining instances)
3. Macro arg parsing: comma in inline comment treated as arg separator (already fixed for
   STYA, but check for similar patterns in other macros)

## Investigation Method
```bash
# Generate listing
/tmp/xasm_new --cpu 65c02 --syntax scmasm \
  /tmp/A2osX-335cd122/LIB/LIBGUI.S.txt \
  -o /tmp/libgui.bin --list > /tmp/libgui.lst 2>&1

# Find first diff byte between stable[16:] and built[16:]
python3 -c "
s=open('/tmp/stable_extracted/LIB/LIBGUI.dump','rb').read()[16:]
b=open('/tmp/A2osX-335cd122-build/stage/lib/libgui','rb').read()[16:]
for i,(a,c) in enumerate(zip(s,b)):
    if a!=c:
        print(f'First diff at code offset {i} (vaddr \${0x2000+i:04X}): stable=\${a:02X} built=\${c:02X}')
        break
"
```

## Acceptance Criteria
- [ ] Root cause of -132B delta identified
- [ ] If assembler bug found: fix implemented and all 1828 tests pass
- [ ] `cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build` exits 0
- [ ] `ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build` exits 0 (1828 tests)
- [ ] Bug documented in `.ai/docs/a2osx-bug-tracker.md`

## Key Files (assembler source)
- `src/syntax/scmasm_syntax.cpp` — macro expansion, label resolution
- `src/syntax/scmasm_directive_handlers.cpp` — .DUMMY, .EQ, .CS/.CZ etc.
- `src/core/assembler.cpp` — branch relaxation, multi-pass
- `src/cpu/cpu_6502.cpp` — instruction encoding

## Related Projects
Related Projects: /Users/bryanw/Projects/Vintage/Apple/A2osX

## Knowledge Graph Instructions

**At task start, query both KGs before reading any source files:**

```
# Current project (xasm++)
mcp__kg__get_preflight_context({task: "investigate libgui -132B delta branch relaxation"})
mcp__kg__search_knowledge({query: "libgui branch relaxation investigation"})
mcp__kg__search_knowledge({query: "EquateAtom DUMMY section bug"})

# Related project (A2osX) — symbols, macros, source structure
mcp__kg__get_preflight_context({task: "LIBGUI source structure macros", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
mcp__kg__search_knowledge({query: "LIBGUI macro definitions", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
mcp__kg__search_knowledge({query: "DUMMY section ZP labels LIBGUI", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
```

**Write every significant finding to the KG immediately** (prefix with [INVESTIGATION]).
**Write a [COMPLETION] summary before TaskComplete.**
