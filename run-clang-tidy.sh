#!/bin/bash
# Run clang-tidy on all C++ source files

set -e

CLANG_TIDY=/opt/homebrew/opt/llvm/bin/clang-tidy
OUTPUT_FILE=build/clang-tidy-report.txt

echo "Running clang-tidy analysis on xasm++ codebase..."
echo "Output will be saved to: $OUTPUT_FILE"
echo ""

# Find all C++ source files
FILES=$(find src -name "*.cpp" -type f | sort)
TOTAL=$(echo "$FILES" | wc -l | tr -d ' ')

echo "Analyzing $TOTAL C++ files..."
echo ""

# Run clang-tidy
# Note: --extra-arg isystem flags provide stdlib headers to Homebrew LLVM's clang-tidy.
# They duplicate what Apple's compiler injects, causing multiple std::size_t declarations
# visible to clang-tidy's type analysis, which triggers false-positive
# bugprone-infinite-loop on standard for(size_t i=0; i<=n; ++i) loops.
# That check is therefore excluded in .clang-tidy.
$CLANG_TIDY -p build $FILES \
  --extra-arg="-isystem/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1" \
  --extra-arg="-isystem/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include" \
  > "$OUTPUT_FILE" 2>&1

# Count issues
WARNINGS=$(grep -c "warning:" "$OUTPUT_FILE" || true)
ERRORS=$(grep -c "error:" "$OUTPUT_FILE" || true)

echo "✅ Analysis complete!"
echo ""
echo "Results:"
echo "  Warnings: $WARNINGS"
echo "  Errors: $ERRORS"
echo "  Full report: $OUTPUT_FILE"
echo ""

# Show summary
echo "Top issues by category:"
grep "warning:" "$OUTPUT_FILE" | \
  sed 's/.*\[\(.*\)\]/\1/' | \
  sort | uniq -c | sort -rn | head -20
