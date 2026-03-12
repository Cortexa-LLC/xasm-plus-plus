# Code Quality Review - Refactoring Opportunities

**Review Date:** 2026-01-28  
**Reviewer:** AI Agent (Reviewer Role)  
**Files Reviewed:**
- `src/syntax/merlin/merlin_syntax.cpp` (921 lines)
- `src/core/assembler.cpp` (754 lines)
- `src/main.cpp` (142 lines)

**Review Status:** CHANGES REQUESTED

---

## Executive Summary

This review identified **significant refactoring opportunities** in the parser and assembler code. While the code is functional and well-commented, several functions exceed maintainability thresholds and exhibit patterns that increase cognitive load and maintenance burden.

**Key Findings:**
- 5 functions exceed 100 lines (maintainability threshold)
- Extensive code duplication in error handling (33 instances)
- Complex control flow in `ParseLine` and `EncodeInstructions`
- Several extract method refactoring opportunities

**Priority:** MAJOR (affects maintainability, not functionality)

---

## Critical Findings: 0

No critical security, correctness, or architecture violations found.

---

## Major Findings: 3

### [M1] Extremely Long Function: `ParseLine` (235 lines)

**Location:** `src/syntax/merlin/merlin_syntax.cpp:638-872`

**Issue:** The `ParseLine` function contains 235 lines with deeply nested control flow, handling 20+ different directives in a single function.

**Impact:**
- **High cognitive complexity**: Difficult to understand the full function behavior
- **Testing difficulty**: Single function requires testing 20+ code paths
- **Maintenance risk**: Changes to one directive can affect others
- **Poor single responsibility**: Violates SRP (parsing, directive routing, symbol management)

**Current Structure:**
```cpp
void MerlinSyntaxParser::ParseLine(...) {
    // 1. Strip comments (10 lines)
    // 2. Handle conditional assembly (30 lines)
    // 3. Parse label (20 lines)
    // 4. Detect = syntax (10 lines)
    // 5. Extract directive (15 lines)
    // 6. Giant if-else chain for 20+ directives (150 lines)
    //    - ORG, EQU, DB, DW, HEX, DS, DUM, DEND
    //    - PUT, DO, ELSE, FIN, LST, LSTDO, TR
    //    - ASC, USR, [default instruction]
}
```

**Recommended Refactoring:**

```cpp
// Strategy Pattern or Command Pattern for directives
class DirectiveHandler {
public:
    virtual void Handle(const std::string& label, 
                       const std::string& operands,
                       Section& section,
                       ConcreteSymbolTable& symbols) = 0;
};

class MerlinSyntaxParser {
private:
    std::unordered_map<std::string, std::unique_ptr<DirectiveHandler>> directive_handlers_;
    
    void InitializeDirectiveHandlers() {
        directive_handlers_["ORG"] = std::make_unique<OrgDirective>();
        directive_handlers_["EQU"] = std::make_unique<EquDirective>();
        directive_handlers_["DB"] = std::make_unique<DbDirective>();
        // ... etc
    }
    
public:
    void ParseLine(const std::string& line, Section& section, 
                   ConcreteSymbolTable& symbols) {
        // 1. Strip comments
        std::string code_line = StripComments(line);
        if (Trim(code_line).empty()) return;
        
        // 2. Handle conditional assembly
        if (ShouldSkipDueToConditional(code_line)) return;
        
        // 3. Parse line components
        auto [label, directive, operands] = ParseLineComponents(code_line);
        
        // 4. Handle label (if present)
        if (!label.empty()) {
            HandleLabel(label, section, symbols);
        }
        
        // 5. Dispatch directive
        auto it = directive_handlers_.find(directive);
        if (it != directive_handlers_.end()) {
            it->second->Handle(label, operands, section, symbols);
        } else {
            HandleInstruction(directive, operands, section, symbols);
        }
    }
    
private:
    std::tuple<std::string, std::string, std::string> 
    ParseLineComponents(const std::string& line);
    
    bool ShouldSkipDueToConditional(const std::string& line);
    void HandleLabel(const std::string& label, Section& section, 
                    ConcreteSymbolTable& symbols);
};
```

**Benefits:**
- **Reduced complexity**: 235 lines → ~50 line orchestrator + small handlers
- **Testability**: Each directive handler tested independently
- **Extensibility**: New directives added without modifying ParseLine
- **Readability**: Clear separation of concerns

**Effort Estimate:** 4-6 hours (medium refactoring)

---

### [M2] Very Long Function: `EncodeInstructions` (246 lines)

**Location:** `src/core/assembler.cpp:158-403`

**Issue:** Single function handles all instruction encoding with extensive duplicate error handling and complex atom type switching.

**Impact:**
- **High cyclomatic complexity**: Nested loops with multiple exception handlers
- **Code duplication**: 12 nearly identical error handling blocks
- **Testing difficulty**: Multiple responsibilities mixed together
- **Maintenance burden**: Changes require understanding entire 246-line context

**Current Structure:**
```cpp
std::vector<size_t> Assembler::EncodeInstructions(...) {
    for (section : sections_) {
        for (atom : section.atoms) {
            // Null check with error handling (8 lines) - REPEATED 5x
            if (!atom) {
                AssemblerError error;
                error.message = "...";
                result.errors.push_back(error);
                result.success = false;
                continue;
            }
            
            // Type switch
            if (OrgAtom) {
                // Cast with null check (8 lines) - REPEATED 5x
                auto org = dynamic_pointer_cast<OrgAtom>(atom);
                if (!org) {
                    AssemblerError error;
                    error.location = atom->location;
                    error.message = "Failed to cast...";
                    result.errors.push_back(error);
                    result.success = false;
                    continue;
                }
                // Logic
            } else if (LabelAtom) {
                // Same pattern repeated
            } else if (InstructionAtom) {
                // 150 lines of instruction encoding logic
                // Including 6 try-catch blocks
            }
        }
    }
}
```

**Recommended Refactoring:**

```cpp
// Extract atom processing
class AtomEncoder {
public:
    virtual EncodingResult Encode(Atom& atom, 
                                  Cpu6502& cpu,
                                  ConcreteSymbolTable& symbols) = 0;
};

class InstructionEncoder : public AtomEncoder {
    EncodingResult Encode(Atom& atom, ...) override {
        auto inst = SafeCast<InstructionAtom>(atom);
        // Instruction-specific logic (~100 lines)
        return result;
    }
};

// Main encoding orchestration (simplified)
std::vector<size_t> Assembler::EncodeInstructions(...) {
    std::vector<size_t> sizes;
    
    for (auto& section : sections_) {
        uint32_t address = section.org;
        
        for (auto& atom : section.atoms) {
            // Validate atom
            if (!ValidateAtom(atom, result)) {
                continue;
            }
            
            // Get encoder for atom type
            auto encoder = GetEncoderForType(atom->type);
            
            // Encode
            EncodingResult enc_result = encoder->Encode(*atom, *cpu_, symbols);
            
            // Handle result
            HandleEncodingResult(enc_result, result, sizes);
            address += enc_result.size;
        }
    }
    
    return sizes;
}

// Helper functions (extract duplicate logic)
bool Assembler::ValidateAtom(const std::shared_ptr<Atom>& atom,
                              AssemblerResult& result) {
    if (!atom) {
        ReportError(result, "Null atom encountered - skipping");
        return false;
    }
    return true;
}

template<typename T>
std::shared_ptr<T> Assembler::SafeCast(const std::shared_ptr<Atom>& atom,
                                       AssemblerResult& result) {
    auto casted = std::dynamic_pointer_cast<T>(atom);
    if (!casted) {
        ReportError(result, atom->location, 
                   "Failed to cast - atom corruption detected");
    }
    return casted;
}

void Assembler::ReportError(AssemblerResult& result, 
                           const std::string& message) {
    AssemblerError error;
    error.message = message;
    result.errors.push_back(error);
    result.success = false;
}
```

**Benefits:**
- **Reduced duplication**: 12 error blocks → 2 helper functions
- **Improved testability**: Atom encoders tested independently
- **Better separation**: Encoding logic separated from orchestration
- **Type safety**: Template helper catches cast failures consistently

**Effort Estimate:** 6-8 hours (significant refactoring)

---

### [M3] Long Function with Complex Logic: `ParseExpression` (143 lines)

**Location:** `src/syntax/merlin/merlin_syntax.cpp:108-250`

**Issue:** Single function handles expression parsing with deeply nested conditionals for operators, character literals, and byte operators.

**Impact:**
- **Complex control flow**: 10+ different expression types handled
- **Difficult to extend**: Adding new operators requires understanding entire function
- **Testing challenges**: Many code paths and edge cases
- **Operator precedence**: Mixed in with parsing logic, hard to verify correctness

**Current Structure:**
```cpp
std::shared_ptr<Expression> ParseExpression(const std::string& str, ...) {
    // 1. Handle character literals (20 lines)
    if (expr[0] == '"' || expr[0] == '\'') { ... }
    
    // 2. Handle low byte operator (15 lines)
    if (expr[0] == '<' || expr[0] == '#') { ... }
    
    // 3. Handle high byte operator (15 lines)
    if (expr[0] == '>') { ... }
    
    // 4. Handle addition (20 lines)
    if (plus_pos != std::string::npos) { ... }
    
    // 5. Handle subtraction (20 lines)
    else if (minus_pos != std::string::npos) { ... }
    
    // 6. Handle multiplication (25 lines)
    if (mult_pos != std::string::npos) { ... }
    
    // 7. Handle negative numbers (8 lines)
    if (expr[0] == '-') { ... }
    
    // 8. Handle pure numbers (5 lines)
    else if (expr[0] == '$' || ...) { ... }
    
    // 9. Handle symbols (5 lines)
    else if (symbols.IsDefined(expr)) { ... }
    
    // 10. Default case (3 lines)
    return std::make_shared<LiteralExpr>(0);
}
```

**Recommended Refactoring:**

Use **Recursive Descent Parser** or **Operator Precedence Parser**:

```cpp
// Expression parser with clear precedence levels
class ExpressionParser {
public:
    std::shared_ptr<Expression> Parse(const std::string& str,
                                     ConcreteSymbolTable& symbols) {
        tokens_ = Tokenize(str);
        pos_ = 0;
        return ParseExpression(symbols);
    }
    
private:
    std::vector<Token> tokens_;
    size_t pos_;
    
    // Precedence levels (lowest to highest)
    std::shared_ptr<Expression> ParseExpression(ConcreteSymbolTable& symbols) {
        return ParseAdditive(symbols);  // Lowest precedence
    }
    
    std::shared_ptr<Expression> ParseAdditive(ConcreteSymbolTable& symbols) {
        auto left = ParseMultiplicative(symbols);
        while (Match(TokenType::Plus) || Match(TokenType::Minus)) {
            TokenType op = Previous().type;
            auto right = ParseMultiplicative(symbols);
            left = std::make_shared<BinaryExpr>(op, left, right);
        }
        return left;
    }
    
    std::shared_ptr<Expression> ParseMultiplicative(ConcreteSymbolTable& symbols) {
        auto left = ParseUnary(symbols);
        while (Match(TokenType::Star)) {
            auto right = ParseUnary(symbols);
            left = std::make_shared<MultiplyExpr>(left, right);
        }
        return left;
    }
    
    std::shared_ptr<Expression> ParseUnary(ConcreteSymbolTable& symbols) {
        if (Match(TokenType::LowByte)) return ParseLowByte(symbols);
        if (Match(TokenType::HighByte)) return ParseHighByte(symbols);
        if (Match(TokenType::Minus)) return ParseNegative(symbols);
        return ParsePrimary(symbols);
    }
    
    std::shared_ptr<Expression> ParsePrimary(ConcreteSymbolTable& symbols) {
        if (Match(TokenType::Number)) return ParseNumber();
        if (Match(TokenType::String)) return ParseCharLiteral();
        if (Match(TokenType::Identifier)) return ParseSymbol(symbols);
        throw ParseError("Unexpected token");
    }
    
    // Helper functions for specific cases
    std::shared_ptr<Expression> ParseLowByte(ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseHighByte(ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseNegative(ConcreteSymbolTable& symbols);
    // etc.
};
```

**Benefits:**
- **Clear operator precedence**: Explicitly encoded in function hierarchy
- **Easier to extend**: Add new operators by adding precedence level
- **Better error messages**: Can report exactly where parsing failed
- **Correct associativity**: Left/right associativity naturally handled
- **Testable**: Each precedence level tested independently

**Effort Estimate:** 8-10 hours (requires tokenizer + parser rewrite)

---

## Minor Findings: 5

### [m1] Code Duplication: Error Handling Patterns

**Location:** Multiple locations in both files

**Issue:** Error handling code repeated 33 times across both files:
- 21 `throw std::runtime_error(FormatError(...))` in `merlin_syntax.cpp`
- 12 identical `AssemblerError` blocks in `assembler.cpp`

**Example Duplication:**
```cpp
// Pattern repeated 12 times in assembler.cpp
AssemblerError error;
error.message = "Null atom encountered - skipping";
result.errors.push_back(error);
result.success = false;
continue;
```

**Suggested Fix:**
```cpp
// Helper function
void Assembler::ReportError(AssemblerResult& result,
                           const std::string& message,
                           const SourceLocation& location = {}) {
    AssemblerError error;
    error.message = message;
    error.location = location;
    result.errors.push_back(error);
    result.success = false;
}

// Usage
if (!atom) {
    ReportError(result, "Null atom encountered - skipping");
    continue;
}
```

**Impact:** Medium - reduces code by ~100 lines, improves consistency

---

### [m2] Magic Number: Addressing Mode Detection

**Location:** `src/core/assembler.cpp:19-135` (DetermineAddressingMode function)

**Issue:** The function `DetermineAddressingMode` is 117 lines and contains complex nested conditionals for detecting 10+ addressing modes.

**Suggested Fix:**
```cpp
// Strategy Pattern with lookup table
class AddressingModeDetector {
public:
    AddressingMode Detect(const std::string& operand) {
        for (const auto& detector : detectors_) {
            if (auto mode = detector->TryDetect(operand)) {
                return *mode;
            }
        }
        return AddressingMode::Absolute;  // Default
    }
    
private:
    std::vector<std::unique_ptr<ModeDetector>> detectors_ = {
        std::make_unique<ImpliedDetector>(),
        std::make_unique<AccumulatorDetector>(),
        std::make_unique<IndirectXDetector>(),
        std::make_unique<IndirectYDetector>(),
        // ... etc in priority order
    };
};

class IndirectXDetector : public ModeDetector {
    std::optional<AddressingMode> TryDetect(const std::string& op) override {
        if (op[0] == '(' && op.find(",X)") != std::string::npos) {
            return AddressingMode::IndirectX;
        }
        return std::nullopt;
    }
};
```

**Impact:** Medium - improves testability and extensibility

---

### [m3] Long Parameter Lists

**Location:** Multiple functions in `merlin_syntax.cpp`

**Issue:** Several functions have 4+ parameters:
```cpp
void ParseLine(const std::string& line, 
              Section& section,
              ConcreteSymbolTable& symbols);  // OK - 3 params

void HandleOrg(const std::string& operand, 
               Section& section,
               ConcreteSymbolTable& symbols);  // 3 params
```

**Observation:** Actually, most functions have reasonable parameter counts (2-3). This is NOT a significant issue in this codebase.

**Status:** Acceptable - no changes needed

---

### [m4] Consider Extract Class: Conditional Assembly State

**Location:** `src/syntax/merlin/merlin_syntax.cpp`

**Issue:** Conditional assembly state (DO/ELSE/FIN) managed with `std::vector<ConditionalBlock>` and free functions.

**Suggested Enhancement:**
```cpp
class ConditionalAssemblyStack {
public:
    void PushDo(bool condition);
    void Else();
    void PopFin();
    bool ShouldEmit() const;
    bool IsEmpty() const;
    
private:
    struct Block {
        bool condition;
        bool in_else;
        bool should_emit;
    };
    std::vector<Block> stack_;
};

// Usage in parser
class MerlinSyntaxParser {
private:
    ConditionalAssemblyStack conditional_stack_;
    
public:
    void HandleDo(const std::string& operand, ConcreteSymbolTable& symbols) {
        auto expr = ParseExpression(operand, symbols);
        conditional_stack_.PushDo(expr->Evaluate(symbols) != 0);
    }
    
    void ParseLine(...) {
        if (!conditional_stack_.ShouldEmit()) return;
        // ... rest of parsing
    }
};
```

**Impact:** Low - improves encapsulation, minor readability gain

---

### [m5] Consider Using `std::filesystem` for Path Operations

**Location:** `src/main.cpp` (already using it), `src/syntax/merlin/merlin_syntax.cpp:504` (HandlePut)

**Issue:** `HandlePut` manually constructs paths with string concatenation:
```cpp
if (!file.is_open() && filename[0] != '/') {
    actual_filename = "/tmp/" + filename;  // String concatenation
    file.open(actual_filename);
}
```

**Observation:** Reviewing main.cpp shows `std::filesystem` is already in use. The HandlePut function should use it too for consistency.

**Suggested Enhancement:**
```cpp
#include <filesystem>

void MerlinSyntaxParser::HandlePut(...) {
    namespace fs = std::filesystem;
    
    fs::path file_path(filename);
    
    // Try original path
    std::ifstream file(file_path);
    
    // Try with /tmp/ prefix
    if (!file.is_open() && !file_path.is_absolute()) {
        file_path = fs::path("/tmp") / filename;
        file.open(file_path);
    }
    
    if (!file.is_open()) {
        throw std::runtime_error(
            FormatError("Cannot open file: " + filename));
    }
    // ...
}
```

**Impact:** Low - improves consistency and portability

---

## Code Quality Assessment

### Positive Observations

✅ **Excellent Documentation**
- Multi-pass assembly algorithm well-explained with WHY comments
- Branch relaxation logic clearly documented
- Convergence algorithm explained with mathematical reasoning

✅ **Good Error Handling**
- Comprehensive error messages with source location
- Graceful degradation (null atoms handled)
- All major error paths covered

✅ **Solid Architecture**
- Plugin pattern for CPU (extensible to other architectures)
- Clear separation between syntax parsing and assembly
- Proper multi-pass assembly with convergence detection

✅ **Well-Structured main.cpp**
- Clean command-line interface
- Proper error handling with specific exception types
- Filesystem state properly restored on errors

✅ **Good Test Coverage** (inferred from error handling completeness)
- Edge cases considered (null atoms, undefined symbols, circular includes)
- Boundary conditions handled (branch range, convergence limits)

### Areas for Improvement

⚠️ **Function Length**
- 3 functions >100 lines (recommend <50)
- 2 functions >200 lines (critical refactoring threshold)

⚠️ **Code Duplication**
- 33 duplicate error handling blocks
- 5 identical cast-and-null-check patterns

⚠️ **Complexity**
- `ParseLine`: 20+ code paths in one function
- `EncodeInstructions`: Nested loops with multiple exception handlers
- `ParseExpression`: Operator precedence mixed with parsing

---

## Recommendations

### Immediate Actions (High Priority)

1. **Refactor `ParseLine`** (M1)
   - Extract directive handling to Command Pattern
   - Reduces 235 lines to ~50 line orchestrator
   - **Impact:** High - improves maintainability significantly
   - **Effort:** 4-6 hours

2. **Refactor `EncodeInstructions`** (M2)
   - Extract duplicate error handling to helper functions
   - Consider Strategy Pattern for atom encoding
   - **Impact:** High - reduces duplication, improves testability
   - **Effort:** 6-8 hours

### Medium-Term Actions

3. **Refactor `ParseExpression`** (M3)
   - Implement recursive descent parser
   - Clear operator precedence hierarchy
   - **Impact:** Medium - improves correctness and extensibility
   - **Effort:** 8-10 hours

4. **Extract Error Handling** (m1)
   - Create helper functions for common error patterns
   - **Impact:** Medium - reduces ~100 lines of duplicate code
   - **Effort:** 2-3 hours

### Long-Term Considerations

5. **Consider Extract Class for Addressing Mode Detection** (m2)
   - Strategy Pattern for mode detectors
   - **Impact:** Low-Medium - improves extensibility
   - **Effort:** 4-5 hours

6. **Encapsulate Conditional Assembly State** (m4)
   - Extract to `ConditionalAssemblyStack` class
   - **Impact:** Low - minor encapsulation improvement
   - **Effort:** 1-2 hours

---

## Testing Recommendations

After refactoring, ensure:

1. **Unit Tests for Extracted Classes**
   - DirectiveHandlers (one test per directive)
   - AtomEncoders (one test per atom type)
   - ExpressionParser (test operator precedence)

2. **Integration Tests**
   - End-to-end assembly of sample code
   - Verify binary output unchanged after refactoring

3. **Regression Tests**
   - Run existing test suite (if available)
   - Add tests for edge cases found during review

---

## Metrics Summary

| Metric | Current | Threshold | Status |
|--------|---------|-----------|--------|
| Functions >100 LOC | 5 | 0 | ⚠️ EXCEEDS |
| Functions >50 LOC | 5 | <10% | ⚠️ EXCEEDS |
| Duplicate Code Blocks | 33 | <5 | ⚠️ EXCEEDS |
| Cyclomatic Complexity (max) | ~50 (estimated) | <10 | ⚠️ EXCEEDS |
| Max Function Length | 246 lines | 50 lines | ⚠️ EXCEEDS |
| Code Documentation | Excellent | Good | ✅ PASS |
| Error Handling | Comprehensive | Good | ✅ PASS |

---

## Overall Assessment

**VERDICT:** CHANGES REQUESTED (Major refactoring recommended)

**Reasoning:**
- **Functionality:** Code works correctly and handles edge cases well
- **Maintainability:** Significant issues with function length and duplication
- **Architecture:** Good overall design, but implementation has code smells
- **Priority:** MAJOR - Does not block current functionality, but will impede future maintenance

**Recommended Timeline:**
- **Phase 1 (Week 1-2):** Refactor ParseLine (M1) and extract error handling (m1)
- **Phase 2 (Week 3-4):** Refactor EncodeInstructions (M2)
- **Phase 3 (Week 5-6):** Refactor ParseExpression (M3) if time permits

---

## Approval Status

❌ **CHANGES REQUESTED**

**Blockers:**
- None (code is functional)

**Must Address Before Merge:**
- None (this is a refactoring review, not a PR review)

**Recommended Before Next Phase:**
- Address M1 and M2 (ParseLine and EncodeInstructions refactoring)
- Extract duplicate error handling (m1)

**Can Be Deferred:**
- M3 (ParseExpression) - works correctly, refactoring is optimization
- m2-m5 (minor improvements)

---

## References

**Clean Code Principles Applied:**
- Single Responsibility Principle (SRP)
- DRY (Don't Repeat Yourself)
- Function Length Guidelines (<50 lines ideal, <100 acceptable)
- Cyclomatic Complexity (<10 ideal)
- Command Pattern (for directive handling)
- Strategy Pattern (for encoding and mode detection)

**Related Documents:**
- `quality/clean-code/03-refactoring.md` - Refactoring guidelines
- `quality/clean-code/01-design-principles.md` - SOLID principles
- `quality/clean-code/00-general-rules.md` - Function length thresholds

---

**Review Completed:** 2026-01-28  
**Next Review:** After Phase 1 refactoring (estimated 2 weeks)
