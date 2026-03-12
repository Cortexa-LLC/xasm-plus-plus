# Implementation Plan: Z80 Phase 6 - Macro System

**Task:** Z80 Phase 6 - Macro System
**Approach:** Leverage existing MacroProcessor from FLEX ASM

## Strategy

Integrate existing MacroProcessor class with Z80 Universal Syntax parser using TDD methodology.

## Files

**Modify:**
- `include/xasm++/syntax/z80_universal_syntax.h` - Add macro processor member
- `src/syntax/z80_universal/z80_universal_syntax.cpp` - Implement handlers
- `tests/unit/test_z80_universal_syntax.cpp` - Add 30+ macro tests

## Phases

1. **MACRO/ENDM** (TDD) - Basic definition and invocation
2. **Parameter Substitution** (TDD) - &param, %param, positional
3. **LOCAL** (TDD) - Local label uniquification
4. **EXITM** (TDD) - Early exit from macro
5. **REPT/IRP/IRPC** (TDD) - Repetition and iteration
6. **Nested Macros** (TDD) - Macro context stacking
7. **Refactor** - Clean up, verify zero warnings

## Estimated: 4-5 hours
