// .INB Path Mapping Tests
// Tests for --path-map CLI option that substitutes virtual paths with actual
// paths

#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/scmasm_syntax.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <map>

using namespace xasm;

// ============================================================================
// Test Fixture
// ============================================================================

class InbPathMappingTest : public ::testing::Test {
protected:
  void SetUp() override {
    parser = std::make_unique<ScmasmSyntaxParser>();
    cpu = std::make_unique<Cpu6502>();
    parser->SetCpu(cpu.get());
    section = Section();
    symbols = ConcreteSymbolTable();
  }

  void TearDown() override {
    // Cleanup test directories
    std::filesystem::remove_all("test_mapping");
    std::filesystem::remove_all("SHARED");
  }

  std::unique_ptr<ScmasmSyntaxParser> parser;
  std::unique_ptr<Cpu6502> cpu;
  Section section;
  ConcreteSymbolTable symbols;
};

// ============================================================================
// Basic Path Mapping Tests
// ============================================================================

TEST_F(InbPathMappingTest, SimplePathSubstitution) {
  // Create actual directory structure
  std::filesystem::create_directories("SHARED");
  std::string actual_file = "SHARED/X.PRINTF.S.txt";
  std::ofstream f(actual_file);
  f << "PRINTF .EQ $1234\n";
  f.close();

  // Set path mapping: usr/src/shared → SHARED
  std::map<std::string, std::string> path_mappings;
  path_mappings["usr/src/shared"] = "SHARED";
  parser->SetPathMappings(path_mappings);

  // Source references virtual path
  std::string source = R"(
        .OR $0800
        .INB usr/src/shared/X.PRINTF.S.txt
        LDA #>PRINTF
)";

  parser->Parse(source, section, symbols);

  // Should resolve PRINTF symbol from mapped file
  EXPECT_TRUE(symbols.IsDefined("PRINTF"));
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("PRINTF", value));
  EXPECT_EQ(value, 0x1234);
}

TEST_F(InbPathMappingTest, CaseInsensitiveFileMapping) {
  // A2osX uses: usr/src/shared/x.printf.s (lowercase in .INB)
  // Maps to: SHARED/X.PRINTF.S.txt (uppercase on disk)

  std::filesystem::create_directories("SHARED");
  std::string actual_file = "SHARED/X.PRINTF.S.txt";
  std::ofstream f(actual_file);
  f << "PRINT .EQ $5678\n";
  f.close();

  std::map<std::string, std::string> path_mappings;
  path_mappings["usr/src/shared"] = "SHARED";
  parser->SetPathMappings(path_mappings);

  // Use lowercase filename (as A2osX does)
  std::string source = R"(
        .OR $0800
        .INB usr/src/shared/x.printf.s
)";

  parser->Parse(source, section, symbols);

  // Should find file despite case mismatch (platform-dependent)
  // On case-insensitive filesystems (macOS, Windows), this works
  // On case-sensitive filesystems (Linux), would need actual case match
  EXPECT_TRUE(symbols.IsDefined("PRINT"));
}

TEST_F(InbPathMappingTest, MultiplePathMappings) {
  // Create multiple mapped directories
  std::filesystem::create_directories("SHARED");
  std::filesystem::create_directories("LIB");

  std::string shared_file = "SHARED/util.s";
  std::ofstream f1(shared_file);
  f1 << "UTIL .EQ $11\n";
  f1.close();

  std::string lib_file = "LIB/math.s";
  std::ofstream f2(lib_file);
  f2 << "MATH .EQ $22\n";
  f2.close();

  std::map<std::string, std::string> path_mappings;
  path_mappings["usr/src/shared"] = "SHARED";
  path_mappings["usr/lib"] = "LIB";
  parser->SetPathMappings(path_mappings);

  std::string source = R"(
        .OR $0800
        .INB usr/src/shared/util.s
        .INB usr/lib/math.s
)";

  parser->Parse(source, section, symbols);

  EXPECT_TRUE(symbols.IsDefined("UTIL"));
  EXPECT_TRUE(symbols.IsDefined("MATH"));

  int64_t util_val, math_val;
  EXPECT_TRUE(symbols.Lookup("UTIL", util_val));
  EXPECT_TRUE(symbols.Lookup("MATH", math_val));
  EXPECT_EQ(util_val, 0x11);
  EXPECT_EQ(math_val, 0x22);

  // Cleanup
  std::filesystem::remove_all("LIB");
}

TEST_F(InbPathMappingTest, MappingWithSubdirectories) {
  // Test mapping with nested directories
  std::filesystem::create_directories("SHARED/drivers");
  std::string driver_file = "SHARED/drivers/keyboard.s";
  std::ofstream f(driver_file);
  f << "KBD .EQ $C000\n";
  f.close();

  std::map<std::string, std::string> path_mappings;
  path_mappings["usr/src/shared"] = "SHARED";
  parser->SetPathMappings(path_mappings);

  std::string source = R"(
        .OR $0800
        .INB usr/src/shared/drivers/keyboard.s
)";

  parser->Parse(source, section, symbols);

  EXPECT_TRUE(symbols.IsDefined("KBD"));
}

TEST_F(InbPathMappingTest, NoMappingUsesOriginalPath) {
  // Without path mapping, should use path as-is
  std::filesystem::create_directories("usr/src/shared");
  std::string actual_file = "usr/src/shared/file.s";
  std::ofstream f(actual_file);
  f << "VALUE .EQ $99\n";
  f.close();

  // No path mappings set
  std::string source = R"(
        .OR $0800
        .INB usr/src/shared/file.s
)";

  parser->Parse(source, section, symbols);

  EXPECT_TRUE(symbols.IsDefined("VALUE"));

  // Cleanup
  std::filesystem::remove_all("usr");
}

TEST_F(InbPathMappingTest, PathMappingWithIncludePaths) {
  // Path mapping should work in combination with include paths
  std::filesystem::create_directories("project/SHARED");
  std::string mapped_file = "project/SHARED/shared.s";
  std::ofstream f(mapped_file);
  f << "COMBINED .EQ $ABCD\n";
  f.close();

  // Set both path mapping and include paths
  std::map<std::string, std::string> path_mappings;
  path_mappings["usr/src/shared"] = "SHARED";
  parser->SetPathMappings(path_mappings);

  std::vector<std::string> include_paths = {"project"};
  parser->SetIncludePaths(include_paths);

  std::string source = R"(
        .OR $0800
        .INB usr/src/shared/shared.s
)";

  parser->Parse(source, section, symbols);

  EXPECT_TRUE(symbols.IsDefined("COMBINED"));

  // Cleanup
  std::filesystem::remove_all("project");
}

TEST_F(InbPathMappingTest, LongestPrefixMatching) {
  // Test that longest matching prefix is used
  std::filesystem::create_directories("SHARED1");
  std::filesystem::create_directories("SHARED2");

  std::string file1 = "SHARED1/file.s";
  std::ofstream f1(file1);
  f1 << "SHORT .EQ $11\n";
  f1.close();

  std::string file2 = "SHARED2/file.s";
  std::ofstream f2(file2);
  f2 << "LONG .EQ $22\n";
  f2.close();

  std::map<std::string, std::string> path_mappings;
  path_mappings["usr/src"] = "SHARED1";
  path_mappings["usr/src/shared"] = "SHARED2"; // More specific
  parser->SetPathMappings(path_mappings);

  std::string source = R"(
        .OR $0800
        .INB usr/src/shared/file.s
)";

  parser->Parse(source, section, symbols);

  // Should use more specific mapping (SHARED2)
  EXPECT_TRUE(symbols.IsDefined("LONG"));
  EXPECT_FALSE(symbols.IsDefined("SHORT"));

  // Cleanup
  std::filesystem::remove_all("SHARED1");
  std::filesystem::remove_all("SHARED2");
}

TEST_F(InbPathMappingTest, PathSeparatorNormalization) {
  // Test that both forward and back slashes work
  std::filesystem::create_directories("SHARED");
  std::string actual_file = "SHARED/file.s";
  std::ofstream f(actual_file);
  f << "NORM .EQ $77\n";
  f.close();

  std::map<std::string, std::string> path_mappings;
  // Use backslash in mapping (Windows-style)
  path_mappings["usr\\src\\shared"] = "SHARED";
  parser->SetPathMappings(path_mappings);

  // Use forward slash in source (Unix-style)
  std::string source = R"(
        .OR $0800
        .INB usr/src/shared/file.s
)";

  parser->Parse(source, section, symbols);

  // Should normalize and match despite different separators
  EXPECT_TRUE(symbols.IsDefined("NORM"));
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(InbPathMappingTest, MappedFileNotFound) {
  // Mapping exists but target file doesn't
  std::filesystem::create_directories("SHARED");

  std::map<std::string, std::string> path_mappings;
  path_mappings["usr/src/shared"] = "SHARED";
  parser->SetPathMappings(path_mappings);

  std::string source = R"(
        .OR $0800
        .INB usr/src/shared/nonexistent.s
)";

  // Should throw error with mapped path in message
  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

TEST_F(InbPathMappingTest, EmptyPathMapping) {
  // Empty mapping should be handled gracefully
  std::map<std::string, std::string> path_mappings;
  path_mappings[""] = "SHARED";
  parser->SetPathMappings(path_mappings);

  std::filesystem::create_directories("SHARED");
  std::string file = "SHARED/file.s";
  std::ofstream f(file);
  f << "EMPTY .EQ $00\n";
  f.close();

  std::string source = R"(
        .OR $0800
        .INB file.s
)";

  parser->Parse(source, section, symbols);
  EXPECT_TRUE(symbols.IsDefined("EMPTY"));
}

TEST_F(InbPathMappingTest, AbsolutePathIgnoresMappings) {
  // Absolute paths should not be affected by mappings
  std::filesystem::create_directories("test_mapping");
  std::filesystem::path abs_file =
      std::filesystem::absolute("test_mapping/absolute.s");
  std::ofstream f(abs_file);
  f << "ABSOLUTE .EQ $FF\n";
  f.close();

  std::map<std::string, std::string> path_mappings;
  path_mappings["test_mapping"] = "SHOULD_NOT_MATCH";
  parser->SetPathMappings(path_mappings);

  std::string source =
      "        .OR $0800\n        .INB " + abs_file.string() + "\n";

  parser->Parse(source, section, symbols);

  EXPECT_TRUE(symbols.IsDefined("ABSOLUTE"));
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(InbPathMappingTest, A2osXStyleMapping) {
  // Real-world A2osX scenario:
  // Source: usr/src/shared/x.printf.s
  // Disk: SHARED/X.PRINTF.S.txt

  std::filesystem::create_directories("SHARED");
  std::string printf_file = "SHARED/X.PRINTF.S.txt";
  std::ofstream f1(printf_file);
  f1 << "; Printf implementation\n";
  f1 << "PRINTF_ENTRY .EQ $2000\n";
  f1.close();

  std::string str_file = "SHARED/X.STRING.S.txt";
  std::ofstream f2(str_file);
  f2 << "; String utilities\n";
  f2 << "STRLEN .EQ $2100\n";
  f2.close();

  std::map<std::string, std::string> path_mappings;
  path_mappings["usr/src/shared"] = "SHARED";
  parser->SetPathMappings(path_mappings);

  std::string source = R"(
        .OR $0800
* Include shared libraries
        .INB usr/src/shared/X.PRINTF.S.txt
        .INB usr/src/shared/X.STRING.S.txt
        
START   LDA #<PRINTF_ENTRY
        LDX #<STRLEN
)";

  parser->Parse(source, section, symbols);

  EXPECT_TRUE(symbols.IsDefined("PRINTF_ENTRY"));
  EXPECT_TRUE(symbols.IsDefined("STRLEN"));

  int64_t printf_addr, strlen_addr;
  EXPECT_TRUE(symbols.Lookup("PRINTF_ENTRY", printf_addr));
  EXPECT_TRUE(symbols.Lookup("STRLEN", strlen_addr));
  EXPECT_EQ(printf_addr, 0x2000);
  EXPECT_EQ(strlen_addr, 0x2100);
}
