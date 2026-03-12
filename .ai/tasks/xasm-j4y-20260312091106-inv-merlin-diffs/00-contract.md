# Contract: Investigate Prince of Persia Merlin Binary Diffs

**Task ID:** xasm-fso
**Created:** 2026-03-12
**Workflow:** Research / Bugfix
**Requires Review:** false

## Problem

18/29 Prince of Persia (Apple II) binary modules differ between xasm++ and the
vasm reference (`build-snap/`). 11 are already identical. Root causes need to
be identified and assembler bugs fixed where applicable.

## Current State

Build command: `make ASM=xasm XASM=xasm++ binaries`
(in `~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/`)
Reference: `build-snap/` (vasm -Fbin, no RW18 headers)

### Diff Summary (2026-03-12)

| File | Delta | Diffs | Likely Cause |
|------|-------|-------|--------------|
| SOUND.bin | +0 | 23 | soundtable $EA04 vs $B6C0 — DUM label |
| SEQTABLE.bin | +0 | 31 | Similar label resolution |
| MASTER.bin | +0 | 1624 | Many address references wrong |
| MOVER.bin | +12 | 2511 | 4 extra branch relaxations |
| SPECIALK.bin | +12 | 1213 | Same |
| SUBS.bin | +12 | 2259 | Same |
| TABLES.bin | +12 | 1115 | Same |
| UNPACK.bin | +8 | 991 | 2-3 extra relaxations |
| AUTO.bin | +521 | 2327 | Cascade from address error |
| COLL.bin | +361 | 1713 | Cascade |
| CTRL.bin | +654 | 2748 | Cascade |
| CTRLSUBS.bin | +565 | 2180 | Cascade |
| FRAMEADV.bin | +742 | 2814 | Cascade |
| GAMEBG.bin | +300 | 1449 | Cascade |
| GRAFIX.bin | +336 | 2338 | Cascade |
| HIRES.bin | +571 | 2430 | Cascade |
| MISC.bin | +243 | 1277 | Cascade |
| TOPCTRL.bin | +369 | 1917 | Cascade |

### Key Hypothesis: DUM label resolution

In EQ.S (included by all modules via `put eq`), there are multiple `dum`
sections that define variable addresses. In SOUND.S, `soundtable` is used
with STA/LDX but vasm resolves it to $B6C0 while xasm++ resolves it to $EA04
(the module's own ORG+offset). $B6C0 = endimspace ($B600) + $C0.

### Key Hypothesis: Branch relaxation over-trigger

Files with exactly +12 bytes (4 x 3-byte expansions) suggest 4 branches that
xasm++ relaxes but vasm does not. These may be affected by incorrect address
resolution.

## Files

- Source: `~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/01 POP Source/Source/`
- EQ.S: contains DUM sections; included by all modules via `put eq`
- GAMEEQ.S: game-specific equates
- Reference bins: `~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/build-snap/`
- xasm++ bins: `~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/build/`

## Investigation Method

1. Examine SOUND.S + EQ.S DUM sections: find where `soundtable` should be
2. Check how xasm++ handles Merlin `dum`/`dend` label scoping
3. Generate xasm++ symbol table for SOUND.S and check soundtable value
4. For +12B files: identify which 4 branches are being incorrectly relaxed
5. Fix any assembler bugs found

## Acceptance Criteria

```
✓ Root cause of soundtable/$B6C0 vs $EA04 identified and fixed (if assembler bug)
✓ Root cause of +12B over-relaxation identified and fixed (if assembler bug)
✓ Rerun comparison: make ASM=xasm XASM=xasm++ binaries
✓ cmake --build build exits 0 and ctest --output-on-failure exits 0
✓ Investigation findings documented in work log
```

## Related Projects

Related Projects: /Users/bryanw/Projects/Vintage/Apple/Prince-of-Persia-Apple-II
