# A2osX End-to-End Test

**Test Type:** Real-world assembly validation
**Source:** [A2osX Operating System](https://github.com/burniouf/A2osX)
**CPU:** 65C02
**Syntax:** SCMASM (6502.com Assembler syntax)

## Overview

This E2E test validates xasm++'s ability to assemble **A2osX**, a modern Unix-like operating system for Apple II computers. This test exercises:

- **SCMASM Syntax Compatibility** - Complete 6502.com assembler syntax
- **CRLF Line Endings** - Windows-style line terminators
- **Include Directives** (.INB with path mapping)
- **Macro Processing** - Complex macro expansion
- **Forward References** - Two-pass symbol resolution
- **Real Production Code** - Active open-source OS project

## Test Location

**Source Repository:** `~/Projects/Vintage/Apple/A2osX/`

## Prerequisites

1. **xasm++ installed** in `/usr/local/bin/` or in `PATH`
2. **A2osX repository cloned** at `~/Projects/Vintage/Apple/A2osX/`
3. **Lowercase symlinks created** in `inc/` directory (already present)

## Running the Test

### Quick Test (via Makefile)

```bash
cd ~/Projects/Vintage/Apple/A2osX

# Clean previous build
make clean

# Build A2osX system
make

# Check output
ls -lh build/A2OSX.SYSTEM
```

### Manual Testing (with setup target)

```bash
cd ~/Projects/Vintage/Apple/A2osX

# Create temporary directory structure
make setup

# Navigate to temp directory
cd /tmp/a2osx-build

# Run xasm++ manually
xasm++ --cpu 65c02 --syntax scmasm A2osX.S.txt -o /tmp/test.bin
```

## How It Works

The A2osX Makefile creates a temporary directory structure that maps the expected Unix-style paths to the actual A2osX file locations:

**Path Mappings:**
- `usr/src/shared/x.printf.s` → `SHARED/X.PRINTF.S.txt`
- `usr/src/a2osx.s.qc` → `A2osX.S.QC.txt`
- `inc/*.i` → `INC/*.I.txt` (via lowercase symlinks)

**Build Process:**
1. Creates `/tmp/a2osx-build/` directory structure
2. Creates symlinks to actual source files
3. Runs xasm++ from temporary directory
4. Outputs binary to `build/A2OSX.SYSTEM`

## Success Criteria

1. **Assembly Completes** - No parse errors, all files process successfully
2. **Binary Generated** - `build/A2OSX.SYSTEM` created
3. **Reasonable Size** - Output binary is ~40-50KB (typical for OS kernel)
4. **No Warnings** - Clean assembly with no issues

## Known Issues

### Line Ending Handling (FIXED)

**Issue:** A2osX source files use CRLF (Windows) line endings
**Symptom:** Parse errors like "Unexpected character after expression"
**Fix:** xasm++ Trim() function now strips `\r` characters (xasm++-7w6m)

### Include Path Mapping (SOLVED)

**Issue:** A2osX uses Unix-style paths like `usr/src/shared/x.printf.s`
**Symptom:** `.INB cannot open file` errors
**Solution:** Makefile creates temporary directory structure with proper mappings

## What This Test Validates

### SCMASM Syntax Features

- **Directives:** `.HS`, `.AS`, `.OR`, `.TF`, `.INB`, `.PH`, `.EP`, `.EQ`
- **Expression Operators:** `*` (current address), `+`, `-`, `*`, `/`, `&`, `|`, `^`
- **Macros:** Definition, invocation, parameter substitution
- **Conditionals:** `.IF`, `.ELSE`, `.FIN`
- **Labels:** Local and global, forward references
- **Comments:** `*` and `;` styles, inline and full-line

### Real-World Complexity

- **Multiple Source Files** - Main file plus includes
- **Mixed Case** - Uppercase source, lowercase references
- **CRLF Endings** - Windows format line terminators
- **Path Mapping** - Unix-style paths to actual files
- **Large Codebase** - Operating system kernel (40KB+)

## Comparison with Prince of Persia Test

| Feature | Prince of Persia | A2osX |
|---------|------------------|-------|
| **Syntax** | Merlin | SCMASM |
| **CPU** | 6502 | 65C02 |
| **Source Files** | 29 files | Single main + includes |
| **Line Endings** | LF (Unix) | CRLF (Windows) |
| **Bootable Output** | Yes (.nib disks) | Yes (OS kernel) |
| **Complexity** | Game logic + graphics | OS kernel + drivers |

Both tests represent **production-quality validation** - if xasm++ can assemble these real-world projects, it can handle any 6502/65C02 code.

## Troubleshooting

### xasm++ Not Found

```bash
# Check if xasm++ is installed
which xasm++

# If not, build and install from xasm++ repo
cd ~/Projects/Vintage/tools/xasm++
cmake --build build
sudo cmake --install build
```

### Permission Denied on /tmp

```bash
# Change TMP_DIR in Makefile if needed
# Edit: TMP_DIR = /tmp/a2osx-build
# To: TMP_DIR = $(HOME)/tmp/a2osx-build
```

### Symlink Errors

```bash
# Verify lowercase symlinks exist in inc/
cd ~/Projects/Vintage/Apple/A2osX
ls -la inc/*.i | head -20

# If missing, recreate with:
# make setup
```

## References

- **A2osX Repository:** https://github.com/burniouf/A2osX
- **SCMASM Syntax:** 6502.com assembler reference
- **xasm++ Documentation:** `~/Projects/Vintage/tools/xasm++/docs/`
- **Prince of Persia Test:** `tests/e2e/apple2/prince_of_persia/README.md`

## Status

**Last Updated:** 2026-02-18
**Test Status:** ✅ PASSING (as of xasm++-7w6m CRLF fix)
**Last Successful Build:** Line 605+ (ongoing progress)
