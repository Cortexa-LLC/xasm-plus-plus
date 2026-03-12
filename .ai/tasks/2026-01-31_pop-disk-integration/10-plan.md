# Implementation Plan

**Task ID:** 2026-01-31_pop-disk-integration
**Created:** 2026-01-31
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Leverage Adam Green's existing POP build system to create bootable disk images using xasm++ assembled binaries. The Makefile already has disk creation targets that use crackle - we just need to verify they work with xasm++ binaries.

The approach is simple: check prerequisites (crackle installed, Virtual ][ available), run the disk creation process, and verify the disks boot in the emulator. Since we have 100% binary compatibility (10/10 files match vasm), the disks should work identically to the vasm reference build.

**Key Technical Decisions:**
- Use existing Makefile targets (no reinvention)
- Rely on crackle for disk image creation (proven tool)
- Test in Virtual ][ emulator (native macOS)

**Patterns to Use:**
- Verification-first approach - Check prerequisites before attempting
- Comparison testing - Compare xasm++ disks to vasm reference

---

## Critical Files Identified

### Files to Read (for context)
```
- ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/Makefile
  - Purpose: Check existing disk creation targets
  - Relevance: May already have `make disks` target

- ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/README.md
  - Purpose: Build instructions and prerequisites
  - Relevance: Documents disk creation process
```

### Files to Potentially Modify
```
1. ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/Makefile (possibly)
   - Current: May have vasm-specific targets
   - Changes: May need ASM variable support for xasm++
```

### No Files to Create
This task uses existing infrastructure.

---

## Step-by-Step Implementation Plan

### Phase 1: Prerequisites Verification
```
Step 1.1: Check crackle installation
  - Action: Verify crackle is in PATH and working
  - Command: which crackle && crackle --help
  - Verification: crackle tool available

Step 1.2: Check Virtual ][ installation
  - Action: Verify Virtual ][ app exists
  - Path: /Applications/Virtual II.app
  - Verification: App can launch

Step 1.3: Review POP Makefile
  - Action: Check for disk creation targets
  - Files: ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/Makefile
  - Verification: Understand disk build process
```

### Phase 2: Disk Creation
```
Step 2.1: Build binaries with xasm++
  - Action: make ASM=<path-to-xasm++> binaries
  - Files: All 29 POP modules
  - Verification: All binaries created, 10/10 match vasm

Step 2.2: Create disk images
  - Action: Run disk creation target (make disks or similar)
  - Files: Output .dsk or .po files
  - Verification: No errors, disk images exist

Step 2.3: Compare disk sizes
  - Action: Compare xasm++ disks to vasm reference
  - Command: ls -lh *.dsk
  - Verification: Sizes match or explain differences
```

### Phase 3: Emulator Testing
```
Step 3.1: Mount disk 1 in Virtual ][
  - Action: Open Virtual ][ and mount first disk
  - Files: Game disk 1
  - Verification: Disk mounts without errors

Step 3.2: Boot and test intro
  - Action: Boot disk and observe intro screen
  - Expected: POP intro plays, title screen appears
  - Verification: Visual confirmation

Step 3.3: Test gameplay
  - Action: Start game, test basic controls
  - Expected: Can move character, interact with environment
  - Verification: Keyboard input works, game logic functions

Step 3.4: Test disk swapping (if multi-disk)
  - Action: If game requires disk 2, swap disks
  - Expected: Game prompts for disk change, continues
  - Verification: Multi-disk support works
```

### Phase 4: Documentation
```
Step 4.1: Document disk creation process
  - Action: Update work log with commands and results
  - Files: 20-work-log.md
  - Verification: Repeatable process documented

Step 4.2: Create acceptance report
  - Action: Fill out 40-acceptance.md
  - Verification: All criteria met and signed off
```

---

## Testing Strategy

### Functional Tests
```
□ Disk Creation:
  - Disks build without errors
  - File sizes reasonable (140KB for 5.25" disks)
  - Checksums match vasm reference (optional)

□ Boot Test:
  - Virtual ][ mounts disk without errors
  - POP boot screen appears
  - No disk read errors

□ Gameplay Test:
  - Title screen displays correctly
  - Can start new game
  - Character movement works (arrow keys, shift)
  - Basic game mechanics work (jumping, guards, doors)
  - Can navigate at least 2-3 levels
```

### Comparison Tests
```
□ Binary Comparison:
  - All 29 binaries match or justify differences
  - 10/10 critical files match vasm (already verified)

□ Disk Comparison:
  - xasm++ disk sizes vs vasm disk sizes
  - If different, understand why (boot sector, catalog, etc.)
```

---

## Success Metrics

```
✓ crackle successfully creates disk images
✓ Virtual ][ mounts disks without errors
✓ POP boots and displays intro
✓ Keyboard controls work
✓ Can play through 2-3 levels
✓ No graphical glitches or crashes
✓ Process documented for future builds
```

---

## Risk Mitigation

### Risk 1: crackle not installed
```
Mitigation:
- Check installation: brew install datajerk/crackle/crackle
- Provide alternative: Use ac tool or A2DskUtil
```

### Risk 2: Disk creation fails
```
Mitigation:
- Compare xasm++ binaries to vasm (already 100% match)
- Check disk layout in Makefile
- Try manual disk creation with ac/A2DskUtil
```

### Risk 3: Virtual ][ emulator issues
```
Mitigation:
- Verify emulator settings (Apple II+ or IIe model)
- Try alternative: AppleWin (via Wine), MAME
- Check disk format (.dsk vs .po vs .2mg)
```

---

## Dependencies

**Must Complete First:**
- ✅ Binary compatibility (100% - xasm++-y7q CLOSED)
- ✅ All 29 POP modules assemble

**External Dependencies:**
- crackle tool installed
- Virtual ][ emulator installed

---

## Estimated Effort

**Time Estimate:** 2-4 hours
- Phase 1: 30 min (verification)
- Phase 2: 1 hour (disk creation)
- Phase 3: 1-2 hours (testing, iteration)
- Phase 4: 30 min (documentation)

**Confidence:** High (straightforward process with known working reference)

---

## Implementation Notes

- Adam Green's build system is mature and well-documented
- 100% binary compatibility means disks should work first try
- Main risk is tooling issues (crackle, emulator), not assembly
- If disk creation works, testing is mostly manual verification

---

**Plan Status:** Ready for execution

**Reviewed By:**
- [x] Orchestrator (2026-01-31)

**Plan Version:** 1.0
**Last Updated:** 2026-01-31
