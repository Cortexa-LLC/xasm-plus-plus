# Task Contract

**Task ID:** 2026-01-29_branch-relaxation
**Beads Task:** xasm++-agk
**Created:** 2026-01-29
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature (.ai-pack/workflows/feature.md)

---

## Task Description

Implement automatic branch relaxation for out-of-range 6502 branch instructions.

### Background and Context

6502 branch instructions (BEQ, BNE, BCC, BCS, etc.) use 8-bit signed relative addressing with a range of -128 to +127 bytes. When a branch target is beyond this range, the assembler must automatically convert the branch into a complementary branch + JMP sequence.

**Example:**
```assembly
; Original code
       BEQ distant_label  ; Target is 500 bytes away

; Relaxed to:
       BNE +3             ; Skip the JMP if condition fails
       JMP distant_label  ; Absolute jump (no range limit)
```

This is a standard feature in professional 6502 assemblers from the 1980s era, including the original Merlin assembler.

### Current State

- Branch instructions fail if target > 127 bytes or < -128 bytes
- Error message: "Branch target out of range"
- Manual workarounds required

### Desired State

- Out-of-range branches automatically relaxed
- Complementary branch + JMP sequence generated
- Multi-pass convergence handles size changes
- Transparent to programmer (works like Merlin)

---

## Success Criteria

```
✓ Out-of-range branches automatically relaxed
✓ Correct complementary branch generated
✓ Multi-pass assembly converges
✓ In-range branches unchanged (performance)
✓ All existing tests passing (574/575)
✓ New relaxation tests added
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ BEQ/BNE/BCC/BCS/BVC/BVS/BMI/BPL relaxation
□ Correct complementary branch selection
□ Range detection (-128 to +127 bytes)
□ JMP absolute addressing used
□ Multi-pass convergence
□ In-range branches not relaxed
□ Error handling for invalid cases
```

### Quality Requirements
```
□ All tests passing (574/575 + new tests)
□ Zero compiler warnings
□ Code review approved
□ TDD process followed
□ Multi-pass convergence tested
```

---

## Technical Approach

### Branch Complementary Pairs

Each branch has a complementary opposite:

| Original | Complementary | Meaning |
|----------|--------------|---------|
| BEQ      | BNE          | Equal → Not Equal |
| BNE      | BEQ          | Not Equal → Equal |
| BCC      | BCS          | Carry Clear → Carry Set |
| BCS      | BCC          | Carry Set → Carry Clear |
| BVC      | BVS          | Overflow Clear → Overflow Set |
| BVS      | BVC          | Overflow Set → Overflow Clear |
| BMI      | BPL          | Minus → Plus |
| BPL      | BMI          | Plus → Minus |

### Relaxation Algorithm

**Pass 1: Detection**
```cpp
if (branch_offset > 127 || branch_offset < -128) {
    // Out of range, mark for relaxation
    atom->SetNeedsRelaxation(true);
}
```

**Pass 2+: Relaxation**
```cpp
if (atom->NeedsRelaxation()) {
    // Generate:
    // 1. Complementary branch +3 (skip JMP)
    // 2. JMP absolute to target
    GenerateComplementaryBranch(atom);
    GenerateJmpAbsolute(atom->GetTarget());
}
```

**Size Changes:**
- Original branch: 2 bytes (opcode + offset)
- Relaxed sequence: 5 bytes (branch + JMP)
- Delta: +3 bytes

This size change may affect subsequent addresses, requiring additional passes until convergence.

### Implementation Location

**File:** `src/cpu/m6502/cpu_6502.cpp`

**Add relaxation logic:**
```cpp
void Cpu6502::RelaxBranch(BranchAtom* atom, Section* section) {
    // Get branch opcode
    uint8_t original_opcode = atom->GetOpcode();
    
    // Get complementary opcode
    uint8_t complement_opcode = GetComplementaryBranch(original_opcode);
    
    // Generate: complement_opcode +3
    section->AddByte(complement_opcode);
    section->AddByte(0x03);  // Skip 3 bytes (JMP instruction)
    
    // Generate: JMP target
    section->AddByte(0x4C);  // JMP absolute opcode
    section->AddWord(atom->GetTargetAddress());
}

uint8_t Cpu6502::GetComplementaryBranch(uint8_t opcode) {
    static const std::map<uint8_t, uint8_t> complements = {
        {0xF0, 0xD0},  // BEQ → BNE
        {0xD0, 0xF0},  // BNE → BEQ
        {0x90, 0xB0},  // BCC → BCS
        {0xB0, 0x90},  // BCS → BCC
        {0x50, 0x70},  // BVC → BVS
        {0x70, 0x50},  // BVS → BVC
        {0x30, 0x10},  // BMI → BPL
        {0x10, 0x30},  // BPL → BMI
    };
    return complements.at(opcode);
}
```

**File:** `src/core/assembler.cpp`

**Multi-pass convergence:**
```cpp
void Assembler::AssembleMultiPass() {
    bool converged = false;
    int pass = 0;
    const int MAX_PASSES = 10;
    
    while (!converged && pass < MAX_PASSES) {
        pass++;
        converged = AssemblePass();
        
        // Check for size changes (relaxations)
        if (!converged) {
            ResetSections();
        }
    }
    
    if (!converged) {
        throw std::runtime_error("Assembly did not converge after " + 
                                 std::to_string(MAX_PASSES) + " passes");
    }
}
```

### Test Cases

1. **Simple out-of-range forward branch:**
   ```assembly
           .org $1000
   start:  BEQ end
           ; ... 200 bytes of code ...
   end:    RTS
   ```
   Expected: BEQ relaxed to BNE +3; JMP end

2. **Out-of-range backward branch:**
   ```assembly
           .org $1000
   loop:   LDA $00
           ; ... 200 bytes of code ...
           BEQ loop
   ```
   Expected: BEQ relaxed to BNE +3; JMP loop

3. **In-range branch (should NOT relax):**
   ```assembly
           .org $1000
   start:  BEQ near
           NOP
           NOP
   near:   RTS
   ```
   Expected: Normal BEQ (2 bytes, no relaxation)

4. **Multiple relaxations in sequence:**
   ```assembly
           .org $1000
   start:  BEQ far1
           BNE far2
           ; ... code ...
   far1:   RTS
   far2:   RTS
   ```
   Expected: Both branches relaxed

5. **Edge cases:**
   - Branch exactly at +127 bytes (in range, no relaxation)
   - Branch exactly at +128 bytes (out of range, relaxation)
   - Branch exactly at -128 bytes (in range, no relaxation)
   - Branch exactly at -129 bytes (out of range, relaxation)

---

## Architecture Considerations

### Multi-Pass Assembly

xasm++ already supports multi-pass assembly. Branch relaxation extends this:

1. **Pass 1:** Assemble normally, detect out-of-range branches
2. **Pass 2+:** Re-assemble with relaxed branches, check convergence
3. **Convergence:** When all addresses stabilize

**Convergence Detection:**
```cpp
bool Assembler::AssemblePass() {
    std::map<std::string, uint64_t> previous_addresses = current_addresses;
    
    // Re-assemble
    AssembleSections();
    
    // Compare addresses
    return (previous_addresses == current_addresses);
}
```

### Performance Optimization

**Only relax when necessary:**
- Check range before relaxation
- Keep in-range branches as 2-byte instructions
- Only pay 3-byte penalty when required

**Early convergence:**
- Most programs converge in 2-3 passes
- Relaxations typically don't cascade significantly

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| Non-convergence (infinite loop) | Low | High | Max pass limit (10), cycle detection |
| Performance impact | Low | Low | Only relax out-of-range, most programs 2-3 passes |
| Breaking existing code | Very Low | Medium | Comprehensive test suite, backward compatibility |
| Incorrect complementary branches | Low | High | Unit tests for all 8 branch types |

---

## References

**Research Document:**
- `docs/investigations/merlin-branch-relaxation.md` - Complete analysis

**Related Code:**
- `src/cpu/m6502/cpu_6502.cpp` - 6502 CPU implementation
- `src/core/assembler.cpp` - Multi-pass assembly logic
- `tests/unit/test_cpu_6502.cpp` - CPU tests

**Historical Context:**
- Merlin assembler (1980s) - Original implementation
- ca65 - Modern assembler with relaxation
- DASM - Another 6502 assembler with this feature

---

## Implementation Phases

**Phase 1: Detection** (1 hour)
- Add range checking to branch instructions
- Mark out-of-range branches
- Add unit tests for detection

**Phase 2: Relaxation** (1 hour)
- Implement complementary branch generation
- Generate JMP absolute
- Test basic relaxation

**Phase 3: Multi-Pass Convergence** (1 hour)
- Integrate with multi-pass assembly
- Add convergence detection
- Test cascading relaxations

**Phase 4: Edge Cases** (30 minutes)
- Test boundary conditions
- Test error cases
- Performance verification

**Total Estimated Effort:** 3-4 hours

---

## Success Milestone

**🎯 Production-Quality Branch Handling**

Upon completion:
- ✅ All branch instructions handle any range
- ✅ Transparent to programmer
- ✅ Authentic Merlin behavior
- ✅ Professional assembler feature parity

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-29
