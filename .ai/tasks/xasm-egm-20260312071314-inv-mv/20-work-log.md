# Work Log

**Task ID:** xasm-uwd
**Started:** 2026-03-12
**Status:** Completed

---

## Work Sessions

### Session 1: 2026-03-12

#### Objectives for This Session
```
✓ Compare bin/mv built vs stable binary
✓ Map all 1041 byte differences
✓ Classify as SYSCALL / source-change / assembler-bug
✓ Update bug tracker (KG)
```

#### Work Completed

**Binary comparison:**
- Built: `/tmp/A2osX-335cd122-build/stage/bin/mv` (2434 bytes)
- Stable: `/tmp/stable_extracted/FULL800/bin/mv#062000` (2434 bytes)
- Diffs: 1041 bytes (task said 1042 - minor discrepancy from prior run)

**Classification results:**
| Category | Blocks | Bytes | Verdict |
|----------|--------|-------|---------|
| SYSCALL shifts (uniform +2) | 20 | 20 | Not a bug |
| Source changes (mixed bytes) | 45 | 1021 | Not an assembler bug |
| Assembler bugs | 0 | 0 | N/A |
| **Total** | **65** | **1041** | **No bug found** |

**Key findings:**

1. **SYSCALL shifts (20 bytes):** All 20 blocks are single-byte diffs where
   `built = stable + 2`. These are SYSCALL dispatch address relocations in the
   binary's relocation table. Expected build variation. Not a bug.

2. **Source changes (1021 bytes):** The binary shows a source-level refactor where
   direct absolute ZP addressing was replaced with ZP-pointer indirect addressing:
   - Stable: `LDA $00xx` (3-byte)
   - Built: `LDY #xx; LDA ($DA),Y` (4-byte via ZPPtr1 pointer at $DA)
   - 24 LDA patterns and 6 STA patterns changed
   - Branch offsets adjusted accordingly
   - Binary size unchanged (compensating optimizations elsewhere)
   - This is an intentional source change in X.CPMVRM.S / X.FILEENUM.S

3. **Assembler bugs:** Zero. xasm++ correctly assembled the modified source.

**Retrospective written:** `.ai/tasks/xasm-egm-20260312071314-inv-mv/retrospective.md`
**KG updated:** Added investigation observation to `bin/mv 1042-diff investigation` entity

#### Session Outcome
Investigation complete. No assembler bugs found. Closing task as "no bug - source change".
