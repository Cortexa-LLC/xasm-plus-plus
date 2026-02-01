#!/usr/bin/env python3
"""
Verify Apple II disk image format and contents.

Usage:
    python3 verify_disk.py disk-images/POP_BOOT.dsk
"""

import sys
from pathlib import Path

DISK_SIZE = 143360  # 35 tracks × 16 sectors × 256 bytes
BYTES_PER_SECTOR = 256

def verify_disk_image(disk_path: Path):
    """Verify disk image format and display basic info."""

    print(f"\n📀 Verifying disk image: {disk_path.name}")
    print("=" * 60)

    if not disk_path.exists():
        print(f"❌ File not found: {disk_path}")
        return False

    # Check file size
    size = disk_path.stat().st_size
    print(f"File size: {size:,} bytes")

    if size != DISK_SIZE:
        print(f"⚠️  Warning: Expected {DISK_SIZE:,} bytes for 5.25\" disk")
        print(f"   Difference: {size - DISK_SIZE:+,} bytes")
    else:
        print(f"✓ Correct size for 5.25\" disk image")

    # Read disk data
    with open(disk_path, 'rb') as f:
        data = f.read()

    # Check boot sector (track 0, sector 0)
    print(f"\nBoot sector (track 0, sector 0):")
    boot_sector = data[0:BYTES_PER_SECTOR]

    # Check if boot sector looks initialized
    non_zero = sum(1 for b in boot_sector if b != 0)
    print(f"  Non-zero bytes: {non_zero}/256")

    if non_zero > 0:
        print(f"  ✓ Boot sector contains data")
        # Show first few bytes
        print(f"  First 16 bytes: {' '.join(f'{b:02X}' for b in boot_sector[:16])}")
    else:
        print(f"  ⚠️  Boot sector is empty")

    # Check VTOC (track 17, sector 0)
    print(f"\nVTOC (Volume Table of Contents):")
    vtoc_offset = (17 * 16 + 0) * BYTES_PER_SECTOR
    vtoc = data[vtoc_offset:vtoc_offset + BYTES_PER_SECTOR]

    dos_version = vtoc[0x03]
    volume_num = vtoc[0x06]
    max_pairs = vtoc[0x27]
    num_tracks = vtoc[0x34]
    sectors_per_track = vtoc[0x35]

    print(f"  DOS version: {dos_version}")
    print(f"  Volume number: {volume_num}")
    print(f"  Max T/S pairs: {max_pairs}")
    print(f"  Tracks: {num_tracks}")
    print(f"  Sectors/track: {sectors_per_track}")

    if num_tracks == 35 and sectors_per_track == 16:
        print(f"  ✓ Valid DOS 3.3 VTOC")
    else:
        print(f"  ⚠️  Unexpected geometry")

    # Check catalog (track 17, sector 1)
    print(f"\nCatalog sector:")
    catalog_offset = (17 * 16 + 1) * BYTES_PER_SECTOR
    catalog = data[catalog_offset:catalog_offset + BYTES_PER_SECTOR]

    next_cat_track = catalog[0x01]
    next_cat_sector = catalog[0x02]
    print(f"  Next catalog: track {next_cat_track}, sector {next_cat_sector}")

    # Count file entries
    file_entries = 0
    for i in range(7):  # Up to 7 entries per sector
        offset = 0x0B + i * 0x23
        file_type = catalog[offset]
        if file_type != 0x00 and file_type != 0xFF:
            file_entries += 1
            # Extract filename (30 bytes, high bit set)
            filename_bytes = catalog[offset + 0x03:offset + 0x03 + 30]
            filename = ''.join(chr(b & 0x7F) for b in filename_bytes if b != 0)
            print(f"  File {file_entries}: {filename.strip()} (type ${file_type:02X})")

    if file_entries > 0:
        print(f"  ✓ Found {file_entries} file(s)")
    else:
        print(f"  No files in catalog (empty disk or catalog not populated)")

    # Overall assessment
    print(f"\n" + "=" * 60)
    if size == DISK_SIZE and dos_version == 3:
        print(f"✅ Valid DOS 3.3 disk image")
        return True
    else:
        print(f"⚠️  Disk image may have issues")
        return False


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 verify_disk.py <disk_image.dsk>")
        sys.exit(1)

    disk_path = Path(sys.argv[1])
    success = verify_disk_image(disk_path)

    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
