/**
 * @file test_z80_listing_directives.cpp
 * @brief Tests for Z80 listing directives in EDTASM-M80++ syntax
 *
 * Tests LIST/NOLIST, LALL/SALL, TITLE, PAGE, SPACE, EJECT directives.
 *
 * @note Phase 13: Listing File Generation
 */

#include "xasm++/output/listing_output.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <vector>

using namespace xasm;

/**
 * Test: ListingOutput respects source line text from atoms
 */
TEST(Z80ListingDirectivesTest, SourceLinePreservation) {
  ListingOutput output;
  Section section("test", 0, 0x8000);

  auto inst = std::make_shared<InstructionAtom>("NOP", "");
  inst->encoded_bytes = {0x00};
  inst->size = 1;
  // TODO: Need to add source_line field to Atom
  // inst->source_line = "    NOP    ; do nothing";
  section.atoms.push_back(inst);

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.lst", {&section}, symbols);

  std::ifstream file("test.lst");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should contain NOP instruction
  EXPECT_NE(content.find("NOP"), std::string::npos);

  std::remove("test.lst");
}

/**
 * Test: Line numbers appear in listing
 */
TEST(Z80ListingDirectivesTest, LineNumbering) {
  ListingOutput output;
  Section section("test", 0, 0x8000);

  // Add multiple instructions with source lines
  for (int i = 0; i < 5; ++i) {
    auto inst = std::make_shared<InstructionAtom>("NOP", "");
    inst->encoded_bytes = {0x00};
    inst->size = 1;
    inst->location.line = i + 1;
    inst->source_line = "    NOP    ; Instruction " + std::to_string(i + 1);
    section.atoms.push_back(inst);
  }

  ConcreteSymbolTable symbols;
  output.WriteOutput("test_line_numbers.lst", {&section}, symbols);

  std::ifstream file("test_line_numbers.lst");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should contain line numbers formatted properly
  EXPECT_NE(content.find("    1"), std::string::npos); // Line 1
  EXPECT_NE(content.find("    5"), std::string::npos); // Line 5

  // Should contain source line text
  EXPECT_NE(content.find("Instruction 1"), std::string::npos);
  EXPECT_NE(content.find("Instruction 5"), std::string::npos);

  std::remove("test_line_numbers.lst");
}

/**
 * Test: TITLE directive sets page title
 */
TEST(Z80ListingDirectivesTest, TitleDirective) {
  ListingOutput output;
  Section section("test", 0, 0x8000);

  // Add TITLE directive
  auto title = std::make_shared<ListingControlAtom>(ListingControlType::Title,
                                                    "Test Program");
  title->location.line = 1;
  title->source_line = "    TITLE \"Test Program\"";
  section.atoms.push_back(title);

  // Add an instruction
  auto inst = std::make_shared<InstructionAtom>("NOP", "");
  inst->encoded_bytes = {0x00};
  inst->size = 1;
  inst->location.line = 2;
  inst->source_line = "    NOP";
  section.atoms.push_back(inst);

  ConcreteSymbolTable symbols;
  output.WriteOutput("test_title.lst", {&section}, symbols);

  std::ifstream file("test_title.lst");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should contain title at top of listing
  EXPECT_NE(content.find("Test Program"), std::string::npos);

  std::remove("test_title.lst");
}

/**
 * Test: PAGE directive forces page break
 */
TEST(Z80ListingDirectivesTest, PageDirective) {
  ListingOutput output;
  Section section("test", 0, 0x8000);

  // Add TITLE directive
  auto title = std::make_shared<ListingControlAtom>(ListingControlType::Title,
                                                    "Test Program");
  title->location.line = 1;
  title->source_line = "    TITLE \"Test Program\"";
  section.atoms.push_back(title);

  // Add an instruction
  auto inst1 = std::make_shared<InstructionAtom>("NOP", "");
  inst1->encoded_bytes = {0x00};
  inst1->size = 1;
  inst1->location.line = 2;
  inst1->source_line = "    NOP    ; Page 1";
  section.atoms.push_back(inst1);

  // Add PAGE directive
  auto page =
      std::make_shared<ListingControlAtom>(ListingControlType::Page, "");
  page->location.line = 3;
  page->source_line = "    PAGE";
  section.atoms.push_back(page);

  // Add another instruction on new page
  auto inst2 = std::make_shared<InstructionAtom>("NOP", "");
  inst2->encoded_bytes = {0x00};
  inst2->size = 1;
  inst2->location.line = 4;
  inst2->source_line = "    NOP    ; Page 2";
  section.atoms.push_back(inst2);

  ConcreteSymbolTable symbols;
  output.WriteOutput("test_page.lst", {&section}, symbols);

  std::ifstream file("test_page.lst");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should contain form feed character (page break)
  EXPECT_NE(content.find('\f'), std::string::npos);

  // Should contain title on both pages
  size_t first_title = content.find("Test Program");
  EXPECT_NE(first_title, std::string::npos);
  size_t second_title = content.find("Test Program", first_title + 1);
  EXPECT_NE(second_title, std::string::npos);

  std::remove("test_page.lst");
}

/**
 * Test: SPACE directive inserts blank lines
 */
TEST(Z80ListingDirectivesTest, SpaceDirective) {
  ListingOutput output;
  Section section("test", 0, 0x8000);

  // Add an instruction
  auto inst1 = std::make_shared<InstructionAtom>("NOP", "");
  inst1->encoded_bytes = {0x00};
  inst1->size = 1;
  inst1->location.line = 1;
  inst1->source_line = "    NOP    ; Before space";
  section.atoms.push_back(inst1);

  // Add SPACE directive (3 blank lines)
  auto space =
      std::make_shared<ListingControlAtom>(ListingControlType::Space, 3);
  space->location.line = 2;
  space->source_line = "    SPACE 3";
  section.atoms.push_back(space);

  // Add another instruction
  auto inst2 = std::make_shared<InstructionAtom>("NOP", "");
  inst2->encoded_bytes = {0x00};
  inst2->size = 1;
  inst2->location.line = 3;
  inst2->source_line = "    NOP    ; After space";
  section.atoms.push_back(inst2);

  ConcreteSymbolTable symbols;
  output.WriteOutput("test_space.lst", {&section}, symbols);

  std::ifstream file("test_space.lst");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should contain blank lines between instructions
  // Look for the pattern: "Before space\n\n\n\n" (newline + 3 spaces)
  size_t before_pos = content.find("Before space");
  EXPECT_NE(before_pos, std::string::npos);

  size_t after_pos = content.find("After space");
  EXPECT_NE(after_pos, std::string::npos);

  // Count newlines between the two instructions
  std::string between = content.substr(before_pos, after_pos - before_pos);
  size_t newline_count = std::count(between.begin(), between.end(), '\n');

  // Should have at least 3 extra newlines from SPACE directive
  EXPECT_GE(newline_count, 3UL);

  std::remove("test_space.lst");
}

/**
 * Test: LIST/NOLIST directives control output visibility
 */
TEST(Z80ListingDirectivesTest, ListNolistDirectives) {
  ListingOutput output;
  Section section("test", 0, 0x8000);

  // Add an instruction (should be visible)
  auto inst1 = std::make_shared<InstructionAtom>("NOP", "");
  inst1->encoded_bytes = {0x00};
  inst1->size = 1;
  inst1->location.line = 1;
  inst1->source_line = "    NOP    ; Visible";
  section.atoms.push_back(inst1);

  // Add NOLIST directive
  auto nolist =
      std::make_shared<ListingControlAtom>(ListingControlType::Nolist, "");
  nolist->location.line = 2;
  nolist->source_line = "    NOLIST";
  section.atoms.push_back(nolist);

  // Add an instruction (should be hidden)
  auto inst2 = std::make_shared<InstructionAtom>("NOP", "");
  inst2->encoded_bytes = {0x00};
  inst2->size = 1;
  inst2->location.line = 3;
  inst2->source_line = "    NOP    ; Hidden";
  section.atoms.push_back(inst2);

  // Add LIST directive
  auto list =
      std::make_shared<ListingControlAtom>(ListingControlType::List, "");
  list->location.line = 4;
  list->source_line = "    LIST";
  section.atoms.push_back(list);

  // Add another instruction (should be visible again)
  auto inst3 = std::make_shared<InstructionAtom>("NOP", "");
  inst3->encoded_bytes = {0x00};
  inst3->size = 1;
  inst3->location.line = 5;
  inst3->source_line = "    NOP    ; Visible again";
  section.atoms.push_back(inst3);

  ConcreteSymbolTable symbols;
  output.WriteOutput("test_list_nolist.lst", {&section}, symbols);

  std::ifstream file("test_list_nolist.lst");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should contain first and third instructions
  EXPECT_NE(content.find("; Visible"), std::string::npos);
  EXPECT_NE(content.find("; Visible again"), std::string::npos);

  // Should NOT contain hidden instruction
  EXPECT_EQ(content.find("; Hidden"), std::string::npos);

  // Should show the NOLIST and LIST directives themselves
  EXPECT_NE(content.find("NOLIST"), std::string::npos);
  EXPECT_NE(content.find("LIST"), std::string::npos);

  std::remove("test_list_nolist.lst");
}

/**
 * Test: LALL/SALL directives control macro expansion visibility
 */
TEST(Z80ListingDirectivesTest, LallSallDirectives) {
  ListingOutput output;
  Section section("test", 0, 0x8000);

  // Add LALL directive (show all macro expansions)
  auto lall =
      std::make_shared<ListingControlAtom>(ListingControlType::Lall, "");
  lall->location.line = 1;
  lall->source_line = "    LALL";
  section.atoms.push_back(lall);

  // Add an instruction (represents macro expansion that should be shown)
  auto inst1 = std::make_shared<InstructionAtom>("NOP", "");
  inst1->encoded_bytes = {0x00};
  inst1->size = 1;
  inst1->location.line = 2;
  inst1->source_line = "    NOP    ; In macro";
  section.atoms.push_back(inst1);

  // Add SALL directive (suppress macro expansions)
  auto sall =
      std::make_shared<ListingControlAtom>(ListingControlType::Sall, "");
  sall->location.line = 3;
  sall->source_line = "    SALL";
  section.atoms.push_back(sall);

  ConcreteSymbolTable symbols;
  output.WriteOutput("test_lall_sall.lst", {&section}, symbols);

  std::ifstream file("test_lall_sall.lst");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should show the LALL and SALL directives
  EXPECT_NE(content.find("LALL"), std::string::npos);
  EXPECT_NE(content.find("SALL"), std::string::npos);

  // Should show the instruction (basic test - macro control is complex)
  EXPECT_NE(content.find("; In macro"), std::string::npos);

  std::remove("test_lall_sall.lst");
}
