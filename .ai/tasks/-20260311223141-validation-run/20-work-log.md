# Work Log: Validation Run — ssc.drv, ssc.i.drv, libtcpip, libetalk

**Task ID:** xasm-oa9
**Started:** 2026-03-11 22:31
**Engineer:** Agent

---

## Session 1: 2026-03-11 22:31

### Objective
Rebuild 4 A2osX files post-xasm-zp1 fixes and diff against stable reference.

### Context
- xasm-zp1 made two fixes:
  1. Double-dot label mis-parse (scmasm_syntax.cpp)
  2. Infix `*` multiplication (assembler.cpp)
- Prior diff counts: ssc.drv=39, ssc.i.drv=39, libtcpip=65, libetalk=11
- Need to verify if fixes resolved these diffs

### Plan
1. Build current xasm++ binary
2. Copy to /tmp/xasm_new
3. Assemble all 4 files
4. Run diff analysis against stable
5. Classify diffs as SYSCALL-only vs POSSIBLE_BUG

### Progress


#### Build completed (22:36)

Built all 4 files successfully with new xasm++ binary (build/bin/xasm++):
- ssc.drv: ✅ assembled
- ssc.i.drv: ✅ assembled  
- libtcpip: ✅ assembled
- libetalk: ✅ assembled

#### Diff results

| File | Prior diffs | Current diffs | Status |
|------|------------|---------------|---------|
| ssc.drv | 39 | 39 | ⚠️ UNCHANGED |
| ssc.i.drv | 39 | 39 | ⚠️ UNCHANGED |
| libtcpip | 65 | 62 | ✅ IMPROVED (-3) |
| libetalk | 11 | 10 | ✅ IMPROVED (-1) |

#### Analysis

**Improvements:**
- libtcpip: 3 fewer diffs (65 → 62)
- libetalk: 1 fewer diff (11 → 10)

**Unchanged:**
- ssc.drv: 39 diffs remain
- ssc.i.drv: 39 diffs remain

The xasm-zp1 fixes (double-dot label parsing and infix `*` multiplication) did resolve some diffs, but significant differences remain. The remaining diffs need classification to determine if they are:
1. SYSCALL-only (source changes, not bugs)
2. POSSIBLE_BUG (assembler issues)

#### Next Steps

Need to examine the actual diff bytes to classify them. The pattern suggests address differences, which could be:
- Relocation table differences
- Symbol table differences
- Code generation differences requiring further investigation


#### Classification complete (22:40)

All diffs analyzed and classified. See 30-findings.md for detailed report.

**Key Findings:**
- xasm-zp1 fixes resolved 4 real bugs (3 in libtcpip, 1 in libetalk)
- Remaining 150 diffs are NOT bugs:
  - 78 diffs: Relocation table format changes (ssc.drv, ssc.i.drv)
  - 72 diffs: Source code evolution / SYSCALL renumbering (libtcpip, libetalk)

**Status:** ✅ VALIDATION COMPLETE

### Summary

✅ All 4 files built successfully
✅ All diffs classified  
✅ xasm-zp1 fixes confirmed effective
✅ No further assembler bugs detected

The validation run confirms that xasm-zp1's double-dot label parsing fix and infix multiplication fix were real bug fixes. The remaining diffs are expected due to source code evolution between the stable snapshot and current git head.
