# Task Contract

**Task ID:** 2026-02-16_fix-inb-paths
**Beads Task:** xasm++-vgyc
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix .INB directive to resolve include file paths relative to source file location.

### Current Issue

A2osX assembly fails at line 605:
```
Parse error: <source>:605: .INB cannot open file: usr/src/shared/x.printf.s 
(tried: usr/src/shared/x.printf.s)
```

**Root Cause:** .INB directive looks for include files relative to current working directory, not relative to the source file's location.

**Example:**
```
Source file: ~/Projects/Vintage/Apple/A2osX/A2osX.S.txt
Line 605:    .INB usr/src/shared/x.printf.s

Current behavior: Look for usr/src/shared/x.printf.s relative to CWD
Expected behavior: Look for ~/Projects/Vintage/Apple/A2osX/usr/src/shared/x.printf.s
```

### Background

This is a common assembler pattern:
- **Relative includes:** `.INB file.s` → Look relative to source file
- **Absolute includes:** `.INB /absolute/path.s` → Use absolute path
- **Current directory:** Already tried as fallback

Many assemblers (including Merlin) resolve includes relative to the source file location, allowing source trees to be relocatable.

### Tasks

1. **Investigate current .INB implementation:**
   - Find HandleInb() in scmasm_directive_handlers.cpp
   - Check how file paths are resolved
   - Identify where current_file path is stored

2. **Implement relative path resolution:**
   - Get source file directory from current_file
   - Resolve include path relative to source directory
   - Try absolute path if file not found
   - Try CWD as fallback

3. **Add tests:**
   - Include file relative to source
   - Include with subdirectory (usr/src/shared/)
   - Nested includes (included file includes another)

4. **Validate A2osX:**
   - Line 605 should now find usr/src/shared/x.printf.s
   - Continue assembly to next issue

### Success Criteria

```
✓ .INB resolves paths relative to source file directory
✓ A2osX line 605 assembles successfully
✓ Test for relative includes passes
✓ Nested includes work
✓ All existing tests pass (1,726/1,739)
✓ Zero warnings
```

**Estimated Effort:** 1-2 hours

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
