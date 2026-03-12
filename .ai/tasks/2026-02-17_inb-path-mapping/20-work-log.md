# Work Log: Path Mapping for .INB Directive

## Session 2026-02-17

### Task Summary
Implement --path-map CLI option for path substitution in .INB directives, targeting A2osX assembly code patterns.

### Implementation Completed

#### 1. Core CLI Implementation
- **File**: `src/main.cpp`
- Added `--path-map` option to CLI11 configuration
- Accepts format: `--path-map virtual/path=actual/path`
- Supports multiple path mappings
- Passes mappings to AssemblerContext

#### 2. Path Mapping Storage
- **File**: `include/core/AssemblerContext.h`, `src/core/AssemblerContext.cpp`
- Added `path_mappings_` member (map of string to string)
- Implemented `add_path_mapping()` method
- Implemented `get_path_mappings()` accessor
- Mappings stored as virtual_path → actual_path

#### 3. Path Resolution Logic
- **File**: `include/directive/FileIncludeUtils.h`, `src/directive/FileIncludeUtils.cpp`
- Added `apply_path_mappings()` static method
- Implements longest-prefix-first matching algorithm
- Normalizes path separators (\ → /)
- Returns mapped path if match found, original path otherwise
- Absolute paths bypass mapping (safety feature)

#### 4. Integration with .INB Directive
- **File**: `src/directive/CoreDirectiveHandlers.cpp`
- Modified `handle_inb()` to use path mappings
- Path mapping applied BEFORE include path search
- Maintains backward compatibility (no mappings = original behavior)

#### 5. Comprehensive Test Suite
- **File**: `tests/unit/test_inb_path_mapping.cpp`
- 12 unit tests covering:
  - Simple path substitution
  - Case-insensitive file mapping
  - Multiple path mappings
  - Subdirectory handling
  - Longest prefix matching
  - Path separator normalization
  - Error handling (file not found)
  - Absolute path safety
  - A2osX-specific use case
- All tests pass ✅

#### 6. End-to-End Validation
- **Directory**: `tests/e2e/a2osx/`
- Created realistic A2osX test scenario
- Demonstrates mapping: `usr/src/shared → SHARED`
- File: `usr/src/shared/printf.s → SHARED/PRINTF.S.txt`
- Assembly successful with path mapping ✅
- Assembly fails without path mapping ✅ (confirms functionality)

### Testing Results
- ✅ All 1688 tests pass (including 12 new path mapping tests)
- ✅ E2E test validates A2osX use case
- ✅ No regressions in existing functionality

### Key Design Decisions

1. **Longest-prefix-first matching**: Allows more specific mappings to override general ones
   - Example: `usr/src/shared/lib` matches before `usr/src/shared`

2. **Path separator normalization**: Handles Windows/Unix differences
   - All paths normalized to forward slashes internally

3. **Absolute path safety**: Absolute paths skip mapping
   - Prevents unintended modifications to system paths

4. **Case-insensitive file search**: Leverages existing SCMASM behavior
   - Maps `printf.s` → finds `PRINTF.S.txt` automatically

5. **Apply mapping BEFORE include search**: Clean separation of concerns
   - Path mapping transforms virtual → actual
   - Include path search finds the actual file

### Verification Steps Completed
- [x] Unit tests written and passing
- [x] E2E test demonstrates A2osX use case
- [x] Full test suite passes (no regressions)
- [x] Code follows existing patterns
- [x] Type hints included (C++ types)
- [x] Docstrings complete (Doxygen comments)
- [x] Error handling implemented

### Example Usage
```bash
# A2osX style mapping
xasm++ --syntax scmasm \
       --path-map usr/src/shared=SHARED \
       -o program.bin \
       main.s

# Multiple mappings
xasm++ --path-map usr/src/shared=SHARED \
       --path-map usr/include=INCLUDE \
       main.s
```

### Files Modified
1. `src/main.cpp` - CLI option
2. `include/core/AssemblerContext.h` - Storage declaration
3. `src/core/AssemblerContext.cpp` - Storage implementation
4. `include/directive/FileIncludeUtils.h` - Resolution declaration
5. `src/directive/FileIncludeUtils.cpp` - Resolution implementation
6. `src/directive/CoreDirectiveHandlers.cpp` - .INB integration
7. `tests/unit/CMakeLists.txt` - Test registration
8. `tests/unit/test_inb_path_mapping.cpp` - New test file
9. `tests/e2e/a2osx/` - E2E test directory (created)

### Next Steps
- Documentation could be added to README or user guide
- Could add warning if path mapping doesn't match any files
- Performance profiling if many mappings used

### Notes
- Implementation is clean and maintainable
- Follows TDD principles (tests written first)
- No breaking changes to existing functionality
- Ready for code review and merge

### Task Completion
- **Beads Task ID**: xasm++-pp4r
- **Status**: CLOSED ✅
- **Completion Date**: 2026-02-17
- All acceptance criteria met
- All tests passing (1688/1688)
- Zero build warnings
- Feature fully functional and documented
