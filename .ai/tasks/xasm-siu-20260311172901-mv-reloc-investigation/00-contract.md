# Contract: Investigate bin/mv 1044-diff 0B-delta

## Task ID
xasm-siu

## Problem
`bin/mv` has **1044 differing bytes** vs stable despite being the **same total size**.
First difference: code offset 4 (vaddr $2004), stable=$91 built=$93.
This is inside the relocation table at the start of the code section. The difference
of 2 between $91 and $93 suggests all relocation entries (and JSR/JMP call sites)
are shifted by 2 bytes relative to stable — classic symptom of a 2-byte code layout
difference somewhere before offset $2004.

## Files
- Source: `/tmp/A2osX-335cd122/BIN/MV.S.txt`
- Built: `/tmp/A2osX-335cd122-build/stage/bin/mv`
- Stable: `/tmp/stable_extracted/BIN/MV.dump`
- Assembler: `/tmp/xasm_new`
- Include path: `/tmp/A2osX-335cd122/INC/`

## Background
xasm++ is a modern C++ cross-assembler for SCMASM syntax (Apple II A2osX OS).
Run 12 state: 71 identical files. All remaining negative-delta files are confirmed
source changes. Remaining work is +0B files (same size, content differs).

A2osX BIN header format (file bytes 0-15):
  [0]    $D8 = CLD
  [1-3]  JMP entry_addr (3 bytes)
  [4-5]  file size (2 bytes, little-endian)
  [6-7]  load addr ($2000)
  [8-15] reserved/other

Code section (file offset 16+, vaddr starting at $2000):
  Relocation table: 2-byte little-endian entries (vaddrs of bytes to patch at load time)
  Terminated by $0000 entry
  Followed by: actual 6502 machine code

## Investigation Method

### Step 1: Generate listing
```bash
/tmp/xasm_new --cpu 65c02 --syntax scmasm \
  /tmp/A2osX-335cd122/BIN/MV.S.txt \
  -o /tmp/mv_built.bin --list > /tmp/mv.lst 2>&1
```

### Step 2: Analyze the relocation table difference
```python
s = open('/tmp/stable_extracted/BIN/MV.dump','rb').read()
b = open('/tmp/A2osX-335cd122-build/stage/bin/mv','rb').read()
sc = s[16:]; bc = b[16:]

# Find end of stable reloc table
i = 0
while i+1 < len(sc) and not (sc[i]==0 and sc[i+1]==0):
    i += 2
stable_reloc_end = i
print(f"Stable reloc table ends at code offset {stable_reloc_end} ({stable_reloc_end//2} entries)")

# Find end of built reloc table  
i = 0
while i+1 < len(bc) and not (bc[i]==0 and bc[i+1]==0):
    i += 2
built_reloc_end = i
print(f"Built reloc table ends at code offset {built_reloc_end} ({built_reloc_end//2} entries)")

# Compare first few reloc entries
for n in range(min(20, min(stable_reloc_end, built_reloc_end)//2)):
    off = n*2
    sv = sc[off] | (sc[off+1]<<8)
    bv = bc[off] | (bc[off+1]<<8)
    match = "==" if sv==bv else f"DIFF (delta={bv-sv:+d})"
    print(f"  entry[{n}]: stable=${sv:04x} built=${bv:04x} {match}")
```

### Step 3: Find the source of the 2-byte layout difference
If all reloc entries are consistently offset by +2, find what's 2 bytes
larger in the built binary before the first reloc entry:
- Could be a 2-byte instruction expansion  
- Could be a data directive emitting 2 extra bytes
- Could be a string or .HS/.DA difference
- Look for the first 2-byte gap by comparing code sections after the reloc table

### Step 4: Map to source, fix if assembler bug
Use the listing to find what instruction/directive is at the differing address.

## Knowledge Graph Instructions

**Query BEFORE reading any source files:**
```
mcp__kg__get_preflight_context({task: "bin/mv 1044 diffs relocation table layout investigation"})
mcp__kg__search_knowledge({query: "mv relocation table layout diff"})
mcp__kg__search_knowledge({query: "bin/mv content diff investigation"})
mcp__kg__search_knowledge({query: "relocation table 2-byte shift assembler bug"})

# A2osX KG — MV source structure, macros used
mcp__kg__get_preflight_context({task: "MV source macros structure", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
mcp__kg__search_knowledge({query: "MV file copy source structure", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
mcp__kg__search_knowledge({query: "DUMMY section ZP labels MV", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
```

**Write every significant finding** to KG with `[INVESTIGATION]` prefix.
**Write `[COMPLETION]` summary** before TaskComplete.

## Acceptance Criteria
- [ ] Root cause of 1044-diff layout shift identified
- [ ] If assembler bug: fix in src/, all 1834 tests pass
- [ ] `cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build` exits 0
- [ ] `ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build` exits 0
- [ ] Findings in `.ai/docs/a2osx-bug-tracker.md`

## Key Assembler Source Files
- `src/syntax/scmasm_syntax.cpp`
- `src/syntax/scmasm_directive_handlers.cpp`
- `src/core/assembler.cpp`
- `src/cpu/cpu_6502.cpp`

## Related Projects
Related Projects: /Users/bryanw/Projects/Vintage/Apple/A2osX
