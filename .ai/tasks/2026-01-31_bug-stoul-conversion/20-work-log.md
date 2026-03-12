# Work Log

**Task ID:** 2026-01-31_bug-stoul-conversion
**Beads Task:** xasm++-8uc
**Started:** 2026-01-31
**Status:** In Progress

---

## Work Sessions

### Session 1: 2026-01-31 (TDD Investigation Phase)

#### Objectives for This Session
```
□ Locate all stoul usage in codebase
□ Reproduce error with MASTER.S
□ Identify exact location of failure
□ Create failing test case (RED phase)
```

#### Work Completed
```
✓ Located 12 stoul calls across 5 files
✓ Reviewed XASM-BUGS-REPORT for context
✓ Identified affected files: 14 POP modules
```

**Files Identified with stoul:**
- `src/core/parse_utils.cpp` - Line 59
- `src/utils/string_utils.cpp` - Line 37
- `src/syntax/simple_syntax.cpp` - Line 16
- `src/syntax/merlin/merlin_syntax.cpp` - Lines 83, 96, 104, 378, 389
- `src/syntax/scmasm/scmasm_syntax.cpp` - Lines 540, 561, 579

#### In Progress
```
○ Creating TDD RED phase test
```

#### Decisions Made
```
1. Decision: Add try-catch blocks around all stoul() calls
   Rationale: stoul() throws std::invalid_argument (a logic_error) when the string cannot be converted. Multiple places in code call stoul() without catching exceptions.

2. Decision: Focus on merlin_syntax.cpp ParseNumber first
   Rationale: This is the most likely culprit for POP assembly failures
```

#### Issues Encountered
```
Issue: stoul() throwing std::invalid_argument uncaught
- Attempted: Added better error messages to ParseNumber
- Found: Multiple places call stoul() without proper exception handling:
  - src/syntax/merlin/merlin_syntax.cpp (ParseNumber)
  - src/utils/string_utils.cpp (ParseHex)
  - src/syntax/simple_syntax.cpp (ParseHex)
  - src/syntax/scmasm/scmasm_syntax.cpp (ParseNumber)
- Root cause: stoul() receives malformed string (likely unresolved symbol)
- Resolution: Need to validate input before calling stoul(), or ensure expressions are fully resolved
```

#### Next Steps
```
□ Create failing test case (TDD RED)
□ Add validation before stoul calls
□ Ensure symbol resolution happens before numeric conversion
□ Fix all stoul calls to handle exceptions properly
```

---

## Overall Progress Summary

### Current Status
```
Phase: Investigation (TDD RED)
Progress: 10% complete
Next Milestone: Reproduce error and create failing test
```

