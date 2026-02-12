// Unit tests for Z80 forward reference handling in data directives
// Tests that DW directive can handle forward label references

#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

class Z80ForwardReferenceTest : public ::testing::Test {
protected:
  std::shared_ptr<CpuZ80> cpu;
  std::shared_ptr<EdtasmM80PlusPlusSyntaxParser> parser;

  void SetUp() override {
    cpu = std::make_shared<CpuZ80>();
    parser = std::make_shared<EdtasmM80PlusPlusSyntaxParser>();
    parser->SetCpu(cpu.get());
  }
};

// Test forward reference in DW directive
TEST_F(Z80ForwardReferenceTest, DWWithForwardReference) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // DW references LABEL before it's defined
  std::string source = "TABLE:   DW LABEL\n"
                       "LABEL:   NOP\n";

  // Should parse successfully (expression deferred)
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  
  // Both labels should be defined
  EXPECT_TRUE(symbols.IsDefined("TABLE"));
  EXPECT_TRUE(symbols.IsDefined("LABEL"));
  
  // DW atom should exist with expression string
  ASSERT_GE(section.atoms.size(), 2);
  
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
  
  // Expression should be stored for later evaluation
  EXPECT_EQ(data_atom->expressions.size(), 1);
  EXPECT_FALSE(data_atom->expressions[0].empty());
}

// Test multiple forward references in DW directive
TEST_F(Z80ForwardReferenceTest, DWWithMultipleForwardReferences) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Multiple forward references (like jump table)
  std::string source = "DISPATCH: DW CMD0, CMD1, CMD2\n"
                       "CMD0:     RET\n"
                       "CMD1:     NOP\n"
                       "CMD2:     RET\n";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  
  // All labels should be defined
  EXPECT_TRUE(symbols.IsDefined("DISPATCH"));
  EXPECT_TRUE(symbols.IsDefined("CMD0"));
  EXPECT_TRUE(symbols.IsDefined("CMD1"));
  EXPECT_TRUE(symbols.IsDefined("CMD2"));
  
  // DW atom should have 3 expressions
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->expressions.size(), 3);
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
}

// Test DB with forward reference (should also work)
TEST_F(Z80ForwardReferenceTest, DBWithForwardReference) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // DB with forward reference (low byte extraction)
  std::string source = "         DB <LABEL\n"
                       "LABEL:   NOP\n";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  
  EXPECT_TRUE(symbols.IsDefined("LABEL"));
  
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Byte);
  EXPECT_EQ(data_atom->expressions.size(), 1);
}

// Test backward reference still works
TEST_F(Z80ForwardReferenceTest, DWWithBackwardReference) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Normal backward reference (already implemented)
  std::string source = "LABEL:   NOP\n"
                       "TABLE:   DW LABEL\n";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  
  EXPECT_TRUE(symbols.IsDefined("LABEL"));
  EXPECT_TRUE(symbols.IsDefined("TABLE"));
  
  // Should have data atom with expression (last atom)
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[3]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
}

// Test mixed forward and backward references
TEST_F(Z80ForwardReferenceTest, DWWithMixedReferences) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "START:   NOP\n"
                       "TABLE:   DW START, END\n"
                       "END:     RET\n";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  
  EXPECT_TRUE(symbols.IsDefined("START"));
  EXPECT_TRUE(symbols.IsDefined("TABLE"));
  EXPECT_TRUE(symbols.IsDefined("END"));
  
  // Data atom is at index 3 (after START label, instruction, TABLE label)
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[3]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->expressions.size(), 2);
}
