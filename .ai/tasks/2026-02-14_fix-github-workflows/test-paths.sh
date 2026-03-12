#!/bin/bash
# Quick validation script to verify path updates are correct

set -e

PROJECT_ROOT="$(git rev-parse --show-toplevel)"
cd "$PROJECT_ROOT"

echo "=========================================="
echo "Path Update Validation"
echo "=========================================="
echo

# 1. Check binary exists at new location
echo "✓ Checking binary location..."
if [ -f "build/bin/xasm++" ]; then
    echo "  ✓ Found: build/bin/xasm++"
else
    echo "  ✗ ERROR: build/bin/xasm++ not found"
    exit 1
fi

# 2. Check old location doesn't exist
echo "✓ Checking old location removed..."
if [ -f "build/src/xasm++" ]; then
    echo "  ✗ ERROR: Old path build/src/xasm++ still exists"
    exit 1
else
    echo "  ✓ Old path correctly removed"
fi

# 3. Check documentation updated
echo "✓ Checking documentation updated..."
if grep -q "build/src/xasm++" README.md 2>/dev/null; then
    echo "  ✗ ERROR: README.md still references old path"
    exit 1
else
    echo "  ✓ README.md uses new path"
fi

# 4. Check scripts updated
echo "✓ Checking scripts updated..."
if grep -q "build/src/xasm++" tests/integration/a2osx/validation_suite.py 2>/dev/null; then
    echo "  ✗ ERROR: validation_suite.py still references old path"
    exit 1
else
    echo "  ✓ validation_suite.py uses new path"
fi

# 5. Check E2E test path
echo "✓ Checking E2E test..."
if grep -q 'build.*bin.*xasm' tests/e2e/apple2/prince_of_persia/test_pop_complete.py 2>/dev/null; then
    echo "  ✓ E2E test uses correct path"
else
    echo "  ✗ ERROR: E2E test path incorrect"
    exit 1
fi

# 6. Run quick test
echo "✓ Running quick assembly test..."
if ./build/bin/xasm++ --version >/dev/null 2>&1; then
    echo "  ✓ Binary executes successfully"
else
    echo "  ✗ ERROR: Binary failed to execute"
    exit 1
fi

echo
echo "=========================================="
echo "All validations passed! ✓"
echo "=========================================="
echo
echo "Ready for:"
echo "  1. git add <modified files>"
echo "  2. git commit -m 'Fix paths for new build structure'"
echo "  3. git push"
echo "  4. Verify GitHub Actions pass"
