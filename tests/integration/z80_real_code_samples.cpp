// Integration tests for Z80/EDTASM-M80++ parser with real-world code samples
// Tests parser against actual M80, ZMAC, and Z80ASM code patterns

#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

using namespace xasm;

// ============================================================================
// Helper Functions
// ============================================================================

class Z80IntegrationTest : public ::testing::Test {
protected:
  std::shared_ptr<CpuZ80> cpu;
  std::shared_ptr<EdtasmM80PlusPlusSyntaxParser> parser;

  void SetUp() override {
    cpu = std::make_shared<CpuZ80>();
    parser = std::make_shared<EdtasmM80PlusPlusSyntaxParser>();
    parser->SetCpu(cpu.get());
  }

  // Helper to assemble code and return success
  bool AssembleCode(const std::string &source, Section &section,
                    ConcreteSymbolTable &symbols) {
    try {
      parser->Parse(source, section, symbols);
      return true;
    } catch (const std::exception &e) {
      std::cerr << "Assembly error: " << e.what() << std::endl;
      return false;
    }
  }

  // Helper to count atoms of a specific type
  size_t CountAtomType(const Section &section, AtomType type) {
    size_t count = 0;
    for (const auto &atom : section.atoms) {
      if (atom->type == type) {
        count++;
      }
    }
    return count;
  }
};

// ============================================================================
// Edge Case: Empty File
// ============================================================================

TEST_F(Z80IntegrationTest, EmptyFile) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "";

  EXPECT_TRUE(AssembleCode(source, section, symbols));
  EXPECT_EQ(section.atoms.size(), 0UL);
}

TEST_F(Z80IntegrationTest, OnlyComments) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "; This is a comment\n"
                       "; Another comment\n"
                       "       ; Indented comment\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));
  EXPECT_EQ(section.atoms.size(), 0UL);
}

TEST_F(Z80IntegrationTest, OnlyWhitespace) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "   \n"
                       "\t\t\n"
                       "        \n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));
  EXPECT_EQ(section.atoms.size(), 0UL);
}

// ============================================================================
// Edge Case: Large Files
// ============================================================================

TEST_F(Z80IntegrationTest, LargeFileWithManyLabels) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Generate 1000 labels and instructions
  std::ostringstream source;
  for (int i = 0; i < 1000; i++) {
    source << "LABEL" << i << ":  NOP\n";
  }

  EXPECT_TRUE(AssembleCode(source.str(), section, symbols));
  EXPECT_EQ(section.atoms.size(), 2000UL); // 1000 labels + 1000 NOPs

  // Verify symbols were created
  for (int i = 0; i < 1000; i++) {
    std::string label = "LABEL" + std::to_string(i);
    EXPECT_TRUE(symbols.IsDefined(label));
  }
}

TEST_F(Z80IntegrationTest, LargeDataBlock) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Generate large data block (10KB)
  std::ostringstream source;
  source << "         ORG $8000\n";
  for (int i = 0; i < 10240; i++) {
    source << "         DB " << (i % 256) << "\n";
  }

  EXPECT_TRUE(AssembleCode(source.str(), section, symbols));

  // Should have ORG + 10240 data atoms
  EXPECT_EQ(section.atoms.size(), 10241UL);
  EXPECT_EQ(CountAtomType(section, AtomType::Org), 1UL);
  EXPECT_EQ(CountAtomType(section, AtomType::Data), 10240UL);
}

// ============================================================================
// Edge Case: Deep Conditional Nesting
// ============================================================================

TEST_F(Z80IntegrationTest, DeepConditionalNesting) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // 10 levels of nesting (all true)
  std::ostringstream source;
  for (int i = 0; i < 10; i++) {
    source << "         IF 1\n";
  }
  source << "         DB $42\n";
  for (int i = 0; i < 10; i++) {
    source << "         ENDIF\n";
  }

  EXPECT_TRUE(AssembleCode(source.str(), section, symbols));
  ASSERT_EQ(section.atoms.size(), 1UL);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST_F(Z80IntegrationTest, DeepConditionalNestingWithFalse) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // 10 levels of nesting (outer false should skip all)
  std::ostringstream source;
  source << "         IF 0\n";
  for (int i = 0; i < 9; i++) {
    source << "         IF 1\n";
  }
  source << "         DB $42\n";
  for (int i = 0; i < 10; i++) {
    source << "         ENDIF\n";
  }

  EXPECT_TRUE(AssembleCode(source.str(), section, symbols));
  EXPECT_EQ(section.atoms.size(), 0UL); // Outer false should skip everything
}

// ============================================================================
// Real M80 Code Patterns
// ============================================================================

TEST_F(Z80IntegrationTest, M80StylePublicDeclaration) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // M80 style: PUBLIC declarations before use
  std::string source = "         PUBLIC START, INIT, EXIT\n"
                       "START:   LD SP,$FFFF\n"
                       "         CALL INIT\n"
                       "         JP EXIT\n"
                       "INIT:    LD A,0\n"
                       "         RET\n"
                       "EXIT:    HALT\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // Verify public symbols
  auto start = symbols.GetSymbol("START");
  auto init = symbols.GetSymbol("INIT");
  auto exit = symbols.GetSymbol("EXIT");

  ASSERT_NE(start, nullptr);
  ASSERT_NE(init, nullptr);
  ASSERT_NE(exit, nullptr);

  EXPECT_TRUE(start->is_exported);
  EXPECT_TRUE(init->is_exported);
  EXPECT_TRUE(exit->is_exported);
}

TEST_F(Z80IntegrationTest, M80StyleExternalReferences) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // M80 style: EXTRN for external symbols
  std::string source = "         EXTRN PRINTF, SCANF, EXIT\n"
                       "START:   LD HL,MSG\n"
                       "         CALL PRINTF\n"
                       "         CALL SCANF\n"
                       "         JP EXIT\n"
                       "MSG:     DB \"Hello\",0\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // Verify external symbols
  auto printf = symbols.GetSymbol("PRINTF");
  auto scanf = symbols.GetSymbol("SCANF");
  auto exit = symbols.GetSymbol("EXIT");

  ASSERT_NE(printf, nullptr);
  ASSERT_NE(scanf, nullptr);
  ASSERT_NE(exit, nullptr);

  EXPECT_TRUE(printf->is_imported);
  EXPECT_TRUE(scanf->is_imported);
  EXPECT_TRUE(exit->is_imported);
}

TEST_F(Z80IntegrationTest, M80StyleHexNumbers) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // M80 uses H suffix for hex (must start with digit)
  std::string source = "         ORG 8000H\n"
                       "         LD A,0FFH\n"
                       "         LD BC,1234H\n"
                       "         DB 0ABH,0CDH,0EFH\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // Verify ORG address
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8000U);

  // Verify data
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[3]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3UL);
  EXPECT_EQ(data_atom->data[0], 0xAB);
  EXPECT_EQ(data_atom->data[1], 0xCD);
  EXPECT_EQ(data_atom->data[2], 0xEF);
}

// ============================================================================
// Real ZMAC Code Patterns
// ============================================================================

TEST_F(Z80IntegrationTest, ZMACStyleLocalLabels) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // ZMAC style: .label for local labels
  std::string source = "MAIN:    LD B,10\n"
                       ".loop:   DEC B\n"
                       "         JR NZ,.loop\n"
                       "         RET\n"
                       "SUB2:    LD C,5\n"
                       ".loop:   DEC C\n"
                       "         JR NZ,.loop\n"
                       "         RET\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // Global labels should be defined
  EXPECT_TRUE(symbols.IsDefined("MAIN"));
  EXPECT_TRUE(symbols.IsDefined("SUB2"));

  // Local labels are scoped to parent
  // (implementation may vary - just ensure it assembles)
}

TEST_F(Z80IntegrationTest, ZMACStyleDollarHex) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // ZMAC style: $ prefix for hex
  std::string source = "         ORG $8000\n"
                       "         LD A,$FF\n"
                       "         LD BC,$1234\n"
                       "         DB $AB,$CD,$EF\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8000U);
}

// ============================================================================
// Real Z80ASM Code Patterns
// ============================================================================

TEST_F(Z80IntegrationTest, Z80ASMStyleLocalLabels) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Z80ASM style: $label for local labels
  std::string source = "MAIN:    LD B,10\n"
                       "$loop:   DEC B\n"
                       "         JR NZ,$loop\n"
                       "         RET\n"
                       "SUB2:    LD C,5\n"
                       "$loop:   DEC C\n"
                       "         JR NZ,$loop\n"
                       "         RET\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // Global labels should be defined
  EXPECT_TRUE(symbols.IsDefined("MAIN"));
  EXPECT_TRUE(symbols.IsDefined("SUB2"));
}

// ============================================================================
// Complex Real-World Pattern: CP/M BDOS Call
// ============================================================================

TEST_F(Z80IntegrationTest, CPMBDOSCallPattern) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Classic CP/M BDOS call pattern
  std::string source = "BDOS     EQU 5\n"
                       "CONOUT   EQU 2\n"
                       "PRINT    EQU 9\n"
                       "\n"
                       "         ORG 100H\n"
                       "START:   LD DE,MSG\n"
                       "         LD C,PRINT\n"
                       "         CALL BDOS\n"
                       "         RET\n"
                       "\n"
                       "MSG:     DB 'Hello, CP/M!$'\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // Verify constants
  EXPECT_TRUE(symbols.IsDefined("BDOS"));
  EXPECT_TRUE(symbols.IsDefined("CONOUT"));
  EXPECT_TRUE(symbols.IsDefined("PRINT"));
  EXPECT_TRUE(symbols.IsDefined("START"));
  EXPECT_TRUE(symbols.IsDefined("MSG"));

  // Verify ORG
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x100U);
}

// ============================================================================
// Complex Real-World Pattern: Interrupt Handler
// ============================================================================

TEST_F(Z80IntegrationTest, InterruptHandlerPattern) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Classic Z80 interrupt handler
  std::string source = "         ORG 0\n"
                       "         JP START\n"
                       "\n"
                       "         ORG 38H\n"
                       "         JP ISR\n"
                       "\n"
                       "         ORG 100H\n"
                       "START:   DI\n"
                       "         LD SP,$FFFF\n"
                       "         IM 1\n"
                       "         EI\n"
                       "LOOP:    HALT\n"
                       "         JP LOOP\n"
                       "\n"
                       "ISR:     PUSH AF\n"
                       "         PUSH BC\n"
                       "         ; Handle interrupt\n"
                       "         POP BC\n"
                       "         POP AF\n"
                       "         EI\n"
                       "         RETI\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // Verify labels
  EXPECT_TRUE(symbols.IsDefined("START"));
  EXPECT_TRUE(symbols.IsDefined("LOOP"));
  EXPECT_TRUE(symbols.IsDefined("ISR"));

  // Verify multiple ORG directives
  EXPECT_EQ(CountAtomType(section, AtomType::Org), 3UL);
}

// ============================================================================
// Complex Real-World Pattern: Lookup Table
// ============================================================================

TEST_F(Z80IntegrationTest, LookupTablePattern) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Jump table pattern
  std::string source = "         ORG $8000\n"
                       "\n"
                       "DISPATCH:\n"
                       "         DW CMD0, CMD1, CMD2, CMD3\n"
                       "\n"
                       "CMD0:    LD A,0\n"
                       "         RET\n"
                       "\n"
                       "CMD1:    LD A,1\n"
                       "         RET\n"
                       "\n"
                       "CMD2:    LD A,2\n"
                       "         RET\n"
                       "\n"
                       "CMD3:    LD A,3\n"
                       "         RET\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // Verify all labels
  EXPECT_TRUE(symbols.IsDefined("DISPATCH"));
  EXPECT_TRUE(symbols.IsDefined("CMD0"));
  EXPECT_TRUE(symbols.IsDefined("CMD1"));
  EXPECT_TRUE(symbols.IsDefined("CMD2"));
  EXPECT_TRUE(symbols.IsDefined("CMD3"));
}

// ============================================================================
// Edge Case: Mixed Number Formats
// ============================================================================

TEST_F(Z80IntegrationTest, MixedNumberFormats) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // All number formats in one program
  std::string source = "         DB $FF        ; Dollar hex\n"
                       "         DB 0FFH       ; H suffix hex\n"
                       "         DB 0xFF       ; 0x prefix hex\n"
                       "         DB 255        ; Decimal\n"
                       "         DB 377O       ; Octal\n"
                       "         DB 11111111B  ; Binary\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // All should produce same value
  ASSERT_EQ(section.atoms.size(), 6UL);
  for (const auto &atom : section.atoms) {
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(atom);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 1UL);
    EXPECT_EQ(data_atom->data[0], 0xFF);
  }
}

// ============================================================================
// Edge Case: String Escapes
// ============================================================================

TEST_F(Z80IntegrationTest, StringWithEscapes) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // String with quotes and special chars
  std::string source = "MSG1:    DB \"Hello, World!\",0\n"
                       "MSG2:    DB 'Single quotes',0\n"
                       "MSG3:    DB \"Tab\\tNewline\\n\",0\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // Verify strings were assembled
  EXPECT_TRUE(symbols.IsDefined("MSG1"));
  EXPECT_TRUE(symbols.IsDefined("MSG2"));
  EXPECT_TRUE(symbols.IsDefined("MSG3"));
}

// ============================================================================
// Edge Case: Expression Evaluation
// ============================================================================

TEST_F(Z80IntegrationTest, ComplexExpressions) {
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "BASE     EQU $8000\n"
                       "OFFSET   EQU $100\n"
                       "SIZE     EQU 256\n"
                       "\n"
                       "         ORG BASE+OFFSET\n"
                       "         DB SIZE/2\n"
                       "         DW BASE+SIZE\n"
                       "         DB (SIZE+1)/2\n";

  EXPECT_TRUE(AssembleCode(source, section, symbols));

  // Verify expressions evaluated correctly
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8100U); // 0x8000 + 0x100

  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data1, nullptr);
  EXPECT_EQ(data1->data[0], 128); // 256/2
}

// ============================================================================
// Performance Test: Rapid Parsing
// ============================================================================

TEST_F(Z80IntegrationTest, RapidParsingPerformance) {
  // Parse same code 100 times - should complete quickly
  std::string source = "START:   LD A,$FF\n"
                       "         LD BC,$1234\n"
                       "         CALL SUB\n"
                       "         RET\n"
                       "SUB:     NOP\n"
                       "         RET\n";

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < 100; i++) {
    ConcreteSymbolTable symbols;
    Section section("test", 0);
    EXPECT_TRUE(AssembleCode(source, section, symbols));
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  // Should complete in under 1 second
  EXPECT_LT(duration.count(), 1000);
}
