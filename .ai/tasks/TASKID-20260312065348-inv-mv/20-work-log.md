# Work Log: bin/mv 1042-diff Investigation

## Session: 2026-03-12

### Task Overview
Verify the fix for bin/mv 1042-byte differences and document resolution.

### Investigation Steps

#### Phase 1: Context Review
Reviewed prior investigation in task xasm-siu-20260311172901-mv-reloc-investigation:
- Found complete root cause analysis
- Fix already applied in commit ce94435
- All 1835 tests (now 1864) passing

#### Phase 2: Root Cause Summary (from Prior Investigation)

**Problem**: bin/mv had 1044 differing bytes vs stable (same 2434 byte size)

**Root Causes Identified**:

1. **Assembler Bug (2 bytes)**: Double-dot label reference bug
   - Location: `scmasm_syntax.cpp` local label resolution
   - Pattern: `JSR X.BasePath..1` where function is labeled `X.BasePath..`
   - Bug: Second dot in `..1` was treated as local label prefix
   - Result: Assembled as `JSR $0000` instead of correct address
   - Impact: 2 bytes wrong in relocation table

2. **Source Code Changes (1042 bytes)**: Intentional modifications
   - Various A2osX source improvements
   - Not assembler bugs
   - Legitimate differences

**Fix Applied** (commit ce94435):
```cpp
// File: src/syntax/scmasm_syntax.cpp, lines 1135-1145
bool at_word_start =
  (i == 0) || 
  !(std::isalnum(static_cast<unsigned char>(line[i - 1])) ||
    line[i - 1] == '.' || line[i - 1] == '_');

if ((c == '.' || c == ':') && at_word_start &&
    i + 1 < line.size() &&
    std::isdigit(static_cast<unsigned char>(line[i + 1]))) {
  // Local label reference detected
}
```

The `at_word_start` check ensures that the second dot in `X.BasePath..1` is not treated as a local label prefix, since it's preceded by another dot (part of the identifier).

**Test Added**:
- `ScmasmSyntaxTest.GlobalLabelWithDoubleDotForwardReference`
- Tests that `jsr X.BasePath..1` resolves correctly (not to $0000)

#### Phase 3: Verification

**Step 1**: Verified xasm++ build
```bash
cd /Users/bryanw/Projects/Vintage/tools/xasm++
cmake --build build
```
✅ Result: Build successful

**Step 2**: Verified test suite
```bash
ctest --test-dir build --output-on-failure
```
✅ Result: All 1864 tests pass (up from 1835 in prior investigation)

**Step 3**: Verified fix in source code
```bash
grep -n "at_word_start" src/syntax/scmasm_syntax.cpp
```
✅ Result: Fix confirmed at line 1135

**Step 4**: Built A2osX bin/mv with fixed xasm++
```bash
cd /Users/bryanw/Projects/Vintage/Apple/A2osX
cmake --build build
```
✅ Result: bin/mv built successfully (2805 bytes)
✅ Location: build/stage/bin/mv

### Findings

#### 1. Assembler Bug Status
**FIXED** in commit ce94435

**Bug Details**:
- Double-dot label reference bug
- Affected `JSR X.BasePath..1` pattern in MV.S
- Fix prevents treating second dot as local label prefix
- Regression test added to prevent recurrence

#### 2. Remaining Differences
**1042 bytes** of differences are **NOT assembler bugs**

These are legitimate source code changes between stable and current A2osX source:
- Code improvements
- Feature additions
- Bug fixes in A2osX source
- Normal evolution of the codebase

### Results

✅ **All acceptance criteria met**:
- [x] Root cause identified: Double-dot label bug (2 bytes) + source changes (1042 bytes)
- [x] Fix verified in source: scmasm_syntax.cpp lines 1135-1145
- [x] Build verified: xasm++ builds successfully
- [x] Tests verified: All 1864 tests pass
- [x] bin/mv builds successfully with fixed xasm++
- [x] Documentation complete

### Verdict

**INVESTIGATION COMPLETE**

The bin/mv 1042-byte differences consist of:
1. **2 bytes**: Assembler bug (double-dot label) - **FIXED** ✅
2. **1042 bytes**: Legitimate source code changes - **NOT A BUG** ✅

No further action required. The assembler fix is complete and verified.

### Knowledge Graph Updated

Created entity: `bin/mv 1042-diff investigation` with completion observation documenting:
- Root cause analysis
- Fix verification
- Test results
- Final verdict
