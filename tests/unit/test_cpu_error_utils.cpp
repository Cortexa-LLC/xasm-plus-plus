// CPU Error Utilities Tests
// Tests for common error handling patterns extracted from CPU implementations

#include "xasm++/cpu/cpu_error_utils.h"
#include <gtest/gtest.h>
#include <stdexcept>

using namespace xasm;
using namespace xasm::cpu;

// ==============================================================================
// Test Group 1: Unsupported Instruction Error
// ==============================================================================

TEST(CpuErrorUtilsTest, ThrowUnsupportedInstruction_ThrowsInvalidArgument) {
  EXPECT_THROW(
      {
        ThrowUnsupportedInstruction("INVALID");
      },
      std::invalid_argument);
}

TEST(CpuErrorUtilsTest, ThrowUnsupportedInstruction_MessageContainsMnemonic) {
  try {
    ThrowUnsupportedInstruction("XYZ");
    FAIL() << "Expected std::invalid_argument";
  } catch (const std::invalid_argument &e) {
    std::string message(e.what());
    EXPECT_NE(message.find("XYZ"), std::string::npos)
        << "Error message should contain mnemonic 'XYZ'";
    EXPECT_NE(message.find("Unsupported"), std::string::npos)
        << "Error message should contain 'Unsupported'";
  }
}

TEST(CpuErrorUtilsTest, ThrowUnsupportedInstruction_ConsistentFormat) {
  try {
    ThrowUnsupportedInstruction("LDA");
    FAIL() << "Expected std::invalid_argument";
  } catch (const std::invalid_argument &e) {
    std::string message(e.what());
    // Should match format: "Unsupported instruction: LDA"
    EXPECT_EQ(message, "Unsupported instruction: LDA");
  }
}

// ==============================================================================
// Test Group 2: Special Encoding Not Supported Error
// ==============================================================================

TEST(CpuErrorUtilsTest, ThrowSpecialEncodingNotSupported_ThrowsInvalidArgument) {
  EXPECT_THROW(
      {
        ThrowSpecialEncodingNotSupported("JMP");
      },
      std::invalid_argument);
}

TEST(CpuErrorUtilsTest, ThrowSpecialEncodingNotSupported_MessageContainsMnemonic) {
  try {
    ThrowSpecialEncodingNotSupported("JSR");
    FAIL() << "Expected std::invalid_argument";
  } catch (const std::invalid_argument &e) {
    std::string message(e.what());
    EXPECT_NE(message.find("JSR"), std::string::npos)
        << "Error message should contain mnemonic 'JSR'";
    EXPECT_NE(message.find("Special encoding"), std::string::npos)
        << "Error message should contain 'Special encoding'";
  }
}

TEST(CpuErrorUtilsTest, ThrowSpecialEncodingNotSupported_ConsistentFormat) {
  try {
    ThrowSpecialEncodingNotSupported("BRA");
    FAIL() << "Expected std::invalid_argument";
  } catch (const std::invalid_argument &e) {
    std::string message(e.what());
    // Should match format: "Special encoding not supported for instruction: BRA"
    EXPECT_EQ(message, "Special encoding not supported for instruction: BRA");
  }
}

// ==============================================================================
// Test Group 3: Invalid Operand Format Errors
// ==============================================================================

TEST(CpuErrorUtilsTest, ThrowExpectedHexValue_ThrowsRuntimeError) {
  EXPECT_THROW(
      {
        ThrowExpectedHexValue();
      },
      std::runtime_error);
}

TEST(CpuErrorUtilsTest, ThrowExpectedHexValue_MessageDescribesExpectedFormat) {
  try {
    ThrowExpectedHexValue();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string message(e.what());
    EXPECT_NE(message.find("hex"), std::string::npos)
        << "Error message should mention 'hex'";
    EXPECT_NE(message.find("$"), std::string::npos)
        << "Error message should mention '$' prefix";
  }
}

TEST(CpuErrorUtilsTest, ThrowExpectedHexValue_ConsistentFormat) {
  try {
    ThrowExpectedHexValue();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string message(e.what());
    // Should match format from cpu_6502.cpp line 2321
    EXPECT_EQ(message, "Expected hex value starting with $");
  }
}

// ==============================================================================
// Test Group 4: Branch Target Errors
// ==============================================================================

TEST(CpuErrorUtilsTest, ThrowBranchTargetMustBeResolved_ThrowsRuntimeError) {
  EXPECT_THROW(
      {
        ThrowBranchTargetMustBeResolved();
      },
      std::runtime_error);
}

TEST(CpuErrorUtilsTest, ThrowBranchTargetMustBeResolved_MessageDescribesRequirement) {
  try {
    ThrowBranchTargetMustBeResolved();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string message(e.what());
    EXPECT_NE(message.find("Branch"), std::string::npos)
        << "Error message should mention 'Branch'";
    EXPECT_NE(message.find("resolved"), std::string::npos)
        << "Error message should mention 'resolved'";
  }
}

TEST(CpuErrorUtilsTest, ThrowBranchTargetMustBeResolved_ConsistentFormat) {
  try {
    ThrowBranchTargetMustBeResolved();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string message(e.what());
    // Should match format from cpu_6502.cpp line 2340
    EXPECT_EQ(message, "Branch target must be resolved address");
  }
}

// ==============================================================================
// Test Group 5: Multi-Operand Requirement Errors
// ==============================================================================

TEST(CpuErrorUtilsTest, ThrowRequiresTwoOperands_ThrowsRuntimeError) {
  EXPECT_THROW(
      {
        ThrowRequiresTwoOperands("MVN", "srcbank,destbank");
      },
      std::runtime_error);
}

TEST(CpuErrorUtilsTest, ThrowRequiresTwoOperands_MessageContainsMnemonicAndFormat) {
  try {
    ThrowRequiresTwoOperands("MVP", "src,dest");
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string message(e.what());
    EXPECT_NE(message.find("MVP"), std::string::npos)
        << "Error message should contain mnemonic 'MVP'";
    EXPECT_NE(message.find("src,dest"), std::string::npos)
        << "Error message should contain expected format 'src,dest'";
    EXPECT_NE(message.find("two operands"), std::string::npos)
        << "Error message should mention 'two operands'";
  }
}

TEST(CpuErrorUtilsTest, ThrowRequiresTwoOperands_ConsistentFormat) {
  try {
    ThrowRequiresTwoOperands("MVN", "srcbank,destbank");
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string message(e.what());
    // Should match format from cpu_6502.cpp line 2376
    EXPECT_EQ(message, "MVN requires two operands: srcbank,destbank");
  }
}

// ==============================================================================
// Test Group 6: Invalid Value Errors (with nested exception)
// ==============================================================================

TEST(CpuErrorUtilsTest, ThrowInvalidValues_ThrowsRuntimeError) {
  EXPECT_THROW(
      {
        ThrowInvalidValues("MVN", "out of range");
      },
      std::runtime_error);
}

TEST(CpuErrorUtilsTest, ThrowInvalidValues_MessageContainsMnemonicAndReason) {
  try {
    ThrowInvalidValues("MVN", "value too large");
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string message(e.what());
    EXPECT_NE(message.find("MVN"), std::string::npos)
        << "Error message should contain mnemonic 'MVN'";
    EXPECT_NE(message.find("value too large"), std::string::npos)
        << "Error message should contain reason 'value too large'";
    EXPECT_NE(message.find("Invalid"), std::string::npos)
        << "Error message should contain 'Invalid'";
  }
}

TEST(CpuErrorUtilsTest, ThrowInvalidValues_ConsistentFormat) {
  try {
    ThrowInvalidValues("MVN", "parse error");
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string message(e.what());
    // Should match format from cpu_6502.cpp line 2401
    // Format: "Invalid bank values for MVN: parse error"
    EXPECT_EQ(message, "Invalid bank values for MVN: parse error");
  }
}
