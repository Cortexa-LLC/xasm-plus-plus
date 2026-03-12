# Implementation Plan

**Task:** Fix dots in symbol names
**Workflow:** Bugfix
**Role:** Engineer

---

## Investigation Phase

### Check Expression Parser

File: `src/core/expression_parser.cpp`

Look for identifier/symbol parsing:
- What characters are allowed?
- Is `.` already included?
- If not, add to allowed character set

### Check Symbol Table

File: `include/xasm++/core/symbol_table.h`

- Does symbol table allow dots in names?
- Any validation that might reject dots?

### Likely Fix Location

Expression parser probably uses regex or character class like:
```cpp
[A-Za-z_][A-Za-z0-9_]*  // Current (no dots)
[A-Za-z_][A-Za-z0-9_.]*  // Fixed (with dots)
```

## Implementation

If dots not supported, add to identifier character set in expression parser.

## Testing

```assembly
; Test dots in symbols
NAMESPACE.SYMBOL .EQ $1234
  lda NAMESPACE.SYMBOL
```

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
