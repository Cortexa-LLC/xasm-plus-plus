# Multi-Pass Assembly Deep Dive

**Version:** 1.0  
**Date:** 2026-02-14  
**Status:** Reference Documentation

---

## Overview

The xasm++ assembler uses a sophisticated multi-pass algorithm to resolve forward references, optimize instruction sizes, and handle branch relaxation. This document provides a detailed explanation of the algorithm, implementation, and edge cases.

---

## Why Multi-Pass Assembly?

### The Forward Reference Problem

**Problem Statement:**

```assembly
; Example: Forward reference
start:
    JMP end         ; Question: How big is this instruction?
                    ; - If 'end' is close: 3 bytes (JMP absolute)
                    ; - If 'end' is far: Still 3 bytes on 6502
                    ; - But address of 'end' depends on instruction sizes!
    
    ; ... 1000 lines of code ...
    
end:
    RTS             ; Address depends on all previous sizes
```

**Key Insight:** The address of `end` depends on the sizes of all instructions between `start` and `end`, but the size of `JMP end` depends on the address of `end`.

**This is a circular dependency that requires iterative resolution.**

---

### The Branch Relaxation Problem

**Problem Statement:**

```assembly
; 6502 branch instruction example
start:
    BNE far_label   ; Question: Can we use a short branch (2 bytes)?
                    ; Or do we need workaround (5 bytes)?
    
    ; ... 200 bytes of code ...
    
far_label:
    RTS
```

**6502 Branch Constraints:**
- Branch instructions: 2 bytes (opcode + signed 8-bit offset)
- Range: -128 to +127 bytes from end of branch instruction
- If target is farther: Must use workaround (invert condition + JMP)

**Example Workaround:**

```assembly
; Original: BNE far_label (out of range)
; Becomes:
    BEQ skip        ; 2 bytes - invert condition
    JMP far_label   ; 3 bytes - absolute jump
skip:
```

**Challenge:** Changing branch size from 2 bytes to 5 bytes shifts all subsequent addresses, potentially causing OTHER branches to go out of range!

---

## Multi-Pass Algorithm

### High-Level Overview

```mermaid
flowchart TD
    Phase1["Phase 1: Initial Pass<br/>(Symbol Collection)<br/>• Assume all instructions at maximum size<br/>• Collect label addresses<br/>• Create initial symbol table"]
    
    Phase2["Phase 2: Fast Optimization<br/>(Passes 2-50)<br/>• Optimize ALL instructions simultaneously<br/>• Re-encode based on updated symbol table<br/>• Check convergence after each pass<br/>• Continue until stable OR 50 passes reached"]
    
    Conv1{"Converged?"}
    
    Phase3["Phase 3: Safe Optimization<br/>(Passes 51-500)<br/>• Optimize ONE instruction per pass<br/>• Prevents oscillation<br/>• Guarantees eventual convergence"]
    
    Conv2{"Converged?<br/>OR Max 500?"}
    
    Output["Success: Generate Output"]
    
    Phase1 --> Phase2
    Phase2 --> Conv1
    Conv1 -->|YES| Output
    Conv1 -->|NO (after 50 passes)| Phase3
    Phase3 --> Conv2
    Conv2 --> Output
    
    style Phase1 fill:#e1f5ff
    style Phase2 fill:#fff4e1
    style Phase3 fill:#ffe1f5
    style Conv1 fill:#fffacd
    style Conv2 fill:#fffacd
    style Output fill:#e1ffe1
```

---

## Implementation Details

### Pass 1: Initial Symbol Collection

**Purpose:** Establish initial addresses for all labels using worst-case size estimates.

**Algorithm:**

```cpp
void Assembler::Pass1_CollectSymbols() {
  uint32_t address = org_address_;
  
  for (auto& atom : atoms_) {
    switch (atom->type) {
      case AtomType::Label: {
        auto label = std::static_pointer_cast<LabelAtom>(atom);
        symbols_.Define(label->name, address);
        label->address = address;
        atom->size = 0;  // Labels take no space
        break;
      }
      
      case AtomType::Instruction: {
        auto inst = std::static_pointer_cast<InstructionAtom>(atom);
        
        // Use MAXIMUM size estimate for first pass
        atom->size = EstimateMaximumSize(inst);
        address += atom->size;
        break;
      }
      
      case AtomType::Data: {
        auto data = std::static_pointer_cast<DataAtom>(atom);
        atom->size = data->data.size();
        address += atom->size;
        break;
      }
      
      case AtomType::Org: {
        auto org = std::static_pointer_cast<OrgAtom>(atom);
        address = org->address;
        atom->size = 0;
        break;
      }
      
      case AtomType::Align: {
        auto align = std::static_pointer_cast<AlignAtom>(atom);
        // Calculate padding to next alignment boundary
        uint32_t padding = (align->alignment - (address % align->alignment)) 
                         % align->alignment;
        atom->size = padding;
        address += padding;
        break;
      }
      
      case AtomType::Space: {
        auto space = std::static_pointer_cast<SpaceAtom>(atom);
        atom->size = space->count;
        address += atom->size;
        break;
      }
    }
  }
}

size_t Assembler::EstimateMaximumSize(
    const std::shared_ptr<InstructionAtom>& inst) {
  
  // Ask CPU plugin for worst-case size
  // Example: 6502 LDA can be 2-3 bytes depending on addressing mode
  // Return maximum (3 bytes) for first pass
  
  return cpu_->EstimateMaximumInstructionSize(inst->mnemonic);
}
```

**Example:**

```assembly
; Source:
start:
    LDA #$42        ; Address: $8000, Size: 2 bytes (initial estimate: 3)
    STA $80         ; Address: $8003, Size: 2 bytes (initial estimate: 3)
    BNE start       ; Address: $8006, Size: 2 bytes (initial estimate: 5)
end:
    RTS             ; Address: $800B, Size: 1 byte

; After Pass 1:
; Symbol table:
;   start = $8000
;   end   = $800B (using worst-case sizes)
```

---

### Phase 2: Fast Optimization (Passes 2-50)

**Purpose:** Quickly converge on optimal instruction sizes by optimizing all instructions simultaneously.

**Algorithm:**

```cpp
AssemblerResult Assembler::Assemble() {
  AssemblerResult result;
  
  // Pass 1: Initial symbol collection
  Pass1_CollectSymbols();
  
  // Passes 2-50: Fast optimization
  std::vector<size_t> previous_sizes;
  std::vector<size_t> current_sizes;
  
  for (int pass = 2; pass <= FAST_PHASE_LIMIT; ++pass) {
    // Encode all instructions with current symbol table
    current_sizes = EncodeAllInstructions(result);
    
    // Check if sizes have stabilized
    if (pass > 2 && CheckConvergence(previous_sizes, current_sizes)) {
      result.pass_count = pass;
      result.success = true;
      return result;  // DONE - converged!
    }
    
    // Update symbol table based on new sizes
    UpdateSymbolAddresses();
    
    // Save sizes for next convergence check
    previous_sizes = current_sizes;
  }
  
  // If we reach here, fast phase didn't converge
  // Continue to safe optimization phase...
  return ContinueSafeOptimization(pass, result);
}
```

**Encoding Pass:**

```cpp
std::vector<size_t> Assembler::EncodeAllInstructions(
    AssemblerResult& result) {
  
  std::vector<size_t> instruction_sizes;
  uint32_t address = org_address_;
  
  for (auto& atom : atoms_) {
    if (atom->type == AtomType::Instruction) {
      auto inst = std::static_pointer_cast<InstructionAtom>(atom);
      
      try {
        // Encode instruction using CPU plugin
        inst->encoded_bytes = cpu_->EncodeInstruction(
            inst->mnemonic,
            ResolveOperand(inst->operand),
            inst->operand);
        
        // Update size based on actual encoding
        atom->size = inst->encoded_bytes.size();
        instruction_sizes.push_back(atom->size);
        
      } catch (const std::exception& e) {
        result.errors.push_back({
          e.what(),
          inst->location
        });
        instruction_sizes.push_back(0);  // Error - size unknown
      }
    }
    
    address += atom->size;
  }
  
  return instruction_sizes;
}
```

**Convergence Check:**

```cpp
bool Assembler::CheckConvergence(
    const std::vector<size_t>& previous_sizes,
    const std::vector<size_t>& current_sizes) const {
  
  if (previous_sizes.size() != current_sizes.size()) {
    return false;  // Size vector changed (shouldn't happen)
  }
  
  // Check if ALL sizes are identical
  for (size_t i = 0; i < previous_sizes.size(); ++i) {
    if (previous_sizes[i] != current_sizes[i]) {
      return false;  // At least one size changed
    }
  }
  
  return true;  // All sizes stable - CONVERGED!
}
```

**Why Fast Phase Works:**

Most programs converge in 2-3 passes:
- Pass 2: Optimize all instructions simultaneously
- Pass 3: Re-check with updated addresses
- Pass 4: Confirm no changes (convergence)

**Typical Pass Counts:**

| Program Complexity | Pass Count | Reason |
|--------------------|------------|---------|
| No forward references | 1 | All addresses known immediately |
| Simple forward refs | 2 | One pass to establish, one to verify |
| Branch relaxation (1 level) | 2-3 | Detect long branch, re-encode, verify |
| Nested dependencies | 3-5 | Multiple cascading size changes |
| Pathological cases | 50+ | Oscillating sizes (switch to safe mode) |

---

### Phase 3: Safe Optimization (Passes 51-500)

**Purpose:** Handle pathological cases where fast optimization causes oscillation.

**Problem: Oscillation Example:**

```assembly
; Pathological case (theoretical):
label1:
    BNE label2      ; Pass N: Short (2 bytes) -> label2 at $8100
    ; ... 125 bytes ...
label2:
    BNE label3      ; Pass N: Short (2 bytes) -> label3 at $817F
    ; ... 125 bytes ...
label3:
    BNE label1      ; Pass N: Short (2 bytes) -> label1 at $8000 (OK)

; Pass N+1: label3 BNE sees label1 at $8005 (if label1 expands)
;   -> Out of range! -> Expand to 5 bytes
;   -> label2 shifts to $8103
;   -> label2 BNE sees label3 at $8182 -> Out of range!
;   -> Expand to 5 bytes
;   -> label3 shifts to $8187
;   -> label3 BNE sees label1 at $8000 -> IN range again!
;   -> Shrink back to 2 bytes
;   -> Back to original state (OSCILLATION!)
```

**Solution: Optimize One Instruction Per Pass**

```cpp
AssemblerResult Assembler::ContinueSafeOptimization(
    int starting_pass,
    AssemblerResult& result) {
  
  for (int pass = starting_pass; pass <= MAX_PASSES; ++pass) {
    bool any_changes = false;
    int changes_this_pass = 0;
    
    // Encode instructions, but stop after FIRST size change
    uint32_t address = org_address_;
    for (auto& atom : atoms_) {
      if (atom->type != AtomType::Instruction) {
        address += atom->size;
        continue;
      }
      
      auto inst = std::static_pointer_cast<InstructionAtom>(atom);
      
      // Encode instruction
      inst->encoded_bytes = cpu_->EncodeInstruction(
          inst->mnemonic,
          ResolveOperand(inst->operand),
          inst->operand);
      
      size_t new_size = inst->encoded_bytes.size();
      
      if (new_size != atom->last_size) {
        // Size changed!
        atom->changes++;
        atom->last_size = new_size;
        atom->size = new_size;
        any_changes = true;
        changes_this_pass++;
        
        // Warn if oscillating
        if (atom->changes > MAX_SIZE_CHANGES) {
          result.errors.push_back({
            "Warning: Instruction size oscillating at " + 
            std::to_string(address),
            inst->location
          });
        }
        
        // KEY: Stop after first change in safe mode
        break;
      }
      
      address += atom->size;
    }
    
    // If no changes, we've converged
    if (!any_changes) {
      result.pass_count = pass;
      result.success = true;
      return result;
    }
    
    // Update symbols for next pass
    UpdateSymbolAddresses();
  }
  
  // Max passes exceeded without convergence
  result.success = false;
  result.errors.push_back({
    "Max passes exceeded without convergence",
    {}
  });
  return result;
}
```

**Why One-Change-Per-Pass Guarantees Convergence:**

1. **Monotonic Progress:** Each pass resolves at least one instruction
2. **No Oscillation:** Only one instruction changes per pass, so no cascading effects
3. **Finite Termination:** At most N passes for N instructions (worst case)

**Example Trace:**

```
Pass 51: Change instruction at $8000 (2→5 bytes)
         Update symbols, re-encode
Pass 52: Change instruction at $8100 (2→5 bytes)
         Update symbols, re-encode
Pass 53: Change instruction at $8200 (2→5 bytes)
         Update symbols, re-encode
Pass 54: No changes -> CONVERGED
```

---

## Branch Relaxation Handling

### 6502 Branch Relaxation

**6502 Branch Instructions:**
- `BCC`, `BCS`, `BEQ`, `BMI`, `BNE`, `BPL`, `BVC`, `BVS`
- Format: `opcode + signed 8-bit offset`
- Size: Always 2 bytes
- Range: -128 to +127 bytes from end of instruction

**Relaxation Strategy:**

```cpp
std::vector<uint8_t> Cpu6502BranchHandler::Encode(
    const std::string& mnemonic,
    const std::string& operand_str,
    uint16_t current_address) const {
  
  // Resolve target address
  uint16_t target_address = ParseTargetAddress(operand_str);
  
  // Calculate relative offset
  // Offset = target - (current + 2)
  // +2 because offset is from END of instruction
  int16_t offset = static_cast<int16_t>(target_address)
                 - static_cast<int16_t>(current_address + 2);
  
  // Check if offset fits in 8-bit signed range
  if (offset >= -128 && offset <= 127) {
    // Short branch - use native branch instruction
    uint8_t opcode = LookupBranchOpcode(mnemonic);
    return {opcode, static_cast<uint8_t>(offset & 0xFF)};
  } else {
    // Long branch - use workaround
    // Original: BNE target
    // Becomes:  BEQ skip (invert condition)
    //           JMP target
    //           skip:
    
    uint8_t inverted_opcode = InvertBranchCondition(mnemonic);
    uint8_t skip_offset = 3;  // Skip over JMP instruction
    
    // JMP absolute encoding (little-endian)
    uint8_t jmp_lo = target_address & 0xFF;
    uint8_t jmp_hi = (target_address >> 8) & 0xFF;
    
    return {
      inverted_opcode, skip_offset,  // BEQ +3
      0x4C, jmp_lo, jmp_hi          // JMP target
    };
  }
}

uint8_t Cpu6502BranchHandler::InvertBranchCondition(
    const std::string& mnemonic) const {
  
  static const std::unordered_map<std::string, uint8_t> inversions = {
    {"BCC", 0xB0},  // BCC -> BCS
    {"BCS", 0x90},  // BCS -> BCC
    {"BEQ", 0xD0},  // BEQ -> BNE
    {"BMI", 0x10},  // BMI -> BPL
    {"BNE", 0xF0},  // BNE -> BEQ
    {"BPL", 0x30},  // BPL -> BMI
    {"BVC", 0x70},  // BVC -> BVS
    {"BVS", 0x50},  // BVS -> BVC
  };
  
  auto it = inversions.find(mnemonic);
  if (it == inversions.end()) {
    throw std::invalid_argument("Not a branch instruction: " + mnemonic);
  }
  
  return it->second;
}
```

**Example:**

```assembly
; Source:
start:
    BNE far_away
    ; ... 200 bytes ...
far_away:
    RTS

; Pass 1: Assume short branch
; start:    BNE far_away    ; $8000-$8001 (2 bytes)
; ...
; far_away: RTS             ; $80C8 (address)

; Pass 2: Calculate offset
; Offset = $80C8 - ($8000 + 2) = $80C8 - $8002 = $00C6 = 198 bytes
; 198 > 127 -> OUT OF RANGE!

; Pass 2: Use long branch workaround
; start:    BEQ skip        ; $8000-$8001 (2 bytes)
;           JMP far_away    ; $8002-$8004 (3 bytes)
; skip:
; ...
; far_away: RTS             ; $80CB (address shifted by +3)

; Pass 3: Verify
; No more size changes -> CONVERGED
```

---

## 6809 Branch Relaxation

**6809 Branch Instructions:**
- Short branches: `BCC`, `BCS`, `BEQ`, etc. (2 bytes, ±127 range)
- Long branches: `LBCC`, `LBCS`, `LBEQ`, etc. (4 bytes, ±32767 range)

**Relaxation Strategy (Simpler than 6502):**

```cpp
std::vector<uint8_t> Cpu6809::EncodeBranch(
    const std::string& mnemonic,
    uint16_t current_address,
    uint16_t target_address) const {
  
  // Calculate offset
  int16_t offset = static_cast<int16_t>(target_address)
                 - static_cast<int16_t>(current_address + 2);
  
  if (offset >= -128 && offset <= 127) {
    // Short branch (2 bytes)
    uint8_t opcode = LookupShortBranchOpcode(mnemonic);
    return {opcode, static_cast<uint8_t>(offset & 0xFF)};
  } else {
    // Long branch (4 bytes)
    // Recalculate offset for 4-byte instruction
    int16_t long_offset = static_cast<int16_t>(target_address)
                        - static_cast<int16_t>(current_address + 4);
    
    if (long_offset < -32768 || long_offset > 32767) {
      throw std::out_of_range("Branch target too far");
    }
    
    uint16_t opcode = LookupLongBranchOpcode(mnemonic);  // 16-bit opcode
    uint8_t offset_hi = (long_offset >> 8) & 0xFF;
    uint8_t offset_lo = long_offset & 0xFF;
    
    return {
      static_cast<uint8_t>(opcode >> 8),
      static_cast<uint8_t>(opcode & 0xFF),
      offset_hi,
      offset_lo
    };
  }
}
```

**6809 Advantage:** Native long branch instructions (no workaround needed)

---

## Z80 Branch Handling

**Z80 Branch Instructions:**
- Relative: `JR`, `JR cc` (2 bytes, ±127 range)
- Absolute: `JP`, `JP cc` (3 bytes, full 16-bit range)

**Relaxation Strategy:**

```cpp
std::vector<uint8_t> CpuZ80::EncodeBranch(
    const std::string& mnemonic,
    uint16_t current_address,
    uint16_t target_address) const {
  
  // Try relative jump first (JR)
  if (mnemonic == "JR" || mnemonic.substr(0, 2) == "JR") {
    int16_t offset = static_cast<int16_t>(target_address)
                   - static_cast<int16_t>(current_address + 2);
    
    if (offset >= -128 && offset <= 127) {
      uint8_t opcode = LookupRelativeJumpOpcode(mnemonic);
      return {opcode, static_cast<uint8_t>(offset & 0xFF)};
    } else {
      // Upgrade to absolute jump (JP)
      std::string absolute_mnemonic = "JP" + mnemonic.substr(2);
      uint8_t opcode = LookupAbsoluteJumpOpcode(absolute_mnemonic);
      return {opcode,
              static_cast<uint8_t>(target_address & 0xFF),
              static_cast<uint8_t>((target_address >> 8) & 0xFF)};
    }
  }
  
  // Absolute jump (JP)
  uint8_t opcode = LookupAbsoluteJumpOpcode(mnemonic);
  return {opcode,
          static_cast<uint8_t>(target_address & 0xFF),
          static_cast<uint8_t>((target_address >> 8) & 0xFF)};
}
```

---

## Edge Cases and Challenges

### 1. Oscillating Sizes

**Problem:**
```assembly
; Theoretical case where sizes oscillate
label1: BNE label2  ; Pass N: 2 bytes -> Pass N+1: 5 bytes -> Pass N+2: 2 bytes
label2: BNE label1  ; Pass N: 2 bytes -> Pass N+1: 5 bytes -> Pass N+2: 2 bytes
```

**Solution:** Safe optimization mode (one change per pass)

**Detection:**
```cpp
if (atom->changes > MAX_SIZE_CHANGES) {
  // Warning: possible oscillation
  // Investigate manually or force size
}
```

---

### 2. Circular Dependencies

**Problem:**
```assembly
; Circular forward references
size1 equ end1 - start1
start1:
    ds size2        ; Reserve size2 bytes
end1:

size2 equ end2 - start2
start2:
    ds size1        ; Reserve size1 bytes
end2:
```

**Solution:** Multi-pass naturally resolves this through iteration

**Example Resolution:**
```
Pass 1: size1 = 0 (unknown), size2 = 0 (unknown)
        Reserve 0 bytes for both
Pass 2: size1 = 0, size2 = 0 (still)
        Reserve 0 bytes for both
Pass 3: Converged (both are 0)
```

---

### 3. Expression Dependencies

**Problem:**
```assembly
value1 equ value2 + 1
value2 equ value3 + 1
value3 equ 42
```

**Solution:** Topological sort of symbol dependencies (future enhancement)

**Current Behavior:** Multi-pass evaluation resolves in 3 passes

---

### 4. Alignment Padding

**Problem:**
```assembly
start:
    nop             ; 1 byte
    align 256       ; Padding depends on current address
    ; If start=$8000, padding=255
    ; If start=$8001, padding=255
    ; If start=$8100, padding=0
```

**Solution:** Recalculate alignment padding each pass

```cpp
case AtomType::Align: {
  auto align = std::static_pointer_cast<AlignAtom>(atom);
  uint32_t padding = (align->alignment - (address % align->alignment))
                   % align->alignment;
  atom->size = padding;
  address += padding;
  break;
}
```

---

## Performance Optimization

### Caching Expression Evaluation

**Problem:** Expressions re-evaluated every pass

**Solution:** Cache constant expressions

```cpp
class ExpressionCache {
public:
  int64_t Evaluate(const Expression* expr, const SymbolTable& symbols) {
    // If expression is constant, cache result
    if (expr->IsConstant()) {
      auto it = cache_.find(expr);
      if (it != cache_.end()) {
        return it->second;  // Cache hit
      }
      int64_t value = expr->Evaluate(symbols);
      cache_[expr] = value;
      return value;
    }
    
    // Non-constant: always re-evaluate
    return expr->Evaluate(symbols);
  }

private:
  std::unordered_map<const Expression*, int64_t> cache_;
};
```

---

### Incremental Symbol Updates

**Problem:** Full symbol table rebuild every pass

**Optimization:** Only update changed atom addresses

```cpp
void Assembler::IncrementalSymbolUpdate() {
  uint32_t address = org_address_;
  
  for (auto& atom : atoms_) {
    if (atom->type == AtomType::Label) {
      auto label = std::static_pointer_cast<LabelAtom>(atom);
      
      if (label->address != address) {
        // Address changed - update symbol table
        symbols_.Update(label->name, address);
        label->address = address;
      }
    }
    
    address += atom->size;
  }
}
```

---

## Testing Strategy

### Unit Tests for Convergence

```cpp
TEST(Assembler, SimpleForwardReference) {
  Assembler asm;
  Cpu6502 cpu;
  ConcreteSymbolTable symbols;
  
  asm.SetCpuPlugin(&cpu);
  asm.SetSymbolTable(&symbols);
  
  Section code("CODE", 0x01, 0x8000);
  code.atoms.push_back(std::make_shared<InstructionAtom>("JMP", "end"));
  code.atoms.push_back(std::make_shared<InstructionAtom>("NOP", ""));
  code.atoms.push_back(std::make_shared<LabelAtom>("end", 0));
  code.atoms.push_back(std::make_shared<InstructionAtom>("RTS", ""));
  
  asm.AddSection(code);
  AssemblerResult result = asm.Assemble();
  
  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.pass_count, 2);  // Should converge in 2 passes
}

TEST(Assembler, BranchRelaxation) {
  // Test that branch relaxation converges
  // ... (create scenario with out-of-range branch)
  
  AssemblerResult result = asm.Assemble();
  
  EXPECT_TRUE(result.success);
  EXPECT_LE(result.pass_count, 5);  // Should converge within 5 passes
}

TEST(Assembler, MaxPassesExceeded) {
  // Create pathological oscillating case
  // ... (artificially create oscillation)
  
  AssemblerResult result = asm.Assemble();
  
  EXPECT_FALSE(result.success);
  EXPECT_TRUE(result.errors.size() > 0);
  EXPECT_THAT(result.errors[0].message, HasSubstr("Max passes"));
}
```

---

## Future Enhancements

### 1. Dependency Analysis

**Goal:** Determine which instructions depend on which symbols

**Benefit:** Optimize only affected instructions when symbols change

```cpp
struct InstructionDependency {
  std::shared_ptr<InstructionAtom> instruction;
  std::set<std::string> depends_on_symbols;
};

// Only re-encode instructions that depend on changed symbols
void Assembler::IncrementalReencode(
    const std::set<std::string>& changed_symbols) {
  
  for (auto& dep : dependencies_) {
    // Check if any dependency changed
    for (const auto& symbol : dep.depends_on_symbols) {
      if (changed_symbols.count(symbol) > 0) {
        // Re-encode this instruction
        Reencode(dep.instruction);
        break;
      }
    }
  }
}
```

---

### 2. Parallel Encoding

**Goal:** Encode independent instructions in parallel

**Challenge:** Shared symbol table (read-only during encoding pass)

```cpp
void Assembler::ParallelEncode() {
  // Identify independent instructions (no shared dependencies)
  std::vector<std::vector<InstructionAtom*>> independent_groups = 
      IdentifyIndependentGroups();
  
  // Encode each group in parallel
  #pragma omp parallel for
  for (auto& group : independent_groups) {
    for (auto* inst : group) {
      inst->encoded_bytes = cpu_->EncodeInstruction(
          inst->mnemonic, /*...*/);
    }
  }
}
```

---

### 3. Smart Convergence Prediction

**Goal:** Predict when convergence will occur

**Benefit:** Early termination in pathological cases

```cpp
bool Assembler::PredictConvergence(const std::vector<size_t>& size_history) {
  // Analyze size change patterns
  if (size_history.size() < 10) return false;
  
  // Check for oscillation pattern
  bool oscillating = IsOscillating(size_history);
  if (oscillating) {
    // Switch to safe mode immediately
    return false;
  }
  
  // Check for monotonic decrease
  bool decreasing = IsMonotonicallyDecreasing(size_history);
  if (decreasing) {
    // Likely to converge soon
    return true;
  }
  
  return false;
}
```

---

## References

- [vasm-ext Multi-Pass Algorithm](http://sun.hasenbraten.de/vasm/) - Reference implementation
- [Assembler Design](https://en.wikipedia.org/wiki/Assembly_language#Assembler) - Wikipedia overview
- [Branch Relaxation](https://en.wikipedia.org/wiki/Relaxation_(iterative_method)) - General concept

---

**Document Version:** 1.0  
**Last Updated:** 2026-02-14  
**Related:** [Architecture README](README.md), [Plugin Architecture](plugin-architecture.md)
