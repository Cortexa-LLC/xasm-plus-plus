# xasm++ Testing Infrastructure

This directory contains the comprehensive test suite for xasm++.

## Test Structure

```
tests/
├── unit/               # Unit tests (GoogleTest)
├── integration/        # Integration tests (cross-plugin)
├── regression/         # Regression tests (vasm compatibility)
├── e2e/               # End-to-end tests (real programs)
└── fixtures/          # Shared test data
```

## Test Levels

### Level 1: Unit Tests (`unit/`)
- **Purpose**: Test individual components in isolation
- **Framework**: GoogleTest
- **Coverage Target**: 90%+ for core, 80%+ for plugins
- **Run**: `ctest` or `make test`

### Level 2: Integration Tests (`integration/`)
- **Purpose**: Test interactions between plugins
- **Framework**: GoogleTest
- **Examples**: Syntax + CPU + Output plugins working together
- **Run**: `ctest -R integration`

### Level 3: Regression Tests (`regression/`)
- **Purpose**: Ensure byte-for-byte compatibility with vasm
- **Framework**: Python test runner
- **Examples**: Assembly test files compared against vasm output
- **Run**: `python3 regression/run_regression.py`

### Level 4: E2E Tests (`e2e/`)
- **Purpose**: Validate with real-world programs
- **Examples**: Prince of Persia (Apple II), Zaxxon (CoCo)
- **Validation**: Programs assemble and run in emulators
- **Run**: `make e2e-test` (Phase 2+)

## Running Tests

### Quick Test (Unit Only)
```bash
cd build
make test
```

### All Tests
```bash
cd build
make test                              # Unit + Integration
cd ../tests/regression
python3 run_regression.py              # Regression
cd ../e2e
make test                              # E2E (Phase 2+)
```

### Coverage Report
```bash
cd build
cmake -DENABLE_COVERAGE=ON ..
make
make test
make coverage  # Generates HTML report
```

## Test Organization (Phase 1+)

### Unit Tests
- `test_cpu_6502.cpp` - 6502 CPU plugin
- `test_cpu_6809.cpp` - 6809 CPU plugin
- `test_cpu_z80.cpp` - Z80 CPU plugin
- `test_scmasm_syntax.cpp` - SCMASM syntax
- `test_expression.cpp` - Expression evaluator
- `test_symbol_table.cpp` - Symbol table

### Integration Tests
- `test_6502_assembly.cpp` - Full 6502 assembly pipeline
- `test_multifile.cpp` - Multi-file assembly
- `test_output_formats.cpp` - Output format generation

### Regression Tests
- `scmasm/` - SCMASM syntax tests against vasm
- `edtasm/` - EDTASM syntax tests
- `merlin/` - Merlin syntax tests
- Each directory contains `.asm` files and expected output

### E2E Tests
- `apple2/prince_of_persia/` - Full game assembly
- `coco/zaxxon/` - Full game assembly
- Includes validation scripts and emulator tests

## Test Development Workflow

1. **RED**: Write failing test
2. **GREEN**: Make it pass
3. **REFACTOR**: Improve code
4. Repeat

See `docs/architecture/07-testing-strategy.md` for complete strategy.
