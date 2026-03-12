# Task Contract: Extract Common CPU Encoding Patterns

**Task ID:** 2026-02-09_common-encoding-patterns
**Beads Task:** xasm++-vray
**Created:** 2026-02-09
**Priority:** P2

Identify and extract common encoding patterns across CPU families (6502, Z80, 6809) into shared utilities.

## Problem Statement

Current CPU implementations have duplicated patterns:
- **Endianness conversion** (little-endian vs big-endian)
- **Multi-byte operand encoding** (16-bit addresses, 16-bit immediates)
- **Bit manipulation** (mask, shift, extract nibbles)
- **Range validation** (8-bit, 16-bit, signed/unsigned)
- **Prefix handling** (Z80: CB/ED/DD/FD prefixes)

This duplication:
- Violates DRY principle
- Increases maintenance burden
- Creates inconsistency risk
- Makes bug fixes require multiple changes

## Objective

Extract common encoding patterns into reusable utilities in `include/xasm++/cpu/encoding_utils.h`.

## Success Criteria

✓ Common patterns identified across all CPU implementations
✓ Shared utilities created and documented
✓ All CPU implementations use shared utilities (zero duplication)
✓ All tests pass after refactoring
✓ Zero behavioral changes (pure refactoring)
✓ Code coverage maintained or improved

## Acceptance Criteria

### Phase 1: Pattern Analysis (Investigation)
- [ ] Analyze Cpu6502, CpuZ80, Cpu6809 implementations
- [ ] Identify duplicated logic patterns
- [ ] Categorize patterns by type
- [ ] Document current duplication metrics
- [ ] Create extraction priority list

### Phase 2: Utility Design
- [ ] Design `encoding_utils.h` API
- [ ] Define utility functions with clear interfaces
- [ ] Document each utility's purpose and usage
- [ ] Ensure type safety (constexpr where possible)
- [ ] Write unit tests for utilities

### Phase 3: Implementation (TDD)
- [ ] Create `include/xasm++/cpu/encoding_utils.h`
- [ ] Create `tests/unit/test_encoding_utils.cpp`
- [ ] Implement utilities with tests (RED-GREEN-REFACTOR)
- [ ] All utility tests passing

### Phase 4: Integration (Incremental Refactoring)
- [ ] Refactor Cpu6502 to use utilities
- [ ] Verify Cpu6502 tests still pass
- [ ] Refactor CpuZ80 to use utilities
- [ ] Verify CpuZ80 tests still pass
- [ ] Refactor Cpu6809 to use utilities
- [ ] Verify Cpu6809 tests still pass

### Phase 5: Verification
- [ ] All CPU tests passing (6502, Z80, 6809)
- [ ] Zero compiler warnings
- [ ] Code duplication metrics improved
- [ ] Documentation complete

## Common Patterns to Extract

### 1. Endianness Conversion

**Current:**
```cpp
// Cpu6502 (little-endian)
std::vector<uint8_t> ToLittleEndian(uint16_t value) const {
  return {static_cast<uint8_t>(value & 0xFF),
          static_cast<uint8_t>((value >> 8) & 0xFF)};
}

// Cpu6809 (big-endian)
std::vector<uint8_t> ToBigEndian(uint16_t value) const {
  return {static_cast<uint8_t>((value >> 8) & 0xFF),
          static_cast<uint8_t>(value & 0xFF)};
}
```

**Proposed:**
```cpp
namespace xasm::encoding {
  constexpr std::array<uint8_t, 2> ToLittleEndian16(uint16_t value);
  constexpr std::array<uint8_t, 2> ToBigEndian16(uint16_t value);
}
```

### 2. Byte Extraction

**Current:** Scattered bit masks and shifts

**Proposed:**
```cpp
namespace xasm::encoding {
  constexpr uint8_t LowByte(uint16_t value);
  constexpr uint8_t HighByte(uint16_t value);
  constexpr uint8_t LowNibble(uint8_t value);
  constexpr uint8_t HighNibble(uint8_t value);
}
```

### 3. Range Validation

**Current:** Ad-hoc checks in implementations

**Proposed:**
```cpp
namespace xasm::encoding {
  constexpr bool FitsIn8Bits(uint32_t value);
  constexpr bool FitsIn16Bits(uint32_t value);
  constexpr bool FitsInSignedByte(int32_t value);
}
```

### 4. Prefix Handling (Multi-byte opcodes)

**Current:** Manual vector construction

**Proposed:**
```cpp
namespace xasm::encoding {
  std::vector<uint8_t> WithPrefix(uint8_t prefix, std::initializer_list<uint8_t> bytes);
  std::vector<uint8_t> WithPrefixes(std::initializer_list<uint8_t> prefixes, std::initializer_list<uint8_t> bytes);
}
```

## Metrics

**Before Extraction:**
- Code duplication: ~15-20% across CPU implementations
- Endianness logic: Duplicated 3 times
- Bit manipulation: Scattered across files

**After Extraction:**
- Code duplication: <5%
- Endianness logic: Single source of truth
- Bit manipulation: Centralized and tested
- ~100-150 LOC reduction across implementations

## Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Breaking existing behavior | High | Comprehensive tests, incremental refactoring |
| Performance regression | Low | Use constexpr for compile-time evaluation |
| API design wrong | Medium | Start with simple utilities, iterate based on usage |

## Files to Create/Modify

**New Files:**
- `include/xasm++/cpu/encoding_utils.h` (utilities)
- `tests/unit/test_encoding_utils.cpp` (unit tests)

**Modified Files:**
- `src/cpu/m6502/cpu_6502.cpp` (use utilities)
- `src/cpu/z80/cpu_z80.cpp` (use utilities)
- `src/cpu/m6809/cpu_6809.cpp` (use utilities)
- `include/xasm++/cpu/cpu_6502.h` (remove ToLittleEndian)
- `include/xasm++/cpu/cpu_z80.h` (remove ToLittleEndian)
- `include/xasm++/cpu/cpu_6809.h` (remove ToBigEndian)

## Estimated Effort

- Pattern analysis: 1 hour
- Utility design: 1 hour
- Implementation (TDD): 2-3 hours
- Integration refactoring: 2-3 hours
- Verification: 1 hour

**Total:** 7-9 hours

## Dependencies

- None (can run in parallel with other architectural work)

## Stakeholders

- **Owner:** Orchestrator
- **Implementer:** Engineer
- **Reviewer:** Code quality review

**Approved:** Orchestrator 2026-02-09
