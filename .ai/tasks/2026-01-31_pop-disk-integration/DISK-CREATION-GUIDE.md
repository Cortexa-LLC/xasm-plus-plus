# Prince of Persia Disk Image Creation Guide

## Overview

This guide documents the process of creating bootable 5.25" disk images for Prince of Persia (Apple II) using the Adam Green build system and crackle tool.

**Status:** ✅ Process verified working with vasm
**Status:** ❌ Blocked with xasm++ (assembler bugs)

## Prerequisites

### Required Tools

1. **Assembler** (choose one):
   - ✅ **vasm** - Proven working, all 29 modules assemble
   - ❌ **xasm++** - Currently has bugs preventing 19/29 modules from assembling

2. **crackle** - Disk image creation tool
   - Repository: https://github.com/datajerk/crackle
   - Location: `/Users/bryanw/Projects/Vintage/tools/snapNcrackle/crackle/Debug/crackle`

3. **Virtual ][** - Apple II emulator for testing
   - Location: `/Applications/Virtual ][.app`
   - Website: http://www.virtualii.com/

### Source Code

- **POP Source Repository:** `~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II`
- **Build System:** Adam Green's Makefile-based build

## Build Process

### Step 1: Set Environment Variables

```bash
# Set assembler (vasm or xasm++)
export ASM=vasm

# Set crackle path
export CRACKLE=/Users/bryanw/Projects/Vintage/tools/snapNcrackle/crackle/Debug/crackle
```

### Step 2: Navigate to POP Directory

```bash
cd ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II
```

### Step 3: Build Disk Images

```bash
# Clean previous builds
make clean

# Build 5.25" disk images
make ASM=vasm disk525
```

**Expected Output:**
```
Assembling BOOT.S...
Assembling EQ.S...
...
[29 source files assembled]
...
Creating 5.25" Side A disk image...
Created: build/PrinceOfPersia_SideA.nib
Creating 5.25" Side B disk image...
Created: build/PrinceOfPersia_SideB.nib

=== 5.25" Disk Images Created ===
-rw-r--r--  1 user  staff   228K timestamp build/PrinceOfPersia_SideA.nib
-rw-r--r--  1 user  staff   228K timestamp build/PrinceOfPersia_SideB.nib
```

### Step 4: Verify Files Created

```bash
ls -lh build/*.nib
```

**Expected Files:**
- `build/PrinceOfPersia_SideA.nib` - 228KB (bootable system disk)
- `build/PrinceOfPersia_SideB.nib` - 228KB (game data disk)

## Testing in Virtual ][

### Step 1: Launch Emulator

```bash
open "/Applications/Virtual ][.app"
```

### Step 2: Load Disk Image

1. In Virtual ][ menu: **File → Open Disk Image**
2. Navigate to: `~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/build/`
3. Select: `PrinceOfPersia_SideA.nib`
4. Click: **Open**

### Step 3: Boot the Disk

- Press: **Control+Reset** (or use menu: **Computer → Reset**)
- Emulator should boot from disk

### Step 4: Verify Game Functionality

**Expected Boot Sequence:**
1. Apple II boot screen
2. Disk drive sound/animation
3. Broderbund logo
4. Prince of Persia title screen
5. Demo mode or ready to play

**Test Checklist:**
- ✓ Intro screen displays correctly
- ✓ Arrow keys control character movement
- ✓ Shift key starts game
- ✓ Graphics render properly
- ✓ Sound effects play
- ✓ Collision detection works
- ✓ Level transitions work
- ✓ Can save/load game (Side B disk)

## Disk Image Format Details

### .nib Format (Nibble Format)

**Technical Specifications:**
- **Size:** 232,960 bytes (228KB)
- **Format:** Raw nibble data (6-and-2 encoding)
- **Tracks:** 35 tracks
- **Sectors:** 16 sectors per track
- **Sector Size:** 416 bytes (nibble-encoded, 256 bytes data)

**Compatibility:**
- ✅ Virtual ][ (macOS)
- ✅ AppleWin (Windows)
- ✅ MAME (Multi-platform)
- ✅ OpenEmulator (macOS)

### Alternative Formats

The build system can also create:
- `.dsk` - DOS-ordered disk image (232,960 bytes)
- `.po` - ProDOS-ordered disk image (143,360 bytes)

```bash
# Create .dsk format
make ASM=vasm disk525 DISKFORMAT=dsk

# Create .po format  
make ASM=vasm disk525 DISKFORMAT=po
```

## Known Issues with xasm++

### Current Blockers (19 of 29 files fail)

**Issue #1: DS Directive Parser Bug**
- **Files Affected:** BOOT.S, TABLES.S, FRAMEDEF.S
- **Error:** "DS: Multiplication requires operands on both sides"
- **Root Cause:** `ds $900-*` incorrectly parsed as multiplication
- **Expected:** DS directive with program counter subtraction

**Issue #2: Expression Evaluation Bug**
- **Files Affected:** MASTER.S, HIRES.S, GRAFIX.S, CTRL.S, CTRLSUBS.S, TOPCTRL.S, MOVER.S, FRAMEADV.S, COLL.S, GAMEBG.S, SUBS.S, SOUND.S, MISC.S, UNPACK.S (14 files)
- **Error:** "Logic error: stoul: no conversion"
- **Root Cause:** Unknown expression evaluation issue

**Issue #3: Conditional Assembly Bug**
- **Files Affected:** SPECIALK.S
- **Error:** "FIN without matching DO" at line 248
- **Root Cause:** DO/FIN conditional block handling incorrect

### Impact

Only 10 of 29 POP modules successfully assemble with xasm++:
- ✅ BGDATA.S, EQ.S, GAMEEQ.S, HRPARAMS.S, HRTABLES.S
- ✅ SEQTABLE.S, SEQDATA.S, MOVEDATA.S, SOUNDNAMES.S, VERSION.S

These are data-only files with no complex logic. The 19 files containing actual game code all fail.

**Recommendation:** Use vasm until xasm++ bugs are fixed.

## Troubleshooting

### Problem: "crackle: command not found"

**Solution:**
```bash
# Verify crackle is built
ls /Users/bryanw/Projects/Vintage/tools/snapNcrackle/crackle/Debug/crackle

# If not found, build crackle:
cd ~/Projects/Vintage/tools/snapNcrackle/crackle
xcodebuild -configuration Debug
```

### Problem: "Virtual ][: Application not found"

**Solution:**
```bash
# Check if installed
ls "/Applications/Virtual ][.app"

# If not installed, download from:
# http://www.virtualii.com/
```

### Problem: "Assembler errors during build"

**Solution:**
```bash
# Use vasm instead of xasm++
make ASM=vasm disk525

# Check build logs
cat /tmp/vasm-disk.log
```

### Problem: "Disk image won't boot in Virtual ]["

**Checklist:**
1. Verify file size: Should be 228KB for .nib format
2. Check file not corrupted: `file build/PrinceOfPersia_SideA.nib`
3. Try alternative emulator (AppleWin)
4. Rebuild from clean state: `make clean && make ASM=vasm disk525`

## Build System Architecture

### Makefile Targets

```makefile
disk525:        # Build 5.25" disk images (both sides)
clean:          # Remove all build artifacts
build-vasm:     # Build using vasm assembler
build-xasm:     # Build using xasm++ assembler (currently broken)
```

### Build Flow

```
Source Files (.S)
    ↓
Assembler (vasm/xasm++)
    ↓
Binary Files (.bin)
    ↓
Layout Files (.layout)
    ↓
crackle (disk image creator)
    ↓
Disk Images (.nib)
```

### Directory Structure

```
Prince-of-Persia-Apple-II/
├── 01 POP Source/
│   └── Source/          # Assembly source files (29 modules)
├── 04 Support/
│   └── MakeDisk/
│       ├── S/           # Build-specific source files
│       └── Layout/      # Disk layout definitions
├── build/               # Output directory
│   ├── *.bin           # Assembled binaries
│   ├── *.nib           # Disk images
│   └── *.lst           # Listing files (optional)
└── Makefile            # Build system
```

## References

### Documentation
- [Adam Green's POP Repository](https://github.com/adamgreen/Prince-of-Persia-Apple-II)
- [crackle Tool](https://github.com/datajerk/crackle)
- [Virtual ][ Emulator](http://www.virtualii.com/)
- [Apple II Disk Formats](http://fileformats.archiveteam.org/wiki/Apple_II_disk_image)

### Related Tasks
- **Binary Compatibility:** `.ai/tasks/2026-01-31_binary-compatibility/`
- **Disk Integration:** `.ai/tasks/2026-01-31_pop-disk-integration/`

---

**Document Version:** 1.0  
**Last Updated:** 2026-01-31  
**Status:** ✅ Process verified with vasm, ❌ Blocked with xasm++
