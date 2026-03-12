# Task Contract

**Task ID:** 2026-01-30_error-messages
**Created:** 2026-01-30
**Requestor:** Project Team
**Assigned Role:** Engineer
**Workflow:** Feature
**Beads Task:** xasm++-6w9

---

## Task Description

Enhance the xasm++ assembler's error diagnostic system to provide context-rich, actionable error messages similar to Rust compiler diagnostics.

### Background and Context

Modern compilers like Rust's rustc have set a high bar for error message quality, providing:
- Source code context with line numbers and visual markers
- Suggestions for fixing common mistakes (e.g., "did you mean X?")
- Color-coded output for better readability
- Multi-line explanations when needed

The xasm++ assembler currently provides basic error messages without this level of detail, making it harder for users to debug their assembly code.

### Current State

The current error system (`include/xasm++/core/assembler_error.h`) provides:
- Basic `AssemblerError` struct with `message` and `location` (file, line, column)
- Errors collected in `AssemblerResult.errors` vector
- Plain text output without colors or context
- No suggestions or helpful hints
- No source code snippets in error output

### Desired State

After completion, error messages should provide:
- **Source context:** Show the offending line with line number and column marker
- **Visual indicators:** Use colors for error severity (red for errors, yellow for warnings)
- **Helpful suggestions:** Provide "did you mean?" suggestions for common mistakes
- **Clear formatting:** Multi-line output with proper indentation
- **Actionable information:** Users can immediately understand and fix the issue

Example target output:
```
error: undefined symbol 'PLAYER_X'
    --> PLAYER.S:42:10
       |
    42 |     LDA PLAYER_X
       |         ^^^^^^^^ not defined
       |
    help: did you mean 'PLAYER_Y'?
```

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ Error messages include source file, line, and column information
✓ Source code context displayed with line numbers
✓ Visual markers (^^^^^) point to the exact location of errors
✓ Color-coded output (errors in red, warnings in yellow, hints in cyan)
✓ "Did you mean?" suggestions for undefined symbols
✓ All existing tests still passing (704/704)
✓ New tests added for error formatting (minimum 10 test cases)
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements
```
□ Enhanced AssemblerError structure with source context
□ Error formatter that generates Rust-style output
□ Color support via ANSI escape codes (with --no-color flag)
□ Symbol suggestion system for "did you mean?" feature
□ Multi-line error output with proper formatting
□ Integration with existing error reporting in AssemblerCore
```

### Quality Requirements
```
□ All tests passing (704/704 minimum)
□ Code coverage ≥ 85% for new error formatting code
□ No linting errors or compiler warnings
□ Code review approved
□ Documentation complete (doc comments for new APIs)
```

### Non-Functional Requirements
```
□ Performance acceptable (error formatting <10ms per error)
□ Terminal compatibility (works on ANSI-compatible terminals)
□ Error messages are clear and actionable
□ Color output respects NO_COLOR environment variable
```

---

## Constraints and Dependencies

### Constraints
```
□ Must maintain backward compatibility with existing error API
□ Must not break existing error handling in AssemblerCore
□ Colors must be optional (support --no-color flag)
□ Must work on both macOS and Linux terminals
```

### Dependencies
```
□ Requires reading source files to display context lines
□ May need external library for terminal color support (or ANSI codes)
□ Depends on existing AssemblerError and AssemblerResult structures
```

### Out of Scope
```
✗ Error recovery (continuing after errors)
✗ IDE integration (LSP protocol)
✗ Error message localization (i18n)
✗ Interactive error fixing
✗ Web-based error viewer
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: ~5 files
  - `include/xasm++/core/assembler_error.h` (enhanced structure)
  - `src/core/error_formatter.cpp` (new file for formatting)
  - `include/xasm++/core/error_formatter.h` (new file)
  - `src/main.cpp` (integration with CLI output)
  - `tests/unit/test_error_formatter.cpp` (new tests)
- Lines of code estimate: ~400-600 lines
- New concepts/patterns: Error formatting, terminal colors
- Integration complexity: Medium (integrate with existing error system)
- Risk level: Low (isolated changes, well-defined requirements)

---

## Lean Flow Analysis (MANDATORY)

**Purpose:** Prevent token limit failures and verification chaos

### Batch Size Assessment

**Estimated Files:** 5 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 5 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
5 × 3,000 tokens = 15,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 15,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose

**Justification:**
- 5 files is well within limits
- Estimated 15K tokens is safe
- Single agent execution
- High cohesion (all error formatting related)
- No parallelization needed

---

## Resources and References

### Relevant Files
```
- include/xasm++/core/assembler_error.h - Current error structure
- src/core/assembler_core.cpp - Error reporting logic
- src/main.cpp - CLI output
- tests/unit/test_merlin_syntax.cpp - Example test patterns
```

### Documentation
```
- Rust Compiler Error Index: https://doc.rust-lang.org/error-index.html
- ANSI Color Codes: https://en.wikipedia.org/wiki/ANSI_escape_code
```

### Examples
```
- Rust rustc error format
- Clang compiler diagnostics
- GCC -fdiagnostics-color output
```

---

## Assumptions

```
1. Users have ANSI-compatible terminals (macOS Terminal, iTerm2, Linux terminals)
2. Source files are available for reading to display context
3. Symbol table is accessible for "did you mean?" suggestions
4. Existing error locations (file, line, column) are accurate
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Terminal compatibility issues
   - Probability: Low
   - Impact: Medium
   - Mitigation: Provide --no-color flag, use standard ANSI codes

2. Performance impact of reading source files for context
   - Probability: Low
   - Impact: Low
   - Mitigation: Cache source lines, only read on error

3. Symbol suggestion algorithm false positives
   - Probability: Medium
   - Impact: Low
   - Mitigation: Use Levenshtein distance threshold, limit suggestions
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Project Team [2026-01-30]
- [ ] Agent: Engineer [Pending]

**Changes to Contract:**
[None yet]

---

## Notes

This feature will significantly improve the developer experience for xasm++ users by providing clear, actionable error messages. The Rust compiler is an excellent model to follow.

Priority is P3 (high) as better diagnostics directly improve user productivity and reduce support burden.

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-31
