# Task Contract: EDTASM Handler Extraction - Phase 6c.3 Completion

**Task ID:** 2026-02-11_edtasm-handler-extraction
**Beads Task:** xasm++-hhbr
**Created:** 2026-02-11
**Requestor:** Orchestrator (continuation of xasm++-5j00)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Complete the EDTASM-M80++ directive handler extraction by implementing the remaining 47 lambda functions as free functions. The skeleton infrastructure (15%) is complete from task xasm++-5j00. This task completes the remaining 85% of the extraction work.

### Background and Context

**Phase 6 Progress:**
- Phase 6a: Signature unification ✅ Complete
- Phase 6b: Core handler extraction pattern established ✅ Complete
- Phase 6c.1: Merlin extraction ✅ Complete (26 handlers)
- Phase 6c.2: SCMASM extraction ✅ Complete (11 handlers)
- Phase 6c.3: EDTASM extraction ⚠️ 15% complete (skeleton only)

**Previous Work (xasm++-5j00):**
An engineer agent created the skeleton infrastructure but ran out of time (10min timeout vs 6-8hr estimated work):
- ✅ Created `edtasm_directive_handlers.h` with 39 function declarations
- ✅ Created `edtasm_directive_handlers.cpp` with skeleton implementations
- ✅ Updated CMakeLists.txt
- ✅ Build succeeds, 96% tests passing
- ❌ 47 lambda implementations NOT extracted
- ❌ Registrations still use inline lambdas
- ❌ Magic numbers/strings not eliminated

### Current State

**Files Created (Skeleton):**
```
include/xasm++/directives/edtasm_directive_handlers.h  (39 declarations)
src/directives/edtasm/edtasm_directive_handlers.cpp   (skeleton implementations)
```

**Parser State:**
- `edtasm_m80_plusplus_syntax.cpp`: 1858 lines (target: ~600)
- `InitializeDirectiveRegistry()`: Still uses 47 inline lambda functions
- Directives: Still embedded in parser class
- Magic strings/numbers: NOT eliminated

### Desired State

**Complete Handler Extraction:**
- All 47 lambda implementations extracted as free functions
- Registry updated to call free function handlers
- Parser class reduced to ~600 lines
- All magic numbers replaced with named constants
- All magic strings replaced with directive constants
- Build clean (0 warnings)
- All tests passing

---

## Success Criteria

```
✓ 47 directive handlers implemented (not just declared)
✓ InitializeDirectiveRegistry() calls free functions (not inline lambdas)
✓ Parser file reduced from 1858 lines to ~600 lines
✓ Zero magic numbers (use opcodes_z80.h constants)
✓ Zero magic strings (use directive_constants.h)
✓ Build succeeds with 0 warnings
✓ All tests passing (96%+ maintained)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Extract all 47 lambda implementations as free functions
□ Update InitializeDirectiveRegistry() to call handlers
□ Verify all directives work identically to before
□ Maintain or improve test pass rate (96%+)
```

### Code Quality Requirements (CRITICAL)
```
□ NO magic numbers - use opcodes_z80.h constants
□ NO magic strings - use/extend directive_constants.h
□ Follow Phase 6b/6c.1/6c.2 pattern exactly
□ Free functions with DirectiveContext parameter
□ Proper error handling with exceptions
```

### Quality Requirements
```
□ All tests passing (96%+)
□ Build with 0 warnings
□ TDD followed where applicable
□ Code formatted per project standards
□ Functions documented
```

---

## Implementation Strategy

### Phase 1: Analysis (30 min)
1. Read existing skeleton implementations
2. Identify which handlers need full implementation
3. List all magic numbers/strings to eliminate
4. Review Phase 6c.1/6c.2 patterns

### Phase 2: Handler Implementation (4-5 hours)
For each of 47 handlers:
1. Copy logic from lambda in `InitializeDirectiveRegistry()`
2. Adapt to free function signature
3. Replace magic numbers with constants
4. Replace magic strings with directive constants
5. Test incrementally (build + test after every 5-10 handlers)

### Phase 3: Registry Update (1 hour)
1. Update `InitializeDirectiveRegistry()` to call free functions
2. Remove inline lambda bodies
3. Verify all handlers registered correctly

### Phase 4: Verification (1-2 hours)
1. Run full test suite
2. Fix any test failures
3. Verify build has 0 warnings
4. Confirm parser file size reduced to ~600 lines

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 4-6 files
- `src/directives/edtasm/edtasm_directive_handlers.cpp` - Main work (47 handlers)
- `include/xasm++/directives/edtasm_directive_handlers.h` - Already has declarations
- `src/syntax/edtasm/edtasm_m80_plusplus_syntax.cpp` - Registry updates
- `include/xasm++/directives/directive_constants.h` - Extend with EDTASM constants
- `include/xasm++/cpu/opcodes_z80.h` - May need to verify constants exist
- `tests/unit/test_edtasm_m80_plusplus_syntax.cpp` - May need fixes

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 4-6 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
Main implementation file: ~15K tokens (large, 47 handlers)
Other files: 4 × 2K = 8K tokens
Total: ~23K tokens

Agent Output Limit: 25K-32K tokens

Status: ✅ SAFE (within limits)
```

### Work In Progress Planning

```
Planned WIP: 1 agent (Engineer - sequential execution)
Status: ✅ IDEAL
```

### Decomposition Decision

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files (6 files) AND token budget ≤42K (23K) AND WIP ≤3 (1 agent)

---

## Resources and References

### Pattern Reference (CRITICAL - Read First)
```
.ai/tasks/2026-02-10_god-class-phase6b-core/ENGINEER-REPORT.md
.ai/tasks/2026-02-11_god-class-phase6c1-merlin/40-completion-summary.md
.ai/tasks/2026-02-11_god-class-phase6c2-scmasm/SUMMARY.md
```

### Previous Work (Context)
```
.ai/tasks/2026-02-11_god-class-phase6c3-edtasm/00-contract.md (original)
.ai/tasks/2026-02-11_god-class-phase6c3-edtasm/20-work-log.md (15% work done)
```

### Existing Skeleton Files
```
include/xasm++/directives/edtasm_directive_handlers.h (declarations ready)
src/directives/edtasm/edtasm_directive_handlers.cpp (need implementations)
```

### Constant Files
```
include/xasm++/cpu/opcodes_z80.h - Z80 opcodes
include/xasm++/directives/directive_constants.h - Extend with EDTASM names
```

---

## Directive List (47 Handlers to Extract)

Based on previous work, these handlers need implementation:

**Data Definition (~15):**
DB, DW, DS, DEFB, DEFW, DEFM, DEFS, DC, DM, DZ, etc.

**Conditional Assembly (~13):**
IF, ELSE, ENDIF, IFT, IFF, IFDEF, IFNDEF, IFB, IFNB, IFEQ, IFNE, etc.

**Macro System (~7):**
MACRO, ENDM, LOCAL, EXITM, REPT, IRP, IRPC

**Listing Control (~3):**
LIST, NOLIST, TITLE, PAGE

**Other (~9):**
ORG, EQU, SET, INCLUDE, END, PUBLIC, EXTERN, etc.

*(Exact list available in edtasm_directive_handlers.h)*

---

## Estimated Complexity

**Complexity:** Medium-Large

**Rationale:**
- Number of files affected: 4-6 files
- Lines of code: ~1200 lines to extract
- Pattern established: Following proven Phase 6b/6c.1/6c.2 pattern
- Integration complexity: Medium (47 handlers to wire up)
- Risk level: Low (pattern proven, tests provide safety net)
- Time estimate: 6-8 hours (mostly mechanical extraction)

---

## Assumptions

```
1. Skeleton infrastructure from xasm++-5j00 is correct
2. Pattern from Phase 6b/6c.1/6c.2 applies directly
3. Existing tests provide adequate coverage
4. All lambda implementations can be extracted (no hidden dependencies)
5. opcodes_z80.h has all needed constants
```

---

## Risk Assessment

```
1. Large number of handlers (47)
   - Probability: N/A (known scope)
   - Impact: High (time-consuming)
   - Mitigation: Incremental approach, test every 5-10 handlers

2. Test failures during extraction
   - Probability: Medium
   - Impact: Medium (slows progress)
   - Mitigation: Follow pattern exactly, test incrementally

3. Missing constants
   - Probability: Low
   - Impact: Low (easy to add)
   - Mitigation: Check opcodes_z80.h early, add as needed

4. Time overrun
   - Probability: Low
   - Impact: Medium
   - Mitigation: Focus on functionality first, polish later if needed
```

---

## Special Instructions

**Critical Pattern to Follow:**

From Phase 6b/6c.1/6c.2, the pattern is:

1. **Free Function Signature:**
   ```cpp
   void HandleXxxDirective(Section& section, SymbolTable& symbols,
                           const std::string& operand);
   ```

2. **Extract Logic:**
   - Copy from lambda in `InitializeDirectiveRegistry()`
   - Remove captures (pass as parameters if needed)
   - Replace magic strings with constants

3. **Registry Update:**
   ```cpp
   registry_.Register(DIR_XXX, [this](Section& section, SymbolTable& symbols,
                                       const std::string& line) {
       std::string operand = ParseOperand(line);
       HandleXxxDirective(section, symbols, operand);
   });
   ```

**Magic String Elimination:**
- ALL directive names must use constants from `directive_constants.h`
- Add EDTASM namespace if needed
- Example: `"IF"` → `edtasm::directives::IF`

**Magic Number Elimination:**
- ALL opcodes must use constants from `opcodes_z80.h`
- Example: `0xC3` → `Z80Opcodes::JP_nn`

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Orchestrator (2026-02-11)
- [ ] Agent: Engineer (TBD)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-11
