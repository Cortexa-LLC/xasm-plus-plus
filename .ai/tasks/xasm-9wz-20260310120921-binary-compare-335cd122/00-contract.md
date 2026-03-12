# Contract: Binary Comparison xasm++ vs SCMASM (335cd122)

## Objective
Validate xasm++ SCMASM correctness by assembling A2osX commit 335cd122 (BIN Layout v1)
with xasm++ and comparing output binaries against STABLE.800.po — which was assembled
from that same commit by the original SCMASM assembler.

## Related Projects
Related Projects: /Users/bryanw/Projects/Vintage/Apple/A2osX

## Background
- STABLE.800.po = `/Users/bryanw/Projects/Vintage/Apple/A2osX/.Floppies/STABLE.800.po`
- STABLE was assembled from commit 335cd122 ("A2osX.95:LATEST & LAST bugfixes") by real SCMASM
- Current HEAD uses c11287b3 (BIN Layout v2) — different source, not comparable
- 335cd122 must be checked out into a separate worktree to avoid disturbing HEAD

## Acceptance Criteria
1. A2osX at 335cd122 assembles with xasm++ without errors
2. Script extracts all ProDOS files from both our build image and STABLE.800.po
3. Per-file byte-exact comparison report is produced
4. Any differences are categorized (known layout change vs potential assembler bug)
5. Report saved to .ai/tasks/xasm-9wz-*/comparison-report.md

## Key Files
- Reference image: /Users/bryanw/Projects/Vintage/Apple/A2osX/.Floppies/STABLE.800.po
- A2osX repo: /Users/bryanw/Projects/Vintage/Apple/A2osX/
- xasm++ binary: /Users/bryanw/Projects/Vintage/tools/xasm++/build/bin/xasm++
- ProDOS image tool: acx (find with `which acx` or locate acx.jar)
