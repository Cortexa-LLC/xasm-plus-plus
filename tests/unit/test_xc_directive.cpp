/**
 * @file test_xc_directive.cpp
 * @brief Tests for XC directive CPU mode switching
 *
 * Tests the proper implementation of the XC directive which enables/disables
 * 65C02/65816 CPU modes in Merlin syntax.
 */

#include "xasm++/atom.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/merlin_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// XC Directive - CPU Mode Switching
// ============================================================================

TEST(XcDirectiveTest, XcEnablesCpu65C02Mode) {
  Cpu6502 cpu;
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Initial mode should be 6502
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

  // XC (no operand) or XC ON should enable 65C02 mode
  parser.Parse(" xc", section, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);
}

TEST(XcDirectiveTest, XcOnEnablesCpu65C02Mode) {
  Cpu6502 cpu;
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Initial mode should be 6502
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

  // XC ON should enable 65C02 mode
  parser.Parse(" xc on", section, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);
}

TEST(XcDirectiveTest, XcOffDisablesCpu65C02Mode) {
  Cpu6502 cpu;
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // First enable 65C02 mode
  parser.Parse(" xc", section, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);

  // XC OFF should disable 65C02 mode (back to 6502)
  Section section2("test", 0);
  parser.Parse(" xc off", section2, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);
}

TEST(XcDirectiveTest, XcCaseInsensitive) {
  Cpu6502 cpu;
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Test uppercase XC
  parser.Parse(" XC", section, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);

  // Test XC OFF uppercase
  Section section2("test", 0);
  parser.Parse(" XC OFF", section2, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

  // Test mixed case
  Section section3("test", 0);
  parser.Parse(" Xc On", section3, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);
}

TEST(XcDirectiveTest, XcGeneratesCpuModeAtom) {
  Cpu6502 cpu;
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // XC directive generates a CpuModeAtom so encoding replays the mode change.
  parser.Parse(" xc", section, symbols);
  ASSERT_EQ(section.atoms.size(), 1UL);
  auto mode_atom = std::dynamic_pointer_cast<CpuModeAtom>(section.atoms[0]);
  ASSERT_NE(mode_atom, nullptr);
  EXPECT_EQ(mode_atom->mode, 1); // 1 = 65C02

  Section section2("test", 0);
  parser.Parse(" xc off", section2, symbols);
  ASSERT_EQ(section2.atoms.size(), 1UL);
  auto mode_atom2 = std::dynamic_pointer_cast<CpuModeAtom>(section2.atoms[0]);
  ASSERT_NE(mode_atom2, nullptr);
  EXPECT_EQ(mode_atom2->mode, 0); // 0 = 6502
}

TEST(XcDirectiveTest, XcMultipleToggles) {
  Cpu6502 cpu;
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Toggle multiple times
  parser.Parse(" xc", section, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);

  Section section2("test", 0);
  parser.Parse(" xc off", section2, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

  Section section3("test", 0);
  parser.Parse(" xc on", section3, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);

  Section section4("test", 0);
  parser.Parse(" xc off", section4, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);
}

// ============================================================================
// XC Directive - Instruction Set Integration
// ============================================================================

TEST(XcDirectiveTest, Enable65C02InstructionsTSB) {
  Cpu6502 cpu;
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Enable 65C02 mode
  parser.Parse(" xc", section, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);

  // TSB should now be available (65C02 instruction)
  parser.Parse(" TSB $80", section, symbols);

  // Should have CpuModeAtom (xc) + InstructionAtom (TSB)
  ASSERT_EQ(section.atoms.size(), 2UL);
  auto inst = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[1]);
  ASSERT_NE(inst, nullptr);
  EXPECT_EQ(inst->mnemonic, "TSB");
}

TEST(XcDirectiveTest, Enable65C02InstructionsTRB) {
  Cpu6502 cpu;
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Enable 65C02 mode
  parser.Parse(" xc", section, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);

  // TRB should now be available (65C02 instruction)
  parser.Parse(" TRB $80", section, symbols);

  // Should have CpuModeAtom (xc) + InstructionAtom (TRB)
  ASSERT_EQ(section.atoms.size(), 2UL);
  auto inst = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[1]);
  ASSERT_NE(inst, nullptr);
  EXPECT_EQ(inst->mnemonic, "TRB");
}

TEST(XcDirectiveTest, Enable65C02InstructionsPHY) {
  Cpu6502 cpu;
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Enable 65C02 mode
  parser.Parse(" xc", section, symbols);
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);

  // PHY should now be available (65C02 instruction)
  parser.Parse(" PHY", section, symbols);

  // Should have CpuModeAtom (xc) + InstructionAtom (PHY)
  ASSERT_EQ(section.atoms.size(), 2UL);
  auto inst = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[1]);
  ASSERT_NE(inst, nullptr);
  EXPECT_EQ(inst->mnemonic, "PHY");
}

TEST(XcDirectiveTest, GrafixSUsagePattern) {
  Cpu6502 cpu;
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Simulate GRAFIX.S usage pattern:
  // Enable 65C02, use TSB/TRB, then disable
  std::string source = R"(
        xc
        TSB $80
        TRB $80
        PHY
        xc off
        LDA #$00
    )";

  parser.Parse(source, section, symbols);

  // Should have 2 CpuModeAtoms (xc, xc off) + 4 InstructionAtoms (TSB, TRB, PHY, LDA)
  EXPECT_EQ(section.atoms.size(), 6UL);
}

TEST(XcDirectiveTest, Cpu65C02InstructionWithoutXcReturnsEmptyVector) {
  Cpu6502 cpu;

  // Initial mode should be 6502
  EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

  // Try to encode 65C02 instruction without XC enabled
  // Should return empty vector (instruction not available in 6502 mode)
  auto bytes_phy = cpu.EncodePHY();
  EXPECT_TRUE(bytes_phy.empty())
      << "PHY should return empty vector in 6502 mode";

  auto bytes_plx = cpu.EncodePLX();
  EXPECT_TRUE(bytes_plx.empty())
      << "PLX should return empty vector in 6502 mode";

  auto bytes_tsb = cpu.EncodeTSB(0x80, AddressingMode::ZeroPage);
  EXPECT_TRUE(bytes_tsb.empty())
      << "TSB should return empty vector in 6502 mode";
}
