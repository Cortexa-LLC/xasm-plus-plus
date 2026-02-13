# A2oSX Build Validation Test Suite

## Overview

This directory contains validation tests for building the A2oSX Operating System with xasm++. A2oSX is a Unix-like operating system for the Apple II series, consisting of approximately 190 source files written in 65C02 assembly using SCMASM syntax.

**Goal:** Validate that xasm++ can serve as the official build tool for A2oSX.

## Current Status

### ✅ Working Features (5/5 tests pass)

1. **Basic Assembly** - `.OP` and `.OR` directives work correctly
2. **Symbol Definition** - `.EQ` directive defines constants
3. **ASCII Strings** - `.AS` directive embeds ASCII text
4. **Hex Data** - `.HS` directive embeds hex bytes
5. **Data Bytes** - `.DA` directive defines byte arrays

### ❌ Blocking Issues

Three critical bugs prevent A2oSX assembly:

#### BUG-001: Symbol Resolution in Addressing Modes

**Impact:** CRITICAL - Prevents assembly of 99% of A2oSX code

**Description:** Symbols defined with `.EQ` work in immediate mode (`LDA #VALUE`) but fail in all other addressing modes.

**Example:**
```assembly
ZPTEMP  .EQ $80

LDA #ZPTEMP      ; ✅ Works - immediate mode
STA ZPTEMP       ; ❌ Fails - "Unsupported instruction: ZPTEMP"
LDA ZPTEMP,X     ; ❌ Fails - indexed mode
```

**Required Fix:** Symbol resolution must work in all addressing modes:
- Zero page: `LDA $80`, `STA ZPTEMP`
- Absolute: `LDA $2000`, `JMP START`
- Indexed: `LDA $80,X`, `STA ZPTEMP,Y`
- Indirect: `JMP (VECTOR)`, `LDA (ZPPTR),Y`

**Priority:** P0 - Blocking all real-world assembly

---

#### BUG-002: .BS Directive Misinterpretation

**Impact:** HIGH - `.BS` used 280 times (2.9% of A2oSX directives)

**Description:** `.BS` (Block Storage) currently interprets input as binary digits (0/1) instead of byte count.

**Example:**
```assembly
BUFFER  .BS 256    ; ❌ Fails - "Invalid binary digit: 2"
                   ; Should reserve 256 bytes
```

**Expected Behavior:**
- `.BS N` should reserve N bytes of space
- Implementation options:
  - Emit N zero bytes
  - Reserve space without emission (if supported)

**Priority:** P1 - Workaround possible but requires significant code changes

---

#### BUG-003: Expression Support

**Impact:** HIGH - Required for pointer manipulation and multi-byte addressing

**Description:** Complex expressions not supported:
- Low byte: `#<LABEL`
- High byte: `#>LABEL`  
- Offset: `LABEL+N`

**Example:**
```assembly
MESSAGE .AS "Hello"
ZPPTR   .EQ $80

LDA #<MESSAGE      ; ❌ Fails - low byte expression
STA ZPPTR
LDA #>MESSAGE      ; ❌ Fails - high byte expression
STA ZPPTR+1        ; ❌ Fails - offset expression
```

**Required Fix:** Support standard 6502 assembler expressions:
- `<` operator: Extract low byte (LSB) of 16-bit address
- `>` operator: Extract high byte (MSB) of 16-bit address
- `+` operator: Add offset to symbol value
- `-` operator: Subtract offset from symbol value

**Priority:** P1 - Essential for 16-bit address manipulation

---

## Test Suite Usage

### Quick Start

```bash
# Run tests that should pass with current xasm++
cd tests/integration/a2osx
./validation_suite.py

# Run all tests (including blocked ones)
./validation_suite.py --all

# Verbose output
./validation_suite.py --verbose

# Specify xasm++ location
./validation_suite.py --xasm /path/to/xasm++
```

### Expected Output

```
A2oSX Build Validation Suite
xasm++: ../../../build/src/xasm++
Tests: 11
================================================================================

[PASS] minimal_assembly: Minimal assembly with .OP and .OR
[PASS] eq_directive: Symbol definition with .EQ
[SKIP] symbol_addressing: Using .EQ symbols in addressing modes
[PASS] ascii_string: ASCII string with .AS directive
[PASS] hex_data: Hex data with .HS directive
[PASS] data_bytes: Data bytes with .DA directive
[SKIP] block_storage: Reserve space with .BS directive
[SKIP] expression_low_byte: Low byte expression #<LABEL
[SKIP] expression_high_byte: High byte expression #>LABEL
[SKIP] expression_offset: Offset expression LABEL+N
[SKIP] a2osx_simple: Simplified A2oSX-style code

================================================================================
Summary:
  Total:   5
  Passed:  5
  Failed:  0
  Blocked: 0
  Skipped: 0
  Pass rate: 100.0%
```

## Test Files

### Level 1: Basic Features (Working)

- `test_minimal.S` - Minimal assembly (`.OP`, `.OR`, basic instructions)
- `test_eq.S` - Symbol definition with `.EQ` in immediate mode
- `test_data_as.S` - ASCII strings with `.AS`
- `test_data_hs.S` - Hex data with `.HS`
- `test_data_da.S` - Data bytes with `.DA`

### Level 2-5: Advanced Features (Blocked)

- `test_symbols.S` - Symbol usage in addressing modes (BUG-001)
- `test_bs.S` - Block storage directive (BUG-002)
- `test_expr_lo.S` - Low byte expression (BUG-003)
- `test_expr_hi.S` - High byte expression (BUG-003)
- `test_expr_offset.S` - Offset expression (BUG-003)

### Level 6: Real-World Code (Blocked)

- `sample_simple.S` - Simplified A2oSX-style code (multiple bugs)

## Manual Testing

```bash
# Test a single file
./build/src/xasm++ --syntax scmasm --cpu 65c02 \
  tests/integration/a2osx/test_minimal.S \
  -o test_minimal.bin

# Verify binary output
xxd test_minimal.bin
```

## Directory Structure

```
tests/integration/a2osx/
├── README.md               # This file
├── validation_suite.py     # Automated test runner
├── test_minimal.S          # Basic assembly test
├── test_eq.S              # .EQ directive test
├── test_data_as.S         # .AS directive test
├── test_data_hs.S         # .HS directive test
├── test_data_da.S         # .DA directive test
├── test_symbols.S         # Symbol addressing (blocked)
├── test_bs.S              # Block storage (blocked)
├── test_expr_lo.S         # Low byte expression (blocked)
├── test_expr_hi.S         # High byte expression (blocked)
├── test_expr_offset.S     # Offset expression (blocked)
└── sample_simple.S        # Real A2oSX-style code (blocked)
```

## Validation Criteria

For xasm++ to become the official A2oSX build tool:

### Must Have (P0)
- ✅ SCMASM syntax support
- ✅ 65C02 instruction set
- ✅ Basic directives (`.OP`, `.OR`, `.EQ`, `.AS`, `.HS`, `.DA`)
- ❌ **Symbol resolution in all addressing modes** (BUG-001)

### Should Have (P1)
- ❌ **Block storage directive `.BS`** (BUG-002)
- ❌ **Expression support (`<`, `>`, `+`, `-`)** (BUG-003)
- ⚠️ Macro support (`.MA`, `.ENDM`)
- ⚠️ Conditional assembly (`.DO`, `.FIN`)

### Nice to Have (P2)
- ⚠️ Listing output (`.LIST`)
- ⚠️ Cross-reference generation
- ⚠️ Multiple source file support (`.INB`)

## Next Steps

### For xasm++ Developers

1. **Fix BUG-001** (Symbol Resolution) - This is the most critical blocker
   - Audit symbol table lookup in addressing mode parsing
   - Ensure symbols work in: ZP, ABS, indexed, indirect modes
   - Add unit tests for each addressing mode

2. **Fix BUG-002** (`.BS` Directive)
   - Change `.BS` from binary-string to block-storage
   - Should emit N zero bytes (or reserve space)
   - Add test: `.BS 256` should produce 256 bytes

3. **Fix BUG-003** (Expression Support)
   - Implement `<` operator (low byte extraction)
   - Implement `>` operator (high byte extraction)
   - Implement `+`/`-` operators (offset arithmetic)
   - Add expression evaluator tests

4. **Validate with Real A2oSX Code**
   - Clone A2oSX repository
   - Attempt to build each of 190 source files
   - Document any additional issues
   - Create regression test suite

### For A2oSX Maintainers

Current recommendation: **Wait for P0 bug fixes before migration**

The blocking issues (especially BUG-001) make xasm++ unsuitable for A2oSX assembly at this time. Once the three critical bugs are fixed, we can:

1. Re-run validation suite (should see 100% pass rate)
2. Test build of full A2oSX source tree
3. Compare binary output with current assembler
4. Document migration guide
5. Update A2oSX build system

## Bug Reports

Formal bug reports should be filed with:
- Bug title and description
- Impact assessment
- Minimal reproduction case
- Expected vs. actual behavior
- Suggested fix (if known)

## References

- **A2oSX Project:** [GitHub](https://github.com/burniouf/A2osX)
- **SCMASM Syntax:** Original assembler documentation
- **65C02 Reference:** WDC W65C02S datasheet
- **xasm++ Documentation:** `docs/` directory

## Maintenance

This validation suite should be updated when:
- New A2oSX source patterns are identified
- Bugs are fixed (move tests from "blocked" to "working")
- Additional directives are implemented
- Edge cases are discovered

**Last Updated:** 2026-02-12
**Maintainer:** xasm++ project team
**Status:** Initial validation complete, awaiting bug fixes
