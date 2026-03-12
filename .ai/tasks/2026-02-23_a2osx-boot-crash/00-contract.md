# Contract: Investigate A2osX Boot Crash at $1FE2

## Problem Statement
A2osX images built with xasm++ crash during boot. The Apple II monitor shows:
```
1FE2-    A=10 X=08 Y=01 P=34 S=EE
```

The CPU halted/broke to the monitor at address $1FE2.
- A=$10, X=$08, Y=$01, P=$34, S=$EE

## Goal
Identify the xasm++ assembler bug causing wrong bytes/addresses in the kernel binary
and propose a targeted fix.

## Acceptance Criteria
1. Identify what instruction/bytes are at $1FE2 in the assembled kernel
2. Identify what SHOULD be there (from source or reference)
3. Pinpoint the xasm++ code path responsible
4. Propose a fix with test case

## Key Locations
- A2osX source:  /Users/bryanw/Projects/Vintage/Apple/A2osX/
- A2osX kernel:  /Users/bryanw/Projects/Vintage/Apple/A2osX/build/stage/sys/kernel
- A2osX disk:    /Users/bryanw/Projects/Vintage/Apple/A2osX/build/a2osx-mini.po
- xasm++ source: /Users/bryanw/Projects/Vintage/tools/xasm++/
- Recent fixes:  git log --oneline (commits 6fc5824, e9b47ad, etc.)

## Beads Task
xasm++-m77r
