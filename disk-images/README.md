# Prince of Persia Disk Images for Virtual ][

Created from xasm++ assembled binaries.

## Files

- **POP_BOOT.dsk** - Bootable disk with simple test program
- **POP_DATA.dsk** - Data disk with all assembled POP binaries

## Testing with Virtual ][

### 1. Install Virtual ][

Download from: https://www.virtualii.com/

Or install via Homebrew:
```bash
brew install --cask virtualii
```

### 2. Load Boot Disk

1. Launch Virtual ][
2. Go to **Disk** → **Insert Disk 1...**
3. Select `POP_BOOT.dsk`
4. Click **Reset** (or press Cmd+R)

Expected: You should see "HELLO FROM XASM++" on the screen.

### 3. Load Data Disk

1. Go to **Disk** → **Insert Disk 2...**
2. Select `POP_DATA.dsk`
3. From DOS prompt, type: `CATALOG` to see files

## Assembled Files

The following Prince of Persia source files were assembled with xasm++:

 1. BGDATA.S → BGDATA.bin (491 bytes)
 2. EQ.S → EQ.bin (7,592 bytes)
 3. GAMEEQ.S → GAMEEQ.bin (3,401 bytes)
 4. HRPARAMS.S → HRPARAMS.bin (2,500 bytes)
 5. HRTABLES.S → HRTABLES.bin (2,480 bytes)
 6. MOVEDATA.S → MOVEDATA.bin (0 bytes)
 7. SEQDATA.S → SEQDATA.bin (0 bytes)
 8. SEQTABLE.S → SEQTABLE.bin (2,546 bytes)
 9. SOUNDNAMES.S → SOUNDNAMES.bin (0 bytes)
10. VERSION.S → VERSION.bin (29 bytes)

## Disk Format

- **Type:** 5.25" floppy disk image (.dsk format)
- **Size:** 143,360 bytes (35 tracks × 16 sectors × 256 bytes)
- **Filesystem:** DOS 3.3
- **Compatibility:** Virtual ][, AppleWin, MAME, real hardware via ADTPro

## Advanced: Creating Your Own Bootable Disk

To create a fully bootable Prince of Persia disk:

1. You'll need the original game's boot code
2. Assemble all POP source files
3. Use a tool like AppleCommander or CiderPress to:
   - Create a DOS 3.3 formatted disk
   - Copy boot code to track 0
   - Copy all binaries at their correct load addresses
   - Set up the proper file catalog

## Verification

To verify the assembled binaries match the reference:

```bash
cd tests/e2e/apple2/prince_of_persia
python3 test_pop_complete.py
```

This will compare xasm++ output with vasm reference binaries.

## Resources

- **Virtual ][ Homepage:** https://www.virtualii.com/
- **Prince of Persia Source:** https://github.com/jmechner/Prince-of-Persia-Apple-II
- **xasm++ Documentation:** docs/
- **Test Results:** tests/e2e/apple2/prince_of_persia/output/test_report.md
