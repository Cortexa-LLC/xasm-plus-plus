# Implementation Plan: Refactor Z80 Implementation

**Task:** Refactor Z80 Implementation - Remove Magic Numbers
**Approach:** Systematic replacement of magic numbers with named constants

## Strategy

Replace all magic number opcodes in `src/cpu/z80/cpu_z80.cpp` with named constants from the newly created `opcodes_z80.h`.

## Phases

1. **Add Include** - Add opcodes_z80.h include
2. **Basic Opcodes** - Replace basic opcode magic numbers
3. **CB-Prefixed** - Replace CB-prefix opcodes
4. **ED-Prefixed** - Replace ED-prefix opcodes
5. **DD/FD-Prefixed** - Replace IX/IY register opcodes
6. **Verify** - Run tests, check for remaining magic numbers
7. **Build** - Verify zero warnings

## Estimated: 3 hours
