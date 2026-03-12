# Common CPU Encoding Patterns - Task Summary

**Task ID:** 2026-02-09_common-encoding-patterns
**Status:** ✅ COMPLETE
**Date:** 2026-02-12

## Overview

Extracted common encoding patterns from CPU implementations (6502, Z80, 6809) into a reusable header-only utility library. This reduces code duplication and provides a consistent API for byte manipulation across all CPU encoders.

## Deliverables

### 1. Encoding Utilities Header
**File:** `include/xasm++/cpu/encoding_utils.h`

Header-only library providing:
- Endianness conversion (little-endian, big-endian, 24-bit)
- Byte extraction (low/high byte, nibbles)
- Range validation (8-bit, 16-bit, signed byte)
- Vector construction helpers (prefix handling)

**Design Decisions:**
- Header-only: No linking required, easy integration
- Constexpr: Compile-time evaluation where possible
- Type-safe: Uses uint8_t, uint16_t, uint32_t appropriately
- Well-documented: Examples and explanations for each function

### 2. Comprehensive Test Suite
**File:** `tests/unit/test_encoding_utils.cpp`

- 28 unit tests covering all functions
- Edge case testing (boundary values, empty inputs)
- 100% API coverage
- All tests passing ✓

## Impact Analysis

**Code Duplication Identified:**
- Endianness logic: 3 implementations (inline in 6502, methods in Z80/6809)
- Bit manipulation: 50+ instances of `& 0xFF`, `>> 8` patterns
- Vector construction: Repeated in every encoding method

**Estimated Reduction (Future Refactoring):**
- 100-200 lines of duplicated code
- Improved consistency across CPU implementations
- Easier to add new CPUs (reuse utilities)

## Usage Examples

### Endianness Conversion
```cpp
#include "xasm++/cpu/encoding_utils.h"
using namespace xasm::encoding;

// Little-endian (6502, Z80)
auto bytes = ToLittleEndian16(0x1234);  // → [0x34, 0x12]

// Big-endian (6809)
auto bytes = ToBigEndian16(0x1234);     // → [0x12, 0x34]

// 24-bit for 65816
auto bytes = ToLittleEndian24(0x123456); // → [0x56, 0x34, 0x12]
```

### Byte Extraction
```cpp
uint16_t addr = 0x1234;
uint8_t low = LowByte(addr);   // 0x34
uint8_t high = HighByte(addr); // 0x12

uint8_t byte = 0x5A;
uint8_t lo_nibble = LowNibble(byte);  // 0x0A
uint8_t hi_nibble = HighNibble(byte); // 0x05
```

### Range Validation
```cpp
if (!FitsIn8Bits(value)) {
    return Error("Value out of range for 8-bit operand");
}

if (!FitsInSignedByte(offset)) {
    return Error("Displacement too large for relative addressing");
}
```

### Vector Helpers
```cpp
// Z80 prefix instructions
return WithPrefix(0xCB, {opcode});  // CB prefix for bit ops
return WithPrefix(0xDD, {0x21, lo, hi});  // IX register prefix

// Multiple prefixes (DDCB for IX bit ops)
return WithPrefixes({0xDD, 0xCB}, {offset, opcode});
```

## Integration

The utilities are ready to use. To integrate into CPU implementations:

1. Include the header: `#include "xasm++/cpu/encoding_utils.h"`
2. Use namespace: `using namespace xasm::encoding;`
3. Replace manual bit manipulation with utility functions

**Example Refactoring:**
```cpp
// Before:
std::vector<uint8_t> bytes = {opcode};
bytes.push_back(value & 0xFF);
bytes.push_back((value >> 8) & 0xFF);

// After:
auto addr_bytes = ToLittleEndian16(value);
return WithPrefix(opcode, addr_bytes);
```

## Testing

```bash
# Build and run tests
cd build
make test_encoding_utils
./tests/unit/test_encoding_utils

# Expected output:
# [==========] Running 28 tests from 1 test suite.
# [  PASSED  ] 28 tests.
```

## Next Steps (Future Tasks)

1. **Refactor 6502 encoder** to use encoding_utils
2. **Refactor Z80 encoder** to use encoding_utils (remove ToLittleEndian methods)
3. **Refactor 6809 encoder** to use encoding_utils (remove ToBigEndian methods)
4. **Measure impact** - before/after line counts and duplication metrics

## Files Modified

- `include/xasm++/cpu/encoding_utils.h` (created)
- `tests/unit/test_encoding_utils.cpp` (created)
- `tests/unit/CMakeLists.txt` (updated)

## Quality Assurance

- ✅ All tests passing (28/28)
- ✅ Zero build warnings
- ✅ Comprehensive documentation
- ✅ Header-only design
- ✅ Constexpr optimization
- ✅ Type safety enforced

---

**Task Complete:** Ready for code review and integration into CPU implementations.
