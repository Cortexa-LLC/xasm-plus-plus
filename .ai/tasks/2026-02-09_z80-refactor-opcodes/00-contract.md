# Task Contract: Refactor Z80 Implementation - Remove Magic Numbers

**Task ID:** 2026-02-09_z80-refactor-opcodes
**Beads Task:** xasm++-3syi
**Created:** 2026-02-09

Replace ~100 magic numbers in `src/cpu/z80/cpu_z80.cpp` with named constants from `opcodes_z80.h`.

## Success Criteria
✓ Zero magic numbers in Z80 implementation
✓ All opcodes use named constants from opcodes_z80.h
✓ All existing tests pass
✓ Zero compiler warnings
✓ Code compiles successfully

## Acceptance Criteria
□ Add `#include "xasm++/cpu/opcodes_z80.h"` to implementation
□ Replace all magic number opcodes with named constants (~100 replacements)
□ Update CB-prefixed opcodes to use opcodes::cb::* constants
□ Update ED-prefixed opcodes to use opcodes::ed::* constants
□ Update DD/FD-prefixed opcodes to use opcodes::dd::*, opcodes::fd::* constants
□ Verify no hex literals remain (grep for opcode patterns)
□ All tests pass after refactoring
□ Zero compiler warnings

**Approved:** Orchestrator 2026-02-09
