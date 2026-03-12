# Contract: xasm++_stable Branch — Build STABLE Source with xasm++ and Binary-Compare

**Task ID:** xasm-r87
**Priority:** P0
**Working Directory:** /Users/bryanw/Projects/Vintage/tools/xasm++
**A2osX Directory:** /Users/bryanw/Projects/Vintage/Apple/A2osX

## Goal

Validate that xasm++ produces byte-identical output to the original SCMASM assembler by:
1. Building the STABLE source (commit c11287b3) using xasm++ + our cmake build system
2. Binary-comparing every assembled binary against the known-good STABLE.800.po image

## Current Branch State

The A2osX repo is already on branch `xasm++_stable` at commit `c11287b3`.
This is the exact source that produced `STABLE.800.po` (volume "FULL800", 800K ProDOS FX image).

```bash
cd /Users/bryanw/Projects/Vintage/Apple/A2osX
git branch   # should show * xasm++_stable
git log --oneline -1  # should show c11287b3
```

## What Needs to Be Applied

Cherry-pick only the cmake/build infrastructure commits from `xasm++_support` (NOT the source compat patches):

- `4c8b0cb4` — cmake build system (CMakeLists.txt, cmake/*.cmake)
- `8e97b014` — missing macros/compat stubs (INC/ changes only if needed for build)
- `0fa0ed4b` — xasm++ source compatibility fixes (INC/, SYS/ changes)
- `edad7e17` — Makefile wrapper

**IMPORTANT:** Cherry-pick carefully. If a commit only touches build infrastructure (CMakeLists.txt, cmake/, Makefile), take it whole. If it touches source files (*.S.txt, *.I.txt), review what changed — we only want fixes needed for xasm++ to parse SCMASM syntax, NOT semantic changes that would alter assembled bytes.

The goal is: same source semantics as SCMASM, just made parseable by xasm++.

## Build Steps

```bash
cd /Users/bryanw/Projects/Vintage/Apple/A2osX
# After cherry-picks:
rm -rf build/
cmake . -B build
cmake --build build
```

## Binary Comparison

The STABLE image is at: `/Users/bryanw/Projects/Vintage/Apple/A2osX/.Floppies/STABLE.800.po`
Volume name: FULL800

Extract all files from STABLE and compare with build/stage/:
```bash
# cadius can extract individual files:
cadius EXTRACTFILE /path/to/STABLE.800.po /FULL800/SYS/KERNEL /tmp/stable-files/
# Then: cmp /tmp/stable-files/kernel#... build/stage/sys/kernel
```

Key files to compare (from STABLE catalog seen earlier):
- sys/kernel (SYS $2000, 27716 bytes in STABLE)
- sbin/getty, sbin/login, sbin/initd, sbin/telnetd
- lib/libblkdev
- sys/km/km.inet, sys/km/km.pascal

## Success Criteria

1. All binaries assemble without errors
2. At least sys/kernel binary matches STABLE byte-for-byte
3. Any differences documented with root cause (source change vs xasm++ bug)
4. If differences found: identify whether it's a syntax compatibility issue or semantic difference

## Notes

- STABLE volume name is "FULL800" (not "A2OSX")
- STABLE kernel = 27716 bytes at $2000
- Our current xasm++_support kernel = 29644 bytes (different source, NOT comparable)
- The c11287b3 source has NO `>LDYA $FFFE` in KERNEL.S.INIT2.txt — that was added later
- xasm++ binary: /usr/local/bin/xasm++ (version 0.9.0-b7b739c)
- cadius: /usr/local/bin/cadius

## What NOT To Do

- Do NOT merge changes from xasm++_support that alter assembly output semantics
- Do NOT modify source files (.S.txt) unless strictly necessary for xasm++ parsing
- Do NOT change ORG addresses, instruction encodings, or data values
