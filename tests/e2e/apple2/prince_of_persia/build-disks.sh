#!/usr/bin/env bash
#
# Build Prince of Persia bootable disk images with xasm++
#
# This script builds complete, bootable Apple II disk images for Prince of Persia
# using xasm++ as the assembler. The resulting .nib files can be used in Virtual ][
# or on real Apple //e hardware.
#
# Usage:
#   ./build-disks.sh
#
# Output:
#   ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/build/PrinceOfPersia_SideA.nib
#   ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/build/PrinceOfPersia_SideB.nib
#
# Requirements:
#   - xasm++ installed to PATH (/usr/local/bin/xasm++)
#   - Prince of Persia source at ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II
#   - crackle binary at ~/Projects/Vintage/tools/snapNcrackle/crackle/Debug/crackle

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
POP_DIR="$HOME/Projects/Vintage/Apple/Prince-of-Persia-Apple-II"
CRACKLE_PATH="$HOME/Projects/Vintage/tools/snapNcrackle/crackle/Debug/crackle"
XASM_BIN="xasm++"

echo -e "${BLUE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  Prince of Persia Bootable Disk Builder (xasm++)            ║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Check prerequisites
echo -e "${YELLOW}Checking prerequisites...${NC}"

# Check xasm++
if ! command -v "$XASM_BIN" &> /dev/null; then
    echo -e "${RED}✗ xasm++ not found in PATH${NC}"
    echo "  Install with: sudo cmake --install build"
    exit 1
fi
echo -e "${GREEN}✓ xasm++ found: $(which $XASM_BIN)${NC}"

# Check PoP source
if [ ! -d "$POP_DIR" ]; then
    echo -e "${RED}✗ Prince of Persia source not found${NC}"
    echo "  Expected at: $POP_DIR"
    echo "  Clone with:"
    echo "    git clone https://github.com/jmechner/Prince-of-Persia-Apple-II.git \\"
    echo "      $POP_DIR"
    exit 1
fi
echo -e "${GREEN}✓ PoP source found: $POP_DIR${NC}"

# Check crackle
if [ ! -f "$CRACKLE_PATH" ]; then
    echo -e "${RED}✗ crackle not found${NC}"
    echo "  Expected at: $CRACKLE_PATH"
    echo "  Build snapNcrackle first:"
    echo "    git clone https://github.com/adamgreen/snapNcrackle.git"
    echo "    cd snapNcrackle/crackle && xcodebuild"
    exit 1
fi
echo -e "${GREEN}✓ crackle found: $CRACKLE_PATH${NC}"

echo ""
echo -e "${YELLOW}Building disk images...${NC}"
echo ""

# Clean previous build
cd "$POP_DIR"
make clean > /dev/null 2>&1 || true

# Build with xasm++
if make ASM=xasm XASM="$XASM_BIN" CRACKLE="$CRACKLE_PATH" disk525; then
    echo ""
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  SUCCESS: Bootable disk images created!                     ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""

    # Show disk images
    echo -e "${BLUE}Disk Images:${NC}"
    ls -lh "$POP_DIR/build/"*.nib | while read -r line; do
        echo "  $line"
    done
    echo ""

    # Instructions
    echo -e "${BLUE}To play in Virtual ][:${NC}"
    echo "  1. Launch Virtual ]["
    echo "  2. Insert disk images:"
    echo "     - Drive 1: build/PrinceOfPersia_SideA.nib"
    echo "     - Drive 2: build/PrinceOfPersia_SideB.nib"
    echo "  3. Press F5 or click Reset"
    echo "  4. Game should auto-boot and be playable!"
    echo ""

    echo -e "${GREEN}✓ Build completed successfully${NC}"
    exit 0
else
    echo ""
    echo -e "${RED}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║  FAILED: Disk image build failed                            ║${NC}"
    echo -e "${RED}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "Check error messages above for details."
    echo "Common issues:"
    echo "  - Source file syntax errors"
    echo "  - Missing dependencies"
    echo "  - crackle disk creation failure"
    exit 1
fi
