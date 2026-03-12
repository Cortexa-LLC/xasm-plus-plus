# Contract: Fix Magic Strings in SCMASM Code

**Beads Task:** xasm++-ho0z
**Created:** 2026-02-13
**Owner:** Orchestrator → Engineer
**Priority:** P0 (blocks commit)
**Status:** ✅ COMPLETE

---

## Executive Summary

Fix identified magic strings and numbers in SCMASM implementation:
1. Directive name string literals in comparisons
2. ASCII escape sequence hex values

---

## Specific Issues to Fix

### Issue 1: Magic Directive Name Strings ✅

**File:** `src/syntax/scmasm_syntax.cpp`

**Problem:**
```cpp
if (opcode_upper == ".DO") {
  HandleDo(...);
} else if (opcode_upper == ".LU") {
  HandleLu(...);
} else if (opcode_upper == ".ELSE" || opcode_upper == ".FIN" ||
             opcode_upper == ".ENDU") {
```

**Fix Applied:** ✅
```cpp
using namespace scmasm::directives;
if (opcode_upper == DO) {
  HandleDo(...);
} else if (opcode_upper == LU) {
  HandleLu(...);
} else if (opcode_upper == ELSE ||
           opcode_upper == FIN ||
           opcode_upper == ENDU) {
```

### Issue 2: Magic Escape Sequence Values ✅

**File:** `src/syntax/scmasm_directive_handlers.cpp` (ParseEscapeSequence)

**Problem:**
```cpp
case 'a': s++; return 0x07; // Bell
case 'b': s++; return 0x08; // Backspace
case 'e': s++; return 0x1B; // Escape
case 'f': s++; return 0x0C; // Form feed
case 'n': s++; return 0x0A; // Newline
case 'r': s++; return 0x0D; // Carriage return
case 't': s++; return 0x09; // Tab
case 'v': s++; return 0x0B; // Vertical tab
case '0': s++; return 0x00; // Null
```

**Fix Applied:** ✅
```cpp
// In scmasm_constants.h:
namespace constants {
namespace ascii {
  constexpr uint8_t NULL_CHAR = 0x00;
  constexpr uint8_t BELL = 0x07;
  constexpr uint8_t BACKSPACE = 0x08;
  constexpr uint8_t TAB = 0x09;
  constexpr uint8_t NEWLINE = 0x0A;
  constexpr uint8_t VTAB = 0x0B;
  constexpr uint8_t FORMFEED = 0x0C;
  constexpr uint8_t CR = 0x0D;
  constexpr uint8_t ESCAPE = 0x1B;
}
}

// In handler:
case 'a': s++; return constants::ascii::BELL;
case 'b': s++; return constants::ascii::BACKSPACE;
case 'e': s++; return constants::ascii::ESCAPE;
// etc.
```

---

## Acceptance Criteria

1. [x] All directive name strings replaced with constants
2. [x] All escape sequence hex values replaced with named constants
3. [x] Constants added to appropriate headers
4. [x] All tests still passing (1564 tests)
5. [x] Zero new compiler warnings

---

## Implementation Summary

### Files Modified
1. `include/xasm++/directives/scmasm_directive_constants.h` - Added .LU, .ELSE, .FIN, .ENDU
2. `include/xasm++/directives/scmasm_constants.h` - Added ascii escape constants
3. `src/syntax/scmasm_directive_handlers.cpp` - Replaced 9 escape sequence hex values
4. `src/syntax/scmasm_syntax.cpp` - Replaced 5 directive name strings

### Test Results
- ✅ All 1564 tests pass
- ✅ Zero compilation errors
- ✅ Zero new warnings

---

## Estimated Effort

**Planned:** 30-60 minutes  
**Actual:** ~45 minutes

---

**Completed:** 2026-02-13  
**Engineer:** AI Engineer  
**Status:** Ready for review
