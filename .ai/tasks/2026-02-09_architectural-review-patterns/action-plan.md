# Action Plan: Addressing Architectural Issues in xasm++

**Date:** 2026-02-09
**Status:** Proposed
**Priority:** High
**Estimated Total Effort:** 36-46 hours

---

## Overview

This action plan addresses the architectural issues identified in the comprehensive code review, with primary focus on the **opcode definition inconsistency** between CPU plugins.

---

## High Priority Actions (Complete within 2 weeks)

### Action 1: Create Z80 Opcode Header

**Priority:** 🔴 HIGH
**Effort:** 4 hours
**Dependencies:** None
**Assignee:** TBD

**Description:**
Create `include/xasm++/cpu/opcodes_z80.h` containing all Z80 opcode constants.

**Tasks:**
1. Create header file structure
2. Document basic opcodes (LD, ADD, SUB, etc.) - ~80 opcodes
3. Document CB-prefixed opcodes (bit operations) - ~40 opcodes
4. Document ED-prefixed opcodes (extended instructions) - ~30 opcodes
5. Document DD-prefixed opcodes (IX register operations)
6. Document FD-prefixed opcodes (IY register operations)
7. Add inline comments describing each instruction
8. Organize by instruction category

**Deliverables:**
- `include/xasm++/cpu/opcodes_z80.h` (500-700 lines)
- Documentation of ~150 total opcodes
- Organized namespace structure (opcodes::*, opcodes::cb::*, opcodes::ed::*)

**Success Criteria:**
- All Z80 opcodes defined as named constants
- Clear organization by instruction type
- Comments explain instruction purpose
- Includes helper functions for calculated opcodes (bit operations)

**Example Structure:**
```cpp
namespace xasm {
namespace opcodes {
  // 8-bit loads
  constexpr uint8_t LD_A_n = 0x3E;
  constexpr uint8_t LD_B_n = 0x06;
  // ... more

  namespace prefix {
    constexpr uint8_t CB = 0xCB;
    constexpr uint8_t ED = 0xED;
    constexpr uint8_t DD = 0xDD;
    constexpr uint8_t FD = 0xFD;
  }

  namespace cb {
    constexpr uint8_t BIT_BASE = 0x40;
    constexpr uint8_t BitOpcode(uint8_t base, uint8_t bit, uint8_t reg);
  }
}
}
```

---

### Action 2: Create 6809 Opcode Header

**Priority:** 🔴 HIGH
**Effort:** 6 hours
**Dependencies:** None
**Assignee:** TBD

**Description:**
Create `include/xasm++/cpu/opcodes_6809.h` containing all 6809 opcode constants including 3-page structure.

**Tasks:**
1. Create header file structure
2. Document page 1 opcodes (standard) - ~60 opcodes
3. Document page 2 opcodes (prefix $10) - ~25 opcodes
4. Document page 3 opcodes (prefix $11) - ~15 opcodes
5. Document indexed addressing post-byte encoding
6. Add register encoding constants
7. Add comments explaining 6809's multi-page architecture
8. Document complex addressing mode calculations

**Deliverables:**
- `include/xasm++/cpu/opcodes_6809.h` (600-800 lines)
- Documentation of ~100 total opcodes
- Organized namespace structure (page1, page2, page3, indexed)
- Post-byte encoding helpers

**Success Criteria:**
- All 6809 opcodes defined as named constants
- 3-page structure clearly documented
- Indexed addressing encoding explained
- Register encoding constants defined

**Example Structure:**
```cpp
namespace xasm {
namespace opcodes {
  // Page 1 (standard opcodes)
  constexpr uint8_t LDA_IMM = 0x86;
  constexpr uint8_t LDA_DIR = 0x96;
  // ... more

  namespace prefix {
    constexpr uint8_t PAGE2 = 0x10;
    constexpr uint8_t PAGE3 = 0x11;
  }

  namespace page2 {
    constexpr uint8_t LBRA = 0x16;  // Long branch
    constexpr uint8_t LDY_IMM = 0x8E;
    // ... more
  }

  namespace indexed {
    constexpr uint8_t REG_X = 0x00;
    constexpr uint8_t REG_Y = 0x20;
    constexpr uint8_t INDIRECT = 0x10;
    // ... more
  }
}
}
```

---

### Action 3: Refactor Z80 Implementation

**Priority:** 🔴 HIGH
**Effort:** 3 hours
**Dependencies:** Action 1 (Z80 opcode header must exist)
**Assignee:** TBD

**Description:**
Update `src/cpu/z80/cpu_z80.cpp` to use named opcode constants.

**Tasks:**
1. Add `#include "xasm++/cpu/opcodes_z80.h"` to implementation
2. Replace all magic numbers with named constants (~100 replacements)
3. Update complex opcode calculations (CB-prefixed, etc.)
4. Add comments referencing opcode header for complex cases
5. Verify no magic numbers remain

**Deliverables:**
- Updated `src/cpu/z80/cpu_z80.cpp` (no magic numbers)
- All opcodes use named constants
- Comments added for complex encodings

**Success Criteria:**
- Zero magic numbers in Z80 implementation
- All opcodes use `opcodes::*` constants
- Code compiles and passes existing tests
- Grep for `0x[0-9A-F][0-9A-F]` returns no opcode literals

**Example Refactoring:**
```cpp
// BEFORE:
std::vector<uint8_t> CpuZ80::EncodeLD_A_n(uint8_t value) const {
  return {0x3E, value};
}

// AFTER:
std::vector<uint8_t> CpuZ80::EncodeLD_A_n(uint8_t value) const {
  return {opcodes::LD_A_n, value};
}
```

---

### Action 4: Refactor 6809 Implementation

**Priority:** 🔴 HIGH
**Effort:** 3 hours
**Dependencies:** Action 2 (6809 opcode header must exist)
**Assignee:** TBD

**Description:**
Update `src/cpu/m6809/cpu_6809.cpp` to use named opcode constants.

**Tasks:**
1. Add `#include "xasm++/cpu/opcodes_6809.h"` to implementation
2. Replace all magic numbers with named constants (~100 replacements)
3. Update page 2/3 instructions to use prefix constants
4. Update indexed addressing to use post-byte constants
5. Verify no magic numbers remain

**Deliverables:**
- Updated `src/cpu/m6809/cpu_6809.cpp` (no magic numbers)
- All opcodes use named constants
- Clear indication of page 2/3 instructions

**Success Criteria:**
- Zero magic numbers in 6809 implementation
- All opcodes use `opcodes::*` constants
- Code compiles and passes existing tests
- Grep for `0x[0-9A-F][0-9A-F]` returns no opcode literals

**Example Refactoring:**
```cpp
// BEFORE:
std::vector<uint8_t> Cpu6809::EncodeNOP() const {
  return {0x12};
}

std::vector<uint8_t> Cpu6809::EncodeLBRA(int16_t offset) const {
  return {0x10, 0x16, hi, lo};
}

// AFTER:
std::vector<uint8_t> Cpu6809::EncodeNOP() const {
  return {opcodes::NOP};
}

std::vector<uint8_t> Cpu6809::EncodeLBRA(int16_t offset) const {
  return {opcodes::prefix::PAGE2, opcodes::page2::LBRA, hi, lo};
}
```

---

### Action 5: Complete 6809 Indexed Addressing

**Priority:** 🔴 HIGH
**Effort:** 16-20 hours
**Dependencies:** Action 4 (refactored implementation)
**Assignee:** TBD

**Description:**
Implement the 16 indexed addressing sub-modes for the 6809, currently marked as TODO.

**Background:**
The 6809's indexed addressing is its most powerful feature with 16 sub-modes:
- 5-bit offset (no post-byte)
- 8-bit offset from register
- 16-bit offset from register
- Auto-increment/decrement (1 or 2)
- Extended indirect
- 8 combinations of these patterns

**Tasks:**
1. Research 6809 indexed addressing post-byte encoding
2. Implement post-byte calculation for each mode
3. Update EncodeLDA, EncodeLDB, EncodeLDD for indexed modes
4. Update EncodeJMP, EncodeJSR for indexed addressing
5. Implement for all instructions supporting indexed mode (~30 instructions)
6. Add comprehensive tests for each mode
7. Test with real-world assembly code examples

**Deliverables:**
- Complete indexed addressing implementation
- All 16 sub-modes supported
- Comprehensive test coverage (50+ test cases)
- Documentation of post-byte encoding logic
- Example assembly code that uses indexed modes

**Success Criteria:**
- All 16 indexed addressing modes implemented
- Zero TODO comments for indexed addressing
- All tests pass
- Can assemble real 6809 code using indexed addressing
- Post-byte encoding verified against 6809 reference manual

**Complexity Note:**
This is the most complex action item due to the 6809's sophisticated indexed addressing. Each of the 16 modes has different post-byte encoding rules.

**Example Modes:**
```
,R       - Register indirect (no offset)
n,R      - 8-bit offset
nn,R     - 16-bit offset
,R+      - Auto-increment by 1
,R++     - Auto-increment by 2
,-R      - Auto-decrement by 1
,--R     - Auto-decrement by 2
A,R      - Accumulator offset
B,R      - Accumulator offset
D,R      - D register offset
[,R]     - Indirect
[n,R]    - Indirect with offset
[nn,R]   - Indirect with extended offset
```

---

### Action 6: Update Test Files

**Priority:** 🔴 HIGH
**Effort:** 2 hours
**Dependencies:** Actions 3, 4 (refactored implementations)
**Assignee:** TBD

**Description:**
Update test files to use named opcode constants instead of magic numbers.

**Tasks:**
1. Update `tests/unit/test_cpu_z80.cpp`
   - Replace magic numbers with `opcodes::*` constants
   - Add `#include "xasm++/cpu/opcodes_z80.h"`

2. Update `tests/unit/test_cpu6809.cpp` (if exists)
   - Replace magic numbers with `opcodes::*` constants
   - Add `#include "xasm++/cpu/opcodes_6809.h"`

3. Update any integration tests using Z80/6809 opcodes

4. Add comments explaining test expectations

**Deliverables:**
- Updated test files with named constants
- Clear test expectations
- No magic numbers in test assertions

**Success Criteria:**
- All tests still pass
- Test code uses named constants
- Test expectations are self-documenting

**Example Refactoring:**
```cpp
// BEFORE:
TEST(CpuZ80Test, LD_A_n) {
  CpuZ80 cpu;
  auto bytes = cpu.EncodeLD_A_n(0x42);
  EXPECT_EQ(bytes[0], 0x3E);  // What is 0x3E?
  EXPECT_EQ(bytes[1], 0x42);
}

// AFTER:
TEST(CpuZ80Test, LD_A_n) {
  CpuZ80 cpu;
  auto bytes = cpu.EncodeLD_A_n(0x42);
  EXPECT_EQ(bytes[0], opcodes::LD_A_n);  // Clear: LD A, n opcode
  EXPECT_EQ(bytes[1], 0x42);
}
```

---

### Action 7: Verify Against CPU Reference Manuals

**Priority:** 🔴 HIGH
**Effort:** 2 hours
**Dependencies:** Actions 1, 2 (opcode headers exist)
**Assignee:** TBD

**Description:**
Systematically verify all opcode constants against official CPU reference manuals.

**Tasks:**
1. Obtain Z80 reference documentation
   - Zilog Z80 CPU User Manual
   - Z80 instruction set reference

2. Obtain 6809 reference documentation
   - Motorola 6809 Programming Manual
   - 6809 instruction set reference

3. Spot-check critical opcodes:
   - Load/store instructions (10 samples)
   - Arithmetic instructions (10 samples)
   - Branch instructions (10 samples)
   - Special instructions (5 samples)

4. Create verification checklist

5. Document any discrepancies found

**Deliverables:**
- Verification checklist (completed)
- List of any corrections needed
- Documentation of verification process
- Reference manual citations in opcode headers

**Success Criteria:**
- All spot-checked opcodes verified correct
- Any errors corrected
- Verification documented

**Verification Template:**
```
Instruction: LD A, n
Expected Opcode: 0x3E
Reference: Z80 User Manual page 123
Header Definition: opcodes::LD_A_n = 0x3E
Status: ✅ VERIFIED

Instruction: LDA #immediate (6809)
Expected Opcode: 0x86
Reference: 6809 Programming Manual page 45
Header Definition: opcodes::LDA_IMM = 0x86
Status: ✅ VERIFIED
```

---

## Medium Priority Actions (Complete within 1 month)

### Action 8: Extract Common Encoding Patterns

**Priority:** 🟡 MEDIUM
**Effort:** 8-12 hours
**Dependencies:** Actions 3, 4 (refactored implementations)
**Assignee:** TBD

**Description:**
Identify and extract common encoding patterns into base class helpers to reduce duplication.

**Tasks:**
1. Analyze encoding patterns across CPU families
2. Identify common patterns:
   - Byte order conversion (little/big endian)
   - Multi-byte operand encoding
   - Register encoding
   - Address mode calculation

3. Create helper methods in base classes:
   - `CpuPluginBase::EncodeLittleEndian(uint16_t)`
   - `CpuPluginBase::EncodeBigEndian(uint16_t)`
   - `CpuPluginBase::EncodeRegisterPair(uint8_t reg)`

4. Refactor CPU implementations to use helpers

5. Update tests to verify helper functions

**Deliverables:**
- Enhanced CPU plugin base class with helpers
- Refactored implementations using helpers
- Reduced code duplication
- Tests for helper functions

**Success Criteria:**
- Common encoding logic extracted to base class
- At least 20% reduction in duplicated code
- All existing tests still pass

**Example:**
```cpp
// Base class helper:
class CpuPluginBase {
protected:
  std::vector<uint8_t> EncodeLittleEndian(uint16_t value) const {
    return {static_cast<uint8_t>(value & 0xFF),
            static_cast<uint8_t>((value >> 8) & 0xFF)};
  }
};

// Usage in Z80:
std::vector<uint8_t> CpuZ80::EncodeLD_BC_nn(uint16_t value) const {
  std::vector<uint8_t> result = {opcodes::LD_BC_nn};
  auto bytes = EncodeLittleEndian(value);  // Use helper
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}
```

---

### Action 9: Improve Z80 Test Coverage

**Priority:** 🟡 MEDIUM
**Effort:** 6-8 hours
**Dependencies:** Action 3 (refactored Z80 implementation)
**Assignee:** TBD

**Description:**
Add comprehensive test coverage for Z80 extended instructions and all addressing modes.

**Tasks:**
1. Identify gaps in current test coverage
2. Add tests for ED-prefixed instructions (~30 tests)
3. Add tests for CB-prefixed instructions (bit operations) (~40 tests)
4. Add tests for IX/IY register operations (~30 tests)
5. Add tests for block operations (LDIR, CPIR, etc.) (~10 tests)
6. Test edge cases and error conditions
7. Verify against Z80 reference manual

**Deliverables:**
- 100+ new test cases for Z80
- Comprehensive coverage of instruction set
- Edge case testing
- Test documentation

**Success Criteria:**
- Test coverage for Z80 plugin > 90%
- All instruction categories covered
- Edge cases tested
- All tests pass

---

### Action 10: Add Coding Standards Document

**Priority:** 🟡 MEDIUM
**Effort:** 4-6 hours
**Dependencies:** Actions 1-7 (establish pattern)
**Assignee:** TBD

**Description:**
Document coding standards for xasm++ with emphasis on opcode header pattern.

**Tasks:**
1. Create `docs/CODING_STANDARDS.md`
2. Document opcode header pattern
3. Document naming conventions
4. Document error handling patterns
5. Document testing requirements
6. Document documentation requirements (Doxygen)
7. Provide examples and anti-patterns
8. Reference existing code as examples

**Deliverables:**
- `docs/CODING_STANDARDS.md` (comprehensive guide)
- Pattern examples
- Anti-pattern warnings
- Checklist for new code

**Success Criteria:**
- Clear coding standards established
- Opcode header pattern documented
- Examples from codebase referenced
- New contributors can follow standards

**Content Outline:**
```markdown
# xasm++ Coding Standards

## 1. Opcode Definition Pattern

### Required Pattern
All CPU plugins MUST define opcodes in a separate header:
- Location: `include/xasm++/cpu/opcodes_<cpu>.h`
- Format: `constexpr uint8_t MNEMONIC_MODE = 0xXX;`
- Organization: By instruction category

### Rationale
[Explain benefits]

### Examples
[Show 6502, Z80, 6809 patterns]

### Anti-Patterns
❌ Never use magic numbers in implementation files
❌ Never define opcodes in .cpp files

## 2. Naming Conventions
[...]

## 3. Error Handling
[...]

## 4. Testing Requirements
[...]
```

---

## Low Priority Actions (Nice to have)

### Action 11: Refactor Long Methods

**Priority:** 🟢 LOW
**Effort:** 4-6 hours
**Dependencies:** None
**Assignee:** TBD

**Description:**
Extract switch cases in long encoding methods into separate helper methods.

**Tasks:**
1. Identify methods > 100 lines
2. Extract switch cases into helpers
3. Improve readability
4. Maintain existing tests

**Example:**
```cpp
// BEFORE (100+ lines):
std::vector<uint8_t> EncodeLDA(uint32_t operand, AddressingMode mode) const {
  switch (mode) {
    case Immediate: /* 10 lines */ break;
    case ZeroPage: /* 10 lines */ break;
    // ... 10 more cases
  }
}

// AFTER:
std::vector<uint8_t> EncodeLDA(uint32_t operand, AddressingMode mode) const {
  switch (mode) {
    case Immediate: return EncodeLDA_Immediate(operand);
    case ZeroPage: return EncodeLDA_ZeroPage(operand);
    // ... cleaner
  }
}
```

---

### Action 12: Use Address Type Consistently

**Priority:** 🟢 LOW
**Effort:** 6-8 hours
**Dependencies:** None
**Assignee:** TBD

**Description:**
Replace raw uint16_t/uint32_t with Address type for better type safety.

**Tasks:**
1. Audit codebase for primitive address types
2. Refactor to use `xasm::Address` type
3. Update method signatures
4. Update tests
5. Verify all address operations

**Benefits:**
- Better type safety
- Built-in formatting
- Bank/offset extraction methods
- Clearer code intent

---

## Timeline and Dependencies

```
Week 1:
  ├── Action 1: Create Z80 opcode header (4h)
  ├── Action 2: Create 6809 opcode header (6h)
  └── Action 7: Verify opcodes (2h)

Week 2:
  ├── Action 3: Refactor Z80 implementation (3h)
  ├── Action 4: Refactor 6809 implementation (3h)
  ├── Action 6: Update test files (2h)
  └── Action 5: Start 6809 indexed addressing (8h of 20h)

Week 3:
  ├── Action 5: Complete 6809 indexed addressing (12h remaining)
  └── Action 9: Improve Z80 tests (start, 4h of 8h)

Week 4:
  ├── Action 9: Complete Z80 tests (4h remaining)
  ├── Action 8: Extract common patterns (8h)
  └── Action 10: Add coding standards (4h)
```

---

## Success Metrics

### Quantitative Metrics

| Metric | Current | Target | How to Measure |
|--------|---------|--------|----------------|
| Magic numbers in Z80 | ~100 | 0 | `grep -c "0x[0-9A-F]" src/cpu/z80/*.cpp` |
| Magic numbers in 6809 | ~100 | 0 | `grep -c "0x[0-9A-F]" src/cpu/m6809/*.cpp` |
| 6809 TODO count | 15 | 0 | `grep -c "TODO" src/cpu/m6809/*.cpp` |
| Z80 test coverage | ~70% | >90% | Code coverage tool |
| Pattern consistency | 33% | 100% | Manual review |

### Qualitative Metrics

✅ **Code Review Speed**
- Before: 5 minutes per opcode lookup
- After: Immediate understanding

✅ **New Contributor Onboarding**
- Before: Need CPU manual for code review
- After: Code is self-documenting

✅ **Maintainability**
- Before: Hard to verify correctness
- After: Easy to verify against opcode headers

---

## Risk Assessment

### Risk 1: Breaking Changes

**Probability:** Low
**Impact:** Medium
**Mitigation:**
- Comprehensive testing after each refactoring step
- Keep existing tests passing
- Code review before merging

### Risk 2: Time Overrun on 6809 Indexed Addressing

**Probability:** Medium
**Impact:** Medium
**Mitigation:**
- Break into smaller increments
- Implement and test one mode at a time
- Can defer some modes if needed (mark as unsupported)

### Risk 3: Opcode Verification Errors

**Probability:** Low
**Impact:** High
**Mitigation:**
- Systematic verification against reference manuals
- Spot-check critical instructions
- Add integration tests with known-good binaries

---

## Communication Plan

### Status Updates

- **Weekly:** Status update on high-priority actions
- **Biweekly:** Review progress against timeline
- **End of Phase:** Comprehensive review

### Deliverable Reviews

- Each opcode header reviewed before use
- Each refactored implementation reviewed
- All tests reviewed for correctness

### Documentation Updates

- Update architecture docs as work progresses
- Document any deviations from plan
- Keep README updated with status

---

## Appendix: Effort Estimation

### Total Effort Breakdown

| Priority | Actions | Total Hours |
|----------|---------|-------------|
| HIGH     | 7 actions | 36-40 hours |
| MEDIUM   | 3 actions | 18-26 hours |
| LOW      | 2 actions | 10-14 hours |
| **TOTAL** | **12 actions** | **64-80 hours** |

### Staffing Recommendations

**Option A: Single Developer (Part-Time)**
- Timeline: 8-10 weeks (8 hours/week)
- Complete high-priority items in 5 weeks
- Complete medium-priority items by week 8

**Option B: Two Developers**
- Timeline: 4-5 weeks
- Developer 1: Actions 1, 3, 6, 7, 9 (Z80 focus)
- Developer 2: Actions 2, 4, 5, 8, 10 (6809 focus)

**Recommendation:** Option A (single developer) to maintain consistency

---

**Plan Created:** 2026-02-09
**Plan Owner:** TBD
**Next Review:** After week 1 completion
**Status:** ⏳ Awaiting approval
