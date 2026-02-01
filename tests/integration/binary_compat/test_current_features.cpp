// Binary compatibility tests for CURRENTLY IMPLEMENTED features
// These tests should all PASS with current xasm++ implementation

#include "framework/binary_compat_test.h"

namespace xasm {
namespace test {

// Test for currently working features
TEST_P(BinaryCompatTest, CurrentFeaturesBinaryMatch) {
    BINARY_COMPAT_TEST_BODY();
}

// Name generator for parameterized tests
std::string CurrentFeaturesTestCaseName(const ::testing::TestParamInfo<BinaryCompatTestCase>& info) {
    std::string filename = info.param.source_file;
    size_t last_slash = filename.find_last_of('/');
    if (last_slash != std::string::npos) {
        filename = filename.substr(last_slash + 1);
    }
    size_t dot = filename.find_last_of('.');
    if (dot != std::string::npos) {
        filename = filename.substr(0, dot);
    }
    // Add syntax prefix for clarity
    return info.param.syntax + "_" + info.param.cpu + "_" + filename;
}

// Tests that work with current implementation
// Only includes test cases that don't use unimplemented features:
// - No SCMASM syntax (not implemented yet)
// - No DA, DCI, INV, FLS, END directives (not implemented yet)
// - No macros (macro system has issues)
INSTANTIATE_TEST_SUITE_P(
    WorkingFeatures,
    BinaryCompatTest,
    ::testing::Values(
        // Merlin 6502: Basic opcodes (PASSING)
        BinaryCompatTestCase{
            "test_sources/merlin/6502/01_basic_ops.asm",
            "merlin",
            "6502",
            "vasm6502_merlin",
            ""
        }
        // Add more test cases here as features are implemented
        // TODO: Add 02_addressing_modes after implementing END directive
        // TODO: Add 03_data_directives after implementing DA, DCI, INV, FLS, END
        // TODO: Add 04_macros after fixing macro system
        // TODO: Add 05_conditionals after implementing END directive
        // TODO: Add SCMASM tests after implementing SCMASM syntax
    ),
    CurrentFeaturesTestCaseName
);

} // namespace test
} // namespace xasm
