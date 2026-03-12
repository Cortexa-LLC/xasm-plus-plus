# Refactoring Work Breakdown Strategy

**Date:** 2026-01-26
**Purpose:** Break down comprehensive refactoring into agent-friendly micro-tasks

---

## ✅ Phase 1: COMPLETE

**Status:** Done (with token limit issues in validation)
**What Was Done:**
- [C1] ParseHex security fix with comprehensive validation
- New `parse_utils.h` and `parse_utils.cpp` with safe parsing
- 15 new tests added (444 total, all passing)

**Manual Validation:**
- ✅ All 444 tests passing
- ✅ Zero compiler warnings
- ✅ ParseHex has 5-layer validation
- ✅ Comprehensive error reporting

**Remaining from Phase 1:**
- [C2] Null checks after dynamic_pointer_cast (not completed)

---

## 🔧 Micro-Task Breakdown Strategy

### Principle: One File, One Issue, One Task

**Each task should:**
- Focus on 1-2 specific files (not entire codebase)
- Address 1 specific issue (not multiple)
- Have <50 line changes expected
- Include only relevant context
- Complete in <20 agent turns

**Example Good Task:**
```
Task: Add null checks to assembler.cpp dynamic casts
Scope: assembler.cpp lines 180-220 only
Issue: [C2] from code review
Expected: 3-5 null checks added, 5-10 test cases
Context: Only provide relevant code sections
```

**Example Bad Task (Too Broad):**
```
Task: Fix all dynamic cast issues across codebase
Scope: All files
Issue: Multiple issues
Expected: Unknown
Context: Entire codebase
```

---

## 📋 Revised Task Breakdown (Micro-Tasks)

### Group A: Critical Fixes (2 tasks)

#### Task A1: [C1] ParseHex Validation ✅ COMPLETE
- **File:** `src/core/parse_utils.cpp`, `include/xasm++/parse_utils.h`
- **Lines:** New files
- **Issue:** ParseHex security risk
- **Status:** COMPLETE

#### Task A2: [C2] Dynamic Cast Safety - assembler.cpp
- **Beads:** Create new task: `xasm++-cast-checks`
- **File:** `src/core/assembler.cpp`
- **Lines:** 180-220 (specific locations)
- **Issue:** Null checks after dynamic_pointer_cast
- **Scope:** Add 3-5 null checks with error messages
- **Tests:** 5-10 test cases for cast failures
- **Estimated:** 1 hour, <15 turns

---

### Group B: God Method Refactoring (5 tasks)

Break the 344-line `Assemble()` method into 5 separate refactoring tasks:

#### Task B1: Extract ResolveSymbols()
- **Beads:** `xasm++-refactor-symbols`
- **File:** `src/core/assembler.cpp`
- **Lines:** Extract symbol resolution logic (approx lines 50-100)
- **Changes:** Create new private method `ResolveSymbols()`
- **Tests:** Unit tests for symbol resolution
- **Estimated:** 2 hours, <20 turns

#### Task B2: Extract EncodeInstructions()
- **Beads:** `xasm++-refactor-encode`
- **File:** `src/core/assembler.cpp`
- **Lines:** Extract instruction encoding logic (approx lines 200-300)
- **Changes:** Create new private method `EncodeInstructions()`
- **Tests:** Unit tests for encoding
- **Estimated:** 2 hours, <20 turns

#### Task B3: Extract CheckConvergence()
- **Beads:** `xasm++-refactor-convergence`
- **File:** `src/core/assembler.cpp`
- **Lines:** Extract convergence checking (approx lines 320-340)
- **Changes:** Create new private method `CheckConvergence()`
- **Tests:** Unit tests for convergence
- **Estimated:** 1 hour, <15 turns

#### Task B4: Extract HandleErrors()
- **Beads:** `xasm++-refactor-errors`
- **File:** `src/core/assembler.cpp`
- **Lines:** Extract error handling/reporting
- **Changes:** Create new private method `HandleErrors()`
- **Tests:** Unit tests for error cases
- **Estimated:** 1 hour, <15 turns

#### Task B5: Reduce Assemble() to Coordinator
- **Beads:** `xasm++-refactor-assemble`
- **File:** `src/core/assembler.cpp`
- **Lines:** Main Assemble() method
- **Changes:** Rewrite to call extracted methods (target: <50 lines)
- **Tests:** Integration tests verify behavior unchanged
- **Estimated:** 1 hour, <15 turns

---

### Group C: Dispatch Table (2 tasks)

#### Task C1: Design Dispatch Table
- **Beads:** `xasm++-dispatch-design`
- **File:** `include/xasm++/instruction_handler.h` (new)
- **Changes:** Design interface and data structure
- **Deliverable:** Header file with interface
- **Estimated:** 1 hour, <10 turns

#### Task C2: Implement Dispatch Table
- **Beads:** `xasm++-dispatch-impl`
- **File:** `src/core/assembler.cpp`
- **Lines:** Replace 208-line if-else chain
- **Changes:** Use hash map from C1
- **Tests:** Benchmark performance improvement
- **Estimated:** 2 hours, <20 turns

---

### Group D: Constants & Validation (3 tasks)

#### Task D1: Extract Opcode Constants
- **Beads:** `xasm++-opcode-constants`
- **File:** `include/xasm++/opcodes.h` (new)
- **Changes:** Create enum class for all opcodes
- **Scope:** Define constants only, no refactoring yet
- **Estimated:** 1 hour, <10 turns

#### Task D2: Use Opcode Constants in cpu_6502.cpp
- **Beads:** `xasm++-use-opcodes-cpu`
- **File:** `src/cpu/m6502/cpu_6502.cpp`
- **Changes:** Replace magic numbers with constants from D1
- **Estimated:** 2 hours, <20 turns

#### Task D3: Add String Validation
- **Beads:** `xasm++-string-validation`
- **Files:** Various (identify specific locations first)
- **Changes:** Add bounds checking to string operations
- **Estimated:** 2 hours, <20 turns

---

### Group E: God Class Refactoring (4 tasks)

#### Task E1: Design Cpu6502 Decomposition
- **Beads:** `xasm++-cpu-design`
- **Deliverable:** Architecture document
- **Changes:** Design how to split Cpu6502
- **No code changes:** Planning only
- **Estimated:** 1 hour, <10 turns

#### Task E2: Extract AddressingModes Class
- **Beads:** `xasm++-addressing-modes`
- **Files:** New class `Cpu6502AddressingModes`
- **Changes:** Extract addressing mode logic
- **Estimated:** 3 hours, <25 turns

#### Task E3: Extract BranchHandler Class
- **Beads:** `xasm++-branch-handler`
- **Files:** New class `Cpu6502BranchHandler`
- **Changes:** Extract branch logic (including relaxation)
- **Estimated:** 2 hours, <20 turns

#### Task E4: Update Cpu6502 to Use Composition
- **Beads:** `xasm++-cpu-composition`
- **File:** `src/cpu/m6502/cpu_6502.cpp`
- **Changes:** Delegate to extracted classes
- **Estimated:** 2 hours, <20 turns

---

### Group F: Code Duplication (2 tasks)

#### Task F1: Analyze Duplication Patterns
- **Beads:** `xasm++-duplication-analysis`
- **Deliverable:** Analysis document
- **Changes:** Document patterns in 50+ Encode methods
- **No code changes:** Analysis only
- **Estimated:** 1 hour, <10 turns

#### Task F2: Extract Common Encoding Logic
- **Beads:** `xasm++-extract-encoding`
- **File:** `src/cpu/m6502/cpu_6502.cpp`
- **Changes:** Create shared encoding utilities
- **Scope:** Reduce duplication from 95% to <10%
- **Estimated:** 3 hours, <25 turns

---

### Group G: Minor Issues (3 tasks)

#### Task G1: Add Const-Correctness
- **Beads:** `xasm++-const-correct`
- **Files:** Headers in `include/xasm++/`
- **Changes:** Add const to non-modifying methods
- **Estimated:** 2 hours, <20 turns

#### Task G2: Add Doxygen Documentation
- **Beads:** `xasm++-doxygen`
- **Files:** Public interfaces
- **Changes:** Add Doxygen comments
- **Estimated:** 3 hours, <25 turns

#### Task G3: Standardize Naming
- **Beads:** `xasm++-naming`
- **Files:** Various (identify inconsistencies first)
- **Changes:** Apply consistent naming convention
- **Estimated:** 2 hours, <20 turns

---

## 🎯 Execution Strategy

### Sequential Execution (Recommended)

Execute in order, one task at a time:

1. **Complete Task A2** (dynamic cast checks) - blocks nothing
2. **Complete Group B** (God Method) - sequential, B1→B2→B3→B4→B5
3. **Complete Group C** (Dispatch) - C1→C2 (C2 depends on C1)
4. **Complete Group D** (Constants) - D1→D2, D3 parallel
5. **Complete Group E** (God Class) - E1→E2→E3→E4 (sequential)
6. **Complete Group F** (Duplication) - F1→F2 (sequential)
7. **Complete Group G** (Minor) - G1, G2, G3 can be parallel

### Parallel Opportunities

These can run in parallel (no dependencies):
- A2 + B1 (different scopes)
- D2 + D3 (different files)
- G1 + G2 + G3 (different concerns)

### Quality Gates Per Task

After EACH micro-task:
1. Run all tests (must pass)
2. Run Tester validation (if code changed)
3. Run Reviewer validation (if significant)
4. Commit if approved
5. Move to next task

**Benefit:** Each validation has minimal context, less likely to hit token limits.

---

## 📊 Progress Tracking

### Summary View

| Group | Tasks | Completed | Status |
|-------|-------|-----------|--------|
| A (Critical) | 2 | 1 | 🟡 In Progress |
| B (God Method) | 5 | 0 | ⏳ Pending |
| C (Dispatch) | 2 | 0 | ⏳ Pending |
| D (Constants) | 3 | 0 | ⏳ Pending |
| E (God Class) | 4 | 0 | ⏳ Pending |
| F (Duplication) | 2 | 0 | ⏳ Pending |
| G (Minor) | 3 | 0 | ⏳ Pending |
| **Total** | **21** | **1** | **5% Complete** |

### Estimated Total Time

- **Critical (A):** 1 hour remaining
- **Major (B-E):** 25 hours
- **Minor (F-G):** 13 hours
- **Total:** ~39 hours (5 working days)

With 3 agents in parallel: ~13 hours (2 working days)

---

## 🚀 Next Immediate Action

**Create and execute Task A2:**

```bash
bd create "Fix Dynamic Cast Safety - assembler.cpp

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/2026-01-26_dynamic-cast-safety/

Add null checks after dynamic_pointer_cast calls in assembler.cpp (lines 180-220).

Addresses [C2] from code review. Add 3-5 null checks with descriptive error messages." --priority 1
```

Then spawn Engineer focused ONLY on this narrow task.

---

## 💡 Key Insights

**Why This Works:**
1. **Narrow scope** - Agent reads 1-2 files, not entire codebase
2. **Clear boundaries** - Specific line ranges, specific issue
3. **Fast completion** - <20 turns means <200k tokens
4. **Independent** - Can run multiple in parallel
5. **Trackable** - Each task has clear done criteria

**Why Previous Approach Failed:**
1. **Too broad** - "Fix all 19 issues" is unbounded
2. **No boundaries** - Agent explores everything
3. **Context explosion** - Accumulates entire codebase in context
4. **Dependencies unclear** - Hard to parallelize
5. **Progress unclear** - What's "done"?

---

## 📖 Template for Future Micro-Tasks

```markdown
# Task: [Brief Description]

**Beads Task:** xasm++-[short-id]
**Issue:** [C1], [M1], etc. from code review
**File(s):** Specific file paths (max 2-3 files)
**Lines:** Specific line ranges if applicable
**Scope:** [1-2 sentence description of EXACTLY what to do]
**Expected Changes:** X lines modified, Y tests added
**Dependencies:** None / Depends on [task-id]
**Estimated:** X hours, <Y turns
**Success Criteria:**
- [ ] Specific criterion 1
- [ ] Specific criterion 2
- [ ] All tests passing (no regressions)
```

---

**Last Updated:** 2026-01-26
**Next Step:** Create Task A2 and execute
