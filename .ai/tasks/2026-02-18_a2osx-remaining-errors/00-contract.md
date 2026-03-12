# Task Contract: Investigate Remaining A2osX Errors

**Task ID:** 2026-02-18_a2osx-remaining-errors
**Beads Task:** xasm++-yywb
**Created:** 2026-02-18
**Assigned Role:** Spelunker
**Workflow:** Research
**Priority:** P1

---

## Context

After fixing CRLF, editor commands, and macros, A2osX assembly reduced from 788 to 504 errors (36% reduction).

---

## Remaining Errors (504 total)

### 1. Local Labels `:1` (16 errors)
```
error: Invalid argument for :1: Unsupported instruction: :1
```

**Questions:**
- Is `:1` SCMASM local label syntax?
- How should local labels work in macros?
- Where are these `:1` labels used?

### 2. API Constants (488 errors)
```
error: Invalid argument for _WRITE: Unsupported instruction: _WRITE
error: Invalid argument for _STRLEN: Unsupported instruction: _STRLEN
error: Invalid argument for _STRCMP: Unsupported instruction: _STRCMP
(~120 different constants)
```

**Questions:**
- Where are these defined? (likely .EQ directives)
- What should they resolve to?
- Are they syscall numbers or API indices?

### 3. Path References (4 errors)
```
error: Invalid argument for USR/SRC/A2OSX.S: Unsupported instruction: USR/SRC/A2OSX.S
```

**Questions:**
- Is this from a SAVE/ASM directive?
- Should these be stripped like editor commands?

---

## Investigation Tasks

```
□ Run A2osX build and capture error samples
□ Search A2osX source for :1 label definitions
□ Search for _WRITE, _STRLEN definitions (.EQ?)
□ Check if API constants are in included files
□ Determine SCMASM local label syntax rules
□ Document findings and fix recommendations
```

---

## Deliverables

1. **Root cause analysis** for each error type
2. **Implementation recommendations** for Engineer
3. **Test cases** for each fix
4. **Priority/complexity** estimates

---

## Test Command

```bash
cd ~/Projects/Vintage/Apple/A2osX
XASM=~/Projects/Vintage/tools/xasm++/build/bin/xasm++ make clean && make
```

---

**Previous Work:** xasm++-jgx2 (macro investigation)
**Next Step:** Engineer implementation based on findings
