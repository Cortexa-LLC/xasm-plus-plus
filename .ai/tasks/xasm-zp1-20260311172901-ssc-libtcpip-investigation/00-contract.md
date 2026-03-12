# Contract: Investigate drv/ssc.drv, drv/ssc.i.drv, lib/libtcpip, lib/libetalk

## Task ID
xasm-zp1

## Problem
Four files have same size but different content vs stable:
- `drv/ssc.drv`: 39 diffs, first at v$22b4 stable=$01 built=$00
- `drv/ssc.i.drv`: 39 diffs, first at v$22c2 stable=$01 built=$00
- `lib/libtcpip`: 65 diffs, first at v$2090 stable=$80 built=$00
- `lib/libetalk`: 11 diffs, first at v$2099 stable=$a0 built=$00

## Files
Sources: `/tmp/A2osX-335cd122/DRV/SSC.DRV.S.txt`, `DRV/SSC.I.DRV.S.txt`,
         `/tmp/A2osX-335cd122/LIB/LIBTCPIP.S.txt` (check exact name),
         `/tmp/A2osX-335cd122/LIB/LIBETALK.S.txt` (check exact name)
Built: `/tmp/A2osX-335cd122-build/stage/`
Stable: `/tmp/stable_extracted/`
Assembler: `/tmp/xasm_new`
Include path: `/tmp/A2osX-335cd122/INC/`

## Background
xasm++ is a C++ cross-assembler for SCMASM/Apple II. Run 12: 71 identical.
SYSCALL address change ($1000→$0140, $E200→$0153) explains many +0B content diffs.
Need to determine whether these four files' diffs are purely SYSCALL/source changes
or contain real assembler bugs.

Analysis hints:
- `stable=$01 built=$00` at v$22b4 in SSC.DRV — $01 is low byte of an address
  like $XX01. Not an obvious SYSCALL pattern. Could be a ZP symbol resolving to 0.
- `stable=$80 built=$00` in libtcpip at v$2090 — $80 could be high byte of $8000
  or BRA opcode. If it's $00 in built, might be undefined symbol.
- `stable=$a0 built=$00` in libetalk at v$2099 — $A0 is LDY # opcode or Apple II
  high-bit space. Difference suggests a missing byte value.

## Knowledge Graph Instructions

**MANDATORY: Query KG before reading any source files:**
```
# xasm++ KG
mcp__kg__get_preflight_context({task: "ssc.drv libtcpip libetalk content diff investigation"})
mcp__kg__search_knowledge({query: "ssc driver serial card content diff"})
mcp__kg__search_knowledge({query: "libtcpip libetalk diff investigation"})
mcp__kg__search_knowledge({query: "SYSCALL address change relocation"})

# A2osX KG — SSC/TCP/Etalk source structure
mcp__kg__get_preflight_context({task: "SSC DRV LIBTCPIP LIBETALK source", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
mcp__kg__search_knowledge({query: "SSC serial card driver source structure", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
mcp__kg__search_knowledge({query: "LIBTCPIP LIBETALK macros symbols", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
```

**Write every finding** to KG with `[INVESTIGATION]` prefix.
**Write `[COMPLETION]` summary** before TaskComplete.

## Investigation Method (for each file)

```python
# Find all diff positions and their context
import pathlib
for name, stable_path, built_path in [
    ('ssc.drv', '/tmp/stable_extracted/DRV/SSC.DRV.dump', '/tmp/A2osX-335cd122-build/stage/drv/ssc.drv'),
    ('ssc.i.drv', '/tmp/stable_extracted/DRV/SSC.I.DRV.dump', '/tmp/A2osX-335cd122-build/stage/drv/ssc.i.drv'),
]:
    s = open(stable_path,'rb').read()[16:]
    b = open(built_path,'rb').read()[16:]
    diffs = [(i, s[i], b[i]) for i in range(min(len(s),len(b))) if s[i]!=b[i]]
    print(f"{name}: {len(diffs)} diffs")
    for i,sv,bv in diffs[:10]:
        print(f"  v${0x2000+i:04x}: stable=${sv:02x} built=${bv:02x}")
```

Key question: Are the diff values consistent with SYSCALL address bytes?
- SYSCALL ($1000→$0140): low byte $00→$40, high byte $10→$01
- SYSCALL2 ($E200→$0153): low byte $00→$53, high byte $E2→$01
- SLEEP ($1010→$0150): low byte $10→$50, high byte $10→$01
If all diffs match these patterns → confirmed SYSCALL source change.
If not → investigate further.

## Acceptance Criteria
- [ ] Each file classified: SYSCALL-only source change OR assembler bug
- [ ] If assembler bug found: fix implemented, 1834 tests pass
- [ ] Bug tracker updated with findings
- [ ] `cmake --build` exits 0, `ctest` exits 0

## Related Projects
Related Projects: /Users/bryanw/Projects/Vintage/Apple/A2osX
