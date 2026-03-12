# Task Contract

**Task ID:** 2026-01-31_pop-disk-integration
**Beads Task:** xasm++-0fi
**Created:** 2026-01-31
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Standard

---

## Task Description

Create bootable 5.25" disk images for Prince of Persia using xasm++ binaries and test in Virtual ][ emulator.

### Background and Context

xasm++ has achieved 100% binary compatibility with vasm (10/10 files match byte-for-byte). The original user request was to "create the 5.25" disk images using our new assembler and then test with Virtual ][". Now that binary compatibility is achieved, we can proceed with disk creation.

### Current State

- ✅ All 29 POP modules assemble successfully with xasm++
- ✅ 10/10 critical modules match vasm byte-for-byte (100% compatibility)
- ✅ Adam Green's POP build system successfully builds with xasm++
- ⏸️ No disk images created yet
- ⏸️ Not tested in Virtual ][ emulator

### Desired State

- Bootable 5.25" disk images created using crackle tool
- Images boot in Virtual ][ emulator
- Prince of Persia game runs correctly
- Documented process for future builds

---

## Success Criteria

```
✓ Disk images created without errors
✓ Images boot in Virtual ][
✓ POP intro screen displays
✓ Game controls respond correctly
✓ Can navigate through levels
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Bootable disk images created (.dsk or .po format)
□ Virtual ][ emulator can mount images
□ POP boots and displays intro screen
□ Keyboard controls work (start game, movement)
□ Game logic functions correctly (collision, levels)
```

### Quality Requirements
```
□ Build process documented
□ No warnings or errors during disk creation
□ Images compatible with standard Apple II emulators
```

### Non-Functional Requirements
```
□ Disk creation process automated (Makefile)
□ Can reproduce builds consistently
```

---

## Constraints and Dependencies

### Constraints
```
□ Must use Adam Green's existing build system
□ Must work with Virtual ][ emulator (macOS)
□ Must use crackle for disk image creation
```

### Dependencies
```
✅ xasm++ binary compatibility complete (10/10 files)
✅ All 29 POP modules assemble
□ crackle tool available
□ Virtual ][ emulator installed
```

### Out of Scope
```
✗ Creating custom disk layouts
✗ Modifying POP game code
✗ Supporting other emulators (AppleWin, MAME)
✗ Performance optimization
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Number of files affected: 1-2 (Makefile, scripts)
- Lines of code estimate: ~20-50 (mostly Makefile targets)
- New concepts/patterns: No (using existing crackle tool)
- Integration complexity: Low (build system already exists)
- Risk level: Low (known working reference with vasm)

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 1-2 files (Makefile modifications, maybe helper script)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE
├─ 15-26 files → ❌ TOO LARGE
└─ 27+ files   → ❌ CRITICAL

Your Task: 1-2 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
2 files × 3,000 tokens = 6,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
└─ Your Task: 6,000 tokens → ✅ SAFE
```

### Work In Progress Planning

**Concurrent Execution Assessment:**
```
Planned WIP: 1 agent (single Engineer)

Status: ✅ IDEAL
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [x] YES - Batch size 1-2 files AND token budget ~6K AND WIP 1
- [ ] NO - MUST decompose

---

## Resources and References

### Relevant Files
```
- ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/Makefile - Build system
- ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/README.md - Build instructions
- /Users/bryanw/Projects/Vintage/tools/xasm++/build/xasm++ - Assembler binary
```

### Documentation
```
- Adam Green's POP repository: https://github.com/adamgreen/Prince-of-Persia-Apple-II
- crackle tool: https://github.com/datajerk/crackle
- Virtual ][ emulator: http://www.virtualii.com/
```

### Examples
```
- POP Makefile already has `make disks` target (uses vasm)
- Just need to adapt to use xasm++
```

---

## Assumptions

```
1. crackle tool is installed and in PATH
2. Virtual ][ emulator is installed on macOS
3. Adam Green's build system Makefile has disk creation targets
4. Disk format is standard Apple II (.dsk or .po)
5. Binary compatibility means disks will work identically to vasm build
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. crackle not installed or broken
   - Probability: Low
   - Impact: Medium (blocks disk creation)
   - Mitigation: Check installation, provide install instructions

2. Disk creation fails with xasm++ binaries
   - Probability: Very Low (100% binary compatibility)
   - Impact: High
   - Mitigation: Compare xasm++ vs vasm binaries during creation

3. Virtual ][ emulator issues
   - Probability: Low
   - Impact: Medium (can't verify boot)
   - Mitigation: Try alternative emulator (AppleWin on Wine)
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: User (implicit via original request)
- [x] Agent: Orchestrator (2026-01-31)

---

## Notes

This is the final step to complete the original user request. With 100% binary compatibility achieved, creating bootable disks should be straightforward using the existing build infrastructure.

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-31
