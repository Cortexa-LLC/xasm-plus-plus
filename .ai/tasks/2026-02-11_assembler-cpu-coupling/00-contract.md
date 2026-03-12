# Task Contract: Core Assembler CPU Coupling Removal

**Task ID:** 2026-02-11_assembler-cpu-coupling
**Beads Task:** xasm++-syq1
**Created:** 2026-02-11
**Requestor:** User (identified architectural violation)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Remove CPU-specific code and magic strings from `src/core/assembler.cpp`, restoring proper plugin abstraction. The core assembler currently has hardcoded 6502-specific logic that violates the plugin architecture and prevents clean addition of new CPU architectures.

### Background and Context

**User Discovery:** While reviewing god-class cleanup work (Phase 6), user identified that `src/core/assembler.cpp` contains 6502-specific code with magic strings and numbers, directly contradicting the magic number/string elimination work being done throughout the codebase.

**Architectural Principle Violated:** The assembler uses a plugin architecture (`CpuPlugin` interface) to support multiple CPU architectures. The core assembler should be CPU-agnostic and delegate ALL CPU-specific logic to plugins. Currently, this abstraction is violated by hardcoded 6502 knowledge in the core.

**Magic Number/String Problem:** User mandated that ALL magic strings and numbers must be eliminated throughout god-class cleanup. The assembler.cpp violations directly contradict this requirement.

### Current State

`src/core/assembler.cpp` (lines 1-920) contains:

1. **CPU-specific includes** (lines 4-5):
   ```cpp
   #include "xasm++/cpu/cpu_6502.h"
   #include "xasm++/cpu/opcodes_6502.h"
   ```

2. **Hardcoded branch handling** (lines 540-586):
   - Magic string literals: `"BEQ"`, `"BNE"`, `"BCC"`, `"BCS"`, `"BMI"`, `"BPL"`, `"BVC"`, `"BVS"`, `"BLT"`, `"BRA"` (lines 541-544)
   - Explicit opcode mapping (lines 553-572)
   - Direct cast breaking abstraction: `static_cast<Cpu6502 *>(cpu_)` (line 576)
   - Calling 6502-specific method: `EncodeBranchWithRelaxation()` (line 577)

3. **65816-specific MVN/MVP handling** (lines 589-640):
   - Magic string literals: `"MVN"`, `"MVP"` (line 589)
   - Another direct cast to `Cpu6502*` (line 621)
   - Special-case encoding logic in core

**Impact:**
- Violates Open/Closed Principle (must modify core to add new CPUs)
- Prevents clean Z80/6809 support (no branch relaxation for these CPUs)
- Contradicts magic string elimination mandate
- Creates tight coupling between core and 6502 implementation

### Desired State

**Clean Plugin Abstraction:**

1. **Core assembler** (`assembler.cpp`):
   - NO CPU-specific includes
   - NO magic strings for mnemonics
   - NO direct casts to concrete CPU types
   - ALL encoding delegated through `CpuPlugin` interface
   - Only knows generic concepts: `AtomType`, `AddressingMode`, `Section`

2. **Extended CpuPlugin interface** (`cpu_plugin.h`):
   - Add virtual method for branch relaxation detection
   - Add virtual method for multi-byte instruction handling
   - CPU plugins handle ALL special cases

3. **6502 Plugin** (`cpu_6502.cpp`):
   - Implements branch relaxation in plugin
   - Handles MVN/MVP encoding in plugin
   - Uses mnemonic constants (no magic strings)

**Example transformation:**

```cpp
// BEFORE (assembler.cpp) - WRONG
if (mnemonic == "BEQ") branch_opcode = Opcodes::BEQ;
Cpu6502 *cpu6502 = static_cast<Cpu6502 *>(cpu_);
inst->encoded_bytes = cpu6502->EncodeBranchWithRelaxation(...);

// AFTER (assembler.cpp) - CORRECT
inst->encoded_bytes = cpu_->EncodeInstruction(mnemonic, value, operand, current_address);
// CPU plugin handles everything including relaxation
```

---

## Success Criteria

```
✓ Zero CPU-specific includes in src/core/assembler.cpp
✓ Zero magic strings for mnemonics in assembler.cpp
✓ Zero direct casts to concrete CPU types
✓ All instruction encoding through CpuPlugin interface
✓ All existing tests passing (97%+ coverage maintained)
✓ Build clean with 0 warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Branch instructions encode correctly (all mnemonics)
□ Branch relaxation works (short → long when needed)
□ MVN/MVP instructions encode correctly
□ All CPU architectures still work (6502, Z80, 6809)
□ Multi-pass assembly converges correctly
□ Forward references resolve correctly
```

### Code Quality Requirements (CRITICAL)
```
□ NO magic strings in assembler.cpp (use mnemonic constants)
□ NO CPU-specific includes in assembler.cpp
□ NO static_cast to concrete CPU types in assembler.cpp
□ ALL CPU logic in CpuPlugin implementations
□ CpuPlugin interface extended cleanly (virtual methods)
□ Branch relaxation in CPU plugin (not core)
```

### Quality Requirements
```
□ All tests passing (97%+)
□ No warnings
□ TDD followed (RED-GREEN-REFACTOR)
□ Code formatted per project standards
□ Functions documented
```

---

## Constraints and Dependencies

### Constraints
```
□ MUST maintain backward compatibility (all tests pass)
□ MUST NOT break existing CPU plugins (Z80, 6809)
□ MUST use virtual methods (no template metaprogramming)
□ MUST eliminate ALL magic strings (user mandate)
```

### Dependencies
```
□ Depends on: God-Class Phase 6 understanding (pattern established)
□ Blocks: Clean CPU architecture support
□ Reference: .ai/tasks/2026-02-10_god-class-phase6b-core/ (magic string elimination pattern)
```

### Out of Scope
```
✗ Adding new CPU architectures (only refactoring existing)
✗ Optimizing branch relaxation algorithm (preserve existing behavior)
✗ Changing CpuPlugin interface radically (extend, don't rewrite)
✗ Addressing mode optimization (separate concern)
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: 5-7 files
- Lines of code estimate: ~200 lines moved, ~50 lines new interface
- New concepts/patterns: Extend existing plugin interface (familiar pattern)
- Integration complexity: Medium (multi-pass assembly interaction)
- Risk level: Medium (affects core assembler, needs careful testing)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 5-7 files
- `src/core/assembler.cpp` - Remove CPU-specific code
- `include/xasm++/assembler.h` - Interface changes (if needed)
- `include/xasm++/cpu/cpu_plugin.h` - Extend interface
- `include/xasm++/cpu/cpu_6502.h` - Add branch relaxation
- `src/cpu/m6502/cpu_6502.cpp` - Implement in plugin
- `tests/unit/test_assembler.cpp` - Update tests
- `tests/integration/test_branch_relaxation.cpp` - Verify behavior

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 5-7 files → ✅ IDEAL (at upper bound, acceptable)
```

### Batch Size Justification

Files: 7 (within ideal range, no decomposition needed)

**Why not decomposed further:**
- High cohesion: All files relate to single architectural concern (plugin abstraction)
- Single change: Moving branch relaxation from core to plugin
- Already minimal viable batch: Cannot split without losing context

**Contingency for token limits:**
- If token limit hit during implementation, will split into:
  - Subtask 1: CpuPlugin interface extension (3 files)
  - Subtask 2: Assembler.cpp refactoring (4 files)

**Estimated tokens:** ~7 × 3,000 = 21,000 tokens
**Status:** Within 25K-32K limit? YES ✅ (SAFE)

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
7 × 3,000 tokens = 21,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 21,000 tokens → ⚠️ APPROACHING LIMIT (but acceptable)
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (Engineer - sequential execution)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files (7 files) AND token budget ≤42K (21K) AND WIP ≤3 (1 agent)
- [ ] NO - MUST decompose

**Justification:** This is a highly cohesive refactoring focused on a single architectural concern. All files are tightly coupled to the plugin abstraction change. Splitting would require passing context between subtasks, increasing overall complexity.

---

## Resources and References

### Relevant Files
```
- src/core/assembler.cpp:540-640 - Hardcoded 6502 logic to remove
- include/xasm++/cpu/cpu_plugin.h - Interface to extend
- src/cpu/m6502/cpu_6502.cpp - Plugin implementation
- docs/architecture/cpu-plugin-architecture.md - Plugin design (if exists)
```

### Documentation
```
- .ai/tasks/2026-02-10_god-class-phase6b-core/00-contract.md - Magic string elimination pattern
- .ai-pack/quality/clean-code/magic-numbers.md - Magic number/string standards
- .ai-pack/workflows/refactor.md - Refactoring workflow
```

### Examples
```
- Phase 6b directive extraction: Established pattern for moving logic from core to plugins
- DirectiveRegistry pattern: Similar abstraction (directive handlers → plugins)
```

---

## Assumptions

```
1. CpuPlugin interface can be extended with virtual methods without breaking existing plugins
2. Branch relaxation logic can be moved to CPU plugin without changing behavior
3. All existing tests provide adequate coverage to detect regressions
4. Z80/6809 plugins don't need branch relaxation (6502-specific feature)
5. MVN/MVP are 6502-family specific (no equivalent in Z80/6809)
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Breaking multi-pass assembly convergence
   - Probability: Medium
   - Impact: High (assembly produces incorrect output)
   - Mitigation: Comprehensive test coverage, TDD with existing tests as RED phase

2. Plugin interface change breaks Z80/6809 plugins
   - Probability: Low
   - Impact: High (other CPUs stop working)
   - Mitigation: Make new virtual methods with default implementations

3. Branch relaxation behavior changes subtly
   - Probability: Medium
   - Impact: High (wrong code generation)
   - Mitigation: Keep exact same algorithm, just move location

4. Performance regression in multi-pass assembly
   - Probability: Low
   - Impact: Low (slower assembly)
   - Mitigation: Profile if concerned, but virtual call overhead is negligible
```

---

## Special Instructions

**Refactoring Strategy:**

1. **Phase 1: Extend CpuPlugin Interface**
   - Add `virtual bool RequiresBranchRelaxation(const std::string& mnemonic) const`
   - Add `virtual std::vector<uint8_t> EncodeBranchWithRelaxation(...) const`
   - Add default implementations (return false, throw exception)

2. **Phase 2: Implement in Cpu6502**
   - Move branch detection logic to `RequiresBranchRelaxation()`
   - Move branch relaxation algorithm to plugin
   - Remove magic strings, use mnemonic constants

3. **Phase 3: Refactor Assembler.cpp**
   - Remove CPU-specific includes
   - Replace hardcoded branch check with `cpu_->RequiresBranchRelaxation()`
   - Replace direct cast with plugin method call
   - Remove MVN/MVP special case (delegate to plugin)

4. **Phase 4: Verify**
   - All tests pass
   - No warnings
   - No magic strings in assembler.cpp
   - Clean plugin abstraction

**TDD Approach:**
- RED: Existing tests should pass (baseline)
- GREEN: Refactor while keeping tests green
- REFACTOR: Clean up, eliminate magic strings

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: User (2026-02-11)
- [ ] Agent: Engineer (TBD)

**Changes to Contract:**
None yet.

---

## Notes

**User Context:** This task emerged from god-class cleanup Phase 6 work. The user has been systematically eliminating magic numbers/strings throughout the codebase and discovered this significant violation in the core assembler.

**Architectural Significance:** This is not just a cleanup task - it's restoring a fundamental architectural principle (plugin abstraction). Successfully completing this unblocks clean support for additional CPU architectures.

**Relation to Phase 6:** While discovered during Phase 6 (directive handler extraction), this is a separate concern. Phase 6 is about extracting handlers from parser classes. This task is about removing CPU coupling from the core assembler.

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-11
