# Contract: Investigate xasm++ vs SCMASM Binary Differences on STABLE Source

**Task ID:** xasm-aii
**Priority:** P1
**Working Directory:** /Users/bryanw/Projects/Vintage/tools/xasm++
**A2osX Directory:** /Users/bryanw/Projects/Vintage/Apple/A2osX

## Goal

Binary-compare xasm++ assembled output against STABLE.800.po and identify root
causes of all differences. Fix any xasm++ bugs found.

## Setup

- A2osX branch: xasm++_stable (at commit 9212fd12)
- A2osX source: /Users/bryanw/Projects/Vintage/Apple/A2osX/
- Build: cmake --build /Users/bryanw/Projects/Vintage/Apple/A2osX/_build --target mini
- STABLE reference: /Users/bryanw/Projects/Vintage/Apple/A2osX/.Floppies/STABLE.800.po
- xasm++ source: /Users/bryanw/Projects/Vintage/tools/xasm++
- xasm++ build: cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build

## Known Differences (byte sizes: stable vs xasm++)

- sys/kernel:   27716 vs 27623  (-93 bytes) — first diff at byte 14
- sbin/getty:     785 vs   806  (+21 bytes) — first diff at byte 3
- sbin/login:     743 vs   733  (-10 bytes) — first diff at byte 3
- sbin/initd:     439 vs   420  (-19 bytes) — first diff at byte 3
- lib/libblkdev: 4154 vs  2949 (-1205 bytes) — first diff at byte 3

## Binary Structure of A2osX Programs

A2osX BIN programs (65C02, load at $2000) start with:
- Byte 0: $D8 (CLD)
- Bytes 1-2: $7C lo hi (JMP ($lo+$hi*256, X)) — 65C02 absolute indexed indirect
- Bytes 3+: header structure with version info, entry-point jump table

The jump table address stored at bytes 1-2 pointing to the function table
is what differs — meaning the header SIZE differs between SCMASM and xasm++ output.

## Investigation Approach

### Step 1: Start with smallest binary (SBIN/INITD)

```bash
# Extract STABLE initd
cadius EXTRACTFILE /Users/bryanw/Projects/Vintage/Apple/A2osX/.Floppies/STABLE.800.po \
    /FULL800/SBIN/INITD /tmp/stable-files/sbin/

# Generate xasm++ listing for initd
# Find the source in CMakeLists.txt:
grep -i initd /Users/bryanw/Projects/Vintage/Apple/A2osX/CMakeLists.txt
# Then: look at A2osXHelpers.cmake for how xasm++ is invoked
# Add --list /tmp/initd-xasmpp.lst to the xasm++ command for comparison
```

### Step 2: Compare listings to find code generation differences

Key things to look for:
- Branch relaxation: SCMASM uses B!cc/JMP 5-byte pattern; xasm++ may differ
- Macro expansion differences (A2osX uses custom SCMASM macros heavily)
- Any instruction that encodes differently

### Step 3: Investigate libblkdev (-1205 bytes)

This is massive. Find LIBBLKDEV source and check if xasm++ is silently dropping
code (e.g., skipping a .INB include, failing a conditional, etc.)

```bash
grep -i libblkdev /Users/bryanw/Projects/Vintage/Apple/A2osX/CMakeLists.txt
# Check LIB/LIBBLKDEV.S.txt for .INB includes
```

### Step 4: Fix xasm++ bugs found

For each bug:
1. Write minimal test case in tests/scmasm/
2. Fix in src/syntax/scmasm_syntax.cpp or scmasm_directive_handlers.cpp
3. cmake --build build && ctest passes

## Key Files

- STABLE extracted: /tmp/stable-files/ (created by cadius EXTRACTFILE)
- xasm++ assembled: /Users/bryanw/Projects/Vintage/Apple/A2osX/_build/stage/
- SCMASM syntax parser: src/syntax/scmasm_syntax.cpp
- SCMASM directive handlers: src/syntax/scmasm_directive_handlers.cpp
- SCMASM header: include/xasm++/syntax/scmasm_syntax.h
- A2osX macros: /Users/bryanw/Projects/Vintage/Apple/A2osX/INC/MACROS.I.txt
- Test dir: tests/

## Build Commands

```bash
# xasm++
cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build
ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build --output-on-failure

# A2osX (force kernel rebuild)
rm /Users/bryanw/Projects/Vintage/Apple/A2osX/_build/stage/sys/kernel
cmake --build /Users/bryanw/Projects/Vintage/Apple/A2osX/_build --target mini
```

## Acceptance Criteria

1. Root cause of each difference documented in 20-work-log.md
2. Each xasm++ bug fixed with test case added
3. All xasm++ tests pass (cmake --build build && ctest exits 0)
4. At minimum: sys/kernel matches STABLE byte-for-byte after fixes
