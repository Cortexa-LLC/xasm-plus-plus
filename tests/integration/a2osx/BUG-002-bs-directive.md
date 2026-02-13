# BUG-002: .BS Directive Misinterpreted as Binary String

**Priority:** P1 - HIGH
**Status:** Open
**Discovered:** 2026-02-12
**Impact:** Blocks 280 usages (2.9% of A2oSX directives)

## Summary

The `.BS` (Block Storage) directive currently interprets its argument as a binary digit string (0/1 only) instead of a byte count for space reservation.

## Description

In SCMASM syntax, `.BS N` should reserve N bytes of space (either by emitting N zero bytes or reserving space without emission). Currently, xasm++ treats the argument as a binary number and fails if it contains digits other than 0 or 1.

## Minimal Reproduction

```assembly
* File: bug002_repro.S
        .OP 65C02
        .OR $3000
        
BUFFER  .BS 256    ; Reserve 256 bytes
```

### Steps to Reproduce

```bash
$ ./build/src/xasm++ --syntax scmasm --cpu 65c02 bug002_repro.S -o output.bin

error: Invalid binary digit: 2
```

## Expected Behavior

`.BS N` should reserve N bytes of space:

```assembly
BUFFER  .BS 256    ; Reserve 256 bytes

; Should result in one of:
; Option A: Emit 256 zero bytes (256 bytes in output file)
; Option B: Reserve space without emission (update location counter by 256)
```

### Usage in A2oSX

```assembly
* Real A2oSX examples:
BUFFER  .BS 256            ; I/O buffer
STACK   .BS $100           ; Stack space (256 bytes)
TABLE   .BS MAXENTRIES*4   ; Table with calculated size
```

## Actual Behavior

The directive treats its argument as binary digits:
- ❌ `.BS 256` - Error: "Invalid binary digit: 2"
- ❌ `.BS $100` - Error: "Invalid binary digit: $"
- ✅ `.BS 10` - Works but means 2 decimal (binary 10), not 10 bytes!
- ✅ `.BS 1` - Works but means 1 byte

## Analysis

### Current Implementation

The `.BS` directive appears to be implemented as **B**inary **S**tring, parsing a string of 0s and 1s:

```assembly
DATA .BS 10110101    ; Intended: Parse as binary = 181 decimal
                     ; But SCMASM uses .BS for Block Storage!
```

### SCMASM Semantics

In SCMASM (and most 6502 assemblers), `.BS` means **B**lock **S**torage:

```assembly
BUFFER .BS 256       ; Reserve 256 bytes of space
```

### Conflict

These are two completely different directives with the same mnemonic:
- **Binary String** (current): Convert binary digits to bytes
- **Block Storage** (needed): Reserve N bytes of space

## Impact Assessment

**Severity:** HIGH

1. **A2oSX Impact:**
   - 280 uses of `.BS` across source files
   - Required for buffers, stacks, tables
   - Workaround: Use `.DA` with 256 repeated zeros (very verbose)

2. **Frequency:**
   - A2oSX analysis: 9,695 directives total
   - 280 `.BS` directives = 2.9%
   - Common pattern in system code

3. **Use Cases:**
   - I/O buffers (256-512 bytes typical)
   - Stack space (256 bytes = $100)
   - Screen buffers (1024+ bytes)
   - Data tables with computed sizes

## Suggested Fix

### Option 1: Rename Current .BS (Recommended)

1. Rename current `.BS` directive to `.BIN` (Binary Number)
2. Implement new `.BS` as Block Storage
3. Update documentation

```assembly
* Binary number (old .BS behavior)
VALUE .BIN 10110101    ; = 181 decimal

* Block storage (new .BS behavior, SCMASM-compatible)
BUFFER .BS 256         ; Reserve 256 bytes
```

### Option 2: Add New Directive

1. Keep `.BS` as Binary String
2. Add `.DS` (Define Storage) for block storage
3. Document the difference

```assembly
DATA   .BS 10110101    ; Binary string = 181 decimal
BUFFER .DS 256         ; Reserve 256 bytes
```

**Recommendation:** Option 1, because:
- SCMASM compatibility is primary goal
- `.BS` as Block Storage is industry standard
- Binary string can use clearer name (`.BIN`, `.BINARY`)

### Implementation

```cpp
// Current (incorrect for SCMASM):
void assemble_BS(const string& arg) {
    // Parse as binary digits 0/1
    for (char c : arg) {
        if (c != '0' && c != '1') error("Invalid binary digit");
        emit_byte(c - '0');
    }
}

// Correct for SCMASM:
void assemble_BS(const string& arg) {
    // Parse as byte count
    int count = evaluate_expression(arg);
    
    // Option A: Emit zeros
    for (int i = 0; i < count; i++) {
        emit_byte(0x00);
    }
    
    // Option B: Just advance location counter
    // (if assembler supports BSS-style reservation)
    location_counter += count;
}
```

### Test Cases to Add

```cpp
TEST(Directives, BlockStorage_Small) {
    const char* source = R"(
        .OR $2000
BUFFER  .BS 10
    )";
    // Should emit 10 zero bytes or advance LC by 10
    // Verify: location counter = $200A
}

TEST(Directives, BlockStorage_Large) {
    const char* source = R"(
        .OR $3000
BUFFER  .BS 256
    )";
    // Should emit 256 zero bytes
    // Verify: location counter = $3100
}

TEST(Directives, BlockStorage_Hex) {
    const char* source = R"(
        .OR $2000
BUFFER  .BS $100
    )";
    // Should emit 256 ($100) zero bytes
    // Verify: location counter = $2100
}

TEST(Directives, BlockStorage_Expression) {
    const char* source = R"(
COUNT   .EQ 10
        .OR $2000
BUFFER  .BS COUNT*4
    )";
    // Should emit 40 zero bytes
    // Verify: location counter = $2028
}
```

## Alternative: Support Both Syntaxes

Some assemblers use context to distinguish:
- `.BS 10110101` (all 0/1) → Binary string
- `.BS 256` (has 2-9) → Block storage

**Not recommended:** Ambiguous and error-prone.

## Related Directives

SCMASM storage directives:
- `.BS` - Block Storage (this bug)
- `.DA` - Define Address/Data
- `.AS` - ASCII String ✅ Working
- `.HS` - Hex String ✅ Working

## Verification

Once fixed, verify with:

```bash
cd tests/integration/a2osx
./validation_suite.py --all

# Should see test_bs.S pass
# Should see sample_simple.S improve (one less blocker)
```

## References

- A2oSX source: Uses `.BS` for buffers throughout
- SCMASM documentation: `.BS N` reserves N bytes
- Industry standard: Most assemblers use `.BS`/`.DS` for block storage
- Alternative names: `.RES` (reserve), `.BLKB` (block bytes), `.DSB` (define space bytes)

---

**Reporter:** Validation Engineer
**Date:** 2026-02-12
**Version:** xasm++ 0.1.0
