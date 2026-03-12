# Contract: Add Missing CLI Options

**Task ID:** xasm-7yy
**Priority:** P1
**Working Directory:** /Users/bryanw/Projects/Vintage/tools/xasm++
**Model:** claude-sonnet-4-6

## Problem

xasm++ is missing several standard cross-assembler CLI options that users and build systems expect.

## Requirements

### 1. Rename `--include-path` → `--include`
- Keep `-I` short alias
- `--include-path` was the old name; `--include` is the standard convention

### 2. New Options (all priorities)

| Option | Description |
|--------|-------------|
| `-D NAME[=VALUE]` / `--define` | Pre-define symbol as equate. NAME alone = 1. Can repeat. |
| `--warn LEVEL` / `-W0` `-W1` `-W2` `-W3` | Warning level (0=none,1=default,2=extra,3=all) |
| `--no-warn` | Suppress all warnings |
| `--werror` | Treat warnings as errors |
| `-q` / `--quiet` | Suppress success/info output; only print errors |
| `--verbose` | Show pass count, included file names |
| `--org ADDR` | Override origin address (hex 0xNNNN or decimal) |
| `--format FORMAT` | Output format: `binary` (default), `ihex`, `srec` |
| `--max-passes N` | Override 50-pass default limit |
| `--label-map FILE` | Write ADDR NAME label map file for debuggers |

## Files to Modify

1. `include/xasm++/cli/command_line_options.h` — add new fields
2. `src/cli_parser.cpp` — rename `--include-path`, add all new options
3. `include/xasm++/assembler.h` — add `SetMaxPasses(int)` declaration
4. `src/core/assembler.cpp` — add `max_passes_` field, use instead of `MAX_PASSES` constant
5. `src/main.cpp` — wire up all new options

## Key Implementation Notes

- `symbols.Define(name, SymbolType::Equate, make_shared<LiteralExpr>(value))` for `-D`
- `#include "xasm++/expression.h"` has `LiteralExpr`
- `IntelHexWriter` / `SrecWriter` use `Write(sections, ostream)` — open file, then call
- `Section::org` is a public `uint64_t` field — set directly after `Parse()`
- For `--org` parsing: accept `0x`-prefixed hex or plain decimal via `std::stoull`
- For `--label-map`: iterate `symbols.GetAllSymbols()`, sort by address, write `$ADDR LABEL\n`
- Warning flags: store in opts for future use; no warning emission infrastructure yet
- `AssemblerResult::pass_count` is available for `--verbose` output

## Acceptance Criteria

```bash
# Must all pass:
cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build
ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build --output-on-failure
xasm++ --help             # shows all new options
xasm++ -D FOO=42 input.s  # FOO pre-defined as 42
xasm++ --quiet input.s    # no output on success
xasm++ --verbose input.s  # shows "Assembly completed in N passes"
xasm++ --format ihex -o out.hex input.s  # Intel HEX output
xasm++ --max-passes 10 input.s           # respects limit
xasm++ --org 0x800 input.s               # overrides ORG
```
