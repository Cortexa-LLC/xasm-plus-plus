# Binary compatibility tests - compare xasm++ with reference assemblers

# Find vasm-ext binaries
set(VASM_EXT_DIR "$ENV{HOME}/Projects/Vintage/tools/vasm-ext")
set(VASM_MERLIN "${VASM_EXT_DIR}/vasm6502_merlin")
set(VASM_SCMASM "${VASM_EXT_DIR}/vasm6502_scmasm")

# Check if vasm binaries exist
if(NOT EXISTS "${VASM_MERLIN}")
    message(WARNING "vasm6502_merlin not found at ${VASM_MERLIN} - binary compat tests will be skipped")
    return()
endif()

if(NOT EXISTS "${VASM_SCMASM}")
    message(WARNING "vasm6502_scmasm not found at ${VASM_SCMASM} - binary compat tests will be skipped")
    return()
endif()

# Create binary compatibility test executable
add_executable(binary_compat_test
    test_merlin_compat.cpp
    test_scmasm_compat.cpp
    test_current_features.cpp
)

target_link_libraries(binary_compat_test
    PRIVATE
        GTest::gtest_main
)

target_include_directories(binary_compat_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/framework
)

# Pass vasm paths and test data to test executable
target_compile_definitions(binary_compat_test
    PRIVATE
        VASM_MERLIN_PATH="${VASM_MERLIN}"
        VASM_SCMASM_PATH="${VASM_SCMASM}"
        XASM_BINARY_PATH="$<TARGET_FILE:xasm++>"
        TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        TEST_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)

# Discover tests
gtest_discover_tests(binary_compat_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
