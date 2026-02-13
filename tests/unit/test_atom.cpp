// Unit tests for Atom classes
// TDD: Tests written FIRST, then implementation

#include "xasm++/atom.h"
#include <gtest/gtest.h>

namespace xasm {

// Test AtomType enum exists
TEST(AtomTest, AtomTypeEnum) {
  AtomType type = AtomType::Label;
  EXPECT_EQ(type, AtomType::Label);

  type = AtomType::Instruction;
  EXPECT_EQ(type, AtomType::Instruction);

  type = AtomType::Data;
  EXPECT_EQ(type, AtomType::Data);
}

// Test base Atom class
TEST(AtomTest, BaseAtomCreation) {
  Atom atom(AtomType::Label);
  EXPECT_EQ(atom.type, AtomType::Label);
  EXPECT_EQ(atom.size, 0U);
  EXPECT_EQ(atom.last_size, 0U);
  EXPECT_EQ(atom.changes, 0U);
  EXPECT_EQ(atom.next, nullptr);
}

// Test LabelAtom
TEST(AtomTest, LabelAtomCreation) {
  LabelAtom label("start", 0x8000);
  EXPECT_EQ(label.type, AtomType::Label);
  EXPECT_EQ(label.name, "start");
  EXPECT_EQ(label.address, 0x8000UL);
}

// Test InstructionAtom
TEST(AtomTest, InstructionAtomCreation) {
  InstructionAtom instr("LDA", "#$42");
  EXPECT_EQ(instr.type, AtomType::Instruction);
  EXPECT_EQ(instr.mnemonic, "LDA");
  EXPECT_EQ(instr.operand, "#$42");
}

// Test DataAtom
TEST(AtomTest, DataAtomCreation) {
  std::vector<uint8_t> data = {0x42, 0x43, 0x44};
  DataAtom dataAtom(data);
  EXPECT_EQ(dataAtom.type, AtomType::Data);
  EXPECT_EQ(dataAtom.data.size(), 3UL);
  EXPECT_EQ(dataAtom.data[0], 0x42);
  EXPECT_EQ(dataAtom.size, 3U);
}

// Test SpaceAtom
TEST(AtomTest, SpaceAtomCreation) {
  SpaceAtom space(256);
  EXPECT_EQ(space.type, AtomType::Space);
  EXPECT_EQ(space.count, 256U);
  EXPECT_EQ(space.size, 256U);
}

// Test AlignAtom
TEST(AtomTest, AlignAtomCreation) {
  AlignAtom align(16);
  EXPECT_EQ(align.type, AtomType::Align);
  EXPECT_EQ(align.alignment, 16UL);
}

// Test OrgAtom
TEST(AtomTest, OrgAtomCreation) {
  OrgAtom org(0x8000);
  EXPECT_EQ(org.type, AtomType::Org);
  EXPECT_EQ(org.address, 0x8000UL);
}

// Test atom chaining (linked list)
TEST(AtomTest, AtomChaining) {
  auto label = std::make_shared<LabelAtom>("start", 0x8000);
  auto instr = std::make_shared<InstructionAtom>("LDA", "#$42");

  label->next = instr;

  EXPECT_NE(label->next, nullptr);
  EXPECT_EQ(label->next, instr);
}

} // namespace xasm
