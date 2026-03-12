#!/usr/bin/env bash
#
# A2oSX SCMASM Directive Analysis Script
# Purpose: Extract all SCMASM directives from A2oSX source files
# Author: Engineer Agent
# Date: 2026-02-13
#

set -euo pipefail

# Configuration
A2OSX_DIR="$HOME/Projects/Vintage/Apple/A2osX"
TASK_DIR="$(cd "$(dirname "$0")" && pwd)"
OUTPUT_FILE="$TASK_DIR/directive-usage-report.txt"
TEMP_FILE="$TASK_DIR/.directives_temp.txt"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}A2oSX SCMASM Directive Analysis${NC}"
echo "========================================"
echo ""

# Check if A2oSX directory exists
if [ ! -d "$A2OSX_DIR" ]; then
  echo -e "${RED}ERROR: A2oSX directory not found at: $A2OSX_DIR${NC}"
  echo "Please clone the A2oSX repository first."
  exit 1
fi

echo -e "${YELLOW}Scanning A2oSX source files...${NC}"
echo "Source directory: $A2OSX_DIR"
echo ""

# Find all .S and .S.txt files
FILE_COUNT=$(find "$A2OSX_DIR" -type f \( -name "*.S" -o -name "*.S.txt" \) | wc -l | tr -d ' ')
echo "Files to scan: $FILE_COUNT"
echo ""

# Extract directives
# Strategy:
# 1. Find all lines starting with optional whitespace followed by '.'
# 2. Use awk to extract just the first field (the directive)
# 3. Convert to uppercase for consistent counting
# 4. Sort and count unique occurrences

echo -e "${YELLOW}Extracting directives...${NC}"

find "$A2OSX_DIR" -type f \( -name "*.S" -o -name "*.S.txt" \) -exec cat {} \; | \
  grep -E '^\s*\.[A-Za-z][A-Za-z0-9]*' | \
  awk '{print $1}' | \
  tr '[:lower:]' '[:upper:]' | \
  sort | \
  uniq -c | \
  sort -rn > "$TEMP_FILE"

# Generate report
{
  echo "A2oSX SCMASM Directive Usage Report"
  echo "===================================="
  echo "Generated: $(date)"
  echo "Source: ~/Projects/Vintage/Apple/A2osX/"
  echo ""
  echo "This report lists all SCMASM directives found in A2oSX source files,"
  echo "sorted by usage frequency (most common first)."
  echo ""
  echo "Format: <count> <directive>"
  echo ""
  echo "Directives:"
  echo "-----------"
  echo ""
  cat "$TEMP_FILE"
  echo ""
  echo "Summary Statistics:"
  echo "-------------------"
  echo "Total directive occurrences: $(awk '{sum+=$1} END {print sum}' "$TEMP_FILE")"
  echo "Unique directives: $(wc -l < "$TEMP_FILE" | tr -d ' ')"
  echo "Source files scanned: $FILE_COUNT"
} > "$OUTPUT_FILE"

# Clean up
rm -f "$TEMP_FILE"

# Display results
echo -e "${GREEN}Analysis complete!${NC}"
echo ""
echo "Results saved to: $OUTPUT_FILE"
echo ""
echo "Top 30 most-used directives:"
echo "----------------------------"
head -40 "$OUTPUT_FILE" | tail -30

echo ""
echo -e "${GREEN}Done!${NC}"
