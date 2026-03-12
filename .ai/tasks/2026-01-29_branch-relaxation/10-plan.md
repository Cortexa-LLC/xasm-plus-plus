# Implementation Plan

**Task:** Branch Relaxation - Automatic Long Branch Handling
**Beads Task:** xasm++-agk
**Workflow:** Feature (.ai-pack/workflows/feature.md)
**Role:** Engineer

---

## Workflow Selection

**Using:** `.ai-pack/workflows/feature.md`

This is a new feature adding automatic branch relaxation to the assembler.

---

## Implementation Approach

### Phase 1: Research Review (Complete)

Already done - see `docs/investigations/merlin-branch-relaxation.md`

Key findings:
- Original Merlin assembler had this feature
- Complementary branch + JMP pattern
- Multi-pass convergence required

### Phase 2: Detection Logic

**File:** `src/cpu/m6502/cpu_6502.cpp`

Add range checking to branch instruction encoding:

```cpp
// In branch instruction handler
int16_t offset = CalculateBranchOffset(current_pc, target_address);

if (offset < -128 || offset > 127) {
    // Mark for relaxation
    atom->SetNeedsRelaxation(true);
    return false;  // Trigger another pass
}
```

**Tests:**
- Test branch exactly at ±127 (in range)
- Test branch at ±128 (out of range)
- Test very large offsets (±1000)

### Phase 3: Relaxation Generation

**Add complementary branch mapping:**

```cpp
static const std::map<uint8_t, uint8_t> BRANCH_COMPLEMENTS = {
    {0xF0, 0xD0},  // BEQ → BNE
    {0xD0, 0xF0},  // BNE → BEQ
    {0x90, 0xB0},  // BCC → BCS
    {0xB0, 0x90},  // BCS → BCC
    {0x50, 0x70},  // BVC → BVS
    {0x70, 0x50},  // BVS → BVC
    {0x30, 0x10},  // BMI → BPL
    {0x10, 0x30},  // BPL → BMI
};
```

**Add relaxation generator:**

```cpp
void Cpu6502::RelaxBranch(Atom* atom, Section* section) {
    uint8_t original = atom->GetOpcode();
    uint8_t complement = BRANCH_COMPLEMENTS.at(original);
    
    // Generate: <complement> +3
    section->AddByte(complement);
    section->AddByte(0x03);
    
    // Generate: JMP <target>
    section->AddByte(0x4C);  // JMP absolute
    section->AddWord(atom->GetTargetAddress());
}
```

**Tests:**
- Test each branch type relaxes correctly
- Verify complementary branch selection
- Verify JMP target address

### Phase 4: Multi-Pass Integration

**File:** `src/core/assembler.cpp`

Already has multi-pass support. Enhance convergence detection:

```cpp
bool Assembler::Assemble() {
    bool converged = false;
    int pass = 0;
    const int MAX_PASSES = 10;
    
    while (!converged && pass < MAX_PASSES) {
        pass++;
        size_t previous_size = GetTotalSize();
        
        converged = AssemblePass();
        
        size_t current_size = GetTotalSize();
        if (previous_size != current_size) {
            converged = false;  // Size changed, need another pass
        }
    }
    
    if (!converged) {
        throw AssemblerError("Failed to converge after " + 
                             std::to_string(MAX_PASSES) + " passes");
    }
    
    return true;
}
```

**Tests:**
- Test single relaxation converges in 2 passes
- Test multiple relaxations
- Test cascading size changes

### Phase 5: Edge Cases and Error Handling

**Test cases:**
1. Branch exactly at boundary (±127, ±128)
2. Very large offsets (±10000)
3. Multiple branches to same target
4. Branch target changes during assembly
5. Non-convergence detection

**Error messages:**
- "Assembly did not converge after N passes"
- "Branch target unreachable" (if JMP also fails)

---

## Execution Strategy

**Sequential implementation (phases depend on each other):**

1. **Detection** - Foundation for relaxation
2. **Relaxation** - Depends on detection
3. **Multi-Pass** - Uses relaxation
4. **Edge Cases** - Validates all above

**Why sequential:**
- Each phase builds on previous
- Cannot test relaxation without detection
- Cannot test convergence without relaxation

**Estimated timeline:**
- Phase 1: ✓ Complete (research)
- Phase 2: 1 hour (detection)
- Phase 3: 1 hour (relaxation)
- Phase 4: 1 hour (multi-pass)
- Phase 5: 30 minutes (edge cases)

**Total:** 3-4 hours

---

## Test Strategy

### Unit Tests

**File:** `tests/unit/test_branch_relaxation.cpp` (new)

```cpp
TEST(BranchRelaxation, DetectsOutOfRangeForward) {
    // Branch +200 bytes (out of range)
}

TEST(BranchRelaxation, DetectsOutOfRangeBackward) {
    // Branch -200 bytes (out of range)
}

TEST(BranchRelaxation, DoesNotRelaxInRange) {
    // Branch ±50 bytes (in range)
}

TEST(BranchRelaxation, RelaxesBEQToBNEPlusJMP) {
    // Verify correct relaxation
}

TEST(BranchRelaxation, ConvergesInTwoPasses) {
    // Single relaxation converges quickly
}

TEST(BranchRelaxation, HandlesMultipleRelaxations) {
    // Multiple out-of-range branches
}
```

### Integration Tests

**File:** `tests/integration/test_branch_relaxation_integration.cpp` (new)

Test complete assembly programs with out-of-range branches.

---

## Dependencies and Risks

**Dependencies:**
- None (builds on existing multi-pass infrastructure)

**Risks:**

1. **Non-convergence** - Cascading size changes never stabilize
   - Mitigation: Max pass limit (10), detect cycles
   - Probability: Low (most programs converge in 2-3 passes)

2. **Performance** - Extra passes slow down assembly
   - Mitigation: Only relax when necessary, track size changes
   - Probability: Low (negligible for small programs)

3. **Incorrect complementary branches** - Wrong branch generated
   - Mitigation: Comprehensive unit tests for all 8 types
   - Probability: Very Low (simple mapping table)

---

## Success Criteria Checklist

```
□ Detection logic identifies out-of-range branches
□ Relaxation generates correct complementary + JMP
□ Multi-pass assembly converges
□ In-range branches not affected
□ All 8 branch types tested
□ Edge cases handled
□ Error messages clear
□ All tests passing (574/575 + new tests)
□ Zero compiler warnings
□ Code review approved
```

---

**Plan Version:** 1.0
**Last Updated:** 2026-01-29
