// Error Reporting Tests
// Tests for M8 Refactoring: Error messages with file, line, and context

#include "xasm++/syntax/merlin_syntax.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>
#include <regex>

using namespace xasm;

// ============================================================================
// Helper function to check if error contains file:line format
// ============================================================================

bool HasFileLineFormat(const std::string& error_msg) {
    // Match pattern: filename:line: error: message
    // Or simpler: something:number:
    std::regex pattern(R"([^:]+:\d+:)");
    return std::regex_search(error_msg, pattern);
}

// ============================================================================
// Test: Invalid Hex Number Errors
// ============================================================================

TEST(ErrorReportingTest, InvalidHexNumberIncludesLocation) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    try {
        parser.Parse("         DB $", section, symbols);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        EXPECT_TRUE(HasFileLineFormat(error_msg)) 
            << "Error message should include file:line: '" << error_msg << "'";
        EXPECT_TRUE(error_msg.find("Invalid hex number") != std::string::npos)
            << "Error message should describe the problem: '" << error_msg << "'";
    }
}

TEST(ErrorReportingTest, InvalidHexNumberMalformedIncludesLocation) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    try {
        parser.Parse("         DB $XYZ", section, symbols);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        EXPECT_TRUE(HasFileLineFormat(error_msg))
            << "Error message should include file:line: '" << error_msg << "'";
    }
}

// ============================================================================
// Test: Invalid Binary Number Errors
// ============================================================================

TEST(ErrorReportingTest, InvalidBinaryNumberIncludesLocation) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    try {
        parser.Parse("         DB %", section, symbols);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        EXPECT_TRUE(HasFileLineFormat(error_msg))
            << "Error message should include file:line: '" << error_msg << "'";
        EXPECT_TRUE(error_msg.find("Invalid binary number") != std::string::npos)
            << "Error message should describe the problem: '" << error_msg << "'";
    }
}

// ============================================================================
// Test: Undefined Symbol Errors
// ============================================================================

TEST(ErrorReportingTest, UndefinedSymbolIncludesLocation) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    try {
        parser.Parse("         DS UNDEFINED_SYMBOL", section, symbols);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        EXPECT_TRUE(HasFileLineFormat(error_msg))
            << "Error message should include file:line: '" << error_msg << "'";
        EXPECT_TRUE(error_msg.find("Undefined symbol") != std::string::npos)
            << "Error message should describe the problem: '" << error_msg << "'";
    }
}

// ============================================================================
// Test: ORG Directive Errors
// ============================================================================

TEST(ErrorReportingTest, OrgMissingOperandIncludesLocation) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    try {
        parser.Parse("         ORG", section, symbols);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        EXPECT_TRUE(HasFileLineFormat(error_msg))
            << "Error message should include file:line: '" << error_msg << "'";
    }
}

// ============================================================================
// Test: DUM Directive Errors
// ============================================================================

TEST(ErrorReportingTest, DumMissingOperandIncludesLocation) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    try {
        parser.Parse("         DUM", section, symbols);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        EXPECT_TRUE(HasFileLineFormat(error_msg))
            << "Error message should include file:line: '" << error_msg << "'";
    }
}

// ============================================================================
// Test: Conditional Assembly Errors
// ============================================================================

TEST(ErrorReportingTest, ElseWithoutDoIncludesLocation) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    try {
        parser.Parse("         ELSE", section, symbols);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        EXPECT_TRUE(HasFileLineFormat(error_msg))
            << "Error message should include file:line: '" << error_msg << "'";
        EXPECT_TRUE(error_msg.find("ELSE without matching DO") != std::string::npos)
            << "Error message should describe the problem: '" << error_msg << "'";
    }
}

TEST(ErrorReportingTest, FinWithoutDoIncludesLocation) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    try {
        parser.Parse("         FIN", section, symbols);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        EXPECT_TRUE(HasFileLineFormat(error_msg))
            << "Error message should include file:line: '" << error_msg << "'";
    }
}

// ============================================================================
// Test: Multi-line Error Location Accuracy
// ============================================================================

TEST(ErrorReportingTest, ErrorOnLine3ShowsCorrectLine) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    std::string source = 
        "         DB $01\n"
        "         DB $02\n"
        "         DB $\n"     // Error on line 3
        "         DB $04\n";

    try {
        parser.Parse(source, section, symbols);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        EXPECT_TRUE(HasFileLineFormat(error_msg))
            << "Error message should include file:line: '" << error_msg << "'";
        // Check that line 3 is mentioned
        EXPECT_TRUE(error_msg.find(":3:") != std::string::npos ||
                   error_msg.find("line 3") != std::string::npos)
            << "Error should indicate line 3: '" << error_msg << "'";
    }
}

// ============================================================================
// Test: Error Format Consistency
// ============================================================================

TEST(ErrorReportingTest, AllErrorsHaveConsistentFormat) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;

    // Test multiple error types to ensure consistency
    std::vector<std::string> error_cases = {
        "         DB $",        // Invalid hex
        "         DB %",        // Invalid binary
        "         ORG",         // Missing operand
        "         ELSE",        // ELSE without DO
        "         DS UNDEF"     // Undefined symbol
    };

    for (const auto& error_case : error_cases) {
        Section section("test", 0);
        try {
            parser.Parse(error_case, section, symbols);
            FAIL() << "Expected error for: " << error_case;
        } catch (const std::runtime_error& e) {
            std::string error_msg = e.what();
            EXPECT_TRUE(HasFileLineFormat(error_msg))
                << "All errors should have file:line format. Case: " << error_case
                << ", Error: " << error_msg;
        }
    }
}

// ============================================================================
// Test: Filename in Error Messages
// ============================================================================

TEST(ErrorReportingTest, ErrorIncludesFilename) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    try {
        parser.Parse("         DB $", section, symbols);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        // Should have SOME filename (even if it's <stdin> or similar)
        EXPECT_TRUE(HasFileLineFormat(error_msg))
            << "Error should include filename component: '" << error_msg << "'";
    }
}
