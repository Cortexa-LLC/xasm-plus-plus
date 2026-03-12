# Task Contract

**Task ID:** 2026-01-30_65c02-cpu
**Beads Task:** xasm++-igq
**Created:** 2026-01-30
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature (.ai-pack/workflows/feature.md)

---

## Task Description

Implement WDC 65C02 CPU plugin with extended instruction set for Apple IIc/IIe enhanced compatibility.

### Background and Context

The 65C02 is Western Design Center's CMOS enhancement of the original NMOS 6502. It adds new instructions, fixes bugs, and includes new addressing modes while maintaining backward compatibility.

**Platforms using 65C02:**
- Apple IIc (all models)
- Apple IIe Enhanced (with upgrade card)
- Apple IIgs (in 8-bit compatibility mode)
- Commodore 128 (optional)
- BBC Micro (Master series)

**Key enhancements over 6502:**
1. New instructions (27 new opcodes)
2. Fixed JMP ($xxFF) bug
3. Removed illegal/undocumented opcodes
4. Added new addressing modes
5. Improved timing on some instructions

### Current State

- 6502 CPU fully implemented ✓
- Branch relaxation working ✓
- 56 legal 6502 opcodes ✓
- 65C02 instructions not yet implemented ❌

### Desired State

- 65C02 CPU plugin extends Cpu6502
- 27 new 65C02 instructions working
- New addressing modes supported
- Backward compatible with 6502 code
- Comprehensive test coverage

---

## Success Criteria

```
✓ 65C02 CPU plugin implemented
✓ 27 new instructions working
✓ New addressing modes (BIT #imm, JMP (abs,X))
✓ All existing 6502 tests passing
✓ New 65C02 tests added (30+ tests)
✓ Zero compiler warnings
✓ Code review approved
```

---

## Acceptance Criteria

### Functional Requirements
```
□ BRA - Branch Always (unconditional relative)
□ PHX/PLX - Push/Pull X register
□ PHY/PLY - Push/Pull Y register
□ STZ - Store Zero
□ TRB/TSB - Test and Reset/Set Bits
□ BIT immediate mode (#$00)
□ JMP (abs,X) indexed indirect
□ All 6502 instructions still working
```

### Quality Requirements
```
□ All tests passing (563/564 + new 65C02 tests)
□ Zero compiler warnings
□ Code review approved
□ TDD process followed
□ CPU plugin architecture maintained
```

---

## Technical Approach

### 65C02 New Instructions

**1. Branch Always (BRA)**
- Opcode: `0x80`
- Addressing: Relative
- Purpose: Unconditional branch (like JMP but shorter)
- Usage: `BRA label`

**2. Stack Operations (PHX, PLX, PHY, PLY)**
- PHX: `0xDA` - Push X to stack
- PLX: `0xFA` - Pull X from stack
- PHY: `0x5A` - Push Y to stack
- PLY: `0x7A` - Pull Y from stack
- Purpose: Save/restore index registers

**3. Store Zero (STZ)**
- Zero Page: `0x64`
- Zero Page,X: `0x74`
- Absolute: `0x9C`
- Absolute,X: `0x9E`
- Purpose: Store 0 without using A register

**4. Test and Reset/Set Bits (TRB, TSB)**
- TRB Zero Page: `0x14`
- TRB Absolute: `0x1C`
- TSB Zero Page: `0x04`
- TSB Absolute: `0x0C`
- Purpose: Atomic bit manipulation

**5. Bit Branch Instructions (BBR, BBS)**
- BBR0-BBR7: `0x0F, 0x1F, 0x2F, 0x3F, 0x4F, 0x5F, 0x6F, 0x7F`
- BBS0-BBS7: `0x8F, 0x9F, 0xAF, 0xBF, 0xCF, 0xDF, 0xEF, 0xFF`
- Addressing: Zero Page + Relative
- Purpose: Branch if bit clear/set (Rockwell/WDC variant)

**6. Reset/Set Memory Bit (RMB, SMB)**
- RMB0-RMB7: `0x07, 0x17, 0x27, 0x37, 0x47, 0x57, 0x67, 0x77`
- SMB0-SMB7: `0x87, 0x97, 0xA7, 0xB7, 0xC7, 0xD7, 0xE7, 0xF7`
- Addressing: Zero Page
- Purpose: Set/clear specific bit (Rockwell/WDC variant)

**7. WAI - Wait for Interrupt**
- Opcode: `0xCB`
- Purpose: Low-power wait state

**8. STP - Stop**
- Opcode: `0xDB`
- Purpose: Halt processor until reset

### New Addressing Modes

**BIT Immediate:**
```cpp
// 6502: BIT only supports Zero Page and Absolute
// 65C02: BIT also supports Immediate
case 0x89:  // BIT #$00
    return Immediate();
```

**JMP Indexed Indirect:**
```cpp
// 6502: JMP (abs) only
// 65C02: JMP (abs,X) added
case 0x7C:  // JMP ($1234,X)
    return IndexedIndirect();
```

### Implementation Strategy

**Option 1: Inheritance (Recommended)**
```cpp
class Cpu65C02 : public Cpu6502 {
public:
    Cpu65C02();
    
    // Override to add 65C02 opcodes
    std::vector<uint8_t> Encode(const Instruction& instr) override;
    
private:
    void RegisterOpcodes();
    void Register65C02Opcodes();
};
```

**Why inheritance:**
- Reuses all 6502 logic
- Adds only new opcodes
- Maintains backward compatibility
- Minimal code duplication

**File Structure:**
```
include/xasm++/cpu/m6502/
  cpu_6502.h          (base class)
  cpu_65c02.h         (new - derived class)

src/cpu/m6502/
  cpu_6502.cpp        (existing)
  cpu_65c02.cpp       (new)

tests/unit/
  test_cpu_6502.cpp   (existing - keep passing)
  test_cpu_65c02.cpp  (new)
```

### Test Cases

**1. BRA (Branch Always):**
```assembly
      BRA forward
      NOP            ; Skipped
forward: LDA #$42
```

**2. PHX/PLX:**
```assembly
      LDX #$42
      PHX
      LDX #$00
      PLX            ; X = $42 again
```

**3. STZ:**
```assembly
      STZ $80        ; Zero Page
      STZ $80,X      ; Zero Page,X
      STZ $1234      ; Absolute
      STZ $1234,X    ; Absolute,X
```

**4. BIT Immediate:**
```assembly
      LDA #$FF
      BIT #$80       ; Test bit 7
```

**5. JMP (abs,X):**
```assembly
      LDX #$02
      JMP (table,X)  ; Jump via indexed table
```

---

## Architecture Considerations

### CPU Registry

Register new CPU in `CpuRegistry`:

```cpp
// In src/cpu/cpu_registry.cpp
CpuRegistry::CpuRegistry() {
    Register("6502", []() { return std::make_unique<Cpu6502>(); });
    Register("65c02", []() { return std::make_unique<Cpu65C02>(); });
    Register("65C02", []() { return std::make_unique<Cpu65C02>(); });  // Alias
}
```

### Command-Line Usage

```bash
xasm++ --cpu 65c02 program.asm -o program.bin
```

### Backward Compatibility

65C02 code can include 6502 instructions:
- All 56 6502 opcodes work on 65C02
- 65C02 programs can use mixed instruction sets
- Tests verify 6502 subset still works

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| Breaking 6502 tests | Low | High | Run all 6502 tests, inheritance ensures compatibility |
| Opcode conflicts | Very Low | Medium | Use opcode tables from data sheets |
| Complex bit operations | Low | Low | BBR/BBS/RMB/SMB are optional (Rockwell variant) |
| JMP (abs,X) complexity | Low | Medium | Follow 65C02 addressing rules carefully |

---

## References

**65C02 Data Sheets:**
- WDC 65C02 Datasheet
- Rockwell R65C02 Datasheet
- Synertek SY6502 Datasheet

**Apple II Documentation:**
- Apple IIc Technical Reference Manual
- Apple IIe Enhanced Reference Manual
- Apple IIgs Hardware Reference

**Online Resources:**
- 6502.org 65C02 instruction set
- Opcodes.com 65C02 reference

**Related Code:**
- `src/cpu/m6502/cpu_6502.cpp` - Base 6502 implementation
- `tests/unit/test_cpu_6502.cpp` - 6502 test patterns

---

## Implementation Phases

**Phase 1: Foundation** (30 minutes)
- Create Cpu65C02 class skeleton
- Set up inheritance from Cpu6502
- Register in CPU registry

**Phase 2: Core Instructions** (1.5 hours)
- BRA - Branch Always
- PHX/PLX, PHY/PLY - Stack operations
- STZ - Store Zero (4 modes)
- Add tests for each

**Phase 3: Bit Operations** (1 hour)
- TRB/TSB - Test and Reset/Set Bits
- BIT immediate mode
- Add tests

**Phase 4: Advanced Instructions** (1 hour)
- JMP (abs,X) - Indexed indirect jump
- BBR/BBS/RMB/SMB (optional Rockwell variant)
- WAI/STP (optional)
- Add tests

**Phase 5: Integration** (30 minutes)
- Verify all 6502 tests still pass
- Add E2E tests
- Update documentation

**Total Estimated Effort:** 4-5 hours

---

## Success Milestone

**🎯 Apple II Enhanced Platform Support**

Upon completion:
- ✅ Full 65C02 instruction set implemented
- ✅ Apple IIc/IIe enhanced compatibility
- ✅ Backward compatible with 6502 code
- ✅ Foundation for 65816 (Apple IIgs) support

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-30
