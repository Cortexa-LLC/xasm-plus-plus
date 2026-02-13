#!/usr/bin/env python3
"""
Build Prince of Persia bootable disk images with xasm++

This script builds complete, bootable Apple II disk images for Prince of Persia
using xasm++ as the assembler. The resulting .nib files can be used in Virtual ][
or on real Apple //e hardware.

Usage:
    python3 build_disks.py
    ./build_disks.py

Output:
    ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/build/PrinceOfPersia_SideA.nib
    ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/build/PrinceOfPersia_SideB.nib

Requirements:
    - xasm++ installed to PATH (/usr/local/bin/xasm++)
    - Prince of Persia source at ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II
    - crackle binary at ~/Projects/Vintage/tools/snapNcrackle/crackle/Debug/crackle
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

# ANSI color codes
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

# Configuration
POP_DIR = Path.home() / "Projects/Vintage/Apple/Prince-of-Persia-Apple-II"
CRACKLE_PATH = Path.home() / "Projects/Vintage/tools/snapNcrackle/crackle/Debug/crackle"
XASM_BIN = "xasm++"


def print_header():
    """Print script header"""
    print(f"{Colors.BLUE}╔══════════════════════════════════════════════════════════════╗{Colors.NC}")
    print(f"{Colors.BLUE}║  Prince of Persia Bootable Disk Builder (xasm++)            ║{Colors.NC}")
    print(f"{Colors.BLUE}╚══════════════════════════════════════════════════════════════╝{Colors.NC}")
    print()


def check_prerequisites():
    """Check that all required tools and files exist"""
    print(f"{Colors.YELLOW}Checking prerequisites...{Colors.NC}")

    # Check xasm++
    if not shutil.which(XASM_BIN):
        print(f"{Colors.RED}✗ xasm++ not found in PATH{Colors.NC}")
        print("  Install with: sudo cmake --install build")
        return False
    print(f"{Colors.GREEN}✓ xasm++ found: {shutil.which(XASM_BIN)}{Colors.NC}")

    # Check PoP source
    if not POP_DIR.exists():
        print(f"{Colors.RED}✗ Prince of Persia source not found{Colors.NC}")
        print(f"  Expected at: {POP_DIR}")
        print("  Clone with:")
        print(f"    git clone https://github.com/jmechner/Prince-of-Persia-Apple-II.git \\")
        print(f"      {POP_DIR}")
        return False
    print(f"{Colors.GREEN}✓ PoP source found: {POP_DIR}{Colors.NC}")

    # Check crackle
    if not CRACKLE_PATH.exists():
        print(f"{Colors.RED}✗ crackle not found{Colors.NC}")
        print(f"  Expected at: {CRACKLE_PATH}")
        print("  Build snapNcrackle first:")
        print("    git clone https://github.com/adamgreen/snapNcrackle.git")
        print("    cd snapNcrackle/crackle && xcodebuild")
        return False
    print(f"{Colors.GREEN}✓ crackle found: {CRACKLE_PATH}{Colors.NC}")

    return True


def build_disk_images():
    """Build the disk images using make"""
    print()
    print(f"{Colors.YELLOW}Building disk images...{Colors.NC}")
    print()

    # Change to PoP directory
    os.chdir(POP_DIR)

    # Clean previous build
    try:
        subprocess.run(["make", "clean"],
                      stdout=subprocess.DEVNULL,
                      stderr=subprocess.DEVNULL,
                      check=False)
    except:
        pass

    # Build with xasm++
    try:
        result = subprocess.run(
            ["make",
             f"ASM=xasm",
             f"XASM={XASM_BIN}",
             f"CRACKLE={CRACKLE_PATH}",
             "disk525"],
            check=True
        )
        return True
    except subprocess.CalledProcessError:
        return False


def print_success():
    """Print success message and instructions"""
    print()
    print(f"{Colors.GREEN}╔══════════════════════════════════════════════════════════════╗{Colors.NC}")
    print(f"{Colors.GREEN}║  SUCCESS: Bootable disk images created!                     ║{Colors.NC}")
    print(f"{Colors.GREEN}╚══════════════════════════════════════════════════════════════╝{Colors.NC}")
    print()

    # Show disk images
    print(f"{Colors.BLUE}Disk Images:{Colors.NC}")
    build_dir = POP_DIR / "build"
    for nib_file in sorted(build_dir.glob("*.nib")):
        size = nib_file.stat().st_size / 1024
        print(f"  {nib_file.name:30s} {size:>6.0f} KB")
    print()

    # Instructions
    print(f"{Colors.BLUE}To play in Virtual ][:${Colors.NC}")
    print("  1. Launch Virtual ][")
    print("  2. Insert disk images:")
    print("     - Drive 1: build/PrinceOfPersia_SideA.nib")
    print("     - Drive 2: build/PrinceOfPersia_SideB.nib")
    print("  3. Press F5 or click Reset")
    print("  4. Game should auto-boot and be playable!")
    print()

    print(f"{Colors.GREEN}✓ Build completed successfully{Colors.NC}")


def print_failure():
    """Print failure message"""
    print()
    print(f"{Colors.RED}╔══════════════════════════════════════════════════════════════╗{Colors.NC}")
    print(f"{Colors.RED}║  FAILED: Disk image build failed                            ║{Colors.NC}")
    print(f"{Colors.RED}╚══════════════════════════════════════════════════════════════╝{Colors.NC}")
    print()
    print("Check error messages above for details.")
    print("Common issues:")
    print("  - Source file syntax errors")
    print("  - Missing dependencies")
    print("  - crackle disk creation failure")


def main():
    """Main entry point"""
    print_header()

    # Check prerequisites
    if not check_prerequisites():
        sys.exit(1)

    # Build disk images
    if build_disk_images():
        print_success()
        sys.exit(0)
    else:
        print_failure()
        sys.exit(1)


if __name__ == "__main__":
    main()
