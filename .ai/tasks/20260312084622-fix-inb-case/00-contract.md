# Contract: Fix .INB Case-Sensitive Include Path Failure

**Task ID:** xasm-21d
**Created:** 2026-03-12
**Workflow:** Bugfix
**Requires Review:** true

## Problem

Several A2osX modules (mac2unix, xmastree, nfsmount, and others) fail assembly
with an error like:
  `Cannot open file: inc/macros.i` (or similar path)

The source files use `.INB inc/macros.i` but the actual file on disk is
`INC/MACROS.I.txt` (uppercase directory and filename with .txt extension).

SCMASM `.INB` directive = include binary/text file by path. The xasm++ include
path handling is case-sensitive and doesn't handle the `.txt` extension stripping
used by A2osX source files.

## Current State

- A2osX uses: `.INB inc/macros.i` (lowercase, no .txt)
- Actual file: `INC/MACROS.I.txt` (uppercase, .txt extension)
- xasm++ fails with "Cannot open file"
- This affects all files that include MACROS.I via .INB

## Desired State

- `.INB inc/macros.i` should successfully find `INC/MACROS.I.txt`
- Either via case-insensitive path matching or .txt extension fallback
- No regression in other include handling

## Key Files

- `src/syntax/scmasm_syntax.cpp` — HandleInb() / include resolution
- `src/syntax/scmasm_directive_handlers.cpp` — SCMASM directive handlers
- A2osX source: `~/Projects/Vintage/Apple/A2osX/`
  - Example: any source using `.INB inc/macros.i`

## Investigation Notes

The issue may be in one or more of these areas:
1. Path case-folding (macOS is case-insensitive by default but paths may differ)
2. Extension stripping: need to try both `filename` and `filename.txt`
3. Directory case: `inc/` vs `INC/`

On macOS, the filesystem IS case-insensitive, so `inc/macros.i` should find
`INC/MACROS.I` automatically. The `.txt` extension is the likely blocker.

## Acceptance Criteria

```
✓ .INB inc/macros.i successfully includes INC/MACROS.I.txt in A2osX context
✓ mac2unix, xmastree, nfsmount assemble without "Cannot open file" errors
✓ cmake --build build exits 0
✓ ctest --output-on-failure exits 0 (1865+ tests pass)
```

## Related Projects

Related Projects: /Users/bryanw/Projects/Vintage/Apple/A2osX
