# Task Contract: Z80 Syntax Use Mnemonic Constants

**Task ID:** 2026-02-10_z80-use-mnemonic-constants
**Beads Task:** xasm++-xvlw
**Created:** 2026-02-10
**Priority:** P2

Replace string literals in Z80 syntax parser with mnemonic constants.

## Problem Statement

Z80 syntax parser uses string literals like `"LD"`, `"ADD"`, `"SUB"` instead of constants from `Z80Mnemonics` namespace.

## Objective

Replace all instruction name string literals with constants from `opcodes_z80.h`.

## Success Criteria

✓ All string literals replaced with Z80Mnemonics constants
✓ Zero magic strings for instruction names
✓ All tests pass
✓ Zero compiler warnings

## Implementation

```cpp
// Before:
if (mnemonic == "LD") { ... }

// After:
#include "xasm++/cpu/opcodes_z80.h"
using namespace xasm::Z80Mnemonics;
if (mnemonic == LD) { ... }
```

## Estimated Effort

2-3 hours

**Approved:** Orchestrator 2026-02-10
