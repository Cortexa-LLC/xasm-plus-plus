# Task Contract: Replace Magic Values in edtasm_m80_plusplus_syntax

**Task ID:** 2026-02-13_edtasm-m80-magic-values
**Beads Task:** xasm++-tu1s
**Created:** 2026-02-13
**Owner:** Orchestrator → Engineer (delegated)
**Type:** Refactoring

---

## Problem Statement

The file `edtasm_m80_plusplus_syntax.cpp` contains magic numbers, strings, and characters without named constants, making the code harder to read and maintain.

**Examples of Magic Values:**
- `case '$': return 16;` - Hexadecimal radix prefix
- `case '%': return 2;` - Binary radix prefix
- `case '@': return 8;` - Octal radix prefix
- `text[1] == 'x'` - Hex prefix check
- `text[1] == 'b'` - Binary prefix check
- `text[1] == 'o'` - Octal prefix check

This creates:
- **Reduced readability** - Character literals lack semantic meaning
- **Maintenance burden** - Changes require finding all occurrences
- **Inconsistency** - Other parsers (like Rockwell) have been fixed

## Requirements

### Functional Requirements
- [ ] Add namespace with named constants for all magic values
- [ ] Replace all magic character literals with named constants
- [ ] Replace magic radix numbers with named constants
- [ ] Follow the Rockwell fix pattern (commit 7341e43)
- [ ] Maintain identical behavior (pure refactoring)
- [ ] All tests must continue passing

### Pattern to Follow

**Reference:** Commit 7341e43 (Rockwell magic values fix)

**Before:**
```cpp
case '$': return 16;
case '%': return 2;
if (text[1] == 'x') { ... }
```

**After:**
```cpp
namespace {
    // Numeric literal prefixes
    constexpr char HEX_PREFIX_DOLLAR = '$';
    constexpr char HEX_PREFIX_0X = 'x';
    constexpr char BIN_PREFIX_PERCENT = '%';
    constexpr char BIN_PREFIX_0B = 'b';
    constexpr char OCT_PREFIX_AT = '@';
    constexpr char OCT_PREFIX_0O = 'o';

    // Radix values
    constexpr int RADIX_BINARY = 2;
    constexpr int RADIX_OCTAL = 8;
    constexpr int RADIX_HEXADECIMAL = 16;

    // String delimiters
    constexpr char SINGLE_QUOTE = '\'';
    constexpr char DOUBLE_QUOTE = '"';
}

case HEX_PREFIX_DOLLAR: return RADIX_HEXADECIMAL;
case BIN_PREFIX_PERCENT: return RADIX_BINARY;
if (text[1] == HEX_PREFIX_0X) { ... }
```

## Magic Values to Replace

### Category 1: Radix Prefixes
| Location | Current | Replacement Constant |
|----------|---------|---------------------|
| Line 39 | `'$'` | `HEX_PREFIX_DOLLAR` |
| Line 45 | `'x'` | `HEX_PREFIX_0X` |
| Line 48 | `'%'` | `BIN_PREFIX_PERCENT` |
| Line 54 | `'b'` | `BIN_PREFIX_0B` |
| Line 57 | `'@'` | `OCT_PREFIX_AT` |
| Line 63 | `'o'` | `OCT_PREFIX_0O` |

### Category 2: Radix Values
| Location | Current | Replacement Constant |
|----------|---------|---------------------|
| Line 39 | `16` | `RADIX_HEXADECIMAL` |
| Line 48 | `2` | `RADIX_BINARY` |
| Line 57 | `8` | `RADIX_OCTAL` |

### Category 3: String Delimiters
| Location | Current | Replacement Constant |
|----------|---------|---------------------|
| Line 95 | `'\''` | `SINGLE_QUOTE` |
| Line 96 | `'"'` | `DOUBLE_QUOTE` |

### Category 4: Instruction Size Constants
| Location | Current | Replacement Constant |
|----------|---------|---------------------|
| Various | `1` | `INSTRUCTION_SIZE_SINGLE_BYTE` |
| Various | `2` | `INSTRUCTION_SIZE_TWO_BYTES` |
| Various | `3` | `INSTRUCTION_SIZE_THREE_BYTES` |

### Category 5: Instruction Mnemonics
| Location | Current | Replacement Constant |
|----------|---------|---------------------|
| Various | `"JR"` | `Z80Mnemonics::JR` |
| Various | `"DJNZ"` | `Z80Mnemonics::DJNZ` |
| Various | `"JP"` | `Z80Mnemonics::JP` |
| Various | `"CALL"` | `Z80Mnemonics::CALL` |
| Various | `"RST"` | `Z80Mnemonics::RST` |

**Note:** These mnemonics are already defined in `include/xasm++/cpu/opcodes_z80.h` under the `Z80Mnemonics` namespace, following the established pattern (similar to `RockwellMnemonics` in commit 7341e43).

### Category 6: Operand Delimiters
| Location | Current | Replacement Constant |
|----------|---------|---------------------|
| Various | `","` | `OPERAND_DELIMITER` |

## Success Criteria

- [ ] All magic values replaced with named constants
- [ ] Constants defined in anonymous namespace at file top
- [ ] All edtasm_m80_plusplus tests passing (100%)
- [ ] All total tests passing (100%)
- [ ] Code compiles without warnings
- [ ] Pattern matches Rockwell fix (commit 7341e43)
- [ ] Single atomic commit

## Constraints

- **No functional changes** - Pure refactoring
- **No test changes** - Tests should not need modification
- **Follow established pattern** - Match Rockwell fix exactly
- **Single commit** - Atomic refactoring for easy review/revert

## Estimated Effort

2-3 hours (straightforward refactoring, ~20 replacements)

---

## Additional Magic Values (Lower Priority)

Deep review of the file revealed additional magic values that could be refactored in future work:
- Radix suffix characters: `'H'`, `'h'`, `'O'`, `'o'`, `'Q'`, `'q'`, `'B'`, `'b'`, `'D'`, `'d'` (lines 70-854)
- Comment delimiter: `';'` (line 343)
- Escape sequences: `'n'`, `'r'`, `'t'`, `'\\'` (lines 1173-1196)
- Macro parameter prefixes: `'&'`, `'%'` (lines 920, 929)
- Label/identifier delimiters: `':'`, `'_'`, `'.'`, `'?'` (lines 759-1016)
- Date/time calculation literals: `1900`, `10000`, `100` (lines 294-300)
- Default radix: `10` (line 281)
- Control strings: `"ON"`, `"OFF"` (lines 1999-2016)

These are **not included in this task** because:
- Some are already defined (radix prefixes in anonymous namespace lines 27-38)
- Many are clear in context (e.g., character arithmetic `c - '0'`)
- Lower impact on readability than instruction-related magic values

**Consider for future refactoring if needed.**

---

**Status:** READY FOR IMPLEMENTATION
