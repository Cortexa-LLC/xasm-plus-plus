# Negative-Delta Files Investigation

## Goal
Determine why xasm++ produces FEWER bytes than stable for these 4 files, and identify the assembler bug(s) responsible.

## Files Under Investigation
| File | Delta | Stable size | Built size |
|------|-------|-------------|------------|
| sys/pm/pm.nsc    | -48 | 498  | 450  |
| sys/pm/pm.vsdrive | -21 | 1482 | 1461 |
| sys/pm.vedrive   | -20 | 6644 | 6624 |
| bin/bmp2pix      | -7  | 1696 | 1689 |

## Key Paths
- Source: `/tmp/A2osX-335cd122/` (git worktree at 335cd122)
- Build: `/tmp/A2osX-335cd122-build/stage/`
- Stable reference: `/tmp/stable_extracted/`
- Assembler: `/tmp/xasm_new`
- xasm++ source: `/Users/bryanw/Projects/Vintage/tools/xasm++/`

## Acceptance Criteria
- Root cause identified for each file's negative delta
- Specific assembler bug(s) documented (which directive/instruction is being dropped or emitted wrong)
- Fix recommended or implemented
