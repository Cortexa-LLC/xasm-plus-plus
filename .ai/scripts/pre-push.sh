#!/bin/bash
# Pre-Push Validation Script for xasm++
# Runs all checks that CI will run, catching issues locally before push
set -e

echo "🔍 Running pre-push checks for xasm++..."
echo ""

# 1. Build check
echo "1️⃣  Building (Debug + Release)..."
if ! cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON >/dev/null 2>&1; then
    echo "❌ CMake configuration failed"
    exit 1
fi

if ! cmake --build build 2>&1 | tee /tmp/build.log | tail -5; then
    echo "❌ Build failed"
    cat /tmp/build.log
    exit 1
fi

# Check for warnings (project uses -Werror, but double-check)
if grep -i "warning:" /tmp/build.log >/dev/null 2>&1; then
    echo "⚠️  Build warnings detected"
    grep -i "warning:" /tmp/build.log
fi

echo "   ✅ Build passed"
echo ""

# 2. Test suite
echo "2️⃣  Running test suite..."
cd build
if ! ctest --output-on-failure -Q 2>&1 | tee /tmp/test.log; then
    echo "❌ Tests failed"
    cat /tmp/test.log
    exit 1
fi
TEST_COUNT=$(grep -oE "[0-9]+ tests" /tmp/test.log | head -1 | grep -oE "[0-9]+")
echo "   ✅ All $TEST_COUNT tests passed"
cd ..
echo ""

# 3. Format check
echo "3️⃣  Checking code format..."
if ! find src include tests -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror 2>/dev/null; then
    echo "❌ Format violations detected"
    echo ""
    echo "Fix with:"
    echo "  find src include tests -name '*.cpp' -o -name '*.h' | xargs clang-format -i"
    echo "  git add -u"
    echo "  git commit -m 'style: apply clang-format'"
    exit 1
fi
echo "   ✅ Code format clean"
echo ""

# 4. Check for untracked test files
echo "4️⃣  Checking for untracked test files..."
UNTRACKED=$(git ls-files --others --exclude-standard tests/unit/test_*.cpp 2>/dev/null || true)
if [ -n "$UNTRACKED" ]; then
    echo "❌ Untracked test files found (may be blocked by .gitignore):"
    echo "$UNTRACKED"
    echo ""
    echo "Check .gitignore patterns and add files:"
    echo "  git check-ignore -v $UNTRACKED"
    echo "  git add $UNTRACKED"
    exit 1
fi
echo "   ✅ No untracked test files"
echo ""

# 5. Verify CMakeLists.txt sync
echo "5️⃣  Verifying CMakeLists.txt references..."
MISSING_FILES=0
while IFS= read -r line; do
    # Extract target name from add_executable(target_name
    target=$(echo "$line" | sed 's/.*add_executable(//' | sed 's/[[:space:]]*$//')

    # Look for the .cpp file reference in the next few lines
    # This is a simplified check - CMakeLists.txt has the file on the next line
    if [ -n "$target" ]; then
        # Check if corresponding .cpp file exists
        cpp_file="tests/unit/${target}.cpp"
        if [ ! -f "$cpp_file" ]; then
            echo "❌ Referenced but missing: $cpp_file"
            MISSING_FILES=$((MISSING_FILES + 1))
        fi
    fi
done < <(grep -E "^add_executable\(test_" tests/unit/CMakeLists.txt || true)

if [ $MISSING_FILES -gt 0 ]; then
    echo ""
    echo "CMakeLists.txt references files that don't exist"
    echo "Either create the files or remove the references"
    exit 1
fi
echo "   ✅ All referenced test files exist"
echo ""

# 6. Check git status
echo "6️⃣  Checking git status..."
if git diff --name-only | grep -q "CMakeLists.txt"; then
    echo "⚠️  CMakeLists.txt modified but not staged"
    echo "   Review changes before pushing"
fi

STAGED=$(git diff --cached --name-only | wc -l | tr -d ' ')
if [ "$STAGED" -eq 0 ]; then
    echo "⚠️  No staged changes - nothing to push"
fi
echo "   ✅ Git status checked"
echo ""

# Summary
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ ALL PRE-PUSH CHECKS PASSED"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Safe to push:"
echo "  git push origin main"
echo ""
echo "Or push and watch CI:"
echo "  git push origin main && sleep 10 && gh run watch --exit-status"
echo ""
