# A2oSX Build Validation - File Index

## 📖 Documentation (Read These First)

| File | Size | Purpose | Audience |
|------|------|---------|----------|
| **QUICKSTART.md** | 2.0 KB | Quick reference, 5-minute overview | Everyone |
| **SUMMARY.md** | 4.3 KB | Executive summary, key findings | Management, stakeholders |
| **README.md** | 8.6 KB | Comprehensive guide, full details | Developers, A2oSX maintainers |

## 🐛 Bug Reports (Fix These)

| File | Size | Priority | Impact | Fix Time |
|------|------|----------|--------|----------|
| **BUG-001-symbol-addressing.md** | 4.9 KB | P0 CRITICAL | 99% blockage | 1-2 days |
| **BUG-002-bs-directive.md** | 5.9 KB | P1 HIGH | 2.9% (280 uses) | Few hours |
| **BUG-003-expression-support.md** | 8.3 KB | P1 HIGH | 5.7% (550 uses) | 1-2 days |

## 🧪 Test Framework

| File | Size | Type | Description |
|------|------|------|-------------|
| **validation_suite.py** | 11 KB | Executable | Automated test runner (Python 3) |

## ✅ Test Files - Working (5 tests, all pass)

| File | Size | Tests | Expected |
|------|------|-------|----------|
| **test_minimal.S** | 84 B | Basic .OP/.OR | ✅ PASS |
| **test_eq.S** | 131 B | .EQ directive | ✅ PASS |
| **test_data_as.S** | 140 B | .AS (ASCII) | ✅ PASS |
| **test_data_hs.S** | 155 B | .HS (hex) | ✅ PASS |
| **test_data_da.S** | 144 B | .DA (data bytes) | ✅ PASS |

## ❌ Test Files - Blocked (6 tests)

| File | Size | Tests | Blocker |
|------|------|-------|---------|
| **test_symbols.S** | 280 B | Symbol addressing | BUG-001 |
| **test_bs.S** | 240 B | Block storage | BUG-002 |
| **test_expr_lo.S** | 226 B | Low byte `<` | BUG-003 |
| **test_expr_hi.S** | 228 B | High byte `>` | BUG-003 |
| **test_expr_offset.S** | 238 B | Offset `+` | BUG-003 |
| **sample_simple.S** | 787 B | Real A2oSX code | Multiple |

## 📊 Sample Files (Reference)

| File | Size | Description |
|------|------|-------------|
| **sample_simple.S** | 787 B | Simplified A2oSX patterns |
| **sample_medium.S** | 1.7 KB | Medium complexity |
| **sample_complex.S** | 3.6 KB | Full A2oSX patterns |

## 📁 Task Packet (Project Documentation)

| File | Purpose |
|------|---------|
| `.ai/tasks/2026-02-12_a2osx-validation/00-contract.md` | Original requirements |
| `.ai/tasks/2026-02-12_a2osx-validation/10-plan.md` | Implementation plan |
| `.ai/tasks/2026-02-12_a2osx-validation/20-work-log.md` | Detailed work notes |
| `.ai/tasks/2026-02-12_a2osx-validation/40-acceptance.md` | Completion report |

## 🎯 Reading Guide

### First Time? Start Here:
1. **QUICKSTART.md** (2 min) - Get oriented
2. **SUMMARY.md** (5 min) - Understand findings
3. Run validation: `./validation_suite.py`

### Developer? Fix Bugs:
1. **BUG-001** (P0) - Critical blocker
2. **BUG-002** (P1) - High priority
3. **BUG-003** (P1) - High priority
4. Re-run: `./validation_suite.py --all`

### A2oSX Maintainer?
1. **SUMMARY.md** - Current status
2. **README.md** - Migration timeline
3. Wait for bug fixes (estimated 3-5 days)

### Deep Dive?
1. **README.md** - Full documentation
2. **test_*.S** - Test cases
3. **sample_*.S** - Real code patterns
4. **40-acceptance.md** - Complete findings

## 📈 Statistics

- **Total Files:** 20
- **Documentation:** 4 files, 23.4 KB
- **Bug Reports:** 3 files, 19.1 KB
- **Test Framework:** 1 file, 11 KB
- **Test Files:** 11 files, 2.8 KB
- **Sample Files:** 3 files, 6.1 KB

**Total Deliverable:** 62.4 KB of comprehensive validation framework

## 🚀 Quick Commands

```bash
# Run validation
./validation_suite.py

# Verbose output
./validation_suite.py -v

# Include blocked tests
./validation_suite.py --all

# Manual test
../../../build/src/xasm++ --syntax scmasm --cpu 65c02 test_minimal.S -o test.bin

# View binary output
xxd test_minimal.S.bin
```

## ✨ Status Summary

| Category | Count | Pass | Blocked | Status |
|----------|-------|------|---------|--------|
| **Working Tests** | 5 | 5 | 0 | ✅ 100% |
| **Blocked Tests** | 6 | 0 | 6 | ⚠️ 3 bugs |
| **Total Coverage** | 11 | 5 | 6 | 🚧 In progress |

**Overall:** Framework complete, awaiting bug fixes

---

**Created:** 2026-02-12
**Purpose:** A2oSX build validation for xasm++
**Status:** Ready for bug fix phase
**Next:** Fix BUG-001 (P0), then BUG-002/003 (P1)
