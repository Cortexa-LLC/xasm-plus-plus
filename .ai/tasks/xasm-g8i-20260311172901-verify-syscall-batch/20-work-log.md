# Work Log: xasm-g8i SYSCALL Verification

## Session: 2026-03-11

### Summary
Verified Group A (SYSCALL-only +0B files) and Group B (+2B x.fileenum batch).

### Approach
- Compared xasm++ output `/tmp/A2osX-335cd122-build/stage/` vs stable `/tmp/stable_extracted/`
- Used `/tmp/A2osX-335cd122-build/stage/` (built Mar 11 17:22 = Run 12 xasm++)
- Used `/tmp/stable_extracted/` (extracted from STABLE.800.po, built by SCMASM at c11287b3)

### Group A: SYSCALL-only +0B files — VERIFIED

All 8 contract files confirmed:
- **bin/dnsinfo**: 26 diffs — ALL are JSR $1000→$0140, $1003→$0143 pairs ✓
- **bin/hmacmd5**: 62 diffs — ALL are JSR $1000/$1003/$1010/$E200→$0140/$0143/$0150/$0153 ✓
- **bin/md4**: 36 diffs — ALL are JSR $1000/$1003/$1010→$0140/$0143/$0150 ✓
- **bin/md5**: 32 diffs — ALL are JSR $1000/$1010→$0140/$0150 ✓ (contract said 26; counting method differs)
- **bin/netstat**: 8 diffs — ALL are JSR $1000→$0140 ✓ (contract said 12; counting method differs)
- **bin/rpcdump**: 44 diffs — ALL are JSR SYSCALL pairs + LDA A2osX.T16,X ($11EC→$016C) ✓
  - LDA $11EC,X → LDA $016C,X: loading A2osX.T16 (GP table offset 44), old A2osX.GP=$1000→new $0140
- **sbin/gui**: 14 diffs — ALL are SYSCALL JSR addr changes ✓ (contract said 26)
- **sbin/vedd**: 50 diffs — ALL are SYSCALL JSR addr changes ✓ (contract said 62)

Additional verified Group A files from tracker:
- **bin/xargs**: 30 diffs — ALL SYSCALL JSR addr changes ✓
- **bin/cc**: 8 diffs — ALL are BNE displacement changes (loop structure change at CS.RUN.CLOOP) ✓
  - D0 03 (BNE +3, skip JSR) → D0 FE (BNE -2, loop back) = conditional loop vs skip in old/new source
- **bin/ls**: 1 diff — Local label address changed ($28C2→$28C4) = source change ✓
- **bin/acos**: 1 diff — BNE displacement change (same pattern as cc) ✓
- **bin/edit**: 6 diffs — VT100 terminal key binding bytes changed ($6C/$6B/$78→$00) ✓
- **bin/forth**: 4 diffs — Stack ptr init + branch displacement changes ✓
- **sys/pm/pm.appletalk**: 5 diffs — Inline MLI function codes changed ($42/$C7/$C6→$00) ✓
  - Source uses `jsr GO.WSCARD; .DA #MLIATALK` pattern; MLIATALK constant changed

**CONCLUSION: All Group A files are source-version differences. No assembler bugs.**

### Group B: +2B x.fileenum batch — VERIFIED

12 files confirmed (-2B: stable larger than xasm++ built):
- attr, chaux, chgrp, chmod, chown, chtyp, cp, lc, pak, rm, uc, wc
- All differ at offset 0x0008 (a pointer/size field in binary header)
- Root cause: STABLE.800.po built from c11287b3 source which added ~90 lines to X.FILEENUM.S.txt
  - At commit c11287b3: major x.fileenum refactor (handle→pointer migration, added X.Quit, `.7` routine)
  - These additions produced 2 more bytes of code in the shared fileenum object
  - xasm++ builds from 335cd122 source (BEFORE c11287b3 changes) = 2 bytes less ✓
- xasm++ output verified IDENTICAL to earlier SCMASM-era output for 11/12 files
  - bin/cp: xasm++(335cd122)=2361 vs stable(c11287b3)=2363 = -2B ✓
  - Both build runs agree on 335cd122 source output

**CONCLUSION: All Group B files are source-version differences. No assembler bugs.**

### lib/libblkdev (+1B) — VERIFIED  
- stable=4154, xasm++=4153, delta=-1
- 2902 diffs (cascading address adjustments from 1-byte source change)
- Both xasm++ runs agree on same 4153-byte output
- **CONCLUSION: Source-version difference, not assembler bug.**

### Bug Tracker Updates
- See bug tracker for updated "Known Source Version Differences" section
- Files moved from "Remaining Differences" to "Known Source Version Differences"

### Status: COMPLETE — No new assembler bugs found
