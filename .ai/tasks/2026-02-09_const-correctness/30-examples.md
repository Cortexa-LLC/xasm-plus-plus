# Const Correctness Examples

## Before and After Comparisons

### Example 1: EdtasmSyntaxParser::IsCommentLine

**Before:**
```cpp
// Header
bool IsCommentLine(const std::string &line);

// Implementation
bool EdtasmSyntaxParser::IsCommentLine(const std::string &line) {
  // Implementation...
}

// Usage problem:
void process_const_parser(const EdtasmSyntaxParser &parser) {
  // ERROR: Cannot call non-const method on const object
  bool is_comment = parser.IsCommentLine(";comment");  // ❌ Compilation error
}
```

**After:**
```cpp
// Header
bool IsCommentLine(const std::string &line) const;

// Implementation
bool EdtasmSyntaxParser::IsCommentLine(const std::string &line) const {
  // Same implementation...
}

// Usage now works:
void process_const_parser(const EdtasmSyntaxParser &parser) {
  // OK: Can call const method on const object
  bool is_comment = parser.IsCommentLine(";comment");  // ✅ Compiles correctly
}
```

### Example 2: ScmasmSyntaxParser::IsLocalLabel

**Before:**
```cpp
// Header
bool IsLocalLabel(const std::string &label);

// Usage limitation:
class CodeAnalyzer {
  void analyze(const ScmasmSyntaxParser &parser) {
    // ERROR: Cannot call non-const method
    if (parser.IsLocalLabel("@loop")) {  // ❌ Compilation error
      // analyze local label
    }
  }
};
```

**After:**
```cpp
// Header
bool IsLocalLabel(const std::string &label) const;

// Usage now works:
class CodeAnalyzer {
  void analyze(const ScmasmSyntaxParser &parser) {
    // OK: Can call const method
    if (parser.IsLocalLabel("@loop")) {  // ✅ Compiles correctly
      // analyze local label
    }
  }
};
```

### Example 3: Private Utility Methods

**Before:**
```cpp
// Header (private section)
private:
  std::string Trim(const std::string &str);

// Implementation
std::string ScmasmSyntaxParser::Trim(const std::string &str) {
  // Implementation doesn't modify object state
  std::string result = str;
  // ... trimming logic ...
  return result;
}

// Internal usage:
std::string ScmasmSyntaxParser::ParseLabel(const std::string &line) const {
  // ERROR: Cannot call non-const method from const method
  return Trim(line);  // ❌ Logic error - Trim should be const
}
```

**After:**
```cpp
// Header (private section)
private:
  std::string Trim(const std::string &str) const;

// Implementation
std::string ScmasmSyntaxParser::Trim(const std::string &str) const {
  // Same implementation, now properly const
  std::string result = str;
  // ... trimming logic ...
  return result;
}

// Internal usage:
std::string ScmasmSyntaxParser::ParseLabel(const std::string &line) const {
  // OK: Can call const method from const method
  return Trim(line);  // ✅ Correct const propagation
}
```

## Real-World Usage Scenarios

### Scenario 1: Container of Const Parsers

**Before (Wouldn't Compile):**
```cpp
void validate_syntax(const std::vector<ScmasmSyntaxParser> &parsers,
                     const std::vector<std::string> &lines) {
  for (const auto &parser : parsers) {
    for (const auto &line : lines) {
      // ERROR: Cannot call non-const method on const parser
      if (parser.IsCommentLine(line)) {  // ❌ Compilation error
        continue;
      }
      // ... process line ...
    }
  }
}
```

**After (Compiles and Works):**
```cpp
void validate_syntax(const std::vector<ScmasmSyntaxParser> &parsers,
                     const std::vector<std::string> &lines) {
  for (const auto &parser : parsers) {
    for (const auto &line : lines) {
      // OK: Can call const method on const parser
      if (parser.IsCommentLine(line)) {  // ✅ Works correctly
        continue;
      }
      // ... process line ...
    }
  }
}
```

### Scenario 2: Thread-Safe Read Operations

**Before (Not Thread-Safe Without Const):**
```cpp
class ParserCache {
  EdtasmSyntaxParser parser_;
  
  // Not clearly thread-safe - method might modify state?
  bool check_comment(const std::string &line) {
    return parser_.IsCommentLine(line);  // ❌ Unclear if thread-safe
  }
};
```

**After (Clearly Thread-Safe):**
```cpp
class ParserCache {
  EdtasmSyntaxParser parser_;
  
  // Clearly thread-safe - const method guarantees no modification
  bool check_comment(const std::string &line) const {
    return parser_.IsCommentLine(line);  // ✅ Safe for concurrent reads
  }
};
```

### Scenario 3: API Design Clarity

**Before (Ambiguous Intent):**
```cpp
class SyntaxValidator {
public:
  // Does this modify the parser? Unclear from signature
  bool validate(FlexSyntaxParser &parser, const std::string &line);
};
```

**After (Clear Intent):**
```cpp
class SyntaxValidator {
public:
  // Clear: This only reads from parser, doesn't modify it
  bool validate(const FlexSyntaxParser &parser, const std::string &line) const;
};
```

## Compiler Optimization Benefits

### Example: Result Caching

**Before:**
```cpp
bool EdtasmSyntaxParser::IsCommentLine(const std::string &line) {
  // Compiler cannot cache result - method might modify state
  return line.empty() || line[0] == ';';
}

// Called multiple times - compiler must execute each time
for (int i = 0; i < 1000; i++) {
  bool result = parser.IsCommentLine(line);  // ❌ Cannot optimize
}
```

**After:**
```cpp
bool EdtasmSyntaxParser::IsCommentLine(const std::string &line) const {
  // Compiler knows method doesn't modify state - can optimize
  return line.empty() || line[0] == ';';
}

// Called multiple times - compiler can cache result
for (int i = 0; i < 1000; i++) {
  bool result = parser.IsCommentLine(line);  // ✅ Can be optimized
}
```

## Test Coverage Examples

### Test 1: Basic Const Method Call

```cpp
TEST_F(SyntaxConstCorrectnessTest, DocumentConstRequirements) {
  const EdtasmSyntaxParser edtasm_parser;
  
  // This compiles and works because IsCommentLine is const
  bool is_comment = edtasm_parser.IsCommentLine("; comment");
  EXPECT_TRUE(is_comment);
}
```

### Test 2: Const Parser in Container

```cpp
TEST_F(SyntaxConstCorrectnessTest, DocumentConstRequirements) {
  const std::vector<ScmasmSyntaxParser> parsers(3);
  
  // This works because IsCommentLine is const
  for (const auto &parser : parsers) {
    EXPECT_TRUE(parser.IsCommentLine(";"));
  }
}
```

### Test 3: Const Parser Passed to Function

```cpp
void helper(const FlexSyntaxParser &parser) {
  bool result = parser.IsCommentLine("#comment");
  // Works because IsCommentLine is const
}

TEST_F(SyntaxConstCorrectnessTest, DocumentConstRequirements) {
  const FlexSyntaxParser parser;
  helper(parser);  // Compiles and works
}
```

## Summary of Benefits

### Type Safety
- **Before:** Methods could be called incorrectly without compiler error
- **After:** Compiler enforces proper const usage at compile-time

### API Clarity
- **Before:** Unclear if method modifies object state
- **After:** Signature clearly documents read-only operations

### Optimization
- **Before:** Compiler must assume method modifies state
- **After:** Compiler can optimize with knowledge method is read-only

### Thread Safety
- **Before:** Unclear if concurrent calls are safe
- **After:** Const methods guarantee safe concurrent reads

### Maintainability
- **Before:** Developers must read implementation to know if method modifies state
- **After:** Type system documents behavior, reducing cognitive load
