# Pre-Push Checklist for xasm++

**Purpose:** Prevent CI failures by catching issues locally before pushing to GitHub.

**When to use:** Run ALL checks before EVERY `git push origin main`

---

## ✅ Mandatory Checks (MUST ALL PASS)

### 1. Build Check
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build
```
**Expected:** Zero errors, zero warnings (project uses `-Werror`)

---

### 2. Test Suite
```bash
cd build && ctest --output-on-failure
```
**Expected:** All tests pass (currently 1688 tests)

---

### 3. Format Check (CI Lint Job)
```bash
find src include tests -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror
```
**Expected:** No violations

**Fix if failing:**
```bash
find src include tests -name '*.cpp' -o -name '*.h' | xargs clang-format -i
git add -u
git commit -m "style: apply clang-format"
```

---

### 4. Git Status Clean
```bash
git status
```
**Expected:** No untracked test files, no modified CMakeLists.txt without corresponding source files

**Common issues:**
- ❌ Test files created but not added to git
- ❌ CMakeLists.txt referencing non-existent files
- ❌ Test executables (test_*) in root directory (should be gitignored)

---

### 5. CMakeLists.txt Sync Check
```bash
# For each add_executable() in tests/unit/CMakeLists.txt, verify file exists
grep -E "add_executable\(test_" tests/unit/CMakeLists.txt | \
  sed 's/add_executable(//' | sed 's/$/\.cpp/' | \
  while read target; do
    file="tests/unit/${target}.cpp"
    if [ ! -f "$file" ]; then
      echo "❌ MISSING: $file (referenced in CMakeLists.txt)"
    fi
  done
```
**Expected:** No missing files

---

### 6. GitIgnore Pattern Check
```bash
# Verify test source files are NOT ignored
git check-ignore tests/unit/test_*.cpp
```
**Expected:** Empty output (files should NOT be ignored)

**If files are ignored:**
- Check `.gitignore` for overly broad patterns
- Pattern `test_*` blocks everything → Change to `/test_*` (root only)

---

### 7. New Files Staged
```bash
# List all new test files
git ls-files --others --exclude-standard tests/unit/test_*.cpp

# If any listed, verify they're staged
git diff --cached --name-only | grep test_
```
**Expected:** All new test files are staged for commit

---

### 8. Coverage Build (Optional but Recommended)
```bash
cmake -B build_coverage -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON
cmake --build build_coverage
cd build_coverage && ctest --output-on-failure
```
**Expected:** Builds and tests pass with coverage instrumentation

---

## 🔍 Quick Pre-Push Script

**Create:** `.ai/scripts/pre-push.sh`

```bash
#!/bin/bash
set -e

echo "🔍 Running pre-push checks..."

# 1. Build
echo "1️⃣ Building..."
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON > /dev/null
cmake --build build

# 2. Tests
echo "2️⃣ Running tests..."
cd build && ctest --output-on-failure -Q
cd ..

# 3. Format check
echo "3️⃣ Checking format..."
if ! find src include tests -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror 2>/dev/null; then
    echo "❌ Format violations detected. Run:"
    echo "   find src include tests -name '*.cpp' -o -name '*.h' | xargs clang-format -i"
    exit 1
fi

# 4. Verify no ignored test files
echo "4️⃣ Checking for ignored test files..."
if git ls-files --others --exclude-standard tests/unit/test_*.cpp | grep -q .; then
    echo "❌ Untracked test files found (may be ignored by .gitignore):"
    git ls-files --others --exclude-standard tests/unit/test_*.cpp
    exit 1
fi

# 5. Verify CMakeLists.txt sync
echo "5️⃣ Verifying CMakeLists.txt sync..."
MISSING=0
grep -E "add_executable\(test_" tests/unit/CMakeLists.txt | \
  sed 's/.*add_executable(//' | sed 's/$//' | \
  while read target; do
    # Extract just the target name (before newline/space)
    target_name=$(echo "$target" | head -1)
    file="tests/unit/${target_name}.cpp"
    if [ ! -f "$file" ]; then
      echo "❌ MISSING: $file"
      MISSING=1
    fi
  done

if [ $MISSING -eq 1 ]; then
    exit 1
fi

echo "✅ All pre-push checks passed!"
echo ""
echo "Safe to push:"
echo "  git push origin main"
```

**Usage:**
```bash
chmod +x .ai/scripts/pre-push.sh
./.ai/scripts/pre-push.sh && git push origin main
```

---

## 🚨 Common Failure Patterns

### Pattern 1: Missing Test Files
**Symptom:** CI fails with `Cannot find source file: test_xyz.cpp`

**Root cause:**
1. Agent created test in existing file (test_scmasm_syntax.cpp)
2. Agent also updated CMakeLists.txt to reference new file
3. New file never created
4. CMakeLists.txt committed without the file

**Prevention:**
- Check #5 above (CMakeLists.txt Sync Check)
- Review `git diff tests/unit/CMakeLists.txt` before committing

---

### Pattern 2: Ignored Test Files
**Symptom:** CI fails with `Cannot find source file`, but file exists locally

**Root cause:**
1. Test file created locally
2. Overly broad .gitignore pattern (e.g., `test_*`) blocks file
3. File never added to git
4. CI can't find file

**Prevention:**
- Check #6 above (GitIgnore Pattern Check)
- Use `/test_*` instead of `test_*` in .gitignore
- Always verify `git add` succeeds without `-f` flag

---

### Pattern 3: Format Violations
**Symptom:** CI lint job fails

**Root cause:**
1. Code written/modified without formatting
2. Pushed without running clang-format

**Prevention:**
- Check #3 above (Format Check)
- Run clang-format before every commit
- Consider pre-commit hook

---

### Pattern 4: Build Warnings → Errors
**Symptom:** CI build fails with `-Werror` treating warnings as errors

**Root cause:**
1. Warning on different platform (macOS vs Linux vs Windows)
2. Local build didn't show warning
3. Different compiler versions

**Prevention:**
- Build with `-Werror` locally (project default)
- Test on multiple platforms if possible
- Review compiler warnings carefully

---

## 🔧 Git Hooks (Optional)

### Pre-Commit Hook
**File:** `.git/hooks/pre-commit`

```bash
#!/bin/bash

# Auto-format staged C++ files
echo "🎨 Formatting staged C++ files..."
git diff --cached --name-only --diff-filter=ACM | \
  grep -E '\.(cpp|h)$' | \
  xargs -r clang-format -i

# Re-stage formatted files
git diff --name-only | \
  grep -E '\.(cpp|h)$' | \
  xargs -r git add

echo "✅ Pre-commit hook complete"
```

**Install:**
```bash
chmod +x .git/hooks/pre-commit
```

---

### Pre-Push Hook
**File:** `.git/hooks/pre-push`

```bash
#!/bin/bash

# Run pre-push checks
if [ -f ".ai/scripts/pre-push.sh" ]; then
    ./.ai/scripts/pre-push.sh
    if [ $? -ne 0 ]; then
        echo "❌ Pre-push checks failed. Push aborted."
        exit 1
    fi
fi
```

**Install:**
```bash
chmod +x .git/hooks/pre-push
```

---

## 📊 CI Status Dashboard

**Check CI before starting new work:**
```bash
gh run list --limit 5
```

**View failed run details:**
```bash
gh run view <run-id>
gh run view <run-id> --log-failed
```

**Re-run failed jobs:**
```bash
gh run rerun <run-id>
```

---

## 🎯 Bottom Line

**The 30-Second Check:**
```bash
# Build, test, format
cmake --build build && \
cd build && ctest -Q && cd .. && \
find src include tests -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror

# If all pass:
git push origin main
```

**Remember:**
- ✅ **Local success = CI success** (when checks match CI workflow)
- ❌ **Skip checks = CI failure** (expensive, time-consuming)
- 🎯 **Always run pre-push script** (automates all checks)

---

**Last Updated:** 2026-02-17
**Maintained By:** Orchestrator (Claude Sonnet 4.5)
