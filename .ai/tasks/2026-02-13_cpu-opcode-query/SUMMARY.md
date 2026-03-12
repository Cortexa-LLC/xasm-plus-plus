# Task Summary: Refactor - Replace Hardcoded Opcode List with CPU Query

**Status:** ✅ COMPLETE
**Completed:** 2026-02-13
**Engineer:** Engineer Agent

---

## What Was Done

Refactored the SCMASM syntax parser to eliminate 50+ hardcoded opcode strings by adding a `HasOpcode()` method to the CPU plugin interface. Each CPU plugin now maintains its own opcode knowledge, and the parser queries the CPU instead of duplicating this information.

---

## Key Changes

### 1. Interface Extension
Added `HasOpcode()` method to `CpuPlugin` base class:
```cpp
virtual bool HasOpcode(const std::string &mnemonic) const = 0;
```

### 2. CPU Implementations
Implemented `HasOpcode()` for all three CPU plugins:
- **CPU_6502:** 56 opcodes (including 65C02 extensions)
- **CPU_6809:** 59 opcodes
- **CPU_Z80:** 144 opcodes

Each uses an `unordered_set` for O(1) lookup performance.

### 3. Syntax Parser Refactoring
**Before:** Hardcoded list of 50+ opcode strings
**After:** Query CPU via `cpu_->HasOpcode(label_upper)`

Added `SetCpu()` method to allow CPU configuration.

### 4. Bug Fixes
- Added missing LBSR mnemonic to 6809 opcodes
- Fixed duplicate header includes in CPU implementations

### 5. Test Updates
Updated SCMASM test fixture to set CPU plugin, enabling proper opcode recognition.

---

## Impact

### Code Quality
- ✅ **50+ magic strings eliminated**
- ✅ **Single source of truth** (CPU owns opcode knowledge)
- ✅ **Proper separation of concerns** (parser queries, doesn't duplicate)
- ✅ **Zero warnings** maintained

### Test Results
- ✅ **All 1564 tests passing** (100% pass rate)
- ✅ **Zero regressions**
- ✅ **All SCMASM tests passing** (117/117)

### Architecture
- ✅ **Better layering** (syntax parser → CPU plugin)
- ✅ **Extensibility** (new CPUs just implement HasOpcode())
- ✅ **Maintainability** (opcode changes in one place)

---

## Files Changed

**Interface:** `include/xasm++/cpu/cpu_plugin.h`
**CPU Headers:** `cpu_6502.h`, `cpu_6809.h`, `cpu_z80.h`
**CPU Sources:** `cpu_6502.cpp`, `cpu_6809.cpp`, `cpu_z80.cpp`
**Opcodes:** `opcodes_6809.h` (LBSR fix)
**Parser:** `scmasm_syntax.h`, `scmasm_syntax.cpp`
**Tests:** `test_scmasm_syntax.cpp`

---

## Verification

```bash
# Clean build
cmake --build build --clean-first
# Result: ✅ Success, zero warnings

# Full test suite
cd build && ctest
# Result: ✅ 1564/1564 tests passed (100%)

# SCMASM tests specifically
ctest -R ScmasmSyntaxTest
# Result: ✅ 117/117 tests passed (100%)
```

---

## Lessons Learned

1. **Test assumptions matter:** Tests implicitly relied on hardcoded opcodes; needed CPU setup
2. **Header organization:** Discovered and fixed duplicate includes
3. **Backward compatibility:** SetCpu() method allows parser to work with or without CPU
4. **Proper abstraction:** Pseudo-ops (DB, DW, DS) kept separate from CPU opcodes

---

## Next Steps

None required. Task complete and ready for integration.

---

**Documentation:** See work log (20-work-log.md) and acceptance (40-acceptance.md) for full details.
