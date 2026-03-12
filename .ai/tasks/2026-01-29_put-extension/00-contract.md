# Task Contract

**Task ID:** 2026-01-29_put-extension
**Beads Task:** xasm++-0y0
**Created:** 2026-01-29
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature (.ai-pack/workflows/feature.md)

---

## Task Description

Implement PUT extension auto-append for Prince of Persia compatibility.

### Background and Context

Prince of Persia source files use PUT directives without file extensions:
```assembly
PUT SUBS      ; Should load SUBS.S
PUT GAMEEQ    ; Should load GAMEEQ.S
```

This is a Merlin assembler convention. Currently xasm++ requires explicit `.S` extension, blocking 16/29 PoP files.

### Current State

- PUT directive requires explicit extension: `PUT SUBS.S` ✓
- PUT without extension fails: `PUT SUBS` ❌
- 16 PoP files blocked (63 PUT occurrences)

### Desired State

- PUT auto-appends `.S` if no extension: `PUT SUBS` → loads `SUBS.S` ✓
- Explicit extension still works: `PUT SUBS.S` → loads `SUBS.S` ✓
- 16 PoP files unblocked

---

## Success Criteria

```
✓ PUT without extension works (auto-appends .S)
✓ PUT with extension still works (no change)
✓ All existing tests passing (574/575)
✓ New PUT extension tests added
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ PUT SUBS → loads SUBS.S
□ PUT GAMEEQ → loads GAMEEQ.S
□ PUT SUBS.S → loads SUBS.S (no change)
□ PUT ../DIR/FILE → loads ../DIR/FILE.S
□ PUT /abs/path/FILE → loads /abs/path/FILE.S
□ Error handling preserved (file not found, etc.)
```

### Quality Requirements
```
□ All tests passing (574/575 + new tests)
□ Zero compiler warnings
□ Code review approved
□ TDD process followed
```

---

## Implementation Approach

**Location:** `src/syntax/merlin/merlin_syntax.cpp` - `handle_PUT()` function

**Current Logic:**
```cpp
void MerlinSyntaxParser::handle_PUT(const std::string& filename) {
    // Opens filename as-is
}
```

**New Logic:**
```cpp
void MerlinSyntaxParser::handle_PUT(const std::string& filename) {
    std::string resolved = filename;

    // If no extension, append .S
    if (filename.find('.') == std::string::npos) {
        resolved = filename + ".S";
    }

    // Continue with existing logic
}
```

**Test Cases:**
1. `PUT SUBS` → loads `SUBS.S`
2. `PUT SUBS.S` → loads `SUBS.S` (no change)
3. `PUT DIR/FILE` → loads `DIR/FILE.S`
4. Error handling preserved

---

## References

**Prince of Persia Analysis:**
- `docs/investigations/pop-compatibility-analysis.md`
- 16 files affected: MASTER.S, SUBS.S, CTRL.S, etc.
- 63 PUT occurrences without extensions

**Related Code:**
- `src/syntax/merlin/merlin_syntax.cpp` - handle_PUT()
- `tests/unit/test_merlin_syntax.cpp` - PUT tests

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-29
