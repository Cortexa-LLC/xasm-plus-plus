// Binary compatibility tests for SCMASM syntax
// Compares xasm++ output with vasm6502_scmasm reference

#include "framework/binary_compat_test.h"

namespace xasm {
namespace test {

// Test for SCMASM syntax binary compatibility
TEST_P(BinaryCompatTest, ScmasmBinaryMatch) { BINARY_COMPAT_TEST_BODY(); }

// Name generator for parameterized tests
std::string
ScmasmTestCaseName(const ::testing::TestParamInfo<BinaryCompatTestCase> &info) {
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

// Instantiate test with SCMASM 6502 test case
// Note: Only one golden binary exists currently (03_data_directives.bin)
// More test cases will be added as golden binaries are generated
INSTANTIATE_TEST_SUITE_P(Scmasm6502, BinaryCompatTest,
                         ::testing::Values(BinaryCompatTestCase{
                             "test_sources/scmasm/6502/03_data_directives.asm",
                             "scmasm", "6502", "vasm6502_scmasm", ""}),
                         ScmasmTestCaseName);

} // namespace test
} // namespace xasm
