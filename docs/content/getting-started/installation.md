---
sidebar_position: 1
---

# Installation

This guide will help you build and install xasm++ on your system.

## Requirements

Before building xasm++, ensure you have the following:

- **C++ Compiler:** GCC 9+, Clang 10+, or MSVC 2019+
- **CMake:** 3.20 or later
- **Git:** 2.20 or later
- **Google Test:** Automatically fetched by CMake

## Building from Source

### 1. Clone the Repository

```bash
git clone https://github.com/USERNAME/xasm++.git
cd xasm++
```

> **Note:** Update the URL with the actual repository location.

### 2. Initialize Submodules

xasm++ uses the ai-pack framework as a submodule:

```bash
git submodule update --init --recursive
```

### 3. Configure and Build

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build (use -j for parallel builds)
make -j8
```

### 4. Run Tests

Verify your build by running the test suite:

```bash
# Run all tests
ctest

# Or run specific test suites
./tests/unit/test_assembler  # 42 assembler tests
./tests/unit/test_cpu6502    # 155 CPU tests
```

All 197 tests should pass.

## Build Output

After a successful build, you'll find:

- `build/src/xasm++` - Main assembler executable (in development)
- `build/tests/unit/test_assembler` - Assembler unit tests
- `build/tests/unit/test_cpu6502` - 6502 CPU plugin tests

## Platform-Specific Notes

### macOS

If you're using Homebrew, you can install dependencies:

```bash
brew install cmake
```

### Linux

On Debian/Ubuntu systems:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
```

On Fedora/RHEL:

```bash
sudo dnf install gcc-c++ cmake git
```

### Windows

On Windows, you can use:
- Visual Studio 2019 or later with C++ development tools
- CMake (install from [cmake.org](https://cmake.org/download/))
- Git for Windows

Build using CMake GUI or from Visual Studio Developer Command Prompt:

```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Next Steps

Now that you have xasm++ built, check out:
- [Quick Start Guide](quick-start.md) - Basic usage examples
- [API Usage](../api/assembler.md) - Using xasm++ in your code
- [Architecture Overview](../architecture/overview.md) - Understanding the design

## Troubleshooting

### CMake Can't Find Compiler

Make sure your C++ compiler is in your PATH. For GCC/Clang:

```bash
which g++
which clang++
```

### Google Test Download Fails

If CMake fails to fetch Google Test automatically, ensure you have a working internet connection and Git is properly installed.

### Tests Fail

If tests fail, please:
1. Check that you're using a supported compiler version
2. Ensure all submodules are initialized: `git submodule update --init --recursive`
3. Try a clean build: `rm -rf build && mkdir build && cd build && cmake .. && make`
4. Report the issue on [GitHub Issues](https://github.com/USERNAME/xasm++/issues)
