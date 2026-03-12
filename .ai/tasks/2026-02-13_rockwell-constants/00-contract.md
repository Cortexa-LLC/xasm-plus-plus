# Task Contract

**Task ID:** 2026-02-13_rockwell-constants
**Created:** 2026-02-13
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Refactor
**Beads Task:** xasm++-utqb

---

## Task Description

Eliminate 64 magic strings and numeric literals in Rockwell 65C02 support by replacing them with named constants.

### Background and Context

During code review of A2oSX compatibility fixes, an inconsistency was identified: the 65C02 and 65816 instruction sets use named constants from the M6502Mnemonics namespace, but the Rockwell 65C02 extensions (RMB, SMB, BBR, BBS) use magic strings and hex literals directly.

This violates clean code principles and creates maintenance issues:
- Magic strings prevent compile-time validation
- Hex literals lack semantic meaning (0x07 vs RockwellOpcodes::RMB0)
- Inconsistent with existing codebase patterns

### Current State

**File: `src/cpu/cpu_6502.cpp`**

**Lines 2467-2470:** 32 magic mnemonic strings in `HasOpcode()`:
```cpp
// 65C02 Rockwell extensions (RMB, SMB, BBR, BBS)
"RMB0", "RMB1", "RMB2", "RMB3", "RMB4", "RMB5", "RMB6", "RMB7", "SMB0",
"SMB1", "SMB2", "SMB3", "SMB4", "SMB5", "SMB6", "SMB7", "BBR0", "BBR1",
"BBR2", "BBR3", "BBR4", "BBR5", "BBR6", "BBR7", "BBS0", "BBS1", "BBS2",
"BBS3", "BBS4", "BBS5", "BBS6", "BBS7",
```

**Lines 1254-1500+:** 32 magic opcode bytes in `Encode*()` functions:
```cpp
std::vector<uint8_t> Cpu6502::EncodeRMB0(uint8_t operand, AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {0x07, operand}; // Magic hex 0x07
}

std::vector<uint8_t> Cpu6502::EncodeRMB1(uint8_t operand, AddressingMode mode) const {
  return {0x17, operand}; // Magic hex 0x17
}
// ... 30 more functions with magic hex values
```

### Desired State

**File: `include/xasm++/cpu/opcodes_6502.h`**

Add Rockwell constants namespace following existing patterns:
```cpp
namespace RockwellMnemonics {
  constexpr const char* RMB0 = "RMB0";
  constexpr const char* RMB1 = "RMB1";
  // ... 30 more
}

namespace RockwellOpcodes {
  constexpr uint8_t RMB0 = 0x07;
  constexpr uint8_t RMB1 = 0x17;
  // ... 30 more
}
```

**File: `src/cpu/cpu_6502.cpp`**

Replace magic values with constants:
```cpp
// HasOpcode() - use RockwellMnemonics::
RockwellMnemonics::RMB0, RockwellMnemonics::RMB1, ...

// EncodeRMB0() - use RockwellOpcodes::
return {RockwellOpcodes::RMB0, operand};
```

---

## Success Criteria

```
✓ All 32 mnemonic magic strings replaced with RockwellMnemonics:: constants
✓ All 32 opcode magic bytes replaced with RockwellOpcodes:: constants
✓ All 1564 existing tests passing
✓ Zero compiler warnings
✓ Code follows existing patterns (M6502Mnemonics, M6502Opcodes)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Define RockwellMnemonics namespace in opcodes_6502.h (32 constants)
□ Define RockwellOpcodes namespace in opcodes_6502.h (32 constants)
□ Replace 32 magic strings in HasOpcode() with RockwellMnemonics:: references
□ Replace 32 magic bytes in Encode*() functions with RockwellOpcodes:: references
□ No magic strings remain for Rockwell instructions
□ No magic hex literals remain for Rockwell opcodes
```

### Quality Requirements
```
□ All tests passing (1564/1564)
□ Zero compiler warnings
□ No linting errors
□ Code formatted (clang-format)
□ Follows existing constant patterns in codebase
```

### Non-Functional Requirements
```
□ No behavior changes (pure refactoring)
□ Binary-identical output (opcodes unchanged)
□ Compile-time validation via constants
□ Consistent with 65C02 and 65816 patterns
```

---

## Constraints and Dependencies

### Constraints
```
□ MUST follow existing naming pattern: M6502Mnemonics, M6502Opcodes
□ MUST use constexpr for compile-time constants
□ MUST maintain binary compatibility (same opcodes)
□ MUST NOT change behavior (tests prove equivalence)
```

### Dependencies
```
□ No external dependencies
□ Self-contained refactoring (2 files)
```

### Out of Scope
```
✗ Other CPU instruction sets (Z80, 6809)
✗ Other magic values in non-Rockwell code
✗ Functional changes or new features
✗ Performance optimizations
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Number of files affected: 2 (opcodes_6502.h, cpu_6502.cpp)
- Lines of code estimate: ~150 (64 constants + replacements)
- New concepts/patterns: No (following existing pattern)
- Integration complexity: Low (self-contained)
- Risk level: Low (pure refactoring, tests validate)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 2 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 2 files → ✅ IDEAL
```

**Status:** No decomposition needed. Optimal batch size.

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
2 × 3,000 tokens = 6,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 6,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

**Rationale:**
- 2 files (well within IDEAL range)
- 6K tokens (safe, 19K below limit)
- 1 agent (minimal WIP)
- High cohesion (constants + usage)
- Single concern (Rockwell magic values)

---

## Resources and References

### Relevant Files
```
- include/xasm++/cpu/opcodes_6502.h - Add Rockwell constants (M6502Mnemonics pattern)
- src/cpu/cpu_6502.cpp - Replace magic values (lines 1254-1500, 2467-2470)
```

### Documentation
```
- .ai-pack/quality/clean-code/magic-numbers.md - Magic number elimination
- .ai-pack/quality/engineering-standards.md - Clean code standards
```

### Examples
```
- M6502Mnemonics namespace (opcodes_6502.h) - Existing mnemonic pattern
- M6502Opcodes namespace (opcodes_6502.h) - Existing opcode pattern
```

---

## Assumptions

```
1. RockwellMnemonics and RockwellOpcodes namespaces are appropriate names
2. Existing test suite validates behavior preservation
3. No other files reference Rockwell magic values
4. Binary compatibility is critical (opcodes unchanged)
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Behavior change during refactoring
   - Probability: Low
   - Impact: High
   - Mitigation: Tests validate equivalence (1564 tests)

2. Missed magic values in other files
   - Probability: Low
   - Impact: Medium
   - Mitigation: Grep search confirms only cpu_6502.cpp

3. Breaking existing assembly code
   - Probability: Very Low
   - Impact: Critical
   - Mitigation: Pure refactoring (opcodes unchanged)
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan Woodruff (2026-02-13)
- [ ] Agent: Engineer (pending)

**Changes to Contract:**
None.

---

## Notes

This task was identified during A2oSX compatibility work when comparing Rockwell extension implementation to standard 65C02/65816 patterns. The inconsistency creates maintenance burden and violates clean code principles.

The refactoring is low-risk because:
1. Only 2 files affected
2. Comprehensive test coverage (1564 tests)
3. No behavior changes (opcodes identical)
4. Following established codebase patterns

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-13
