/**
 * @file test_z80_directive_constants.cpp
 * @brief Tests for Z80 directive constants usage
 *
 * Verifies that EdtasmM80PlusPlusSyntaxParser uses directive name constants
 * instead of magic strings in directive registry.
 */

#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/directives/common_directives.h"
#include "xasm++/directives/z80_directives.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;
using namespace xasm::CommonDirectives;
using namespace xasm::Z80Directives;

/**
 * Test that directive constants are defined correctly
 * and can be used in directive registration.
 */
TEST(Z80DirectiveConstantsTest, CommonDirectivesAreDefined) {
  // Verify common directive constants exist and have correct values
  EXPECT_STREQ(kORG, "ORG");
  EXPECT_STREQ(kEQU, "EQU");
  EXPECT_STREQ(kSET, "SET");
  EXPECT_STREQ(kDEFL, "DEFL");
  EXPECT_STREQ(kEQUALS, "=");
  EXPECT_STREQ(kDB, "DB");
  EXPECT_STREQ(kDEFB, "DEFB");
  EXPECT_STREQ(kBYTE, "BYTE");
  EXPECT_STREQ(kDM, "DM");
  EXPECT_STREQ(kDEFM, "DEFM");
  EXPECT_STREQ(kDW, "DW");
  EXPECT_STREQ(kDEFW, "DEFW");
  EXPECT_STREQ(kWORD, "WORD");
  EXPECT_STREQ(kDS, "DS");
  EXPECT_STREQ(kDEFS, "DEFS");
  EXPECT_STREQ(kDSPACE, "DSPACE");
  EXPECT_STREQ(kBLOCK, "BLOCK");
  EXPECT_STREQ(kPUBLIC, "PUBLIC");
  EXPECT_STREQ(kGLOBAL, "GLOBAL");
  EXPECT_STREQ(kENTRY, "ENTRY");
  EXPECT_STREQ(kEXTERN, "EXTERN");
  EXPECT_STREQ(kEXTRN, "EXTRN");
  EXPECT_STREQ(kEXT, "EXT");
  EXPECT_STREQ(kIF, "IF");
  EXPECT_STREQ(kELSE, "ELSE");
  EXPECT_STREQ(kENDIF, "ENDIF");
  EXPECT_STREQ(kMACRO, "MACRO");
  EXPECT_STREQ(kENDM, "ENDM");
  EXPECT_STREQ(kREPT, "REPT");
  EXPECT_STREQ(kEND, "END");
}

/**
 * Test that Z80-specific directive constants are defined correctly.
 */
TEST(Z80DirectiveConstantsTest, Z80DirectivesAreDefined) {
  // Verify Z80 directive constants exist and have correct values
  EXPECT_STREQ(kCSEG, "CSEG");
  EXPECT_STREQ(kDSEG, "DSEG");
  EXPECT_STREQ(kASEG, "ASEG");
}

/**
 * Functional test: Verify that directives registered with constants
 * work correctly in parsing.
 */
TEST(Z80DirectiveConstantsTest, DirectivesRegisteredWithConstants) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Test each directive to ensure it's registered correctly
  std::string source = "         ORG 8000H\n"
                       "CONST    EQU $FF\n"
                       "VAR      SET 42\n"
                       "VAR      DEFL 43\n"
                       "LABEL    = 100\n"
                       "         DB $01,$02,$03\n"
                       "         DEFB $04\n"
                       "         BYTE $05\n"
                       "         DM $06\n"
                       "         DEFM $07\n"
                       "         DW $1234\n"
                       "         DEFW $5678\n"
                       "         WORD $9ABC\n"
                       "         DS 10\n"
                       "         DEFS 5\n"
                       "         DSPACE 3\n"
                       "         BLOCK 2\n"
                       "START:   NOP\n"
                       "         PUBLIC START\n"
                       "         GLOBAL START\n"
                       "         ENTRY START\n"
                       "         EXTERN FUNC\n"
                       "         EXTRN FUNC2\n"
                       "         EXT FUNC3\n"
                       "         CSEG\n"
                       "         DSEG\n"
                       "         ASEG\n"
                       "         IF 1\n"
                       "         ELSE\n"
                       "         ENDIF\n"
                       "         MACRO TEST\n"
                       "         ENDM\n"
                       "         REPT 5\n"
                       "         END\n";

  // Should parse without errors
  EXPECT_NO_THROW(parser.Parse(source, section, symbols));

  // Verify key symbols were defined
  EXPECT_TRUE(symbols.IsDefined("CONST"));
  EXPECT_TRUE(symbols.IsDefined("VAR"));
  EXPECT_TRUE(symbols.IsDefined("LABEL"));
  EXPECT_TRUE(symbols.IsDefined("START"));
  EXPECT_TRUE(symbols.IsDefined("FUNC"));
  EXPECT_TRUE(symbols.IsDefined("FUNC2"));
  EXPECT_TRUE(symbols.IsDefined("FUNC3"));
}

/**
 * Test that constants provide compile-time checking.
 * If this compiles, it means we're using constants correctly.
 */
TEST(Z80DirectiveConstantsTest, ConstantsProvideCompileTimeChecking) {
  // This test verifies that directive constants can be used
  // in contexts that require compile-time constants
  constexpr const char *korg_directive = CommonDirectives::kORG;
  constexpr const char *kcseg_directive = Z80Directives::kCSEG;

  EXPECT_STREQ(korg_directive, "ORG");
  EXPECT_STREQ(kcseg_directive, "CSEG");
}
