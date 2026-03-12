# Contract: Investigate bin/mv 1042-diff Same-Size

## Task ID
xasm-81v

## Problem
`bin/mv` has **1042 differing bytes** vs stable despite being the **same total size** (2434 bytes).
Need to determine if this is an assembler bug or intentional source code changes.

## Context
- Related to prior investigation in task xasm-siu (1044 diffs)
- Investigation already completed and fix applied in commit ce94435
- Need to verify fix and document resolution

## Acceptance Criteria
- [ ] Root cause(s) identified and documented
- [ ] If assembler bug: fix verified with tests
- [ ] If source changes: changes documented
- [ ] Work log updated with findings
- [ ] Beads task closed

## Related Work
- Prior investigation: `.ai/tasks/xasm-siu-20260311172901-mv-reloc-investigation/`
- Fix commit: ce94435 (five SCMASM bugs including double-dot label bug)
