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
  EXPECT_STREQ(ORG, "ORG");
  EXPECT_STREQ(EQU, "EQU");
  EXPECT_STREQ(SET, "SET");
  EXPECT_STREQ(DEFL, "DEFL");
  EXPECT_STREQ(EQUALS, "=");
  EXPECT_STREQ(DB, "DB");
  EXPECT_STREQ(DEFB, "DEFB");
  EXPECT_STREQ(BYTE, "BYTE");
  EXPECT_STREQ(DM, "DM");
  EXPECT_STREQ(DEFM, "DEFM");
  EXPECT_STREQ(DW, "DW");
  EXPECT_STREQ(DEFW, "DEFW");
  EXPECT_STREQ(WORD, "WORD");
  EXPECT_STREQ(DS, "DS");
  EXPECT_STREQ(DEFS, "DEFS");
  EXPECT_STREQ(DSPACE, "DSPACE");
  EXPECT_STREQ(BLOCK, "BLOCK");
  EXPECT_STREQ(PUBLIC, "PUBLIC");
  EXPECT_STREQ(GLOBAL, "GLOBAL");
  EXPECT_STREQ(ENTRY, "ENTRY");
  EXPECT_STREQ(EXTERN, "EXTERN");
  EXPECT_STREQ(EXTRN, "EXTRN");
  EXPECT_STREQ(EXT, "EXT");
  EXPECT_STREQ(IF, "IF");
  EXPECT_STREQ(ELSE, "ELSE");
  EXPECT_STREQ(ENDIF, "ENDIF");
  EXPECT_STREQ(MACRO, "MACRO");
  EXPECT_STREQ(ENDM, "ENDM");
  EXPECT_STREQ(REPT, "REPT");
  EXPECT_STREQ(END, "END");
}

/**
 * Test that Z80-specific directive constants are defined correctly.
 */
TEST(Z80DirectiveConstantsTest, Z80DirectivesAreDefined) {
  // Verify Z80 directive constants exist and have correct values
  EXPECT_STREQ(CSEG, "CSEG");
  EXPECT_STREQ(DSEG, "DSEG");
  EXPECT_STREQ(ASEG, "ASEG");
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
  constexpr const char *org_directive = CommonDirectives::ORG;
  constexpr const char *cseg_directive = Z80Directives::CSEG;

  EXPECT_STREQ(org_directive, "ORG");
  EXPECT_STREQ(cseg_directive, "CSEG");
}
