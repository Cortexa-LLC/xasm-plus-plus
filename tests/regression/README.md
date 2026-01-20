# Regression Tests

Regression tests ensure byte-for-byte compatibility with vasm-ext output.

## Structure

```
regression/
├── scmasm/             # SCMASM syntax tests
│   ├── basic.asm
│   ├── expected/
│   │   └── basic.bin   # Golden output from vasm
│   └── README.md
├── edtasm/             # EDTASM syntax tests
├── merlin/             # Merlin syntax tests
└── run_regression.py   # Test runner
```

## Running Tests

```bash
# Run all regression tests
python3 run_regression.py

# Run specific syntax tests
python3 run_regression.py scmasm

# Run single test
python3 run_regression.py scmasm/basic

# Verbose output (show byte differences)
python3 run_regression.py --verbose
```

## Adding New Tests

1. Create `.asm` test file in appropriate directory
2. Assemble with vasm-ext to create golden binary
3. Place golden binary in `expected/` subdirectory
4. Run regression test to verify

Example:
```bash
# Create test
echo "    LDA #\$42" > scmasm/newtest.asm
echo "    RTS" >> scmasm/newtest.asm

# Generate golden binary with vasm
vasm6502_oldstyle -Fbin -o scmasm/expected/newtest.bin scmasm/newtest.asm

# Run test
python3 run_regression.py scmasm/newtest
```

## Test Cases (Phase 2+)

### SCMASM Tests
- Basic instructions and addressing modes
- Expressions and operators
- Macro usage
- Local labels
- Forward references

### EDTASM Tests
- 6809 instructions
- Indexed addressing modes
- Directives

### Merlin Tests (Phase 6+)
- 65816 instructions
- Long addressing
- Merlin-specific directives

## Expected vs Actual

- `expected/` - Golden binaries from vasm-ext (checked into git)
- `actual/` - Output from xasm++ (not in git, generated during tests)
