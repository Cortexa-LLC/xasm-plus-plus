# Task Contract

**Task ID:** 2026-01-29_file-io
**Beads Task:** xasm++-8hi
**Created:** 2026-01-29
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature (.ai-pack/workflows/feature.md)

---

## Task Description

Implement Phase 3.2: File I/O layer for xasm++ assembler - source file reading and binary output writing.

### Background and Context

Phase 3.1 (CLI layer) is complete, providing command-line argument parsing. However, xasm++ still cannot read source files from disk or write assembled binaries. The assembler currently only works with in-memory strings in unit tests.

Phase 3.2 adds file I/O capabilities to bridge CLI arguments with the assembler engine, enabling real-world usage.

### Current State

- CLI layer complete (xasm++ --help works) ✓
- Assembler engine works with in-memory strings ✓
- Cannot read source files from disk ❌
- Cannot write binary output to files ❌
- File paths parsed but not used ❌

### Desired State

After this task:
- Read `.s` source files from disk
- Parse source into lines for assembler
- Write assembled binary to output file
- Handle file I/O errors gracefully
- Support relative and absolute paths

---

## Success Criteria

```
✓ Can read .s source files from disk
✓ Can write .bin binary output to disk
✓ File I/O errors handled with clear messages
✓ Path resolution works (relative & absolute)
✓ All existing tests passing (518/519)
✓ New file I/O tests added and passing
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Read source file: xasm++ input.s -o output.bin
□ Write binary output to specified file
□ Handle missing source file (error message)
□ Handle unreadable file (permissions error)
□ Handle write errors (disk full, permissions)
□ Support relative paths: xasm++ ../src/code.s
□ Support absolute paths: xasm++ /full/path/code.s
□ Binary output matches in-memory assembly results
□ File encoding: UTF-8 with fallback to ASCII
□ Line endings: Handle \n, \r\n, \r
```

### Quality Requirements
```
□ All tests passing (518/519 + new file I/O tests)
□ Code coverage ≥ 85%
□ No linting errors
□ Zero compiler warnings
□ Code review approved (Reviewer role)
□ Integration with CLI layer verified
```

### Non-Functional Requirements
```
□ Error messages actionable (file not found, permission denied, etc.)
□ Performance: File I/O not a bottleneck (<10ms for small files)
□ Cross-platform (macOS, Linux, Windows)
□ Memory efficient (stream large files if needed)
```

---

## Constraints and Dependencies

### Constraints
```
□ Must use std::filesystem (C++17) for path handling
□ Must maintain backward compatibility with in-memory tests
□ Must follow existing code style (STYLE_GUIDE.md)
□ Binary output format matches BinaryOutputPlugin
```

### Dependencies
```
□ Phase 3.1 (CLI layer) complete ✓ (xasm++-oys closed)
□ BinaryOutputPlugin exists (binary output format)
□ Assembler engine works with std::string input
```

### Beads Dependencies
```
✓ Unblocked by xasm++-oys (CLI layer complete)
✓ Blocks xasm++-c5o (Output formats - listing, symbols)
```

### Out of Scope
```
✗ Listing file generation (Phase 3.3)
✗ Symbol table generation (Phase 3.3)
✗ Multiple input file assembly (future phase)
✗ Include file handling (already implemented via PUT directive)
✗ Configuration file support (future phase)
✗ Assembly caching (future optimization)
```

---

## Stakeholders

**Primary:**
- Bryan Woodruff (Project owner)

**Secondary:**
- Future users of xasm++ tool
- Prince of Persia compatibility (depends on file I/O)

---

## Technical Approach

### Design Decisions

**File Operations:**
- Use `std::ifstream` for reading source files
- Use `std::ofstream` for writing binary output
- Use `std::filesystem::path` for path manipulation
- Stream reading for memory efficiency

**Error Handling:**
- Check file existence before opening
- Check read/write permissions
- Provide actionable error messages with file paths
- Return non-zero exit code on errors

**Integration Points:**
1. **CLI → File I/O:** CLI provides input/output file paths
2. **File I/O → Assembler:** Source text passed to assembler engine
3. **Assembler → File I/O:** Binary data written to output file
4. **File I/O → Output Plugin:** Binary output format from BinaryOutputPlugin

### Implementation Plan

1. **Create FileReader class**
   - `ReadSourceFile(const std::filesystem::path& path) -> std::string`
   - Handle UTF-8/ASCII encoding
   - Normalize line endings
   - Error handling for missing/unreadable files

2. **Create FileWriter class**
   - `WriteBinary(const std::filesystem::path& path, const std::vector<uint8_t>& data) -> bool`
   - Handle write errors (permissions, disk full)
   - Ensure output directory exists

3. **Update main.cpp**
   - Use FileReader to load source from CLI input path
   - Pass source text to assembler
   - Use FileWriter to save binary output to CLI output path
   - Handle errors and print to stderr

4. **Add file I/O tests**
   - Test reading existing files
   - Test error handling (missing file, bad permissions)
   - Test writing binary output
   - Test path resolution (relative, absolute)
   - Integration test: read source → assemble → write binary

---

## Test Strategy

### Test Coverage

**Unit Tests:**
- FileReader with valid files
- FileReader error cases (missing, unreadable)
- FileWriter with valid paths
- FileWriter error cases (bad permissions, invalid paths)
- Path resolution (relative, absolute, normalized)

**Integration Tests:**
- End-to-end: Read source file → Assemble → Write binary
- Verify binary output matches expected
- Test with Prince of Persia source files (integration)

**Edge Cases:**
- Empty source files
- Very large source files (>1MB)
- Files with different line endings (\n, \r\n, \r)
- Files with non-ASCII characters
- Output to non-existent directory
- Output to read-only directory

### Expected Test Count
- Existing: 518/519 passing
- New file I/O tests: ~15-20 tests
- Total: ~533-538 tests

---

## Documentation Requirements

```
□ Update README.md with file I/O examples
□ Document error messages and troubleshooting
□ Code comments in FileReader/FileWriter
□ Doxygen comments for public APIs
□ Integration example: xasm++ input.s -o output.bin
```

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| Cross-platform path issues | Medium | 2-3 hours | Use std::filesystem; test on multiple platforms |
| File encoding issues | Low | 1-2 hours | Handle UTF-8 with ASCII fallback |
| Permission errors not caught | Low | 1-2 hours | Test error paths thoroughly |
| Large file performance | Very Low | 1-2 hours | Stream reading; benchmark if needed |

---

## Definition of Done

Task is complete when:

```
✓ All acceptance criteria met
✓ All tests passing (518+ tests)
✓ Integration test: xasm++ can assemble real .s files to .bin
✓ Code review approved by Reviewer
✓ Documentation complete
✓ Zero compiler warnings
✓ Committed to main branch
✓ Beads task closed (bd close xasm++-8hi)
```

---

## References

**Standards:**
- `.ai-pack/quality/engineering-standards.md` - Code quality standards
- `docs/STYLE_GUIDE.md` - Project style guide
- `.ai-pack/workflows/feature.md` - Feature workflow

**Architecture:**
- `docs/architecture/00-overview.md` - System architecture
- `docs/planning/project-plan.md` - Phase 3 details

**Related Tasks:**
- Beads xasm++-oys - CLI Layer (complete) ✓
- Beads xasm++-c5o - Output Formats (blocked by this task)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-29
