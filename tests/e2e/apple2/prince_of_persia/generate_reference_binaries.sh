#!/bin/bash
# Generate reference binaries for PoP files using vasm6502_merlin
# These serve as the "golden" binaries to verify xasm++ correctness

set -e

POP_SOURCE="$HOME/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/01 POP Source/Source"
REFERENCE_DIR="$(dirname "$0")/reference"
VASM="vasm6502_merlin"

# Check if vasm is available
if ! command -v $VASM &> /dev/null; then
    echo "ERROR: vasm6502_merlin not found in PATH"
    echo "Build vasm-ext first: cd ~/Projects/Vintage/tools/vasm-ext && make CPU=6502 SYNTAX=merlin"
    exit 1
fi

# Create reference directory
mkdir -p "$REFERENCE_DIR"

# Only generate references for files that successfully assemble with vasm
# (These are the files xasm++ currently passes)
PASSING_FILES=(
    "AUTO.S" "BGDATA.S" "COLL.S" "CTRL.S" "CTRLSUBS.S"
    "EQ.S" "FRAMEADV.S" "GAMEBG.S" "GAMEEQ.S"
    "HIRES.S" "HRPARAMS.S" "HRTABLES.S" "MASTER.S" "MISC.S" "MOVEDATA.S"
    "MOVER.S" "SEQDATA.S" "SEQTABLE.S" "SOUND.S" "SOUNDNAMES.S"
    "SUBS.S" "TOPCTRL.S" "VERSION.S"
)

echo "Generating reference binaries with vasm6502_merlin..."
echo "=================================================="
echo ""

success=0
failed=0

for file in "${PASSING_FILES[@]}"; do
    basename="${file%.S}"
    echo -n "Generating $basename.bin... "
    
    # vasm parameters to match xasm++ behavior
    # -Fbin = raw binary output
    # -dotdir = allow .S in filenames
    # -wdc02 = 6502 (not 65C02)
    if $VASM -Fbin -dotdir -wdc02 -o "$REFERENCE_DIR/$basename.bin" "$POP_SOURCE/$file" 2>/dev/null; then
        echo "✓"
        ((success++))
    else
        echo "✗ (vasm failed)"
        ((failed++))
    fi
done

echo ""
echo "=================================================="
echo "Generated: $success reference binaries"
if [ $failed -gt 0 ]; then
    echo "Failed:    $failed files"
fi
echo ""
echo "Reference binaries saved to: $REFERENCE_DIR/"
