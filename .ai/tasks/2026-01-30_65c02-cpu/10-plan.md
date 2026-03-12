# Implementation Plan

**Task:** 65C02 CPU Plugin - Extended 6502 Support
**Beads Task:** xasm++-igq
**Workflow:** Feature (.ai-pack/workflows/feature.md)
**Role:** Engineer

---

## Workflow Selection

**Using:** `.ai-pack/workflows/feature.md`

This is a new CPU plugin feature extending the existing 6502 support.

---

## Implementation Approach

### Design Pattern: Inheritance

```
Cpu6502 (base)
    ├─ 56 6502 opcodes
    ├─ 13 addressing modes
    └─ Branch relaxation
         ↓ extends
Cpu65C02 (derived)
    ├─ All 6502 functionality (inherited)
    └─ 27 new 65C02 opcodes
```

**Benefits:**
- Code reuse (no duplication)
- Backward compatibility guaranteed
- Minimal changes to existing code
- Easy to test incrementally

### Phase 1: Foundation

**Create class skeleton:**

```cpp
// include/xasm++/cpu/m6502/cpu_65c02.h
#pragma once
#include "cpu_6502.h"

class Cpu65C02 : public Cpu6502 {
public:
    Cpu65C02();
    ~Cpu65C02() override = default;
    
    std::string GetName() const override { return "65C02"; }
    
private:
    void Register65C02Opcodes();
};
```

**Register in CPU registry:**

```cpp
// src/cpu/cpu_registry.cpp
#include "cpu_65c02.h"

void CpuRegistry::RegisterBuiltinCpus() {
    Register("6502", []() { return std::make_unique<Cpu6502>(); });
    Register("65c02", []() { return std::make_unique<Cpu65C02>(); });
    Register("65C02", []() { return std::make_unique<Cpu65C02>(); });
}
```

**Tests:**
- Verify CPU can be instantiated
- Verify registration works
- Verify 6502 backward compatibility

### Phase 2: Core Instructions (BRA, PHX/PLX, PHY/PLY, STZ)

**BRA - Branch Always:**
```cpp
RegisterOpcode(0x80, "BRA", Relative, 2, 3,
    [](Cpu* cpu, const Operand& op) {
        // Always branch (like BEQ but unconditional)
        return EncodeBranch(0x80, op.offset);
    });
```

**PHX/PLX - Push/Pull X:**
```cpp
RegisterOpcode(0xDA, "PHX", Implied, 1, 3);
RegisterOpcode(0xFA, "PLX", Implied, 1, 4);
```

**PHY/PLY - Push/Pull Y:**
```cpp
RegisterOpcode(0x5A, "PHY", Implied, 1, 3);
RegisterOpcode(0x7A, "PLY", Implied, 1, 4);
```

**STZ - Store Zero (4 addressing modes):**
```cpp
RegisterOpcode(0x64, "STZ", ZeroPage, 2, 3);
RegisterOpcode(0x74, "STZ", ZeroPageX, 2, 4);
RegisterOpcode(0x9C, "STZ", Absolute, 3, 4);
RegisterOpcode(0x9E, "STZ", AbsoluteX, 3, 5);
```

**Tests for each:**
- BRA forward/backward branches
- PHX/PLX preserves/restores X
- PHY/PLY preserves/restores Y
- STZ all 4 modes write 0

### Phase 3: Bit Operations (TRB/TSB, BIT #imm)

**TRB - Test and Reset Bits:**
```cpp
RegisterOpcode(0x14, "TRB", ZeroPage, 2, 5);
RegisterOpcode(0x1C, "TRB", Absolute, 3, 6);
```

**TSB - Test and Set Bits:**
```cpp
RegisterOpcode(0x04, "TSB", ZeroPage, 2, 5);
RegisterOpcode(0x0C, "TSB", Absolute, 3, 6);
```

**BIT Immediate (new mode):**
```cpp
RegisterOpcode(0x89, "BIT", Immediate, 2, 2);
// Note: BIT already exists for ZeroPage and Absolute in 6502
```

**Tests:**
- TRB clears bits
- TSB sets bits
- BIT immediate tests flags

### Phase 4: Advanced Instructions (JMP (abs,X), BBR/BBS, etc.)

**JMP (abs,X) - Indexed Indirect Jump:**
```cpp
RegisterOpcode(0x7C, "JMP", IndexedIndirect, 3, 6);
// Note: JMP already exists for Absolute and Indirect in 6502
```

**BBR/BBS - Bit Branch (optional Rockwell variant):**
```cpp
// BBR0-BBR7 (Branch if Bit Reset)
for (int bit = 0; bit < 8; bit++) {
    RegisterOpcode(0x0F + (bit << 4), "BBR" + std::to_string(bit),
                   ZeroPageRelative, 3, 5);
}

// BBS0-BBS7 (Branch if Bit Set)
for (int bit = 0; bit < 8; bit++) {
    RegisterOpcode(0x8F + (bit << 4), "BBS" + std::to_string(bit),
                   ZeroPageRelative, 3, 5);
}
```

**RMB/SMB - Reset/Set Memory Bit (optional):**
```cpp
// RMB0-RMB7, SMB0-SMB7
// Similar pattern to BBR/BBS
```

**WAI/STP - Power management (optional):**
```cpp
RegisterOpcode(0xCB, "WAI", Implied, 1, 3);
RegisterOpcode(0xDB, "STP", Implied, 1, 3);
```

**Tests:**
- JMP (abs,X) with various X values
- BBR/BBS branch logic
- RMB/SMB bit manipulation

### Phase 5: Integration and Validation

**Verify backward compatibility:**
```bash
# Run all 6502 tests with 65C02 CPU
ctest -R Cpu6502Test
```

**Add E2E tests:**
```assembly
; Apple IIc-style code
      PHX
      PHY
      LDA #$00
      STA result
      BRA done
done: PLY
      PLX
      RTS
```

**Update CMake:**
```cmake
add_library(cpu_65c02
    src/cpu/m6502/cpu_65c02.cpp
)
target_link_libraries(cpu_65c02 cpu_6502)
```

---

## Execution Strategy

**Sequential implementation (phases depend on each other):**

1. **Foundation** - Must exist before adding opcodes
2. **Core Instructions** - Most commonly used, highest priority
3. **Bit Operations** - Builds on core
4. **Advanced Instructions** - Final enhancements
5. **Integration** - Validates everything

**Why sequential:**
- Each phase builds on previous
- Can't test opcodes without foundation
- Can't validate without all instructions

**Estimated timeline:**
- Phase 1: 30 minutes (foundation)
- Phase 2: 1.5 hours (core instructions)
- Phase 3: 1 hour (bit operations)
- Phase 4: 1 hour (advanced)
- Phase 5: 30 minutes (integration)

**Total:** 4-5 hours

---

## Test Strategy

### Unit Tests

**File:** `tests/unit/test_cpu_65c02.cpp` (new)

**Test categories:**
1. **Instantiation** - CPU can be created
2. **Registration** - CPU found in registry
3. **Backward Compatibility** - All 6502 opcodes work
4. **New Instructions** - Each 65C02 opcode tested
5. **Addressing Modes** - BIT #imm, JMP (abs,X)

**Test naming convention:**
```cpp
TEST(Cpu65C02Test, BRA_ForwardBranch)
TEST(Cpu65C02Test, PHX_PushesToStack)
TEST(Cpu65C02Test, STZ_ZeroPage_WritesZero)
TEST(Cpu65C02Test, BIT_Immediate_SetsFlags)
```

### Integration Tests

**File:** `tests/integration/test_65c02_integration.cpp` (new)

Test complete assembly programs using 65C02 features.

---

## Dependencies and Risks

**Dependencies:**
- Cpu6502 (base class) - already implemented ✓
- Opcode registry system - already exists ✓
- Addressing mode handlers - already exist ✓

**Risks:**

1. **Breaking 6502 compatibility** - Inheritance changes affect base
   - Mitigation: Run all 6502 tests, use `override` carefully
   - Probability: Low

2. **Opcode conflicts** - Wrong opcode numbers
   - Mitigation: Cross-reference multiple datasheets
   - Probability: Very Low

3. **Complex addressing modes** - JMP (abs,X) edge cases
   - Mitigation: Comprehensive tests, follow datasheet exactly
   - Probability: Low

---

## Success Criteria Checklist

```
□ Cpu65C02 class created and registered
□ BRA instruction working
□ PHX/PLX working
□ PHY/PLY working
□ STZ (4 modes) working
□ TRB/TSB working
□ BIT immediate mode working
□ JMP (abs,X) working
□ All 6502 tests still passing
□ 30+ new 65C02 tests passing
□ Zero compiler warnings
□ Code review approved
```

---

**Plan Version:** 1.0
**Last Updated:** 2026-01-30
