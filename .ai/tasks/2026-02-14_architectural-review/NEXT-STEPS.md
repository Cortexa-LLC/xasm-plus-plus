# Next Steps: xasm++ Architectural Fixes

**Date:** 2026-02-14  
**Review Status:** COMPLETE - CHANGES REQUIRED  
**Total Issues:** 450+  
**Estimated Effort:** 101-151 hours (12-19 developer days)

---

## Immediate Actions Required

### P0: Critical Architectural Issues (48-72 hours)

**MUST FIX BEFORE PROCEEDING WITH OTHER WORK**

#### Task 1: Fix Lambda Anti-pattern (8-12 hours)
**File:** `src/syntax/core_directive_handlers.cpp`  
**Priority:** P0 - CRITICAL (Architectural Pattern Violation)  

**Actions:**
1. Refactor handler signatures to: `(const std::string &label, const std::string &operand, DirectiveContext &ctx)`
2. Remove lambda wrappers in `RegisterCoreDirectiveHandlers()`
3. Use direct assignment: `registry[directive] = Handler`
4. Update handlers to access context members directly

**Reference Pattern:** `src/syntax/merlin_directive_handlers.cpp` lines 458-500

**Acceptance Criteria:**
- All handlers use (label, operand, context) signature
- No lambdas in RegisterCoreDirectiveHandlers()
- Direct assignment pattern matches Merlin reference
- All tests pass

---

#### Task 2: Fix CPU 6502 Magic Opcodes (20-30 hours)
**Files:** `src/cpu/cpu_6502.cpp`, `include/xasm++/cpu/opcodes_6502.h`  
**Priority:** P0 - CRITICAL  

**Actions:**
1. Add 100+ missing opcode constants to opcodes_6502.h:
   - WAI, STP, PHX, PLX, PHY, PLY, STZ, BRA, TRB, TSB
   - All addressing mode variants
2. Replace all magic hex values in cpu_6502.cpp (72 instances)
3. Replace magic radix values (5 instances)

**Acceptance Criteria:**
- No magic hex values in cpu_6502.cpp
- All opcodes use Opcodes:: constants
- Binary output identical to before refactor
- All tests pass

---

#### Task 3: Fix CPU 6809 Magic Opcodes (20-30 hours)
**Files:** `src/cpu/cpu_6809.cpp`, `include/xasm++/cpu/opcodes_6809.h`  
**Priority:** P0 - CRITICAL  

**Actions:**
1. Create comprehensive opcodes_6809.h with 200+ opcode constants
2. Replace all magic hex values in cpu_6809.cpp (215 instances)
3. Document addressing mode encoding

**Acceptance Criteria:**
- No magic hex values in cpu_6809.cpp
- All opcodes use Opcodes:: constants
- Binary output identical to before refactor
- All tests pass

---

### P1: Major Consistency Issues (41-63 hours)

**SHOULD FIX AFTER P0 COMPLETE**

#### Task 4: Create and Apply Radix Constants (15-20 hours)
**Files:** 10+ files across src/  

**Actions:**
1. Create `include/xasm++/util/radix_constants.h`:
   ```cpp
   constexpr int RADIX_BINARY = 2;
   constexpr int RADIX_OCTAL = 8;
   constexpr int RADIX_DECIMAL = 10;
   constexpr int RADIX_HEXADECIMAL = 16;
   ```
2. Replace 96 instances of magic radix values across:
   - parse_utils.cpp (2 instances)
   - assembler.cpp (1 instance)
   - merlin_syntax.cpp (6 instances)
   - scmasm_syntax.cpp (3 instances)
   - edtasm_syntax.cpp (1 instance)
   - flex_syntax.cpp (1 instance)
   - merlin_directive_handlers.cpp (2 instances)
   - cpu_6502.cpp (5 instances)

**Acceptance Criteria:**
- All std::stoul calls use RADIX_* constants
- No magic 2, 8, 10, 16 values in parsing code
- All tests pass

---

#### Task 5: Fix Parameter Order Inconsistencies (6-8 hours)
**Files:** `src/syntax/core_directive_handlers.cpp` and headers  
**Depends On:** Task 1 (Lambda fix)  

**Actions:**
1. Standardize all handler signatures to (label, operand, context)
2. Update all call sites
3. Ensure consistency across all syntax parsers

**Acceptance Criteria:**
- All handlers use same parameter order
- Matches Merlin reference pattern
- All tests pass

---

#### Task 6: Fix Output Formatter Magic Values (20-35 hours)
**Files:** 5 output formatter files  

**Actions:**
1. Create format constant headers:
   - `include/xasm++/output/intel_hex_format.h`
   - `include/xasm++/output/coco_loadm_format.h`
   - `include/xasm++/output/srec_format.h`
2. Replace 60+ magic values in:
   - binary_output.cpp (2 instances)
   - coco_loadm_writer.cpp (15 instances)
   - intel_hex_writer.cpp (20 instances)
   - srec_writer.cpp (10 instances)

**Acceptance Criteria:**
- All format-specific values use named constants
- Output files identical to before refactor
- All tests pass

---

### P2: Quality Improvements (12-16 hours)

**SHOULD FIX AFTER P1 COMPLETE**

#### Task 7: Eliminate Code Duplication (12-16 hours)

**Actions:**
1. Extract ParseNumber utility function (20 duplicate patterns)
2. Extract ThrowDirectiveError helper (189 duplicate patterns)
3. Extract ParseAndEvaluate helper (25 duplicate patterns)
4. Consolidate Trim implementations (15 duplicate patterns)

**Acceptance Criteria:**
- DRY principle applied
- Reduced code duplication
- All tests pass

---

## Testing Strategy

### After Each Task
- Run full test suite
- Verify binary output identical (assembler produces same machine code)
- Run integration tests
- Check for performance regression

### Before Final Approval
- Cross-assembler comparison (against known-good assemblers)
- Output format validation (hex file parsers)
- Performance benchmarking
- Code review against clean code standards

---

## Success Criteria

### Phase 1 (P0) Complete When:
- [ ] No lambda wrappers in directive registries
- [ ] No magic opcodes in CPU implementations
- [ ] All P0 tests passing
- [ ] Binary output verified identical

### Phase 2 (P1) Complete When:
- [ ] No magic radix values in parsing code
- [ ] Consistent parameter orders across all handlers
- [ ] No magic values in output formatters
- [ ] All P1 tests passing

### Phase 3 (P2) Complete When:
- [ ] DRY principle applied
- [ ] Code duplication minimal
- [ ] All tests passing
- [ ] Code review approval

---

## Timeline Estimate

| Phase | Tasks | Duration | Dependencies |
|-------|-------|----------|--------------|
| **P0** | 1-3 | 48-72 hours (6-9 days) | None |
| **P1** | 4-6 | 41-63 hours (5-8 days) | P0 complete |
| **P2** | 7 | 12-16 hours (1.5-2 days) | P1 complete |
| **Total** | 1-7 | **101-151 hours** | **(12-19 days)** |

---

## Risk Mitigation

### High Risk Areas
1. **CPU opcode refactoring** - Extensive changes, high test coverage needed
2. **Directive handler refactoring** - Core functionality, needs careful validation
3. **Output formatter changes** - Must produce bit-identical output

### Mitigation Strategies
1. Create regression test suite before starting
2. Refactor incrementally (one file at a time)
3. Verify binary output after each change
4. Cross-reference with known-good assemblers
5. Keep detailed change log

---

## Contact

**Questions?** Refer to comprehensive review report in `30-review.md`  
**Patterns?** See "Recommended Fix Patterns" section in review report  
**Reference Code?** Check MerlinSyntaxParser (src/syntax/merlin_directive_handlers.cpp)  

---

**Status:** READY TO BEGIN P0 TASKS  
**Next Action:** Create task for P0.1 (Lambda Anti-pattern Fix)  
**Estimated Completion:** 12-19 developer days from start
