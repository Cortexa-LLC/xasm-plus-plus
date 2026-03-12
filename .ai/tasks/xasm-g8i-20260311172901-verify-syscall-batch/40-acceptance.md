# Acceptance: xasm-g8i SYSCALL Verification

## Checklist

- [x] Each Group A file classified (SYSCALL-only vs other)
  - All 8 contract files (dnsinfo, hmacmd5, md4, md5, netstat, rpcdump, gui, vedd) verified
  - Plus 7 additional Group A files (xargs, cc, ls, acos, edit, forth, sys/pm/pm.appletalk)
  - **ALL confirmed source changes, zero unexplained diffs, zero assembler bugs**

- [x] Group B: x.fileenum.s source change confirmed
  - All 12 files (attr, chaux, chgrp, chmod, chown, chtyp, cp, lc, pak, rm, uc, wc) verified
  - Root cause: c11287b3 added ~90 lines to X.FILEENUM.S.txt (net +2B assembled code)
  - xasm++ building 335cd122 source = 2 bytes less (correct for source version)

- [x] Bug tracker updated: confirmed source changes moved to Known Source Version Diffs
  - Added "SYSCALL-Only Same-Size Files" subsection with detailed per-file diff analysis
  - Added "x.fileenum Shared Source Change" subsection with root cause explanation
  - Updated lib/libblkdev entry to "CONFIRMED source change"
  - Updated "Remaining Differences" Positive-delta table entries to "VERIFIED"
  - Updated "Same-size, different-content" section header to "VERIFIED"
  - Added Run 13 (xasm-g8i) entry to Progress Summary

- [x] If any new assembler bug found: fix + 1834 tests pass
  - **No new assembler bugs found.** All differences are source-version differences.

## Result: PASS — No assembler action needed

Beads Task: xasm-g8i [CLOSED]
