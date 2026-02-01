#!/bin/bash
# Binary Compatibility Test Reference Runner
# Runs vasm-ext assemblers to generate reference binaries

set -euo pipefail

# Usage: ./reference_runner.sh <syntax> <cpu> <source.asm>
# Example: ./reference_runner.sh merlin 6502 test.asm

SYNTAX=$1
CPU=$2
SOURCE=$3

# Get absolute paths
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PROJECT_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)
VASM_DIR="$PROJECT_ROOT/../vasm-ext"

# Create output directory
OUTPUT_DIR="$PROJECT_ROOT/tests/integration/binary_compat/reference_outputs/$SYNTAX/$CPU"
mkdir -p "$OUTPUT_DIR"

# Get base filename without extension
BASENAME=$(basename "$SOURCE" .asm)
OUTPUT_FILE="$OUTPUT_DIR/${BASENAME}.bin"

# Map syntax to vasm binary
case "$SYNTAX" in
  merlin)
    VASM_BIN="vasm6502_merlin"
    ;;
  scmasm)
    VASM_BIN="vasm6502_scmasm"
    ;;
  *)
    echo "ERROR: Unknown syntax '$SYNTAX'"
    exit 1
    ;;
esac

# Map CPU to flags
case "$CPU" in
  6502)
    CPU_FLAGS=""
    ;;
  65c02)
    CPU_FLAGS="-m65c02"
    ;;
  65816)
    CPU_FLAGS="-m65816"
    ;;
  *)
    echo "ERROR: Unknown CPU '$CPU'"
    exit 1
    ;;
esac

# Run vasm-ext
cd "$VASM_DIR"
./"$VASM_BIN" $CPU_FLAGS -Fbin -o "$OUTPUT_FILE" "$PROJECT_ROOT/$SOURCE"

echo "Reference binary generated: $OUTPUT_FILE"
