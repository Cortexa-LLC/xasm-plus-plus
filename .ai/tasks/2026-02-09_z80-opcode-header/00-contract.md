# Task Contract: Create Z80 Opcode Header

**Task ID:** 2026-02-09_z80-opcode-header
**Beads Task:** xasm++-iv0o
**Created:** 2026-02-09

Create `include/xasm++/cpu/opcodes_z80.h` with all Z80 opcode constants following the 6502 pattern.

## Success Criteria
✓ ~150 Z80 opcodes defined as named constants
✓ Organized by category (basic, CB-prefix, ED-prefix, DD/FD-prefix)
✓ Zero magic numbers
✓ Follows opcodes_6502.h pattern

## Acceptance Criteria
□ Basic opcodes (~80): LD, ADD, SUB, INC, DEC, etc.
□ CB-prefixed opcodes (~40): BIT, SET, RES, RLC, RRC, etc.
□ ED-prefixed opcodes (~30): Extended instructions
□ DD-prefixed opcodes: IX register operations
□ FD-prefixed opcodes: IY register operations
□ Namespace organization: opcodes::*, opcodes::cb::*, opcodes::ed::*
□ Inline documentation for each opcode

**Approved:** Orchestrator 2026-02-09
