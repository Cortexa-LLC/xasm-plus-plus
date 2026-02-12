/**
 * Test suite for Z80 Universal Syntax - Macro System
 * Tests MACRO/ENDM, LOCAL, EXITM, REPT, IRP, IRPC
 */

#include <gtest/gtest.h>
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_z80.h"

class Z80MacrosTest : public ::testing::Test {
protected:
  void SetUp() override {
    cpu = std::make_shared<xasm::CpuZ80>();
    parser = std::make_unique<xasm::EdtasmM80PlusPlusSyntaxParser>();
    parser->SetCpu(cpu.get());
    section = xasm::Section();
    symbols = xasm::ConcreteSymbolTable();
  }

  std::shared_ptr<xasm::CpuZ80> cpu;
  std::unique_ptr<xasm::EdtasmM80PlusPlusSyntaxParser> parser;
  xasm::Section section;
  xasm::ConcreteSymbolTable symbols;
};

// ============================================================================
// REPT Tests
// ============================================================================

TEST_F(Z80MacrosTest, ReptBasic) {
  std::string source = R"(
    ORG $8000
    REPT 3
    NOP
    ENDM
  )";

  parser->Parse(source, section, symbols); 
  auto atoms = section.atoms;
  
  // DEBUG: Print all atoms
  std::cout << "Total atoms: " << atoms.size() << std::endl;
  for (size_t i = 0; i < atoms.size(); ++i) {
    if (auto org = std::dynamic_pointer_cast<xasm::OrgAtom>(atoms[i])) {
      std::cout << "  Atom " << i << ": ORG " << std::hex << org->address << std::dec << std::endl;
    } else if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atoms[i])) {
      std::cout << "  Atom " << i << ": INSTRUCTION " << inst->mnemonic << " " << inst->operand << std::endl;
    } else if (auto data = std::dynamic_pointer_cast<xasm::DataAtom>(atoms[i])) {
      std::cout << "  Atom " << i << ": DATA (size=" << data->data.size() << ")" << std::endl;
    } else {
      std::cout << "  Atom " << i << ": UNKNOWN" << std::endl;
    }
  }
  
  // Should have origin + 3 NOPs
  if (atoms.size() < 2) {
    FAIL() << "Expected at least 2 atoms (ORG + NOPs), got " << atoms.size();
  }
  
  // Check for NOP instructions (by mnemonic, not encoded bytes)
  int nop_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      if (inst->mnemonic == "NOP") {
        nop_count++;
      }
    }
  }
  EXPECT_EQ(nop_count, 3);
}

TEST_F(Z80MacrosTest, ReptZero) {
  std::string source = R"(
    ORG $8000
    REPT 0
    NOP
    ENDM
    LD A, 1
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should have origin + LD A,1 but no NOP
  int nop_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      if (inst->encoded_bytes.size() == 1 && inst->encoded_bytes[0] == 0x00) {
        nop_count++;
      }
    }
  }
  EXPECT_EQ(nop_count, 0);
}

TEST_F(Z80MacrosTest, ReptWithInstructions) {
  std::string source = R"(
    ORG $8000
    REPT 2
    LD A, $FF
    LD B, $00
    ENDM
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should have origin + 4 instructions (2 reps * 2 instructions)
  int ld_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      ld_count++;
    }
  }
  EXPECT_EQ(ld_count, 4);
}

// ============================================================================
// IRP Tests
// ============================================================================

TEST_F(Z80MacrosTest, IrpBasic) {
  std::string source = R"(
    ORG $8000
    IRP REG, <A, B, C>
    LD &REG, 0
    ENDM
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should generate: LD A,0; LD B,0; LD C,0
  int ld_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      ld_count++;
    }
  }
  EXPECT_EQ(ld_count, 3);
}

TEST_F(Z80MacrosTest, IrpWithPercent) {
  std::string source = R"(
    ORG $8000
    IRP VAL, <1, 2, 3>
    DB %VAL
    ENDM
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should generate: DB 1; DB 2; DB 3
  int db_count = 0;
  for (const auto &atom : atoms) {
    if (auto data = std::dynamic_pointer_cast<xasm::DataAtom>(atom)) {
      db_count++;
    }
  }
  EXPECT_EQ(db_count, 3);
}

TEST_F(Z80MacrosTest, IrpEmpty) {
  std::string source = R"(
    ORG $8000
    IRP REG, <>
    LD &REG, 0
    ENDM
    NOP
  )";

  parser->Parse(source, section, symbols);
  auto atoms = section.atoms;
  
  // Should only have NOP (IRP with empty list generates nothing)
  int ld_count = 0;
  int nop_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      if (inst->mnemonic == "NOP") {
        nop_count++;
      } else if (inst->mnemonic == "LD") {
        ld_count++;
      }
    }
  }
  EXPECT_EQ(ld_count, 0);
  EXPECT_EQ(nop_count, 1);
}

// ============================================================================
// IRPC Tests
// ============================================================================

TEST_F(Z80MacrosTest, IrpcBasic) {
  std::string source = R"(
    ORG $8000
    IRPC CHAR, ABC
    DB '&CHAR'
    ENDM
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should generate: DB 'A'; DB 'B'; DB 'C'
  int db_count = 0;
  for (const auto &atom : atoms) {
    if (auto data = std::dynamic_pointer_cast<xasm::DataAtom>(atom)) {
      db_count++;
    }
  }
  EXPECT_EQ(db_count, 3);
}

TEST_F(Z80MacrosTest, IrpcWithAngleBrackets) {
  std::string source = R"(
    ORG $8000
    IRPC CHAR, <XYZ>
    DB '%CHAR'
    ENDM
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should generate: DB 'X'; DB 'Y'; DB 'Z'
  int db_count = 0;
  for (const auto &atom : atoms) {
    if (auto data = std::dynamic_pointer_cast<xasm::DataAtom>(atom)) {
      db_count++;
    }
  }
  EXPECT_EQ(db_count, 3);
}

TEST_F(Z80MacrosTest, IrpcEmpty) {
  std::string source = R"(
    ORG $8000
    IRPC CHAR, <>
    DB '&CHAR'
    ENDM
    NOP
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should only have NOP
  int db_count = 0;
  for (const auto &atom : atoms) {
    if (auto data = std::dynamic_pointer_cast<xasm::DataAtom>(atom)) {
      db_count++;
    }
  }
  EXPECT_EQ(db_count, 0);
}

// ============================================================================
// MACRO Tests
// ============================================================================

TEST_F(Z80MacrosTest, MacroBasic) {
  std::string source = R"(
    LOAD_IMMEDIATE MACRO REG, VAL
    LD &REG, %VAL
    ENDM

    ORG $8000
    LOAD_IMMEDIATE A, 42
    LOAD_IMMEDIATE B, 99
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should have 2 LD instructions
  int ld_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      ld_count++;
    }
  }
  EXPECT_EQ(ld_count, 2);
}

TEST_F(Z80MacrosTest, MacroNoParams) {
  std::string source = R"(
    SAVE_ALL MACRO
    PUSH AF
    PUSH BC
    PUSH DE
    ENDM

    ORG $8000
    SAVE_ALL
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should have 3 PUSH instructions
  int push_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      // Check for PUSH mnemonic
      if (inst->mnemonic == "PUSH") {
        push_count++;
      }
    }
  }
  EXPECT_EQ(push_count, 3);
}

TEST_F(Z80MacrosTest, MacroWithLabel) {
  std::string source = R"(
DELAY   MACRO
        LD B, 10
LOOP    DJNZ LOOP
        ENDM

        ORG $8000
        DELAY
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should have LD and DJNZ
  int inst_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      inst_count++;
    }
  }
  EXPECT_GE(inst_count, 2);
}

// ============================================================================
// LOCAL Tests
// ============================================================================

TEST_F(Z80MacrosTest, MacroWithLocal) {
  std::string source = R"(
WAIT    MACRO
        LOCAL SKIP
        LD A, (FLAGS)
        BIT 0, A
        JR NZ, SKIP
        CALL DELAY
SKIP    NOP
        ENDM

        ORG $8000
        WAIT
        WAIT
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Each macro invocation should create unique local labels
  // We should have 2 invocations with different local labels
  // The test mainly checks that it parses without duplicate label errors
  EXPECT_GT(atoms.size(), 0);
}

// ============================================================================
// EXITM Tests
// ============================================================================

TEST_F(Z80MacrosTest, MacroWithExitm) {
  // TODO: This test requires conditional assembly (IF/ENDIF) which is Phase 13
  // For now, test that EXITM is recognized (doesn't cause error)
  std::string source = R"(
TEST MACRO
     NOP
     EXITM
     RET
     ENDM

     ORG $8000
     TEST
  )";

  parser->Parse(source, section, symbols);
  auto atoms = section.atoms;
  
  // Should only have NOP (EXITM exits before RET)
  int nop_count = 0;
  int ret_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      if (inst->mnemonic == "NOP") {
        nop_count++;
      } else if (inst->mnemonic == "RET") {
        ret_count++;
      }
    }
  }
  EXPECT_EQ(nop_count, 1);
  EXPECT_EQ(ret_count, 0);
}

// ============================================================================
// Nested and Complex Tests
// ============================================================================

TEST_F(Z80MacrosTest, MacroCallingMacro) {
  std::string source = R"(
SET_REG MACRO REG, VAL
        LD &REG, %VAL
        ENDM

INIT    MACRO
        SET_REG A, 0
        SET_REG B, 1
        ENDM

        ORG $8000
        INIT
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should expand INIT which calls SET_REG twice
  int ld_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      ld_count++;
    }
  }
  EXPECT_EQ(ld_count, 2);
}

TEST_F(Z80MacrosTest, ReptInsideMacro) {
  std::string source = R"(
FILL_ZERO MACRO COUNT
          REPT %COUNT
          DB 0
          ENDM
          ENDM

          ORG $8000
          FILL_ZERO 5
  )";

  parser->Parse(source, section, symbols); auto atoms = section.atoms;
  
  // Should generate 5 DB 0
  int db_count = 0;
  for (const auto &atom : atoms) {
    if (auto data = std::dynamic_pointer_cast<xasm::DataAtom>(atom)) {
      db_count++;
    }
  }
  EXPECT_EQ(db_count, 5);
}

// ============================================================================
// Error Cases
// ============================================================================

TEST_F(Z80MacrosTest, EndmWithoutMacro) {
  std::string source = R"(
    ORG $8000
    ENDM
  )";

  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

TEST_F(Z80MacrosTest, UnclosedMacro) {
  std::string source = R"(
TEST MACRO
     NOP
     ; Missing ENDM
  )";

  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

TEST_F(Z80MacrosTest, UnclosedRept) {
  std::string source = R"(
    REPT 3
    NOP
    ; Missing ENDM
  )";

  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

TEST_F(Z80MacrosTest, ReptInvalidCount) {
  std::string source = R"(
    REPT ABC
    NOP
    ENDM
  )";

  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

TEST_F(Z80MacrosTest, IrpMissingList) {
  std::string source = R"(
    IRP REG
    NOP
    ENDM
  )";

  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

TEST_F(Z80MacrosTest, IrpcMissingString) {
  std::string source = R"(
    IRPC CHAR
    NOP
    ENDM
  )";

  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

TEST_F(Z80MacrosTest, MacroDuplicateName) {
  std::string source = R"(
    ORG $8000
TEST MACRO
     NOP
     ENDM

TEST MACRO
     RET
     ENDM

    TEST
  )";

  // Should allow redefinition (overwrites previous definition)
  // Second definition should be used
  parser->Parse(source, section, symbols);
  auto atoms = section.atoms;
  
  // Should have one RET instruction (second macro definition)
  int ret_count = 0;
  int nop_count = 0;
  for (const auto &atom : atoms) {
    if (auto inst = std::dynamic_pointer_cast<xasm::InstructionAtom>(atom)) {
      if (inst->mnemonic == "RET") {
        ret_count++;
      } else if (inst->mnemonic == "NOP") {
        nop_count++;
      }
    }
  }
  EXPECT_EQ(ret_count, 1);
  EXPECT_EQ(nop_count, 0);
}
