# Implementation Plan

**Task:** Fix .INB include path resolution
**Workflow:** Bugfix (.ai-pack/workflows/bugfix.md)
**Role:** Engineer

---

## Investigation Phase

### Files to Examine

1. **`src/syntax/scmasm_directive_handlers.cpp`** - HandleInb()
   - Current file path resolution logic
   - Where does it look for files?

2. **`include/xasm++/syntax/scmasm_syntax.h`** - ScmasmSyntaxParser
   - current_file_ member (stores source file path)
   - How is it updated during includes?

3. **Other assembler examples:**
   - Check if other directives do relative path resolution
   - Look for existing path utilities

### Expected Findings

- HandleInb() currently opens files directly (std::ifstream)
- Need to extract directory from current_file_
- Need path resolution utilities (dirname, join)

## Implementation Phase

### Step 1: Understand Current Implementation

Read HandleInb() to see:
- How operand (filename) is parsed
- Where file is opened
- What paths are tried

### Step 2: Add Path Resolution Helper

Create helper function to resolve include paths:
```cpp
std::string ResolveIncludePath(const std::string& include_file, 
                                const std::string& source_file) {
    // 1. If absolute path, use as-is
    if (include_file[0] == '/') return include_file;
    
    // 2. Try relative to source file directory
    std::string source_dir = GetDirectoryPath(source_file);
    std::string relative_path = JoinPaths(source_dir, include_file);
    if (FileExists(relative_path)) return relative_path;
    
    // 3. Try current working directory (fallback)
    if (FileExists(include_file)) return include_file;
    
    // 4. Not found
    return "";
}
```

### Step 3: Modify HandleInb()

**Current code (approximate):**
```cpp
void HandleInb(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
    std::string filename = operand;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    // ... parse file ...
}
```

**New code:**
```cpp
void HandleInb(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
    std::string filename = ResolveIncludePath(
        operand, 
        context.parser_state->GetCurrentFile()
    );
    
    if (filename.empty()) {
        throw std::runtime_error("Cannot open file: " + operand);
    }
    
    std::ifstream file(filename);
    // ... parse file ...
}
```

### Step 4: Update current_file_ During Includes

Ensure that when parsing included file, current_file_ is updated:
```cpp
// Save current file
std::string previous_file = current_file_;
current_file_ = resolved_include_path;

// Parse included file
Parse(file_content, section, symbols);

// Restore previous file
current_file_ = previous_file;
```

### Step 5: Add Tests

**Test 1: Relative include**
```cpp
TEST_F(ScmasmSyntaxTest, InbRelativePath) {
    // Create temporary files
    // /tmp/test/main.s includes subdir/included.s
    // Verify assembly succeeds
}
```

**Test 2: Nested includes**
```cpp
TEST_F(ScmasmSyntaxTest, InbNestedIncludes) {
    // main.s includes a.s
    // a.s includes b.s
    // Verify all resolve correctly
}
```

**Test 3: A2osX pattern**
```cpp
TEST_F(ScmasmSyntaxTest, InbA2osXPattern) {
    // Simulate A2osX directory structure
    // A2osX.S.txt includes usr/src/shared/x.printf.s
    // Verify resolution
}
```

## Testing Strategy

1. **Unit tests:** Relative path resolution
2. **Integration test:** A2osX line 605
3. **Regression:** All existing tests pass

## Files to Modify

- `src/syntax/scmasm_directive_handlers.cpp` (HandleInb)
- `src/syntax/scmasm_syntax.cpp` (if path utilities needed)
- `tests/unit/test_scmasm_syntax.cpp` (add tests)

## Path Utilities Needed

May need to add:
```cpp
std::string GetDirectoryPath(const std::string& filepath);
std::string JoinPaths(const std::string& dir, const std::string& file);
bool FileExists(const std::string& path);
```

Or use standard library equivalents (<filesystem> in C++17).

## Risks

- **Low risk:** Path resolution is well-understood
- **Watch for:** Platform differences (Windows vs Unix paths)
- **Consideration:** Should we support search paths like `-I` flags?

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
