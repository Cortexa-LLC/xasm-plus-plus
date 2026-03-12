# Implementation Plan: ExpressionParser Integration

**Task:** 2026-02-09_god-class-phase2-integration
**Beads ID:** xasm++-6nra
**Workflow:** Refactor (integration)

## Workflow Selection

Using: `.ai-pack/workflows/refactor.md`

**Rationale:** This is a refactoring task integrating a new component to eliminate code duplication while preserving all existing behavior.

## Implementation Approach

### Phase 1: Z80Universal Integration (Pilot) (3-4 hours)

**Goal:** Successfully integrate ExpressionParser into first parser as proof of concept

**Why Z80 first:** Largest parser, most comprehensive tests, good validation

**Steps:**

**1.1 Baseline Measurement (15 min)**

```bash
# Measure current LOC
cloc src/syntax/z80_universal/z80_universal_syntax.cpp
# Record: should be ~2,285 LOC

# Run tests to establish baseline
cd build && ctest -R ".*z80.*universal" --output-on-failure
# Record: should be 100% passing

# Save output comparison sample (for validation)
./xasmpp --cpu z80 --syntax z80universal tests/fixtures/sample.asm > baseline_z80.out
```

**1.2 Analyze Expression Methods (30 min)**

```bash
# Find all expression-related methods
grep -n "Expression\|expression\|Evaluate" \
  include/xasm++/syntax/z80_universal_syntax.h \
  src/syntax/z80_universal/z80_universal_syntax.cpp

# Document methods to replace:
# - EvaluateExpression()
# - ParseExpression()
# - ParseBinaryOp()
# - ParseUnaryOp()
# - ParsePrimary()
# - (and 7 more recursion methods)
```

**1.3 Add ISymbolResolver Interface (30 min)**

```cpp
// include/xasm++/syntax/z80_universal_syntax.h
#include "xasm++/common/expression_parser.h"

class Z80UniversalSyntaxParser : public ISyntaxParser,
                                  public ISymbolResolver {  // ADD THIS
private:
  ExpressionParser expr_parser_;  // ADD THIS

public:
  Z80UniversalSyntaxParser();

  // ISymbolResolver implementation
  bool TryResolveSymbol(const std::string& name, uint32_t& value) override;

  // ... existing methods ...
};
```

```cpp
// src/syntax/z80_universal/z80_universal_syntax.cpp

Z80UniversalSyntaxParser::Z80UniversalSyntaxParser()
  : expr_parser_(this) {  // Initialize with 'this' as resolver
  // ... existing initialization ...
}

bool Z80UniversalSyntaxParser::TryResolveSymbol(
    const std::string& name, uint32_t& value) {
  // Delegate to existing symbol table
  return symbol_table_.TryGetSymbol(name, value);
}
```

**1.4 Replace Expression Methods (1-1.5 hours)**

```cpp
// BEFORE (50+ lines of recursive descent):
uint32_t Z80UniversalSyntaxParser::EvaluateExpression(const std::string& expr) {
  // Complex parsing logic
  size_t pos = 0;
  return ParseLogicalOr(expr, pos);
}

uint32_t Z80UniversalSyntaxParser::ParseLogicalOr(const std::string& expr, size_t& pos) {
  // 10+ lines
}

uint32_t Z80UniversalSyntaxParser::ParseLogicalAnd(...) {
  // 10+ lines
}
// ... 10 more similar methods ...

// AFTER (single delegation):
uint32_t Z80UniversalSyntaxParser::EvaluateExpression(const std::string& expr) {
  return expr_parser_.Parse(expr);
}

// DELETE all 12 recursive descent helper methods
```

**Pattern for each replacement:**
1. Find the method (e.g., `EvaluateExpression`)
2. Replace body with: `return expr_parser_.Parse(expr);`
3. Mark old helper methods for deletion
4. Build and test after each major method replacement

**1.5 Test Incrementally (30 min)**

```bash
# After each method replacement, test immediately
cmake --build build
cd build && ctest -R ".*z80.*universal" --output-on-failure

# If tests fail:
# - Check error message
# - Verify expression format matches ExpressionParser expectations
# - Check symbol resolution is working
# - Compare with baseline output
```

**1.6 Remove Old Code (30 min)**

```bash
# Once all tests pass, remove unused methods:
# - ParseLogicalOr
# - ParseLogicalAnd
# - ParseBitwiseOr
# - ParseBitwiseXor
# - ParseBitwiseAnd
# - ParseShift
# - ParseAddSub
# - ParseMulDiv
# - ParseUnary
# - ParsePrimary
# - ParseNumber
# - ParseIdentifier

# Verify no references remain:
grep -n "ParseLogicalOr\|ParseLogicalAnd" src/syntax/z80_universal/
# Should return nothing
```

**1.7 Verify and Commit (15 min)**

```bash
# Measure LOC reduction
cloc src/syntax/z80_universal/z80_universal_syntax.cpp
# Should be ~1,835 LOC (450 LOC removed)

# Verify output matches baseline
./xasmpp --cpu z80 --syntax z80universal tests/fixtures/sample.asm > new_z80.out
diff baseline_z80.out new_z80.out
# Should be identical

# Commit checkpoint
git add -A
git commit -m "refactor: Z80Universal uses ExpressionParser (-450 LOC)"
```

**Expected Outcome:** Z80Universal parser reduced from 2,285 → ~1,835 LOC, all tests passing

### Phase 2: Merlin Integration (3-4 hours)

**Goal:** Integrate ExpressionParser into Merlin parser

**Steps:** Repeat Phase 1 pattern for Merlin:

**2.1 Baseline (15 min)**
```bash
cloc src/syntax/merlin/merlin_syntax.cpp  # ~1,764 LOC
cd build && ctest -R ".*merlin" --output-on-failure
./xasmpp --cpu 6502 --syntax merlin tests/fixtures/sample_merlin.asm > baseline_merlin.out
```

**2.2 Analyze (30 min)**
```bash
grep -n "Expression" include/xasm++/syntax/merlin_syntax.h src/syntax/merlin/merlin_syntax.cpp
```

**2.3 Add ISymbolResolver (30 min)**
```cpp
// merlin_syntax.h
class MerlinSyntaxParser : public ISyntaxParser, public ISymbolResolver {
  ExpressionParser expr_parser_;
public:
  MerlinSyntaxParser() : expr_parser_(this) {}
  bool TryResolveSymbol(const std::string& name, uint32_t& value) override;
};
```

**2.4 Replace Methods (1-1.5 hours)**
```cpp
uint32_t MerlinSyntaxParser::EvaluateExpression(const std::string& expr) {
  return expr_parser_.Parse(expr);
}
// Delete 12 helper methods
```

**2.5 Test (30 min)**
```bash
cmake --build build
cd build && ctest -R ".*merlin" --output-on-failure
```

**2.6 Remove Old Code (30 min)**

**2.7 Verify and Commit (15 min)**
```bash
cloc src/syntax/merlin/merlin_syntax.cpp  # Should be ~1,364 LOC
diff baseline_merlin.out new_merlin.out    # Should match
git commit -m "refactor: Merlin uses ExpressionParser (-400 LOC)"
```

**Expected Outcome:** Merlin parser reduced from 1,764 → ~1,364 LOC, all tests passing

### Phase 3: SCMASM Integration (3-4 hours)

**Goal:** Integrate ExpressionParser into SCMASM parser

**Steps:** Repeat Phase 1 pattern for SCMASM:

**3.1 Baseline (15 min)**
```bash
cloc src/syntax/scmasm/scmasm_syntax.cpp  # ~1,330 LOC
cd build && ctest -R ".*scmasm" --output-on-failure
./xasmpp --cpu 6809 --syntax scmasm tests/fixtures/sample_scmasm.asm > baseline_scmasm.out
```

**3.2 Analyze (30 min)**
```bash
grep -n "Expression" include/xasm++/syntax/scmasm_syntax.h src/syntax/scmasm/scmasm_syntax.cpp
```

**3.3 Add ISymbolResolver (30 min)**
```cpp
// scmasm_syntax.h
class ScmasmSyntaxParser : public ISyntaxParser, public ISymbolResolver {
  ExpressionParser expr_parser_;
public:
  ScmasmSyntaxParser() : expr_parser_(this) {}
  bool TryResolveSymbol(const std::string& name, uint32_t& value) override;
};
```

**3.4 Replace Methods (1-1.5 hours)**
```cpp
uint32_t ScmasmSyntaxParser::EvaluateExpression(const std::string& expr) {
  return expr_parser_.Parse(expr);
}
// Delete 12 helper methods
```

**3.5 Test (30 min)**
```bash
cmake --build build
cd build && ctest -R ".*scmasm" --output-on-failure
```

**3.6 Remove Old Code (30 min)**

**3.7 Verify and Commit (15 min)**
```bash
cloc src/syntax/scmasm/scmasm_syntax.cpp  # Should be ~980 LOC
diff baseline_scmasm.out new_scmasm.out    # Should match
git commit -m "refactor: SCMASM uses ExpressionParser (-350 LOC)"
```

**Expected Outcome:** SCMASM parser reduced from 1,330 → ~980 LOC, all tests passing

### Phase 4: Final Verification (1-2 hours)

**Goal:** Confirm refactoring success across entire codebase

**Steps:**

**4.1 Full Test Suite (30 min)**
```bash
# Clean rebuild
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run all tests
cd build && ctest --output-on-failure
# Should be 100% passing

# Check for warnings
cmake --build . 2>&1 | grep -i "warning:"
# Should be empty
```

**4.2 LOC Metrics (15 min)**

```bash
# Measure reduction
echo "=== Before ==="
git show HEAD~3:src/syntax/z80_universal/z80_universal_syntax.cpp | wc -l
git show HEAD~2:src/syntax/merlin/merlin_syntax.cpp | wc -l
git show HEAD~1:src/syntax/scmasm/scmasm_syntax.cpp | wc -l

echo "=== After ==="
wc -l src/syntax/*//*.cpp

# Document in acceptance:
# Before: 5,379 LOC total
# After: ~4,179 LOC total
# Reduction: 1,200 LOC (22.3%)
```

**4.3 Duplication Analysis (30 min)**

```bash
# Check for remaining expression parsing duplication
grep -r "ParseLogicalOr\|ParseBitwiseOr" src/syntax/
# Should find NOTHING in parser files
# (Only in ExpressionParser)

# Use duplication detection tool (if available)
jscpd src/syntax/ --min-lines 5 --min-tokens 50
# Should show <5% duplication
```

**4.4 Performance Benchmark (optional, 15-30 min)**

```bash
# If performance concerns exist:
time ./xasmpp --cpu z80 --syntax z80universal large_file.asm
# Compare with baseline (should be equivalent)

# Profile if needed:
perf record ./xasmpp --cpu z80 --syntax z80universal large_file.asm
perf report
# ExpressionParser should not be a hotspot
```

**4.5 Documentation (15 min)**

Update docs/architecture/god-class-refactoring.md:
```markdown
## Phase 2: Expression Parser Integration (COMPLETE)

**Date:** 2026-02-09
**Status:** ✅ Complete

### Results
- Z80Universal: 2,285 → 1,835 LOC (-450, 19.7%)
- Merlin: 1,764 → 1,364 LOC (-400, 22.7%)
- SCMASM: 1,330 → 980 LOC (-350, 26.3%)
- **Total: 5,379 → 4,179 LOC (-1,200, 22.3%)**

### Impact
- Expression duplication: 1,200 LOC → 0 LOC (100% eliminated)
- Overall duplication: 37% → <5%
- Single source of truth for expressions
- All 36 parser expression methods → 12 in ExpressionParser

### Next: Phase 3 - Number Parser extraction
```

**Expected Outcome:** All metrics documented, refactoring success confirmed

## Execution Strategy

**Sequential by parser, incremental within parser:**
- Complete Z80 before starting Merlin
- Complete Merlin before starting SCMASM
- Within each parser: incremental method replacement with testing
- Commit after each parser integration (3 commits total)

**Rollback strategy:**
- Git checkpoint before each parser
- If integration fails, rollback that commit
- Analyze failure, adjust approach, retry

## TDD Approach (Validation Testing)

**Not writing new tests** - using existing parser tests as validation:
1. ✅ GREEN: Tests pass before integration
2. 🔄 REFACTOR: Replace expression methods
3. ✅ GREEN: Tests still pass after integration
4. ♻️ CLEANUP: Remove old code
5. ✅ GREEN: Tests still pass after cleanup

**If tests fail during integration:**
- Indicates behavioral change (regression)
- Must fix before proceeding
- Compare outputs with baseline
- Debug symbol resolution or expression format issues

## Risk Mitigation

**Risk:** Different expression syntax between parsers
- **Mitigation:** Analyzed in Phase 1, ExpressionParser supports all syntaxes

**Risk:** Symbol resolution differences
- **Mitigation:** ISymbolResolver interface allows each parser to handle its own symbols

**Risk:** Breaking existing functionality
- **Mitigation:**
  - Comprehensive test suite
  - Output comparison with baseline
  - Incremental integration (one parser at a time)
  - Git checkpoints for rollback

**Risk:** Performance regression
- **Mitigation:**
  - ExpressionParser already optimized in Phase 1
  - Benchmark if concerns arise
  - Profile and optimize hot paths if needed

## Success Metrics

**Before:**
- Total LOC: 5,379
- Expression parsing LOC: ~1,200 (duplicated 3x)
- Duplication: 37%
- Expression methods: 36 (12 per parser)

**After:**
- Total LOC: ~4,179 (1,200 reduction, 22.3%)
- Expression parsing LOC: 426 (centralized)
- Duplication: <5% (86% improvement)
- Expression methods: 12 (in ExpressionParser)

**Quality:**
- Tests passing: 100%
- Compiler warnings: 0
- Behavioral changes: 0
- Performance regression: 0

## Tools and Commands

```bash
# LOC measurement
cloc src/syntax/*/

# Test specific parser
ctest -R ".*z80.*universal"
ctest -R ".*merlin"
ctest -R ".*scmasm"

# Find expression methods
grep -rn "Expression\|ParseLogical" src/syntax/

# Duplication detection
jscpd src/syntax/ --min-lines 5

# Performance
time ./xasmpp --cpu z80 --syntax z80universal large.asm
```

## Notes

- This realizes the 37% duplication removal identified in god-class investigation
- Largest single LOC reduction in refactoring plan
- Sets precedent for Phases 3-5 (NumberParser, ConditionalAssembler, etc.)
- ExpressionParser is now shared infrastructure for all parsers
