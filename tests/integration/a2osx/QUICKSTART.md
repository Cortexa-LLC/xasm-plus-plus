# A2oSX Validation - Quick Start

## TL;DR

```bash
# Run validation tests
cd tests/integration/a2osx
./validation_suite.py

# Expected: 5/5 tests pass, 6 blocked by known bugs
```

## What Works ✅

```assembly
* These features work perfectly:
        .OP 65C02          ; Processor selection
        .OR $2000          ; Origin address
VALUE   .EQ $42            ; Symbol definition
        LDA #VALUE         ; Immediate mode (with #)
        .AS "Hello"        ; ASCII string
        .HS 48656C6C6F     ; Hex data
        .DA #$01,#$02      ; Data bytes
```

## What Doesn't ❌

```assembly
* These features fail (see bug reports):

VALUE   .EQ $80
        STA VALUE          ; ❌ BUG-001: Symbol without # fails

BUFFER  .BS 256            ; ❌ BUG-002: .BS misinterpreted

        LDA #<MESSAGE      ; ❌ BUG-003: Expression operators
        LDA #>MESSAGE      ; ❌ BUG-003: Not supported
        STA ZPPTR+1        ; ❌ BUG-003: Offset arithmetic
```

## Files

```
README.md          → Full documentation (START HERE)
SUMMARY.md         → Executive summary
validation_suite.py → Automated tests
BUG-00*.md         → Detailed bug reports
test_*.S           → Test source files
```

## Commands

```bash
# Basic validation
./validation_suite.py

# Include blocked tests (will show failures)
./validation_suite.py --all

# Verbose output
./validation_suite.py -v

# Manual test
../../../build/bin/xasm++ --syntax scmasm --cpu 65c02 \
  test_minimal.S -o test.bin
```

## Status

⚠️ **NOT READY for A2oSX**

Need to fix:
1. BUG-001 (P0) - Symbol resolution → CRITICAL BLOCKER
2. BUG-002 (P1) - .BS directive → HIGH PRIORITY  
3. BUG-003 (P1) - Expression support → HIGH PRIORITY

After fixes → Re-run validation → Test real A2oSX files

## Next Steps

1. Read bug reports (BUG-001, BUG-002, BUG-003)
2. Fix bugs (suggested solutions provided)
3. Re-run: `./validation_suite.py --all`
4. Should see 11/11 tests pass
5. Test with real A2oSX source files
6. Update A2oSX build system

## Questions?

See `README.md` for comprehensive documentation.
