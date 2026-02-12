# Output Format Support

This document describes the output formats supported by xasm++.

## Overview

xasm++ supports multiple output formats for cross-platform development targeting vintage computing systems. Each format is implemented as an `OutputWriter` subclass.

## Supported Formats

### Intel HEX Format
- **Extension:** `.hex`
- **Format Name:** "Intel HEX"
- **Target:** Universal (8-bit, 16-bit, 32-bit systems)
- **Features:**
  - Data records (Type 00)
  - Extended Linear Address records (Type 04) for 32-bit addressing
  - Extended Segment Address records (Type 02) for 20-bit addressing
  - End-of-File record (Type 01)
  - Configurable bytes-per-line (default: 16)
- **Use Case:** ROM programming, embedded systems, flash utilities
- **Status:** ✅ Complete

### S-Record Format (Motorola)
- **Extensions:** `.s19` (16-bit), `.s28` (24-bit), `.s37` (32-bit)
- **Format Name:** "S-Record"
- **Target:** Universal (Motorola standard)
- **Features:**
  - Automatic format selection based on address range:
    - S19: Addresses ≤ 0xFFFF (16-bit)
    - S28: Addresses ≤ 0xFFFFFF (24-bit)
    - S37: Addresses > 0xFFFFFF (32-bit)
  - Header record (S0) with filename/metadata
  - Data records (S1/S2/S3)
  - Count records (S5/S6) for verification
  - Start address records (S7/S8/S9)
  - Checksum validation (one's complement)
  - Configurable bytes-per-record (default: 32)
- **Use Case:** ROM programming, cross-development tools, debuggers
- **Status:** ✅ Complete

### TRS-DOS Binary Format
- **Extension:** `.cmd`
- **Format Name:** "TRS-DOS Binary"
- **Target:** TRS-80 Model I/III/4 (Z80 CPU)
- **Operating Systems:** TRS-DOS, NEWDOS/80, compatible
- **Features:**
  - Segment-based format
  - Data segments (Type 0x01) with load address and length
  - Entry point segments (Type 0x02)
  - Multiple segment support (handles address gaps)
  - Little-endian 16-bit addressing
- **Load Command:** `SYSTEM "filename"`
- **Use Case:** TRS-80 machine code programs
- **Status:** ✅ Complete

### CoCo DOS (LOADM) Binary Format
- **Extension:** `.bin`
- **Format Name:** "CoCo DOS (LOADM)"
- **Target:** TRS-80 Color Computer (6809 CPU)
- **Operating Systems:** Color BASIC, Disk Extended Color BASIC (DECB)
- **Features:**
  - Preamble block (0x00 header)
  - Data blocks with address and length
  - Postamble block (0xFF terminator)
  - Optional entry point in postamble
  - Multiple block support (handles address gaps)
  - Big-endian 16-bit addressing (6809 native)
- **Load Commands:**
  - `LOADM "filename"` (load binary)
  - `EXEC &HXXXX` (execute at address)
- **Use Case:** Color Computer machine code programs
- **Status:** ✅ Complete

## Planned Formats

### OS/9 Module Format
- **Target:** OS-9 operating system (6809/Z80)
- **Features:**
  - Module header with sync bytes ($87CD)
  - Module types (program, subroutine, data, trap handler, device driver)
  - Module name
  - CRC checksum (CRC-24)
  - Both 6809 and Z80 variants
- **Status:** 🔜 Planned (separate subtask)

## Usage Examples

### Basic Usage
```cpp
#include "xasm++/output/intel_hex_writer.h"
#include "xasm++/output/srec_writer.h"
#include "xasm++/output/trsdos_writer.h"
#include "xasm++/output/coco_loadm_writer.h"

// Get assembled sections
std::vector<Section> sections = assembler.GetSections();

// Intel HEX output
IntelHexWriter hex_writer;
std::ofstream hex_out("program.hex", std::ios::binary);
hex_writer.Write(sections, hex_out);

// S-Record output (auto-selects S19/S28/S37)
SRecordWriter srec_writer;
std::ofstream srec_out("program.s19", std::ios::binary);
srec_writer.Write(sections, srec_out);

// TRS-DOS binary
TrsDosWriter trsdos_writer;
trsdos_writer.SetEntryPoint(0x8000);
std::ofstream trsdos_out("program.cmd", std::ios::binary);
trsdos_writer.Write(sections, trsdos_out);

// CoCo LOADM binary
CocoLoadmWriter coco_writer;
coco_writer.SetEntryPoint(0x2000);
std::ofstream coco_out("program.bin", std::ios::binary);
coco_writer.Write(sections, coco_out);
```

### Configuration Options
```cpp
// Intel HEX: Custom bytes per line
IntelHexWriter hex_writer;
hex_writer.SetBytesPerLine(32);  // Default: 16

// S-Record: Custom bytes per record
SRecordWriter srec_writer;
srec_writer.SetBytesPerRecord(16);  // Default: 32
srec_writer.SetHeader("MyProgram v1.0");  // Optional header

// Platform binaries: Entry point
TrsDosWriter trsdos_writer;
trsdos_writer.SetEntryPoint(0x8000);  // Execution start address
```

## Format Selection Guide

| Target System | Recommended Format | Alternative |
|---------------|-------------------|-------------|
| ROM/EPROM programming | Intel HEX | S-Record |
| TRS-80 Model I/III/4 | TRS-DOS Binary | Intel HEX |
| TRS-80 Color Computer | CoCo LOADM Binary | S-Record |
| OS-9 systems | OS/9 Module | - |
| Universal/debugging | S-Record | Intel HEX |

## Implementation Details

All format writers:
- Inherit from `OutputWriter` base class
- Use `std::ostream` for output (no file I/O)
- Handle multiple sections and address gaps
- Validate address ranges per format limits
- Generate proper checksums/CRCs
- Follow TDD methodology (100% test coverage)

## Testing

Each format has comprehensive unit tests covering:
- Basic format structure
- Multiple sections and address gaps
- Entry point handling
- Edge cases (empty sections, space atoms)
- Format-specific features
- Checksum/CRC validation

See `tests/unit/test_*_writer.cpp` for test implementations.

## References

- **Intel HEX:** Intel HEX specification (8-bit, 16-bit, 32-bit)
- **S-Record:** Motorola S-Record specification
- **TRS-DOS:** TRS-80 Model I/III/4 Technical Reference Manual
- **CoCo LOADM:** Color Computer BASIC Programming Reference
- **OS/9:** OS-9 Module Format specification

---

**Document Version:** 1.0
**Last Updated:** 2026-02-09
**Status:** Living document (updated as formats added)
