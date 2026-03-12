# Implementation Plan

**Task ID:** 2026-02-16_fix-inb-directive
**Created:** 2026-02-16
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

The .INB directive currently exists but doesn't properly include and parse files. The implementation will use a recursive parsing approach where .INB triggers a nested parse of the included file with the same symbol table and state.

**Key Technical Decisions:**
- **Recursive ParseFile() approach** - Cleaner than inline expansion, better error messages with file/line tracking
- **Shared symbol table** - Included files add symbols to the same global table
- **Include stack tracking** - Prevents infinite recursion with depth limit of 10
- **Path resolution** - Relative to the directory of the file containing the .INB directive

**Patterns to Use:**
- **Recursive Descent** - ParseFile() calls itself for includes
- **State Preservation** - Save/restore file context when entering/exiting includes
- **Error Context** - Track include stack for clear error messages

---

## Critical Files Identified

### Files to Modify

```
1. src/syntax/scmasm_syntax.cpp
   - Current: Main parser with ParseFile() handling single file
   - Changes:
     * Make ParseFile() recursive (can be called for includes)
     * Track include stack to prevent infinite loops
     * Resolve relative paths correctly
     * Maintain line/file context for error messages

2. src/syntax/scmasm_directive_handlers.cpp
   - Current: .INB handler is stub or incomplete
   - Changes:
     * Implement proper .INB directive handler
     * Call ParseFile() recursively for included file
     * Handle path resolution (relative to current file)
     * Error handling for missing files

3. tests/unit/test_scmasm_syntax.cpp
   - Current: Tests for existing directives and features
   - Changes:
     * Add 5+ test cases for .INB directive
     * Test simple include, nested includes, forward refs
     * Test error cases (missing file, circular include)
     * Test A2osX inc/mli.i scenario
```

### Files to Create

```
(No new files needed - modify existing)
```

### Files to Read (for context)

```
- src/syntax/scmasm_syntax.h - Parser interface, include stack structure
- include/xasm++/common/symbol_table.h - How symbols are stored
- ~/Projects/Vintage/Apple/A2osX/inc/mli.i - Real include file to test
- ~/Projects/Vintage/Apple/A2osX/A2osX.S.txt - Main file that uses .INB
```

---

## Step-by-Step Implementation Plan

### Phase 1: Setup and Investigation (RED)

```
Step 1.1: Read existing code
  - Action: Understand current ParseFile() implementation
  - Files: src/syntax/scmasm_syntax.cpp, scmasm_syntax.h
  - Tests: None yet
  - Verification: Clear understanding of parsing flow

Step 1.2: Write failing tests
  - Action: Add 5 test cases for .INB directive (all should fail initially)
  - Files: tests/unit/test_scmasm_syntax.cpp
  - Tests:
    * SimpleInclude - .INB with single symbol definition
    * SymbolUsage - Reference symbol from included file
    * NestedIncludes - .INB within included file
    * ForwardReference - JMP to label in included file
    * MissingFile - Error handling for nonexistent file
  - Verification: All 5 tests fail with "Undefined symbol" or similar

Step 1.3: Test with real A2osX file
  - Action: Try assembling A2osX.S.txt (should fail at line 77)
  - Files: ~/Projects/Vintage/Apple/A2osX/A2osX.S.txt
  - Tests: Manual assembly test
  - Verification: Fails with "Undefined symbol: MLI.GETPREFIX"
```

### Phase 2: Core Implementation (GREEN)

```
Step 2.1: Add include stack tracking
  - Action: Add include_stack_ member to track nested includes
  - Files: src/syntax/scmasm_syntax.h, scmasm_syntax.cpp
  - Tests: Nested include test will verify
  - Verification: Include depth checked before parsing

Step 2.2: Implement recursive ParseFile()
  - Action: Refactor ParseFile() to be callable recursively
  - Files: src/syntax/scmasm_syntax.cpp
  - Details:
    * Save current file context before recursion
    * Parse included file with same symbol table
    * Restore file context after recursion
    * Track line numbers per file for errors
  - Tests: SimpleInclude test should pass
  - Verification: Single-level include works

Step 2.3: Implement path resolution
  - Action: Resolve relative paths relative to current file directory
  - Files: src/syntax/scmasm_syntax.cpp
  - Details:
    * Get directory of current file
    * Resolve relative paths from that directory
    * Support absolute paths as-is
  - Tests: A2osX test (inc/mli.i from A2osX.S.txt)
  - Verification: Relative paths resolve correctly

Step 2.4: Implement .INB directive handler
  - Action: Complete the .INB directive handler
  - Files: src/syntax/scmasm_directive_handlers.cpp
  - Details:
    * Parse .INB directive argument (filename)
    * Resolve path
    * Check include depth limit (max 10)
    * Call ParseFile() recursively
    * Handle errors (file not found, too deep)
  - Tests: All .INB tests should pass
  - Verification: All 5 unit tests passing

Step 2.5: Test with A2osX
  - Action: Assemble A2osX.S.txt with fixed .INB
  - Files: ~/Projects/Vintage/Apple/A2osX/
  - Tests: Manual assembly
  - Verification: Assembles past line 77 (no "Undefined symbol: MLI.GETPREFIX")
```

### Phase 3: Error Handling and Edge Cases

```
Step 3.1: Add circular include detection
  - Action: Check include stack for duplicate files
  - Files: src/syntax/scmasm_syntax.cpp
  - Tests: Add CircularInclude test
  - Verification: Clear error message, no stack overflow

Step 3.2: Improve error messages
  - Action: Include stack trace in error messages
  - Files: src/syntax/scmasm_syntax.cpp
  - Details:
    * Show file:line for error location
    * Show include stack: "included from file:line"
  - Tests: All tests
  - Verification: Clear, helpful error messages

Step 3.3: Run full test suite
  - Action: Ensure no regressions in existing tests
  - Files: All tests
  - Tests: ctest --output-on-failure
  - Verification: All 1673+ tests passing
```

### Phase 4: Documentation and Cleanup (REFACTOR)

```
Step 4.1: Add code comments
  - Action: Document include mechanism
  - Files: src/syntax/scmasm_syntax.cpp
  - Verification: Clear explanation of recursion and state

Step 4.2: Update work log
  - Action: Document implementation details
  - Files: .ai/tasks/2026-02-16_fix-inb-directive/20-work-log.md
  - Verification: Complete record of work done

Step 4.3: Final verification
  - Action: Build, test, and assemble A2osX
  - Tests: Full test suite + A2osX assembly
  - Verification: All tests passing, A2osX progresses further
```

---

## Testing Strategy

### Unit Tests

```
□ SimpleInclude - Test cases:
  - Include file with single symbol definition
  - Reference that symbol in main file
  - Verify symbol resolved correctly

□ SymbolUsage - Test cases:
  - Include file with multiple symbols
  - Use symbols in expressions
  - Verify correct values

□ NestedIncludes - Test cases:
  - main.asm includes inc1.asm includes inc2.asm
  - Symbol defined in inc2.asm
  - Reference from main.asm
  - Verify 2-level nesting works

□ ForwardReference - Test cases:
  - JMP to label before .INB
  - Label defined in included file
  - Verify forward reference resolved

□ ErrorCases - Test cases:
  - Missing file: clear error message
  - Circular include: detected and reported
  - Include depth exceeded: clear limit message
```

### Integration Tests

```
□ A2osX inc/mli.i inclusion
  - Setup: A2osX.S.txt with .INB inc/mli.i
  - Execute: xasm++ A2osX.S.txt
  - Verify: MLI.GETPREFIX symbol defined and usable

□ Multiple includes in same file
  - Setup: File with multiple .INB directives
  - Execute: Assemble
  - Verify: All symbols from all includes available
```

### Acceptance Tests

```
□ A2osX assembly progresses past line 77
  - Given: A2osX.S.txt source files
  - When: Assemble with xasm++ --syntax scmasm
  - Then: No "Undefined symbol: MLI.GETPREFIX" error

□ Real-world compatibility
  - Given: S-C Macro Assembler source with includes
  - When: Assemble with xasm++
  - Then: Same behavior as original assembler
```

### Coverage Target

```
- Overall: 85%+
- Critical paths: 100% (include mechanism, error handling)
- New code: 95%+ (all new include logic tested)
```

---

## Dependencies and Prerequisites

### Technical Dependencies

```
□ C++20 compiler - Already in use
□ GoogleTest - Already in use for unit tests
□ CMake - Already in use for build
```

### Task Dependencies

```
□ xasm++-ijxa (inline comments) - ✅ Completed
□ xasm++-6isx (phase directives) - ✅ Completed
□ xasm++-786z (dots in symbols) - ✅ Completed
```

### Knowledge Dependencies

```
□ Understanding of scmasm_syntax.cpp parsing flow
□ Familiarity with S-C Macro Assembler include behavior
□ Access to A2osX source code for testing
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1: Infinite recursion from circular includes**
- **Probability:** Medium
- **Impact:** High (stack overflow, crash)
- **Mitigation:** Track include stack, enforce depth limit of 10
- **Contingency:** Clear error message helps user fix circular dependency

**Risk 2: Path resolution ambiguity**
- **Probability:** Medium
- **Impact:** Medium (wrong file included)
- **Mitigation:** Document path resolution clearly, test with real A2osX
- **Contingency:** Add --include-path option if needed (future enhancement)

**Risk 3: Line number tracking confusion in errors**
- **Probability:** Low
- **Impact:** Low (confusing error messages)
- **Mitigation:** Maintain file context stack with line numbers
- **Contingency:** Error messages show include stack trace

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. Git restore to last working commit
2. Verify all 1673 tests still passing
3. Document issue for investigation
```

**Rollback Verification:**
```
✓ ctest shows 1673/1673 tests passing
✓ xasm++ builds without warnings
✓ No .INB functionality (back to stub)
```

### Git Rollback

```
# If changes committed
git revert <commit-hash>

# If not committed
git restore src/syntax/
git restore tests/unit/

# Verify
cmake --build build
ctest --output-on-failure
```

---

## Performance Considerations

### Expected Performance Impact

```
- File I/O: Minimal impact (reading include files is necessary)
- Parsing: Linear increase with number of includes (acceptable)
- Memory: Small increase for include stack (max 10 levels × ~100 bytes)
```

### Performance Targets

```
- Include overhead: < 1ms per included file
- Memory overhead: < 1KB for include stack
- No impact on assembly without includes
```

---

## Security Considerations

### Security Checklist

```
□ Input validation: Check include depth limit
□ Path traversal prevention: Validate file paths
□ File access: Only read text files, no execute permissions
□ Error messages: Don't leak sensitive path information
□ Resource limits: Depth limit prevents resource exhaustion
```

### Security Review Points

```
- Path resolution must prevent ../../../etc/passwd attacks
- Include depth limit prevents stack overflow DOS
- File I/O errors handled gracefully
```

---

## Alternative Approaches Considered

### Alternative 1: Inline Expansion

**Pros:**
- Simpler implementation
- No recursion needed
- Easier to understand

**Cons:**
- Loses file context for error messages
- Line numbers become confusing
- Harder to track include stack for errors

**Why Not Chosen:**
Error message quality is important for developer experience. Recursive approach provides better context.

### Alternative 2: Preprocessor Pass

**Pros:**
- Complete separation of include resolution and parsing
- Could support conditional includes later

**Cons:**
- More complex architecture
- Two-pass overhead
- Overkill for current needs

**Why Not Chosen:**
Recursive parsing is simpler and sufficient for S-C Macro Assembler compatibility.

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Phase 1: 1 hour
- Step 1.1: 15 minutes (code reading)
- Step 1.2: 30 minutes (write failing tests)
- Step 1.3: 15 minutes (test A2osX)

Phase 2: 2 hours
- Step 2.1: 15 minutes (include stack)
- Step 2.2: 45 minutes (recursive ParseFile)
- Step 2.3: 30 minutes (path resolution)
- Step 2.4: 30 minutes (.INB handler)
- Step 2.5: 15 minutes (test A2osX)

Phase 3: 1 hour
- Step 3.1: 20 minutes (circular detection)
- Step 3.2: 20 minutes (error messages)
- Step 3.3: 20 minutes (full test suite)

Phase 4: 30 minutes
- Step 4.1: 10 minutes (comments)
- Step 4.2: 10 minutes (work log)
- Step 4.3: 10 minutes (final verification)

Total: 4.5 hours
```

---

## Success Metrics

### Completion Criteria

```
✓ All test steps completed
✓ All 1673+ tests passing
✓ Coverage ≥ 85%
✓ A2osX assembles past line 77
✓ MLI.GETPREFIX symbol defined correctly
✓ Documentation complete
```

### Quality Metrics

```
- Test coverage: 90%+ for new include code
- Performance: < 1ms per include file
- Code quality: No compiler warnings
- Security: Path traversal prevented
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [X] Author: Orchestrator (2026-02-16)
- [ ] Engineer: (Will approve when implementing)

**Revision History:**
```
v1.0 - 2026-02-16 - Initial plan
```

---

## Notes and Considerations

This is a critical fix for A2osX compatibility. The include mechanism is fundamental to S-C Macro Assembler workflow, as system constants and macros are typically defined in separate include files.

Testing strategy emphasizes:
1. TDD approach (RED-GREEN-REFACTOR)
2. Real-world validation with A2osX
3. Edge case coverage (circular includes, missing files)
4. Error message quality

The recursive approach is chosen for its clarity and error handling benefits, even though inline expansion would be simpler.

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
