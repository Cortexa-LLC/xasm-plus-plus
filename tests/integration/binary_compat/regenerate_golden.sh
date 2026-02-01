#!/bin/bash
# Regenerate golden reference binaries using vasm-ext
# This script should be run whenever test sources are updated

set -e

PROJECT_ROOT=$(git rev-parse --show-toplevel)
cd "$PROJECT_ROOT"

VASM_MERLIN=~/Projects/Vintage/tools/vasm-ext/vasm6502_merlin
VASM_SCMASM=~/Projects/Vintage/tools/vasm-ext/vasm6502_scmasm

# Verify vasm binaries exist
if [ ! -x "$VASM_MERLIN" ]; then
    echo "ERROR: $VASM_MERLIN not found or not executable"
    exit 1
fi

if [ ! -x "$VASM_SCMASM" ]; then
    echo "ERROR: $VASM_SCMASM not found or not executable"
    exit 1
fi

echo "Regenerating golden reference binaries..."
echo "Using vasm binaries:"
echo "  Merlin: $VASM_MERLIN"
echo "  SCMASM: $VASM_SCMASM"
echo

# Function to assemble and generate golden file
generate_golden() {
    local syntax=$1
    local cpu=$2
    local source=$3
    local vasm_flags=$4
    
    local vasm_bin
    if [ "$syntax" = "merlin" ]; then
        vasm_bin="$VASM_MERLIN"
    else
        vasm_bin="$VASM_SCMASM"
    fi
    
    local source_path="tests/integration/binary_compat/test_sources/$syntax/$cpu/$source"
    local golden_path="tests/integration/binary_compat/golden/$syntax/$cpu/${source%.asm}.bin"
    
    # Create golden directory if needed
    mkdir -p "$(dirname "$golden_path")"
    
    echo "Generating: $golden_path"
    if $vasm_bin $vasm_flags -Fbin -o "$golden_path" "$source_path" 2>&1 | grep -v "^vasm"; then
        echo "  SUCCESS: $(stat -f%z "$golden_path") bytes"
    else
        echo "  ERROR: Failed to assemble $source_path"
        return 1
    fi
}

# Merlin + 6502
echo "=== Merlin + 6502 ==="
generate_golden merlin 6502 01_basic_ops.asm ""

# Merlin + 65C02
echo
echo "=== Merlin + 65C02 ==="
# Add when 65C02 tests exist
# generate_golden merlin 65c02 01_new_instructions.asm ""

# Merlin + 65816
echo
echo "=== Merlin + 65816 ==="
# Add when 65816 tests exist
# generate_golden merlin 65816 01_16bit_mode.asm ""

# SCMASM + 6502
echo
echo "=== SCMASM + 6502 ==="
# Add when SCMASM tests exist
# generate_golden scmasm 6502 01_directives.asm ""

echo
echo "Golden file regeneration complete!"
echo
echo "Next steps:"
echo "1. Review the generated binaries"
echo "2. Verify they match expected output"
echo "3. Commit the golden files to git"
