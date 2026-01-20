# End-to-End Tests

E2E tests validate xasm++ with complete real-world programs.

## Test Programs

### Prince of Persia (Apple II)
- **Path**: `apple2/prince_of_persia/`
- **CPU**: 6502/65C02
- **Syntax**: Merlin
- **Output**: DOS 3.3 binary + disk image
- **Validation**: AppleWin/MAME emulator
- **Phase**: 6 (requires Merlin syntax)

### Zaxxon (CoCo)
- **Path**: `coco/zaxxon/`
- **CPU**: 6809
- **Syntax**: EDTASM
- **Output**: DECB format
- **Validation**: MAME CoCo3 emulator
- **Phase**: 3

## Running E2E Tests

```bash
cd tests/e2e

# Run all E2E tests
make test

# Run specific test
make apple2-test
make coco-test

# With emulator validation
RUN_EMULATOR=1 make test
```

## Structure

```
e2e/
├── apple2/
│   └── prince_of_persia/
│       ├── src/                # Source files
│       ├── expected/           # Golden binaries
│       ├── Makefile
│       └── validate.sh         # Emulator validation
└── coco/
    └── zaxxon/
        ├── src/zaxxon.asm
        ├── expected/
        ├── Makefile
        └── validate.sh
```

## Validation Criteria

### Assembly Success
- ✓ All source files assemble without errors
- ✓ Output matches expected binary size
- ✓ Byte-for-byte match with golden binary

### Emulator Validation
- ✓ Game loads without errors
- ✓ Title screen displays correctly
- ✓ All screens render properly
- ✓ Gameplay functions correctly

## Adding New E2E Tests

1. Create directory structure
2. Add source files
3. Generate golden binary with vasm
4. Create Makefile
5. Create validation script
6. Document in this README

## Note

E2E tests require:
- Large source files (can be MB+)
- Emulators installed (AppleWin, MAME)
- Longer execution time
- May be skipped in quick CI runs
