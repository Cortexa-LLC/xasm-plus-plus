# Work Log: Extract Common CPU Encoding Patterns

**Task:** 2026-02-09_common-encoding-patterns
**Beads ID:** xasm++-vray

## Progress Updates

### Session 2026-02-12 08:10 - Pattern Analysis

**Phase 1: Pattern Analysis Complete**

Examined CPU implementation files:
- `src/cpu/cpu_6502.cpp` (1556 lines)
- `src/cpu/cpu_z80.cpp` (557 lines)
- `src/cpu/cpu_6809.cpp` (1665 lines)

**Identified Common Patterns:**

1. **Endianness Conversion:**
   - 6502: Little-endian (inline: `value & 0xFF`, `(value >> 8) & 0xFF`)
   - Z80: Little-endian (method: `ToLittleEndian()` at line 127-128)
   - 6809: Big-endian (method: `ToBigEndian()` at line 29-32)
   
2. **Byte Extraction:**
   - All CPUs use: `value & 0xFF` (low byte), `(value >> 8) & 0xFF` (high byte)
   - 6502 also uses: `(value >> 16) & 0xFF` for 65816 24-bit addresses
   
3. **Vector Construction:**
   - All CPUs build `std::vector<uint8_t>` manually
   - Pattern: create vector, insert bytes
   - Z80 uses helper for prefix instructions (CB, DD, FD, ED)

**Duplication Metrics:**
- Endianness logic: Duplicated 3 times (inline in 6502, methods in Z80/6809)
- Bit manipulation (`& 0xFF`, `>> 8`): ~50+ instances across files
- Vector construction: Repeated in every encoding method

**Priority List:**
1. ✅ High: Endianness conversion (ToLittleEndian16, ToBigEndian16)
2. ✅ High: Byte extraction (LowByte, HighByte)
3. ✅ Medium: 24-bit support for 65816 (ToLittleEndian24)
4. ✅ Medium: Vector helpers (WithPrefix, WithPrefixes)

### Next: Phase 2 - Utility Design (TDD implementation)

### Session 2026-02-12 09:50 - Implementation Complete

**Phase 2: TDD Implementation Complete**

**Files Created:**
1. `include/xasm++/cpu/encoding_utils.h` (6459 bytes)
   - Complete header-only implementation
   - Constexpr functions where possible
   - Comprehensive documentation
   
2. `tests/unit/test_encoding_utils.cpp` (6915 bytes)
   - 28 unit tests using GTest
   - Edge case coverage (0x0000, 0xFFFF, etc.)
   - All tests passing ✓

**Implementation Details:**

**Endianness Conversion:**
- `ToLittleEndian16(uint16_t)` → `[low, high]`
- `ToBigEndian16(uint16_t)` → `[high, low]`
- `ToLittleEndian24(uint32_t)` → `[low, mid, high]` (for 65816)

**Byte Extraction:**
- `LowByte(uint16_t)` → low 8 bits
- `HighByte(uint16_t)` → high 8 bits
- `LowNibble(uint8_t)` → low 4 bits
- `HighNibble(uint8_t)` → high 4 bits (shifted)

**Range Validation:**
- `FitsIn8Bits(uint32_t)` → value ≤ 0xFF
- `FitsIn16Bits(uint32_t)` → value ≤ 0xFFFF
- `FitsInSignedByte(int32_t)` → -128 ≤ value ≤ 127

**Vector Helpers:**
- `WithPrefix(prefix, bytes)` → [prefix, bytes...]
- `WithPrefixes(prefixes, bytes)` → [prefixes..., bytes...]

**Test Results:**
```
[==========] Running 28 tests from 1 test suite.
[  PASSED  ] 28 tests.
```

**Quality Checks:**
- ✓ All tests passing (100%)
- ✓ Constexpr functions for compile-time evaluation
- ✓ Comprehensive docstrings with examples
- ✓ Edge cases tested (0x0000, 0xFFFF, boundary values)
- ✓ Header-only design (no library linking required)

### Status: Ready for Review

**Deliverables:**
1. ✓ Common encoding utilities extracted
2. ✓ Comprehensive test coverage
3. ✓ Documentation with usage examples
4. ✓ Clean, type-safe API

**Next Steps:**
- Phase 3: Refactor CPU implementations to use utilities (separate task)
- Phase 4: Measure code reduction impact (separate task)
