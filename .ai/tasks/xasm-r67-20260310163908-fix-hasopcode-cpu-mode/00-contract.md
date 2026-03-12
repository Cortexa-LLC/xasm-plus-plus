# Contract: Fix HasOpcode() to exclude 65816-only opcodes in 65c02 mode

## Task ID
xasm-r67

## Problem

`HasOpcode()` in `src/cpu/cpu_6502.cpp` uses a single flat set containing ALL opcodes
including 65816-only ones (MVN, MVP, PHB, etc.), regardless of the current CPU mode.

When assembling A2osX with `--cpu 65c02`, opcodes like MVN and MVP appear as LABELS
in source files (e.g. `BIN/ASM.65816.O.txt`):
```
MVN    .DA MVN.E+1-*
       .PS "MVN"
       .DA #21
       .PS "54ab"
MVN.E  .DA #0
```

xasm++ incorrectly treats `MVN` at column 0 as an opcode, not a label, and then fails
with: `error: Special encoding failed for MVN: MVN requires two operands: srcbank,destbank`

## Fix Required

In `src/cpu/cpu_6502.cpp` `HasOpcode()` function:

Split the static set into CPU-mode-aware groups:
1. Base 6502 + 65C02 opcodes: always recognized (current base + 65c02 additions)
2. 65816-only opcodes: only recognized when `cpu_mode_ == CpuMode::Cpu65816`

65816-only opcodes to guard: MVN, MVP, COP, WDM, XBA, XCE, REP, SEP, PHB, PLB,
PHD, PLD, PHK, TCD, TCS, TDC, TSC, TXY, TYX, JML, JSL, RTL, PEA, PEI, PER

Note: BRA, STP, WAI, PHX, PLX, PHY, PLY, STZ, TRB, TSB are 65C02 additions
(not 65816-only), so they stay in the always-recognized set.

Rockwell extensions (BBR, BBS, RMB, SMB): already mode-guarded in EncodeInstruction
but also need to be excluded from HasOpcode() when not in Rockwell mode. Check what
`CpuMode::Cpu65C02Rock` means and guard appropriately.

## Implementation

```cpp
bool Cpu6502::HasOpcode(const std::string &mnemonic) const {
  std::string upper = mnemonic;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  // Base 6502 + 65C02 opcodes (always valid)
  static const std::unordered_set<std::string> base_opcodes = {
    // ... base 6502 ...
    // ... 65C02 additions ...
  };
  if (base_opcodes.count(upper)) return true;

  // 65816-only opcodes
  if (cpu_mode_ == CpuMode::Cpu65816) {
    static const std::unordered_set<std::string> opcodes_65816 = {
      M6502Mnemonics::MVN, M6502Mnemonics::MVP, ...
    };
    if (opcodes_65816.count(upper)) return true;
  }

  return false;
}
```

## Acceptance Criteria

cmake --build /Users/bryanw/Projects/Vintage/Apple/A2osX/build  (exits 0 - no MVN/MVP errors)
cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build  (exits 0)
ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build --output-on-failure  (exits 0, 1814+ tests pass)

## Related
Task packet: .ai/tasks/xasm-r67-TIMESTAMP-fix-hasopcode-cpu-mode/
