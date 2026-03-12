# Task Contract

**Task ID:** 2026-01-29_cli-layer
**Beads Task:** xasm++-oys
**Created:** 2026-01-29
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature (.ai-pack/workflows/feature.md)

---

## Task Description

Implement Phase 3.1: Command-line interface for xasm++ assembler using CLI11 library.

### Background and Context

xasm++ has completed Phase 2 (6502 CPU + Merlin syntax) with excellent code quality (A- grade, 518/519 tests passing). However, the assembler currently lacks a CLI interface, preventing real-world usage. Users cannot invoke the assembler from the command line to assemble source files.

Phase 3 (CLI + File I/O) is required to create a fully functional tool. This task implements the first component: command-line argument parsing and validation.

### Current State

- Core assembler engine works (500+ tests passing)
- CLI11 library available but not integrated
- main.cpp exists but has no argument parsing
- Users cannot invoke: `xasm++ input.s -o output.bin`

### Desired State

After this task:
- Users can invoke `xasm++ --help` for usage information
- Users can invoke `xasm++ --version` for version info
- CLI parses arguments: `--cpu`, `--syntax`, `-o`, `--list`, `--symbols`
- Input file validation with clear error messages
- Graceful handling of invalid arguments

---

## Success Criteria

```
✓ xasm++ --help displays usage information
✓ xasm++ --version displays version string
✓ CLI parses all required arguments correctly
✓ Invalid arguments produce clear error messages
✓ All existing tests passing (518/519)
✓ New CLI tests added and passing
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Help system works: xasm++ --help
□ Version info works: xasm++ --version
□ Parse CPU type: --cpu 6502|65c02|65816
□ Parse syntax: --syntax merlin|simple
□ Parse output file: -o <file> or --output <file>
□ Parse listing file: --list <file>
□ Parse symbols file: --symbols <file>
□ Input file required (positional argument)
□ Multiple input files supported
□ Validate file exists before processing
□ Graceful error for missing/invalid arguments
```

### Quality Requirements
```
□ All tests passing (518/519 + new CLI tests)
□ Code coverage ≥ 85%
□ No linting errors
□ Zero compiler warnings
□ Code review approved (Reviewer role)
□ Documentation complete (CLI usage in README)
```

### Non-Functional Requirements
```
□ Error messages actionable and clear
□ Help text follows standard conventions
□ Performance: CLI parsing < 10ms
□ Cross-platform (macOS, Linux, Windows)
```

---

## Constraints and Dependencies

### Constraints
```
□ Must use CLI11 library (already in dependencies)
□ Must maintain backward compatibility with existing tests
□ Must follow existing code style (STYLE_GUIDE.md)
□ Must not break Phase 2 functionality
```

### Dependencies
```
□ CLI11 library (already available via vcpkg/CMake)
□ Phase 2 complete (6502 CPU + Merlin syntax) ✓
□ File I/O layer (Phase 3.2) will use CLI arguments
□ Output plugins (Phase 3.3) will use CLI arguments
```

### Beads Dependencies
```
□ No Beads blockers (this is first Phase 3 task)
✓ Blocks xasm++-8hi (File I/O layer)
✓ Indirectly blocks xasm++-c5o (Output formats)
```

### Out of Scope
```
✗ File reading/writing (Phase 3.2)
✗ Binary output generation (Phase 3.3)
✗ Listing format implementation (Phase 3.3)
✗ Symbol table generation (Phase 3.3)
✗ Configuration file support (future phase)
✗ Environment variable support (future phase)
```

---

## Stakeholders

**Primary:**
- Bryan Woodruff (Project owner)

**Secondary:**
- Future users of xasm++ tool
- Prince of Persia compatibility (depends on complete Phase 3)

---

## Technical Approach

### Design Decisions

**CLI Library:** CLI11
- Mature, header-only C++ library
- Modern C++ style (C++17+)
- Excellent error handling
- Subcommand support (for future)

**Argument Structure:**
```bash
xasm++ [OPTIONS] <input-file> [<input-file>...]

Options:
  --cpu <type>      CPU type: 6502, 65c02, 65816 (default: 6502)
  --syntax <type>   Syntax: merlin, simple (default: merlin)
  -o, --output <file>   Output binary file
  --list <file>     Generate listing file
  --symbols <file>  Generate symbol table file
  -h, --help        Display help
  -v, --version     Display version
```

### Implementation Plan

1. **Update main.cpp with CLI11 integration**
   - Include CLI11.hpp
   - Define CLI app and options
   - Parse arguments
   - Validate input files

2. **Create CLIOptions struct**
   - Holds parsed arguments
   - cpu_type, syntax_type, input_files, output_file, etc.

3. **Add version and help handlers**
   - Version string from CMakeLists.txt
   - Help text with examples

4. **Add error handling**
   - File not found
   - Invalid CPU type
   - Invalid syntax type
   - Missing required arguments

5. **Add CLI tests**
   - test_cli.cpp already exists
   - Add tests for all argument combinations
   - Test error cases

---

## Test Strategy

### Test Coverage

**Unit Tests:**
- Argument parsing correctness
- Validation logic
- Error handling paths

**Integration Tests:**
- End-to-end CLI invocation
- Error message formatting
- Help/version output

**Edge Cases:**
- Missing input file
- Invalid CPU type
- Non-existent file paths
- Multiple input files

### Expected Test Count
- Existing: 518/519 passing
- New CLI tests: ~10-15 tests
- Total: ~528-533 tests

---

## Documentation Requirements

```
□ Update README.md with CLI usage section
□ Add examples: xasm++ input.s -o output.bin
□ Document all command-line options
□ Provide error troubleshooting guide
□ Code comments in main.cpp
□ Doxygen comments for CLIOptions struct
```

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| CLI11 integration issues | Low | 1 day | Library well-documented; test incrementally |
| Breaking existing tests | Low | 1 day | Maintain backward compatibility; test frequently |
| Cross-platform path issues | Medium | 2-3 hours | Use std::filesystem; test on multiple platforms |
| Error message clarity | Low | 1-2 hours | Review with stakeholder; iterate on feedback |

---

## Definition of Done

Task is complete when:

```
✓ All acceptance criteria met
✓ All tests passing (518+ tests)
✓ Code review approved by Reviewer
✓ Documentation complete
✓ Zero compiler warnings
✓ Committed to main branch
✓ Beads task closed (bd close xasm++-oys)
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
- Beads xasm++-8hi - File I/O Layer (blocked by this task)
- Beads xasm++-c5o - Output Formats (blocked by xasm++-8hi)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-29
