# Implementation Plan: Expression Parser Extraction

**Task:** 2026-02-09_god-class-phase1-expression-parser
**Beads ID:** xasm++-9nox
**Workflow:** Refactor (god-class decomposition)

## Workflow Selection

Using: `.ai-pack/workflows/refactor.md`

**Rationale:** This is a complex refactoring task to eliminate god-class violations and reduce code duplication while preserving all existing behavior.

## Implementation Approach

### Phase 1: Analysis & Design (4 hours)

**Goal:** Understand existing expression parsing and design extraction strategy

**Steps:**

1. **Read all 3 expression parsing implementations:**
   ```bash
   # Z80Universal
   grep -A 20 "ParseExpression\|EvaluateExpression" \
     src/syntax/z80_universal/z80_universal_syntax.cpp

   # Merlin
   grep -A 20 "ParseExpression\|EvaluateExpression" \
     src/syntax/merlin/merlin_syntax.cpp

   # SCMASM
   grep -A 20 "ParseExpression\|EvaluateExpression" \
     src/syntax/scmasm/scmasm_syntax.cpp
   ```

2. **Document commonalities and differences:**
   - Common operators (all parsers support +, -, *, /, etc.)
   - Syntax-specific operators (if any)
   - Operator precedence (should be identical)
   - Number format handling
   - Symbol resolution approach

3. **Design ExpressionParser API:**
   ```cpp
   namespace xasm {

   // Strategy for parser-specific symbol resolution
   class ISymbolResolver {
   public:
     virtual ~ISymbolResolver() = default;
     virtual bool TryResolveSymbol(const std::string& name, uint32_t& value) = 0;
   };

   // Main expression parser
   class ExpressionParser {
   public:
     explicit ExpressionParser(ISymbolResolver* resolver);

     // Parse expression, throw on error
     uint32_t Parse(const std::string& expr);

     // Try parse, return false on error
     bool TryParse(const std::string& expr, uint32_t& result);

   private:
     ISymbolResolver* resolver_;
     std::string expr_;
     size_t pos_;

     // Recursive descent methods (precedence order: low to high)
     uint32_t ParseExpression();     // Entry point
     uint32_t ParseLogicalOr();      // ||
     uint32_t ParseLogicalAnd();     // &&
     uint32_t ParseBitwiseOr();      // |
     uint32_t ParseBitwiseXor();     // ^
     uint32_t ParseBitwiseAnd();     // &
     uint32_t ParseShift();          // << >>
     uint32_t ParseAddSub();         // + -
     uint32_t ParseMulDiv();         // * / %
     uint32_t ParseUnary();          // - ~ ! < >
     uint32_t ParsePrimary();        // literals, symbols, parentheses, functions

     // Helper methods
     char Peek() const;
     char Consume();
     void SkipWhitespace();
     uint32_t ParseNumber();
     std::string ParseIdentifier();
     bool Match(const std::string& op);
   };

   } // namespace xasm
   ```

4. **Document in work log:**
   - API design rationale
   - Differences found between parsers
   - Migration strategy

**Expected Outcome:** Clear design document in work log

### Phase 2: Test-First Implementation (TDD) (10-14 hours)

**Goal:** Implement ExpressionParser with 100% test coverage

**RED-GREEN-REFACTOR cycle:**

**2.1 Create test infrastructure (30 min)**

```bash
# Create test file
touch tests/unit/test_expression_parser.cpp
```

```cpp
#include "xasm++/common/expression_parser.h"
#include <catch2/catch_test_macros.hpp>

// Mock symbol resolver for testing
class MockSymbolResolver : public xasm::ISymbolResolver {
  std::map<std::string, uint32_t> symbols_;
public:
  void AddSymbol(const std::string& name, uint32_t value) {
    symbols_[name] = value;
  }

  bool TryResolveSymbol(const std::string& name, uint32_t& value) override {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
      value = it->second;
      return true;
    }
    return false;
  }
};
```

**2.2 For Each Feature: RED → GREEN → REFACTOR**

**Iteration 1: Literals (1 hour)**

```cpp
// RED: Write failing test
TEST_CASE("ExpressionParser - Decimal literals") {
  MockSymbolResolver resolver;
  xasm::ExpressionParser parser(&resolver);

  REQUIRE(parser.Parse("42") == 42);
  REQUIRE(parser.Parse("0") == 0);
  REQUIRE(parser.Parse("65535") == 65535);
}
// Compile fails - ExpressionParser doesn't exist yet
```

```cpp
// GREEN: Minimal implementation
class ExpressionParser {
  ISymbolResolver* resolver_;
  std::string expr_;
  size_t pos_;

public:
  explicit ExpressionParser(ISymbolResolver* resolver) : resolver_(resolver), pos_(0) {}

  uint32_t Parse(const std::string& expr) {
    expr_ = expr;
    pos_ = 0;
    return ParseNumber();
  }

private:
  uint32_t ParseNumber() {
    uint32_t result = 0;
    while (pos_ < expr_.size() && isdigit(expr_[pos_])) {
      result = result * 10 + (expr_[pos_] - '0');
      pos_++;
    }
    return result;
  }
};
// Tests pass
```

```cpp
// REFACTOR: Add error handling, edge cases
TEST_CASE("ExpressionParser - Hex literals") {
  MockSymbolResolver resolver;
  xasm::ExpressionParser parser(&resolver);

  REQUIRE(parser.Parse("$FF") == 255);
  REQUIRE(parser.Parse("0xFF") == 255);
}
```

**Iteration 2: Binary operators (2-3 hours)**

```cpp
// RED
TEST_CASE("ExpressionParser - Addition") {
  MockSymbolResolver resolver;
  xasm::ExpressionParser parser(&resolver);

  REQUIRE(parser.Parse("1 + 2") == 3);
  REQUIRE(parser.Parse("10 + 20 + 30") == 60);
}
// Test fails

// GREEN
uint32_t ParseAddSub() {
  uint32_t result = ParseNumber();  // Will be ParseMulDiv later
  while (pos_ < expr_.size()) {
    SkipWhitespace();
    if (expr_[pos_] == '+') {
      pos_++;
      result += ParseNumber();
    } else if (expr_[pos_] == '-') {
      pos_++;
      result -= ParseNumber();
    } else {
      break;
    }
  }
  return result;
}
// Tests pass

// REFACTOR: Improve structure, add more operators
```

**Iteration 3-10: Repeat for all features (6-10 hours)**
- Multiplication, division, modulo
- Bitwise operators (&, |, ^, <<, >>)
- Logical operators (&&, ||)
- Unary operators (-, ~, !)
- Parentheses
- Function calls (LOW, HIGH)
- Symbol resolution
- Error handling

**2.3 Edge cases (1-2 hours)**

```cpp
TEST_CASE("ExpressionParser - Edge cases") {
  MockSymbolResolver resolver;
  xasm::ExpressionParser parser(&resolver);

  SECTION("Overflow") {
    REQUIRE(parser.Parse("65535 + 1") == 0);  // Wraps to 0
  }

  SECTION("Divide by zero") {
    REQUIRE_THROWS(parser.Parse("10 / 0"));
  }

  SECTION("Invalid syntax") {
    REQUIRE_THROWS(parser.Parse("1 ++ 2"));
    REQUIRE_THROWS(parser.Parse("(1 + 2"));  // Unclosed paren
  }

  SECTION("Undefined symbol") {
    REQUIRE_THROWS(parser.Parse("UNDEFINED_SYMBOL"));
  }
}
```

**Expected Outcome:**
- `expression_parser.h` and `expression_parser.cpp` complete
- `test_expression_parser.cpp` with 100+ test cases
- All tests GREEN
- 100% code coverage for ExpressionParser

### Phase 3: Integration - Z80Universal (Pilot) (2-3 hours)

**Goal:** Prove ExpressionParser works in real parser

**Strategy: Incremental replacement**

1. **Add ExpressionParser member:**
   ```cpp
   // z80_universal_syntax.h
   #include "xasm++/common/expression_parser.h"

   class Z80UniversalSyntaxParser : public ISyntaxParser, public ISymbolResolver {
     // ...
     ExpressionParser expr_parser_;

   public:
     Z80UniversalSyntaxParser() : expr_parser_(this) {}
   };
   ```

2. **Implement ISymbolResolver:**
   ```cpp
   bool Z80UniversalSyntaxParser::TryResolveSymbol(const std::string& name, uint32_t& value) {
     // Delegate to existing symbol table
     return symbol_table_.TryGetSymbol(name, value);
   }
   ```

3. **Replace one expression method:**
   ```cpp
   // Before:
   uint32_t Z80UniversalSyntaxParser::EvaluateExpression(const std::string& expr) {
     // 50+ lines of parsing logic
   }

   // After:
   uint32_t Z80UniversalSyntaxParser::EvaluateExpression(const std::string& expr) {
     return expr_parser_.Parse(expr);
   }
   ```

4. **Test immediately:**
   ```bash
   cmake --build build
   cd build && ctest -R test.*z80.*universal --output-on-failure
   ```

5. **If tests pass, replace remaining expression methods**

6. **Remove old expression parsing code**

7. **Commit:**
   ```bash
   git add -A
   git commit -m "refactor: Z80Universal uses ExpressionParser"
   ```

**Expected Outcome:** Z80Universal parser reduced by ~400 LOC, all tests passing

### Phase 4: Integration - Merlin (2-3 hours)

**Repeat Phase 3 steps for MerlinSyntaxParser**

**Expected Outcome:** Merlin parser reduced by ~350 LOC, all tests passing

### Phase 5: Integration - SCMASM (2-3 hours)

**Repeat Phase 3 steps for ScmasmSyntaxParser**

**Expected Outcome:** SCMASM parser reduced by ~300 LOC, all tests passing

### Phase 6: Verification (1-2 hours)

**Goal:** Confirm refactoring success

**Steps:**

1. **Run full test suite:**
   ```bash
   rm -rf build
   cmake -B build
   cmake --build build
   cd build && ctest --output-on-failure
   ```

2. **Check coverage:**
   ```bash
   cmake -B build -DCMAKE_CXX_FLAGS="--coverage"
   cmake --build build
   cd build && ctest
   gcovr --root .. --filter 'src/common/expression_parser.cpp'
   ```

3. **Measure LOC reduction:**
   ```bash
   # Before (from git history)
   git show HEAD~10:src/syntax/z80_universal/z80_universal_syntax.cpp | wc -l

   # After (current)
   wc -l src/syntax/z80_universal/z80_universal_syntax.cpp
   ```

4. **Performance benchmark (if available):**
   ```bash
   # Run on large assembly file
   time ./xasmpp --input large_file.asm
   ```

5. **Document metrics in acceptance:**
   | Metric | Before | After | Improvement |
   |--------|--------|-------|-------------|
   | Total LOC | 5,379 | ~4,329 | 1,050 (19.5%) |
   | Z80Universal | 2,285 | 1,885 | 400 (17.5%) |
   | Merlin | 1,764 | 1,414 | 350 (19.8%) |
   | SCMASM | 1,330 | 1,030 | 300 (22.6%) |
   | Duplication | 37% | ~8% | 78% reduction |
   | Expression Parser Coverage | N/A | 100% | New component |

**Expected Outcome:** All metrics met, documented in acceptance

## Execution Strategy

**Sequential within phases:**
- Phase 1 must complete before Phase 2
- Phase 2 must complete before Phase 3
- Phases 3-5 must be sequential (one parser at a time)
- Phase 6 is final verification

**Checkpoints:**
- After each parser integration, commit and verify tests
- If any integration fails, rollback that commit and investigate
- Don't proceed to next parser until current one passes all tests

## TDD Approach

**Strict RED-GREEN-REFACTOR for Phase 2:**
1. ❌ RED: Write test for feature
2. ✅ GREEN: Implement minimal code to pass
3. ♻️ REFACTOR: Improve code quality
4. Repeat for next feature

**For Phases 3-5 (Integration):**
- Tests already exist (parser tests)
- Replace implementation with ExpressionParser
- Verify tests still pass (no behavioral change)

## Risk Mitigation

**Risk:** Expression parsing differences between parsers
- **Mitigation:** Thorough Phase 1 analysis, document differences, handle via ISymbolResolver interface

**Risk:** Regression bugs in existing tests
- **Mitigation:** Incremental integration, test after each parser, git checkpoints for rollback

**Risk:** Performance regression
- **Mitigation:** Benchmark before/after, profile if needed, inline hot paths if necessary

**Risk:** New tests don't cover all cases
- **Mitigation:** Achieve 100% line coverage, test edge cases explicitly

## Success Metrics

**Before:**
- Expression parsing: Duplicated 3x
- LOC: 5,379 total, ~1,200 in expression code
- Methods: 36 expression methods across 3 parsers
- Duplication: 37%

**After:**
- Expression parsing: Single implementation
- LOC: ~4,329 total (1,050 reduction)
- Methods: 12 in ExpressionParser (24 removed from parsers)
- Duplication: ~8% (78% improvement)
- Test coverage: 100% for ExpressionParser
- All tests: Passing

## Tools and Commands

```bash
# Build and test
cmake --build build && cd build && ctest

# Coverage
cmake -B build -DCMAKE_CXX_FLAGS="--coverage"
gcovr --root . --filter 'src/common/expression_parser.cpp'

# LOC counting
cloc src/syntax/*/
cloc src/common/expression_parser.cpp

# Performance (if available)
time ./xasmpp --input benchmark.asm
```

## Notes

- This is Phase 1 of 5-phase god-class refactoring plan
- Highest priority due to 37% duplication removal
- Success here enables Phase 2-5 to proceed with confidence
- Template for future extraction work
