# Implementation Plan

**Task ID:** 2026-01-30_error-messages
**Created:** 2026-01-31
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

The implementation will enhance the existing AssemblerError system with rich diagnostic information modeled after Rust's compiler error format. The approach uses a layered architecture: (1) extend the error structure to capture source context, (2) create a dedicated ErrorFormatter for presentation logic, (3) integrate with CLI for colored output.

The key insight is to separate error data capture (AssemblerError) from error presentation (ErrorFormatter). This maintains backward compatibility while enabling rich formatting. The ErrorFormatter will read source files on-demand to display context, use ANSI escape codes for colors, and implement a simple Levenshtein distance algorithm for symbol suggestions.

**Key Technical Decisions:**
- **No external dependencies:** Use ANSI escape codes directly rather than adding color libraries (fmt, termcolor)
- **On-demand source reading:** Cache source lines in memory per file to minimize I/O
- **Optional symbol suggestions:** Leverage existing symbol table without new data structures
- **Backward compatible:** Existing error handling unchanged, new formatter is opt-in

**Patterns to Use:**
- **Builder Pattern** - For constructing rich error messages incrementally
- **Strategy Pattern** - For color vs no-color output modes
- **Singleton** - For source file cache to avoid redundant reads

---

## Critical Files Identified

### Files to Modify
```
1. include/xasm++/core/assembler_error.h
   - Current: Basic AssemblerError struct with message and location
   - Changes: Keep existing struct, add optional context fields (will be populated by formatter)

2. src/main.cpp
   - Current: Plain text error output to stderr
   - Changes: Use ErrorFormatter for rich output, add --no-color flag
```

### Files to Create
```
1. include/xasm++/core/error_formatter.h
   - Purpose: Public API for error formatting
   - Rationale: Separate presentation from error data

2. src/core/error_formatter.cpp
   - Purpose: Implementation of rich error formatting logic
   - Rationale: Complex formatting logic needs dedicated implementation

3. tests/unit/test_error_formatter.cpp
   - Purpose: Comprehensive tests for error formatting
   - Rationale: TDD approach, verify formatting correctness
```

### Files to Read (for context)
```
- include/xasm++/core/assembler_error.h - Current error structure
- src/core/assembler_core.cpp - How errors are created and reported
- src/main.cpp - Current error output implementation
- tests/unit/test_merlin_syntax.cpp - Example test patterns to follow
```

---

## Step-by-Step Implementation Plan

### Phase 1: Foundation - Error Formatter Core
```
Step 1.1: Create ErrorFormatter header with TDD
  - Action: Define ErrorFormatter class with formatError() method
  - Files: include/xasm++/core/error_formatter.h, tests/unit/test_error_formatter.cpp
  - Tests:
    - test_format_basic_error (RED)
    - test_format_with_source_context (RED)
  - Verification: Tests compile but fail (RED state)

Step 1.2: Implement basic error formatting (no colors, no suggestions)
  - Action: Implement ErrorFormatter::formatError() for plain text output
  - Files: src/core/error_formatter.cpp, CMakeLists.txt
  - Tests: Make test_format_basic_error pass (GREEN)
  - Verification: Basic error formatting works

Step 1.3: Add source context display
  - Action: Implement source file reading and context line extraction
  - Files: src/core/error_formatter.cpp
  - Tests: Make test_format_with_source_context pass (GREEN)
  - Verification: Error shows source line with line numbers
```

### Phase 2: Visual Enhancements - Colors and Markers
```
Step 2.1: Add ANSI color support with TDD
  - Action: Add ColorMode enum and ANSI escape code helpers
  - Files: include/xasm++/core/error_formatter.h, src/core/error_formatter.cpp
  - Tests:
    - test_format_with_colors (RED)
    - test_format_no_colors (RED)
    - test_respect_no_color_env (RED)
  - Verification: Tests compile but fail

Step 2.2: Implement color formatting
  - Action: Add ANSI codes for red errors, yellow warnings, cyan hints
  - Files: src/core/error_formatter.cpp
  - Tests: Make color tests pass (GREEN)
  - Verification: Colored output on ANSI terminals

Step 2.3: Add visual column markers (^^^^^)
  - Action: Generate marker line pointing to error column
  - Files: src/core/error_formatter.cpp
  - Tests:
    - test_column_marker_generation (TDD)
    - test_marker_with_tabs_and_spaces (edge case)
  - Verification: Markers accurately point to error location
```

### Phase 3: Symbol Suggestions - "Did You Mean?"
```
Step 3.1: Implement Levenshtein distance algorithm
  - Action: Add levenshteinDistance() helper function
  - Files: src/core/error_formatter.cpp
  - Tests:
    - test_levenshtein_distance (unit test for algorithm)
    - test_find_similar_symbols (TDD)
  - Verification: Algorithm correctly computes edit distance

Step 3.2: Add symbol suggestion system
  - Action: Search symbol table for similar names when symbol undefined
  - Files: src/core/error_formatter.cpp, include/xasm++/core/error_formatter.h
  - Tests:
    - test_suggest_similar_symbol (RED → GREEN)
    - test_no_suggestion_if_too_different (edge case)
    - test_multiple_suggestions (if distance equal)
  - Verification: Relevant suggestions shown for typos
```

### Phase 4: Integration and CLI
```
Step 4.1: Integrate with main.cpp CLI
  - Action: Replace plain error output with ErrorFormatter
  - Files: src/main.cpp
  - Tests: Integration test (manual for now)
  - Verification: xasm++ binary produces rich errors

Step 4.2: Add --no-color command line flag
  - Action: Add CLI option, pass ColorMode to ErrorFormatter
  - Files: src/main.cpp, include/xasm++/cli/command_line_options.h
  - Tests: Manual CLI testing
  - Verification: --no-color disables ANSI codes

Step 4.3: Respect NO_COLOR environment variable
  - Action: Check NO_COLOR env var, auto-disable colors
  - Files: src/main.cpp
  - Tests: Manual testing with NO_COLOR=1
  - Verification: Colors disabled when NO_COLOR set

Step 4.4: Run full test suite
  - Action: Verify all 704 existing tests still pass
  - Files: N/A (verification only)
  - Tests: ctest -j$(nproc)
  - Verification: 704/704 tests passing, no regressions
```

### Phase 5: Documentation and Polish
```
Step 5.1: Add doc comments to public APIs
  - Action: Document ErrorFormatter class and methods
  - Files: include/xasm++/core/error_formatter.h
  - Tests: N/A (documentation)
  - Verification: Doxygen-style comments complete

Step 5.2: Update work log with implementation notes
  - Action: Document decisions, challenges, solutions
  - Files: .ai/tasks/2026-01-30_error-messages/20-work-log.md
  - Tests: N/A (documentation)
  - Verification: Work log reflects actual implementation
```

---

## Testing Strategy

### Unit Tests
```
□ ErrorFormatter::formatError() - Test cases:
  - Basic error formatting (message + location)
  - Error with source context
  - Error with column marker
  - Error with color codes
  - Error without colors (plain text)
  - Multiple errors in sequence

□ Color support - Test cases:
  - ANSI codes for red/yellow/cyan
  - NO_COLOR environment variable respected
  - --no-color flag honored
  - Default color mode

□ Source context reading - Test cases:
  - Read single line from file
  - Handle missing file gracefully
  - Handle empty file
  - Cache hit (don't reread)
  - Invalid line number

□ Symbol suggestions - Test cases:
  - Levenshtein distance calculation
  - Find similar symbol (1-2 edits away)
  - No suggestion if too different (>3 edits)
  - Multiple equally similar symbols
  - Empty symbol table
  - Case sensitivity handling

□ Column marker generation - Test cases:
  - Generate marker for ASCII text
  - Handle tabs correctly (tab width)
  - Handle Unicode characters
  - Column 0 (start of line)
  - Column beyond line length (edge case)
```

### Integration Tests
```
□ CLI integration:
  - Run xasm++ with invalid assembly
  - Verify rich error output
  - Test --no-color flag
  - Test NO_COLOR=1 environment

□ Regression testing:
  - All 704 existing tests pass
  - No behavior changes for error-free code
```

### Manual Testing
```
□ Terminal compatibility:
  - macOS Terminal.app
  - iTerm2
  - Linux terminals (gnome-terminal, xterm)

□ Error scenarios:
  - Undefined symbol
  - Invalid directive
  - Syntax error
  - Multiple errors in one file
  - Errors in multiple files
```

---

## Implementation Sequence (TDD Mandatory)

**CRITICAL: Follow RED-GREEN-REFACTOR cycle for each step**

1. **RED:** Write failing test first
2. **GREEN:** Implement minimal code to pass test
3. **REFACTOR:** Clean up code while keeping tests green

**Workflow:**
```bash
# For each feature:
1. Write test in tests/unit/test_error_formatter.cpp
2. Verify test fails (RED): cmake --build build && ctest --test-dir build
3. Implement feature in src/core/error_formatter.cpp
4. Verify test passes (GREEN): ctest --test-dir build
5. Refactor if needed, keep tests green
6. Commit: git add . && git commit -m "feat: <feature>"
```

---

## Architectural Design

### Class Structure
```cpp
// High-level design (not final API)

class ErrorFormatter {
public:
    enum class ColorMode { Auto, Enabled, Disabled };

    ErrorFormatter(ColorMode mode = ColorMode::Auto);

    // Format single error with rich context
    std::string formatError(
        const AssemblerError& error,
        const SymbolTable* symbols = nullptr  // For suggestions
    );

private:
    ColorMode color_mode_;
    SourceCache source_cache_;  // Cache for source file lines

    // Helper methods
    std::string readSourceLine(const std::string& file, int line);
    std::string generateColumnMarker(int column, int length);
    std::vector<std::string> findSimilarSymbols(
        const std::string& name,
        const SymbolTable* symbols,
        int max_distance = 2
    );
    std::string applyColor(const std::string& text, const std::string& color);
};

class SourceCache {
    // Caches source file contents to avoid repeated reads
    std::unordered_map<std::string, std::vector<std::string>> cache_;
};
```

### ANSI Color Codes
```cpp
// Color constants
constexpr const char* ANSI_RED = "\033[1;31m";
constexpr const char* ANSI_YELLOW = "\033[1;33m";
constexpr const char* ANSI_CYAN = "\033[1;36m";
constexpr const char* ANSI_RESET = "\033[0m";
constexpr const char* ANSI_BOLD = "\033[1m";
```

### Output Format Template
```
{RED}error:{RESET} {message}
    {BOLD}-->{RESET} {file}:{line}:{column}
       |
    {line} | {source_line}
       | {YELLOW}{markers}{RESET} {annotation}
       |
    {CYAN}help:{RESET} {suggestion}
```

---

## Dependency Management

### Build System Changes
```cmake
# Add new source files to CMakeLists.txt
add_library(xasm_core
    # ... existing files ...
    src/core/error_formatter.cpp
)

# Add test executable
add_executable(test_error_formatter
    tests/unit/test_error_formatter.cpp
)
target_link_libraries(test_error_formatter xasm_core gtest_main)
```

### No External Dependencies
- Use standard library only (std::string, std::vector, std::unordered_map)
- ANSI codes via string literals (no termcolor library)
- No fmt or boost dependencies

---

## Risk Mitigation

### Performance Risk: Source File Reading
**Mitigation:**
- Implement SourceCache to avoid repeated reads
- Only read on error (zero cost for error-free code)
- Benchmark: Error formatting should be <10ms

### Compatibility Risk: Terminal Support
**Mitigation:**
- Provide --no-color flag for dumb terminals
- Respect NO_COLOR environment variable (standard)
- Use basic ANSI codes (widely supported)
- Graceful degradation (plain text fallback)

### Complexity Risk: Symbol Suggestions
**Mitigation:**
- Simple Levenshtein distance algorithm (O(n*m))
- Limit to max_distance=2 (avoid expensive suggestions)
- Optional feature (no suggestions if symbol table unavailable)

---

## Success Verification

After implementation, verify:

```bash
# 1. All tests pass
cd build
cmake --build . && ctest -j$(nproc)
# Expected: 704+ tests passing (10+ new tests for ErrorFormatter)

# 2. Manual CLI test
./bin/xasm++ tests/e2e/apple2/invalid.s
# Should show rich error output with colors

# 3. No-color mode works
./bin/xasm++ --no-color tests/e2e/apple2/invalid.s
# Should show plain text output

# 4. Environment variable works
NO_COLOR=1 ./bin/xasm++ tests/e2e/apple2/invalid.s
# Should show plain text output

# 5. Code coverage (if coverage tools available)
# Target: ≥85% coverage for error_formatter.cpp
```

---

## Rollback Plan

If implementation blocked or breaks existing functionality:

1. **Keep existing error system:** AssemblerError unchanged, backward compatible
2. **Feature flag:** Could make ErrorFormatter opt-in via --rich-errors flag
3. **Incremental rollout:** Start with basic formatting, add colors/suggestions later
4. **Revert option:** All changes in isolated files, easy to exclude from build

---

## Future Enhancements (Out of Scope)

- Multi-line error spans (error across multiple lines)
- Warning system (currently only errors)
- Error codes (E001, E002, etc.)
- Localization (i18n)
- JSON output format for IDE integration
- Interactive error fixing suggestions

---

## References

- Rust Error Handling: https://doc.rust-lang.org/book/ch09-00-error-handling.html
- ANSI Escape Codes: https://en.wikipedia.org/wiki/ANSI_escape_code
- Levenshtein Distance: https://en.wikipedia.org/wiki/Levenshtein_distance
- NO_COLOR Standard: https://no-color.org/

---

**Plan Status:** ✅ APPROVED
**Ready for Implementation:** YES
**Assigned to:** Engineer (via agent CLI)

**Last Updated:** 2026-01-31
