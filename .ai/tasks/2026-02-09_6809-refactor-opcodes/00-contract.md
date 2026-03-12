# Task Contract: Refactor 6809 Implementation - Remove Magic Numbers

**Task ID:** 2026-02-09_6809-refactor-opcodes
**Beads Task:** xasm++-jgvi
**Created:** 2026-02-09

Replace ~100 magic numbers in `src/cpu/m6809/cpu_6809.cpp` with named constants from `opcodes_6809.h`.

## Success Criteria
✓ Zero magic numbers in 6809 implementation
✓ All opcodes use named constants from opcodes_6809.h
✓ All existing tests pass
✓ Zero compiler warnings
✓ 3-page structure properly used

## Acceptance Criteria
□ Add `#include "xasm++/cpu/opcodes_6809.h"` to implementation
□ Replace page 1 opcodes with named constants
□ Replace page 2 opcodes ($10 prefix) with opcodes::page2::* constants
□ Replace page 3 opcodes ($11 prefix) with opcodes::page3::* constants
□ Update indexed addressing to use opcodes::indexed::* constants
□ Verify no hex literals remain
□ All tests pass after refactoring
□ Zero compiler warnings

**Approved:** Orchestrator 2026-02-09
