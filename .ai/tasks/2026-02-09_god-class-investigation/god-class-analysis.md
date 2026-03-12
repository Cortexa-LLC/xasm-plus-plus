# God-Class Refactoring Analysis
**Project:** xasm++  
**Date:** 2026-02-09  
**Analyst:** Engineer (AI)  

---

## Executive Summary

Three syntax parser classes exhibit classic god-class anti-pattern with excessive responsibilities:

| Parser | LOC | Methods | Assessment |
|--------|-----|---------|------------|
| Z80UniversalSyntaxParser | 2,285 | 63 | CRITICAL - Severe god-class |
| MerlinSyntaxParser | 1,764 | 40 | HIGH - Major god-class |
| ScmasmSyntaxParser | 1,330 | 27 | MODERATE - God-class |

**Combined Total:** 5,379 LOC across similar responsibilities

**Recommendation:** Systematic refactoring using **Strategy + Template Method** patterns to extract common responsibilities into reusable components.

---

## 1. Responsibility Analysis

### 1.1 Z80UniversalSyntaxParser Responsibilities

**Primary Responsibilities (63 methods):**

#### A. Line Parsing & Tokenization (6 methods)
- `Parse()` - Main entry point, line-by-line processing
- `ParseLine()` - Single line parsing orchestration
- `ParseLabel()` - Label detection and definition
- `StripComments()` - Comment removal
- `Trim()` - Whitespace trimming
- `FormatError()` - Error message formatting

#### B. Directive Handling (23 methods)
**Data Directives (5):**
- `HandleDB()` - Define byte
- `HandleDW()` - Define word (16-bit)
- `HandleD3()` - Define 3-byte (24-bit)
- `HandleDWORD()` - Define double-word (32-bit)
- `HandleDS()` - Define space

**Symbol Directives (4):**
- `HandleEqu()` - Fixed equate
- `HandleSet()` - Redefinable equate
- `HandlePublic()` - Export symbols
- `HandleExtern()` - Import symbols

**Segment Directives (5):**
- `HandleOrg()` - Set origin
- `HandleCseg()` - Code segment
- `HandleDseg()` - Data segment
- `HandleAseg()` - Absolute segment
- `HandleCommon()` - Common block

**Conditional Directives (14):**
- `HandleIf()` - Basic conditional
- `HandleIfdef()` / `HandleIfndef()` - Symbol existence
- `HandleIfeq()` / `HandleIfne()` - Equality comparison
- `HandleIflt()` / `HandleIfgt()` - Relational comparison (<, >)
- `HandleIfle()` / `HandleIfge()` - Relational comparison (<=, >=)
- `HandleIf1()` / `HandleIf2()` - Pass-based conditional
- `HandleIfb()` / `HandleIfnb()` - Blank/non-blank check
- `HandleIfidn()` / `HandleIfdif()` - String comparison
- `HandleElse()` / `HandleEndif()` - Conditional flow control
- `EvaluateCondition()` - Condition expression evaluator

**Macro Directives (7):**
- `HandleMacro()` - Begin macro definition
- `HandleEndm()` - End macro/REPT definition
- `HandleRept()` - Simple repetition
- `HandleIrp()` - Iterate over list
- `HandleIrpc()` - Iterate over string
- `ExpandMacro()` - Macro expansion logic
- `SubstituteParam()` - Parameter substitution

**Other (1):**
- `HandleEnd()` - Assembly termination

#### C. Expression Parsing (12 methods)
**Recursive descent parser with operator precedence:**
- `ParseExpression()` - Entry point
- `ParseLogicalOr()` - || operator (lowest precedence)
- `ParseLogicalAnd()` - && operator
- `ParseBitwiseOr()` - | operator
- `ParseBitwiseXor()` - ^ operator
- `ParseBitwiseAnd()` - & operator
- `ParseComparison()` - ==, !=, <, >, <=, >= operators
- `ParseShift()` - <<, >> operators
- `ParseAddSubtract()` - +, - operators
- `ParseMultiplyDivide()` - *, /, % operators
- `ParseUnary()` - Unary -, ~, !, HIGH(), LOW()
- `ParsePrimary()` - Literals, symbols, parentheses

#### D. Number Parsing (1 method)
- `ParseNumber()` - Hex ($FF, 0xFF, FFH), binary (11110000B), octal (377O), decimal

#### E. Symbol List Parsing (1 method)
- `ParseSymbolList()` - Comma-separated symbol parsing for PUBLIC/EXTERN

#### F. State Management (15+ fields)
- Macro definition state (`in_macro_definition_`, `current_macro_`, `macros_`)
- REPT block state (`in_rept_definition_`, `current_rept_`)
- Conditional stack (`conditional_stack_`)
- Label scoping (`current_scope_`)
- Address tracking (`current_address_`)
- Segment management (`segment_manager_`)
- Source location (`current_file_`, `current_line_`)
- Exit flags (`end_directive_seen_`, `exitm_requested_`)
- Recursion depth (`macro_expansion_depth_`)

**Total Responsibilities:** 6 major categories, 63 methods, 15+ state fields

---

### 1.2 MerlinSyntaxParser Responsibilities

**Primary Responsibilities (40 methods):**

Similar structure to Z80UniversalSyntaxParser with Merlin-specific syntax:

#### A. Line Parsing & Tokenization
- `Parse()` - Main entry point
- `ParseLine()` - Line parsing with Merlin syntax rules
- `ParseLabel()` - Merlin-style label detection
- `StripComments()` - Comment stripping (`;` and `*` styles)
- `Trim()` - Whitespace handling
- `FormatError()` - Error formatting

#### B. Directive Handling (~20 methods)
**Data Directives:**
- `HandleDB()` / `HandleASC()` / `HandleSTR()` - Byte/string data
- `HandleDW()` / `HandleDA()` / `HandleADR()` - Word data
- `HandleDS()` / `HandleDFS()` - Space reservation

**Symbol Directives:**
- `HandleEqu()` - Symbol definition
- `HandleExt()` / `HandleXref()` - External symbols

**Segment Directives:**
- `HandleOrg()` - Origin
- `HandlePut()` / `HandleUse()` - File inclusion
- `HandleSav()` - Save output

**Conditional Directives:**
- `HandleIf()` / `HandleElse()` / `HandleFin()` - Merlin conditionals
- `HandleDo()` / `HandleLup()` - Loop directives

**Macro Directives:**
- `HandleMac()` / `HandleEom()` / `HandlePMC()` - Merlin macros
- `ExpandMacro()` - Macro expansion

#### C. Expression Parsing (~10 methods)
- Recursive descent parser similar to Z80Universal
- Merlin-specific operator support (<, >, ^)

#### D. Number Parsing
- Merlin formats: `$hex`, `%binary`, `@octal`, decimal

#### E. State Management
- Similar state fields to Z80Universal
- Merlin-specific features (PUT stack, conditional nesting)

**Total Responsibilities:** Similar pattern to Z80Universal, adapted for Merlin syntax

---

### 1.3 ScmasmSyntaxParser Responsibilities

**Primary Responsibilities (27 methods):**

Smaller but follows same god-class pattern:

#### A. Line Parsing & Tokenization
- `Parse()`, `ParseLine()`, `ParseLabel()`
- `StripComments()`, `Trim()`, `FormatError()`

#### B. Directive Handling (~12 methods)
- Data directives (byte, word, space)
- Symbol directives (EQU, SET)
- Segment directives (ORG, SECTION)
- Conditional directives (IF/ELSE/ENDIF)

#### C. Expression Parsing (~6 methods)
- Simplified recursive descent parser
- Basic operator support

#### D. Number Parsing
- SCMASM formats: `$hex`, `%binary`, decimal

#### E. State Management
- Minimal state compared to others

**Total Responsibilities:** Reduced scope but same architectural problems

---

## 2. Common Responsibilities Across All Parsers

### Shared Responsibilities (Candidates for Extraction)

| Responsibility | Z80 | Merlin | SCMASM | Extraction Priority |
|----------------|-----|--------|--------|---------------------|
| **Expression Parsing** | 12 methods | ~10 methods | ~6 methods | **CRITICAL** |
| **Number Parsing** | 1 method | 1 method | 1 method | **HIGH** |
| **Conditional Assembly** | 14 methods | ~8 methods | ~4 methods | **HIGH** |
| **Macro Processing** | 7 methods | ~6 methods | ~3 methods | **MEDIUM** |
| **Data Directives** | 5 methods | ~6 methods | ~4 methods | **MEDIUM** |
| **Symbol Management** | 4 methods | ~3 methods | ~2 methods | **LOW** |
| **Line Tokenization** | 6 methods | ~6 methods | ~6 methods | **LOW** |

---

## 3. Refactoring Strategy

### 3.1 Proposed Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     SyntaxParser                            │
│                  (Abstract Base Class)                      │
│  ┌───────────────────────────────────────────────────────┐ │
│  │ Template Method Pattern                               │ │
│  │ - Parse() orchestrates parsing pipeline              │ │
│  │ - Calls protected virtual methods for customization  │ │
│  └───────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                            △
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
┌───────▼─────────┐  ┌──────▼────────┐  ┌──────▼─────────┐
│ Z80UniversalSP  │  │  MerlinSP     │  │  ScmasmSP      │
│ (Concrete)      │  │  (Concrete)   │  │  (Concrete)    │
└─────────────────┘  └───────────────┘  └────────────────┘

┌─────────────────────────────────────────────────────────────┐
│              Extracted Strategy Components                  │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────────┐  ┌──────────────────┐               │
│  │ ExpressionParser │  │ NumberParser     │               │
│  │ (Strategy)       │  │ (Strategy)       │               │
│  └──────────────────┘  └──────────────────┘               │
│  ┌──────────────────┐  ┌──────────────────┐               │
│  │ DirectiveHandler │  │ MacroProcessor   │               │
│  │ (Strategy)       │  │ (Strategy)       │               │
│  └──────────────────┘  └──────────────────┘               │
│  ┌──────────────────┐  ┌──────────────────┐               │
│  │ ConditionalAsm   │  │ SymbolManager    │               │
│  │ (Helper)         │  │ (Facade)         │               │
│  └──────────────────┘  └──────────────────┘               │
└─────────────────────────────────────────────────────────────┘
```

### 3.2 Extraction Plan (Phased Approach)

#### Phase 1: Expression Parser Extraction (CRITICAL PRIORITY)
**Goal:** Extract expression parsing into reusable component

**Files to Create:**
```
include/xasm++/expression_parser.h
src/expression_parser.cpp
tests/test_expression_parser.cpp
```

**Methods to Extract (from Z80Universal):**
- `ParseExpression()` → `ExpressionParser::Parse()`
- `ParseLogicalOr()` through `ParsePrimary()` → 12 recursive methods
- Move to stateless strategy class with configurable operators

**Benefits:**
- **Reusability:** All 3 parsers use same expression logic
- **Testability:** Isolated unit testing of expression parsing
- **Maintainability:** Single source of truth for expressions
- **LOC Reduction:** ~400 LOC removed from each god-class

**Estimated Effort:** 16-24 hours
- Extract class: 4 hours
- Write comprehensive tests: 8 hours
- Integrate into 3 parsers: 6 hours
- Regression testing: 4 hours

---

#### Phase 2: Number Parser Extraction (HIGH PRIORITY)
**Goal:** Extract number format parsing

**Files to Create:**
```
include/xasm++/number_parser.h
src/number_parser.cpp
tests/test_number_parser.cpp
```

**Strategy:**
- Configurable number parser with format rules
- Support: `$hex`, `0xhex`, `FFH`, `11111111B`, `377O`, decimal
- Parser-specific customization via configuration object

**Benefits:**
- Removes duplicated number parsing logic
- Centralizes radix/format handling
- Easy to add new number formats

**Estimated Effort:** 8-12 hours

---

#### Phase 3: Conditional Assembly Extraction (HIGH PRIORITY)
**Goal:** Extract conditional assembly logic

**Files to Create:**
```
include/xasm++/conditional_assembler.h
src/conditional_assembler.cpp
tests/test_conditional_assembler.cpp
```

**Methods to Extract:**
- All `HandleIf*()` methods (14 in Z80Universal)
- `EvaluateCondition()` logic
- Conditional stack management

**Strategy:**
- Create `ConditionalAssembler` helper class
- Maintain stack of conditional blocks
- Expose `ShouldEmit()` query method
- Handle IF/ELSE/ENDIF lifecycle

**Benefits:**
- Removes 14+ methods from each parser
- Centralizes conditional logic
- Easier to add new conditional directives

**Estimated Effort:** 12-16 hours

---

#### Phase 4: Macro Processor Extraction (MEDIUM PRIORITY)
**Goal:** Extract macro processing logic

**Files to Create:**
```
include/xasm++/macro_processor.h
src/macro_processor.cpp
tests/test_macro_processor.cpp
```

**Methods to Extract:**
- `HandleMacro()`, `HandleEndm()`, `ExpandMacro()`
- `HandleRept()`, `HandleIrp()`, `HandleIrpc()`
- `SubstituteParam()` logic
- Macro definition storage and retrieval

**Strategy:**
- Create `MacroProcessor` strategy class
- Support parameter substitution (&PARAM, %PARAM, bare)
- Handle LOCAL symbol uniquification
- Recursion depth protection
- EXITM support

**Benefits:**
- Removes ~7 methods from each parser
- Centralizes macro expansion logic
- Reduces complexity of main parser

**Estimated Effort:** 16-24 hours

---

#### Phase 5: Directive Handler Abstraction (MEDIUM PRIORITY)
**Goal:** Create directive handler framework

**Files to Create:**
```
include/xasm++/directive_handler.h
src/directive_handler.cpp
tests/test_directive_handler.cpp
```

**Strategy:**
- Abstract base class: `DirectiveHandler`
- Concrete handlers for each directive category:
  - `DataDirectiveHandler` (DB, DW, DS, etc.)
  - `SymbolDirectiveHandler` (EQU, SET, PUBLIC, EXTERN)
  - `SegmentDirectiveHandler` (ORG, CSEG, DSEG, ASEG)
- Registry pattern for directive dispatch

**Benefits:**
- Removes 20+ directive methods from main parser
- Enables directive plugins
- Clearer separation of concerns

**Estimated Effort:** 20-32 hours

---

### 3.3 Refactoring Metrics

**Current State:**
- Total LOC: 5,379 (3 god-classes)
- Total Methods: 130 methods
- Average Cyclomatic Complexity: HIGH
- Code Duplication: ~40% (expression/number parsing)

**Target State (after all phases):**
- Parser LOC: ~800 each (2,400 total)
- Extracted Components LOC: ~1,500
- Total LOC: ~3,900 (27% reduction)
- Reusable Components: 6 classes
- Code Duplication: <5%
- Average Cyclomatic Complexity: MEDIUM

**Quality Improvements:**
- ✅ Single Responsibility Principle: Each class has one reason to change
- ✅ Open-Closed Principle: Extend via strategies, not modification
- ✅ Dependency Inversion: Depend on abstractions (ExpressionParser interface)
- ✅ Testability: Unit test each component in isolation
- ✅ Maintainability: Clear boundaries, low coupling

---

## 4. Risk Assessment

### 4.1 Technical Risks

**Risk 1: Regression Bugs**
- **Probability:** MEDIUM
- **Impact:** HIGH
- **Mitigation:**
  - Comprehensive test suite before refactoring
  - Refactor one phase at a time
  - Keep old code until new code passes all tests
  - Compare outputs of old vs. new parsers on real-world code

**Risk 2: Performance Degradation**
- **Probability:** LOW
- **Impact:** MEDIUM
- **Mitigation:**
  - Profile before/after refactoring
  - Use inline functions for hot paths
  - Minimize virtual function overhead in tight loops
  - Benchmark with large assembly files

**Risk 3: API Breaking Changes**
- **Probability:** MEDIUM
- **Impact:** LOW
- **Mitigation:**
  - Maintain public API compatibility
  - Refactor internals only
  - Use adapter pattern if needed

**Risk 4: Incomplete Extraction**
- **Probability:** LOW
- **Impact:** MEDIUM
- **Mitigation:**
  - Start with most isolated component (ExpressionParser)
  - Incremental integration
  - Fallback to old implementation if issues arise

### 4.2 Integration Risks

**Risk:** Coordinating changes across 3 parsers
- **Mitigation:** Refactor shared components first, then integrate one parser at a time

---

## 5. Prioritized Refactoring Plan

### Recommended Execution Order

**Priority 1: Expression Parser (Immediate)**
- Highest reusability (3 parsers)
- Most isolated logic
- Biggest LOC reduction (~400 LOC per parser)
- **Effort:** 16-24 hours

**Priority 2: Number Parser (Immediate)**
- Simple, self-contained
- Low risk
- Removes duplication
- **Effort:** 8-12 hours

**Priority 3: Conditional Assembly (Next Quarter)**
- High complexity
- Moderate reusability
- Significant LOC reduction
- **Effort:** 12-16 hours

**Priority 4: Macro Processor (Next Quarter)**
- Complex state management
- Requires careful design
- Moderate LOC reduction
- **Effort:** 16-24 hours

**Priority 5: Directive Handler Framework (Future)**
- Lowest priority
- Enables directive plugins
- Long-term maintainability benefit
- **Effort:** 20-32 hours

**Total Estimated Effort:** 72-108 hours (9-14 person-days)

---

## 6. Success Metrics

### Quantitative Metrics

**Before Refactoring:**
- Total LOC: 5,379
- Duplicated Code: ~2,000 LOC (37%)
- Average Methods per Class: 43
- Test Coverage: TBD

**After Phase 1-2 (Expression + Number):**
- Total LOC: ~4,200 (22% reduction)
- Duplicated Code: ~800 LOC (19%)
- Average Methods per Class: ~35
- Test Coverage: 85%+ for extracted components

**After All Phases:**
- Total LOC: ~3,900 (27% reduction)
- Duplicated Code: <200 LOC (<5%)
- Average Methods per Class: ~20
- Test Coverage: 90%+ overall

### Qualitative Metrics

**Code Quality:**
- ✅ SOLID principles compliance
- ✅ Clear separation of concerns
- ✅ Reduced cyclomatic complexity
- ✅ Improved readability

**Developer Experience:**
- ✅ Easier to add new directives
- ✅ Faster to implement new parsers
- ✅ Reduced cognitive load
- ✅ Clearer error messages

**Maintainability:**
- ✅ Bugs fixed in one place propagate to all parsers
- ✅ Unit tests catch regressions early
- ✅ Code reviews are faster (smaller changes)

---

## 7. Alternative Approaches Considered

### Alternative 1: Big-Bang Refactoring
**Pros:**
- Faster to complete
- More cohesive architecture

**Cons:**
- HIGH RISK - all parsers broken at once
- Longer testing cycle
- Harder to isolate bugs

**Why Not Chosen:** Too risky, conflicts with incremental delivery principles

### Alternative 2: Duplicate Code and Diverge
**Pros:**
- Zero refactoring effort
- No risk of breaking existing code

**Cons:**
- Technical debt accumulates
- Bugs multiply across 3+ codebases
- New features must be implemented 3+ times

**Why Not Chosen:** Violates DRY principle, long-term maintenance nightmare

### Alternative 3: Parser Generator (Yacc/Bison/ANTLR)
**Pros:**
- Declarative syntax definitions
- Well-tested parsing infrastructure

**Cons:**
- Steep learning curve
- Less control over error messages
- Harder to customize per-syntax quirks
- Complete rewrite required

**Why Not Chosen:** Too disruptive, current architecture can be salvaged

---

## 8. Conclusion

The three syntax parsers exhibit severe god-class anti-pattern with 5,379 LOC and 130 methods performing overlapping responsibilities. **Immediate refactoring is recommended** using a phased approach:

1. **Extract ExpressionParser** (16-24 hours) - Removes 37% duplication
2. **Extract NumberParser** (8-12 hours) - Removes remaining format duplication
3. **Extract ConditionalAssembler** (12-16 hours) - Simplifies control flow
4. **Extract MacroProcessor** (16-24 hours) - Isolates complex state
5. **Create DirectiveHandler framework** (20-32 hours) - Enables extensibility

**Total Effort:** 72-108 hours (9-14 person-days) spread across 2-3 quarters

**Expected Outcome:**
- 27% LOC reduction
- <5% code duplication
- 90%+ test coverage
- Maintainable, extensible architecture

**Risk Level:** MEDIUM (with proper mitigation)

**ROI:** HIGH - Investment pays off in reduced maintenance burden and faster feature development

---

## Appendices

### A. Method Count Breakdown

**Z80UniversalSyntaxParser (63 methods):**
- Line parsing: 6
- Data directives: 5
- Symbol directives: 4
- Segment directives: 5
- Conditional directives: 14
- Macro directives: 7
- Expression parsing: 12
- Number parsing: 1
- Utilities: 9

**MerlinSyntaxParser (40 methods):**
- Similar distribution, scaled down

**ScmasmSyntaxParser (27 methods):**
- Similar distribution, further scaled down

### B. Dependency Graph

```
Z80UniversalSyntaxParser
  ├─ depends on → ConcreteSymbolTable (symbol storage)
  ├─ depends on → Section (atom accumulation)
  ├─ depends on → SegmentManager (address tracking)
  ├─ depends on → CpuZ80 (CPU-specific instruction encoding)
  └─ creates → Expression trees (via ParseExpression)

Refactored:
Z80UniversalSyntaxParser
  ├─ uses → ExpressionParser (strategy)
  ├─ uses → NumberParser (strategy)
  ├─ uses → ConditionalAssembler (helper)
  ├─ uses → MacroProcessor (strategy)
  └─ uses → DirectiveHandler (registry)
```

### C. Test Coverage Requirements

**Per Extracted Component:**
- Unit tests: 90%+ line coverage
- Edge cases: boundary values, error conditions
- Integration tests: with mock parsers
- Regression tests: compare old vs. new outputs

**Per Parser (after integration):**
- End-to-end tests: assemble real-world code
- Compatibility tests: compare with reference assemblers
- Performance tests: benchmark large files

---

**Document Version:** 1.0  
**Last Updated:** 2026-02-09  
**Next Review:** After Phase 1 completion
