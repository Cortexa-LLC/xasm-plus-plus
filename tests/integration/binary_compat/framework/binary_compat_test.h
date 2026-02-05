// Binary compatibility test framework
// Shared infrastructure for comparing xasm++ output with reference assemblers

#ifndef XASM_BINARY_COMPAT_TEST_H
#define XASM_BINARY_COMPAT_TEST_H

#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

namespace xasm {
namespace test {

struct BinaryCompatTestCase {
  std::string source_file;
  std::string syntax;
  std::string cpu;
  std::string vasm_binary;
  std::string vasm_flags;
};

class BinaryCompatTest : public ::testing::TestWithParam<BinaryCompatTestCase> {
protected:
  std::string temp_output_path;

  void SetUp() override {
    // Create temp output path
    temp_output_path = "/tmp/xasm_test_output.bin";
  }

  void TearDown() override {
    // Clean up temp file
    std::remove(temp_output_path.c_str());
  }

  // Read binary file into vector
  std::vector<uint8_t> ReadBinaryFile(const std::string &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
      return {};
    }
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
                                std::istreambuf_iterator<char>());
  }

  // Compare xasm++ output with vasm reference
  void CompareWithReference(const BinaryCompatTestCase &test_case) {
    // Build the xasm++ command (using CMake-provided path)
    std::string xasm_path = XASM_BINARY_PATH;
    std::string test_source_dir = TEST_SOURCE_DIR;

    std::string xasm_cmd = xasm_path +
                           " "
                           "--syntax=" +
                           test_case.syntax +
                           " "
                           "--cpu=" +
                           test_case.cpu +
                           " "
                           "-o " +
                           temp_output_path + " " + test_source_dir + "/" +
                           test_case.source_file;

    // Run xasm++
    int result = std::system(xasm_cmd.c_str());
    ASSERT_EQ(result, 0) << "xasm++ failed to assemble "
                         << test_case.source_file;

    // Read xasm++ output
    auto xasm_binary = ReadBinaryFile(temp_output_path);
    ASSERT_FALSE(xasm_binary.empty()) << "xasm++ produced no output";

    // Read vasm reference (golden file)
    std::string source_filename = test_case.source_file.substr(
        test_case.source_file.find_last_of('/') + 1);
    source_filename = source_filename.substr(0, source_filename.length() - 4) +
                      ".bin"; // Replace .asm with .bin

    std::string golden_path = std::string(TEST_SOURCE_DIR) + "/golden/" +
                              test_case.syntax + "/" + test_case.cpu + "/" +
                              source_filename;

    auto vasm_binary = ReadBinaryFile(golden_path);
    ASSERT_FALSE(vasm_binary.empty())
        << "Golden reference file not found: " << golden_path;

    // Compare byte-by-byte
    ASSERT_EQ(xasm_binary.size(), vasm_binary.size())
        << "Binary size mismatch: xasm++=" << xasm_binary.size()
        << " vasm=" << vasm_binary.size();

    for (size_t i = 0; i < xasm_binary.size(); ++i) {
      ASSERT_EQ(xasm_binary[i], vasm_binary[i])
          << "Byte mismatch at offset " << i
          << ": xasm++=" << static_cast<int>(xasm_binary[i])
          << " vasm=" << static_cast<int>(vasm_binary[i]);
    }
  }
};

// Test body for parameterized tests
#define BINARY_COMPAT_TEST_BODY()                                              \
  auto test_case = GetParam();                                                 \
  CompareWithReference(test_case);

} // namespace test
} // namespace xasm

#endif // XASM_BINARY_COMPAT_TEST_H
