# Convenience Makefile wrapper for CMake build
# This delegates common commands to the build directory

.PHONY: all build test install clean distclean configure help

# Default target
all: build

# Configure CMake (creates build directory)
configure:
	@echo "Configuring CMake..."
	cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build the project
build: configure
	@echo "Building xasm++..."
	cmake --build build -j8

# Run tests
test: build
	@echo "Running tests..."
	cd build && ctest --output-on-failure

# Install xasm++ to /usr/local/bin (requires sudo)
install: build
	@echo "Installing xasm++..."
	$(MAKE) -C build install

# Clean build artifacts (keeps configuration)
clean:
	@echo "Cleaning build artifacts..."
	@if [ -d build ]; then $(MAKE) -C build clean; fi

# Full clean (removes build directory)
distclean:
	@echo "Removing build directory..."
	rm -rf build

# Show available targets
help:
	@echo "xasm++ Makefile targets:"
	@echo "  make           - Build xasm++ (default)"
	@echo "  make build     - Build xasm++"
	@echo "  make test      - Run all tests"
	@echo "  make install   - Install to /usr/local/bin (needs sudo)"
	@echo "  make clean     - Clean build artifacts"
	@echo "  make distclean - Remove build directory"
	@echo "  make configure - Run CMake configuration"
	@echo "  make help      - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build the project"
	@echo "  make test         # Build and test"
	@echo "  sudo make install # Build and install"
