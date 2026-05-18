# Project-Specific Overrides

This file contains project-specific rules that override or extend the ai-pack framework defaults.

## Language/Technology

- Language: C++20
- Build System: CMake 3.20+
- Test Framework: GoogleTest (GTest/GMock)
- CLI Library: CLI11
- Compiler flags: -Wall -Wextra -Wpedantic -Werror (warnings are errors)

## Coding Standards

- **ABSOLUTE PROHIBITION**: NOLINT, NOSONAR, and any suppression comments are FORBIDDEN.
  Every finding MUST be fixed in code. Suppression is never acceptable.
- Enum constants: kUPPER_CASE style (kPUSH, kPOP, kADC_IMM — not kPush, kPop)
- No inline implementations in headers except trivial getters
- Headers (.h): declarations only; Source (.cpp): implementations

## Testing Requirements

- Coverage target: 99%+ (currently 2197 tests passing)
- Run: `ctest --test-dir build --output-on-failure`
- All tests must pass before any commit

## Build/Deploy

```bash
cmake --build build              # Build (zero warnings = MANDATORY due to -Werror)
ctest --test-dir build --output-on-failure  # Test
sudo cmake --install build       # Install to /usr/local/bin/xasm++
```

## Adding New C++ Files (CMakeLists.txt)

### New source file in `src/`

Add the `.cpp` to the correct library in `src/CMakeLists.txt`:

| Directory | Library target |
|-----------|---------------|
| `src/core/` | `xasm_core` |
| `src/cpu/` | `xasm_cpu` |
| `src/syntax/` | `xasm_syntax` |
| `src/output/` | `xasm_output` |
| `src/` (top-level) | `xasm++` executable |

Headers in `include/xasm++/` do **not** need a CMakeLists.txt entry.

### New test file in `tests/unit/`

Add one line to `tests/unit/CMakeLists.txt` using the `xasm_add_unit_test` helper
(defined in `tests/CMakeLists.txt`):

```cmake
xasm_add_unit_test(test_<name>  LIBS <libraries matching the code under test>)
```

Optional keyword arguments:
- `LIBS lib ...` — link libraries beyond gtest (use the same library as the source file)
- `EXTRA_SOURCES src ...` — additional `.cpp` files compiled into this test binary
- `INCLUDE_DIRS dir ...` — extra private include paths
- `SOURCE src.cpp` — override the primary source file name (default: `<target>.cpp`)

Examples:
```cmake
# New CPU encoder test
xasm_add_unit_test(test_cpu_68000  LIBS xasm_cpu xasm_core)

# Test with two source files
xasm_add_unit_test(test_cpu6502    LIBS xasm_cpu xasm_core
                                   EXTRA_SOURCES test_65816_extended.cpp)
```

## Pre-Commit Validation (Phase 5)

For xasm++ the Phase 5 checklist is:

### 5a — Tests
```bash
cmake --build build && ctest --test-dir build --output-on-failure
```
Both must exit 0.

### 5b — Schema / Contract Validation
N/A — xasm++ has no schema or API contract files.

### 5c — Self-Review
- Re-read acceptance criteria
- `git diff` every line about to be committed
- Zero compiler warnings (enforced by -Werror)
- No NOLINT/NOSONAR suppressions added

## Notes

- Apple //e Prince of Persia validation: `cd ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II && make ASM=xasm XASM=xasm++ CRACKLE=~/Projects/Vintage/tools/snapNcrackle/crackle/Debug/crackle disk525`
- A2osX validation: assemble with `--cpu 65c02 --syntax scmasm`
- clang-tidy quality work ongoing — see `.ai/docs/` for status


## GitHub Integration

This project uses GitHub integration for automated Beads ↔ GitHub synchronization.

### Role Extensions

This project extends the following roles with GitHub integration:
- **Orchestrator**: See [.ai/roles/orchestrator-github-extension.md](.ai/roles/orchestrator-github-extension.md)
- **Engineer**: See [.ai/roles/engineer-github-extension.md](.ai/roles/engineer-github-extension.md)
- **Security**: See [.ai/roles/security-github-extension.md](.ai/roles/security-github-extension.md)

### Setup Required

To complete GitHub integration setup:

1. **Initialize integration:**
   ```bash
   ${AI_PACK_ROOT}/scripts/github-integration.py init
   ```

2. **Configure** `${AI_PACK_ROOT}/.github-integration.yml`:
   - Set repository: `your-org/your-repo`
   - Enable agent triggers
   - Configure role-specific options

3. **Authenticate:**
   ```bash
   gh auth login
   # OR
   export GITHUB_TOKEN="ghp_your_token_here"
   ```

4. **Test:**
   ```bash
   ${AI_PACK_ROOT}/scripts/github-integration.py status
   ```

### Documentation

- [Setup Guide](../.ai-pack/docs/GITHUB-INTEGRATION-SETUP.md)
- [Agent Triggers](../.ai-pack/docs/GITHUB-AGENT-TRIGGERS.md)
- [Work Item Patterns](../.ai-pack/docs/WORK-ITEM-PATTERNS.md)
