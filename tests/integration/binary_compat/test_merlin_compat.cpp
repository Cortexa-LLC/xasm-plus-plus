// Binary compatibility tests for Merlin syntax
// Compares xasm++ output with vasm6502_merlin reference

#include "framework/binary_compat_test.h"

namespace xasm {
namespace test {

// Test for Merlin syntax binary compatibility
TEST_P(BinaryCompatTest, MerlinBinaryMatch) {
    BINARY_COMPAT_TEST_BODY();
}

// Name generator for parameterized tests
std::string MerlinTestCaseName(const ::testing::TestParamInfo<BinaryCompatTestCase>& info) {
    // Extract test name from source file path
    std::string filename = info.param.source_file;
    size_t last_slash = filename.find_last_of('/');
    if (last_slash != std::string::npos) {
        filename = filename.substr(last_slash + 1);
    }
    // Remove .asm extension
    size_t dot = filename.find_last_of('.');
    if (dot != std::string::npos) {
        filename = filename.substr(0, dot);
    }
    return filename;
}

// Instantiate test with all Merlin 6502 test cases
INSTANTIATE_TEST_SUITE_P(
    Merlin6502,
    BinaryCompatTest,
    ::testing::Values(
        BinaryCompatTestCase{
            "test_sources/merlin/6502/01_basic_ops.asm",
            "merlin",
            "6502",
            "vasm6502_merlin",
            ""
        },
        BinaryCompatTestCase{
            "test_sources/merlin/6502/02_addressing_modes.asm",
            "merlin",
            "6502",
            "vasm6502_merlin",
            ""
        },
        BinaryCompatTestCase{
            "test_sources/merlin/6502/03_data_directives.asm",
            "merlin",
            "6502",
            "vasm6502_merlin",
            ""
        },
        BinaryCompatTestCase{
            "test_sources/merlin/6502/04_macros.asm",
            "merlin",
            "6502",
            "vasm6502_merlin",
            ""
        },
        BinaryCompatTestCase{
            "test_sources/merlin/6502/05_conditionals.asm",
            "merlin",
            "6502",
            "vasm6502_merlin",
            ""
        }
    ),
    MerlinTestCaseName
);

} // namespace test
} // namespace xasm
