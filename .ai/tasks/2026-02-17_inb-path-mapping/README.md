# Path Mapping for .INB Directive - Implementation Summary

## Overview
This task implements a `--path-map` CLI option for the xasm++ assembler to support virtual-to-actual path mapping in `.INB` directives, specifically targeting A2osX assembly code patterns.

## Problem Statement
A2osX assembly code uses virtual paths like `usr/src/shared/x.printf.s` that need to map to actual filesystem paths like `SHARED/X.PRINTF.S.txt`. The existing `-I` option only adds search directories but doesn't transform path components.

## Solution Implemented

### Core Feature
Added `--path-map virtual=actual` CLI option that:
- Transforms virtual paths to actual paths BEFORE file search
- Supports multiple mappings
- Uses longest-prefix-first matching for specificity
- Normalizes path separators for cross-platform compatibility
- Safely bypasses absolute paths

### Example Usage
```bash
# Map A2osX virtual paths to actual filesystem
xasm++ --syntax scmasm \
       --path-map usr/src/shared=SHARED \
       -o program.bin \
       main.s

# Multiple mappings
xasm++ --path-map usr/src/shared=SHARED \
       --path-map usr/include=INCLUDE \
       main.s
```

## Implementation Details

### Files Modified
1. **src/main.cpp**: Added CLI option parsing
2. **include/core/AssemblerContext.h**: Storage declaration
3. **src/core/AssemblerContext.cpp**: Storage implementation
4. **include/directive/FileIncludeUtils.h**: Path resolution declaration
5. **src/directive/FileIncludeUtils.cpp**: Path resolution implementation
6. **src/directive/CoreDirectiveHandlers.cpp**: Integration with .INB directive
7. **tests/unit/CMakeLists.txt**: Test registration
8. **tests/unit/test_inb_path_mapping.cpp**: Comprehensive test suite
9. **tests/e2e/a2osx/**: End-to-end validation

### Key Algorithms

#### Longest-Prefix Matching
```cpp
// Sorts mappings by length (longest first)
// Ensures specific mappings override general ones
// Example: usr/src/shared/lib matches before usr/src/shared
```

#### Path Normalization
```cpp
// Converts backslashes to forward slashes
// Ensures consistent path comparison across platforms
```

#### Safe Absolute Path Handling
```cpp
// Absolute paths bypass mapping
// Prevents unintended system path modifications
```

## Testing

### Unit Tests (12 tests)
- ✅ Simple path substitution
- ✅ Case-insensitive file mapping
- ✅ Multiple path mappings
- ✅ Subdirectory handling
- ✅ Longest prefix matching
- ✅ Path separator normalization
- ✅ Error handling (file not found)
- ✅ Empty path mapping
- ✅ Absolute path safety
- ✅ Integration with -I option
- ✅ A2osX-specific patterns

### End-to-End Test
- Real A2osX scenario: `usr/src/shared/printf.s → SHARED/PRINTF.S.txt`
- Validates full pipeline: CLI → mapping → file search → assembly
- Binary output verified

### Test Results
```
100% tests passed, 0 tests failed out of 1688
```

## Quality Metrics

- **Build Status**: Clean, 0 warnings
- **Test Coverage**: 12 new tests, all passing
- **Code Quality**: Follows existing patterns, well-documented
- **Performance**: O(n) lookup where n = number of mappings
- **Backward Compatibility**: No breaking changes

## Design Decisions

### Why Apply Mapping BEFORE Include Search?
- Clean separation of concerns
- Path mapping = path transformation
- Include search = file location
- Allows both features to work independently

### Why Longest-Prefix Matching?
- Allows fine-grained control
- More specific mappings override general ones
- Example: `usr/src/shared/lib` can differ from `usr/src/shared`

### Why Normalize Path Separators?
- Cross-platform compatibility
- Windows uses `\`, Unix uses `/`
- Internal consistency simplifies comparison

### Why Skip Absolute Paths?
- Safety: Don't modify system paths
- Predictability: Absolute paths are already exact
- Simplicity: No edge cases with absolute resolution

## Documentation

- ✅ Doxygen comments on all public APIs
- ✅ Inline comments explain complex logic
- ✅ Work log documents implementation journey
- ✅ Acceptance criteria fully documented
- ✅ This README summarizes the feature

## Future Enhancements (Not in Scope)

1. **Warning for unused mappings**: Could warn if mapping never matches
2. **Wildcard support**: Could support patterns like `usr/*/shared=SHARED`
3. **Case-sensitive option**: Could add flag to disable case-insensitive search
4. **Mapping file**: Could load mappings from a configuration file

## References

- **Beads Task**: xasm++-pp4r [CLOSED]
- **Work Log**: 20-work-log.md
- **Acceptance**: 40-acceptance.md
- **Tests**: tests/unit/test_inb_path_mapping.cpp
- **E2E Test**: tests/e2e/a2osx/

## Completion Date
2026-02-17

## Status
✅ COMPLETE - Ready for code review and merge
