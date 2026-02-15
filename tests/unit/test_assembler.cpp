// Assembler tests
// Phase 1: Minimal Viable Assembler - Assembler Engine

#include "xasm++/assembler.h"
#include "xasm++/atom.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>

using namespace xasm;

// Test 1: Assembler creation
TEST(AssemblerTest, Creation) {
  Assembler assembler;

  EXPECT_TRUE(true); // Just verify it compiles
}

// Test 2: Add section to assembler
TEST(AssemblerTest, AddSection) {
  Assembler assembler;

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);
  assembler.AddSection(section);

  EXPECT_EQ(assembler.GetSectionCount(), 1UL);
}

// Test 3: Add multiple sections
TEST(AssemblerTest, AddMultipleSections) {
  Assembler assembler;

  Section text(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
  Section data(".data", static_cast<uint32_t>(SectionAttributes::Data), 0x9000);

  assembler.AddSection(text);
  assembler.AddSection(data);

  EXPECT_EQ(assembler.GetSectionCount(), 2UL);
}

// Test 4: Assemble with no sections (should succeed with 0 passes)
TEST(AssemblerTest, AssembleEmpty) {
  Assembler assembler;

  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.pass_count, 0U);
  EXPECT_EQ(result.errors.size(), 0UL);
}

// Test 5: Assemble with single section
TEST(AssemblerTest, AssembleSingleSection) {
  Assembler assembler;

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);
  auto label = std::make_shared<LabelAtom>("start", 0x8000);
  section.atoms.push_back(label);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_GE(result.pass_count, 1U); // At least one pass
}

// Test 6: Pass count tracking
TEST(AssemblerTest, PassCountTracking) {
  Assembler assembler;

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code));
  auto instr = std::make_shared<InstructionAtom>("NOP", "");
  section.atoms.push_back(instr);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_GE(result.pass_count, 1U);
  EXPECT_LE(result.pass_count, 10U); // MAX_PASSES limit
}

// Test 7: Convergence (no size changes means done)
TEST(AssemblerTest, Convergence) {
  Assembler assembler;

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code));
  // Simple atoms that don't change size
  auto label = std::make_shared<LabelAtom>("start", 0x8000);
  section.atoms.push_back(label);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  // Should converge quickly since nothing changes
  EXPECT_LE(result.pass_count, 10U);
}

// Test 8: Error accumulation
TEST(AssemblerTest, ErrorAccumulation) {
  Assembler assembler;

  // For now, just verify the result has an errors vector
  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code));
  assembler.AddSection(section);

  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.errors.size(), 0UL); // No errors for valid assembly
}

// Test 9: Get pass count limits
TEST(AssemblerTest, PassLimits) {
  EXPECT_EQ(Assembler::FAST_PHASE_LIMIT, 50);
  EXPECT_EQ(Assembler::MAX_PASSES,
            10); // Empirically: 2-3 typical, 5 max observed
}

// Test 10: Reset assembler
TEST(AssemblerTest, Reset) {
  Assembler assembler;

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code));
  assembler.AddSection(section);

  EXPECT_EQ(assembler.GetSectionCount(), 1UL);

  assembler.Reset();

  EXPECT_EQ(assembler.GetSectionCount(), 0UL);
}

// Test 11: Assemble result structure
TEST(AssemblerTest, ResultStructure) {
  AssemblerResult result;
  result.success = true;
  result.pass_count = 5;

  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.pass_count, 5U);
  EXPECT_TRUE(result.errors.empty());
}

// Test 12: Multiple assemble calls
TEST(AssemblerTest, MultipleAssembleCalls) {
  Assembler assembler;

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code));
  assembler.AddSection(section);

  AssemblerResult result1 = assembler.Assemble();
  AssemblerResult result2 = assembler.Assemble();

  EXPECT_TRUE(result1.success);
  EXPECT_TRUE(result2.success);
  // Should give same results
  EXPECT_EQ(result1.pass_count, result2.pass_count);
}

// Test 13: Instruction encoding with CPU plugin
TEST(AssemblerTest, InstructionEncoding) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // Add NOP instruction (implied addressing, 1 byte: EA)
  auto nop = std::make_shared<InstructionAtom>("NOP", "");
  section.atoms.push_back(nop);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  // Verify encoded_bytes was populated
  EXPECT_FALSE(nop->encoded_bytes.empty());
  EXPECT_EQ(nop->encoded_bytes.size(), 1UL);
  EXPECT_EQ(nop->encoded_bytes[0], 0xEA); // NOP opcode
}

// Test 14: LDA immediate encoding
TEST(AssemblerTest, LDAImmediateEncoding) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LDA #$42 (immediate addressing, 2 bytes: A9 42)
  auto lda = std::make_shared<InstructionAtom>("LDA", "#$42");
  section.atoms.push_back(lda);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda->encoded_bytes.size(), 2UL);
  EXPECT_EQ(lda->encoded_bytes[0], 0xA9); // LDA immediate opcode
  EXPECT_EQ(lda->encoded_bytes[1], 0x42); // Operand
}

// Test 15: STA absolute encoding
TEST(AssemblerTest, STAAbsoluteEncoding) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // STA $1234 (absolute addressing, 3 bytes: 8D 34 12)
  auto sta = std::make_shared<InstructionAtom>("STA", "$1234");
  section.atoms.push_back(sta);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(sta->encoded_bytes.size(), 3UL);
  EXPECT_EQ(sta->encoded_bytes[0], 0x8D); // STA absolute opcode
  EXPECT_EQ(sta->encoded_bytes[1], 0x34); // Low byte
  EXPECT_EQ(sta->encoded_bytes[2], 0x12); // High byte
}

// Test 16: Symbol resolution - label as operand
TEST(AssemblerTest, LabelAsOperand) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);
  ConcreteSymbolTable symbols;

  // Define a label at address $8005
  symbols.Define("target", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8005));

  // JMP target (should resolve to JMP $8005)
  auto jmp = std::make_shared<InstructionAtom>("JMP", "target");
  section.atoms.push_back(jmp);

  assembler.AddSection(section);
  assembler.SetSymbolTable(&symbols);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(jmp->encoded_bytes.size(), 3UL);
  EXPECT_EQ(jmp->encoded_bytes[0], 0x4C); // JMP absolute opcode
  EXPECT_EQ(jmp->encoded_bytes[1], 0x05); // Low byte of $8005
  EXPECT_EQ(jmp->encoded_bytes[2], 0x80); // High byte of $8005
}

// Test 17: Backward reference - label defined before use
TEST(AssemblerTest, BackwardReference) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);
  ConcreteSymbolTable symbols;

  // Define label first at $8000
  symbols.Define("loop", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8000));

  // Then reference it later (backward reference)
  auto jmp = std::make_shared<InstructionAtom>("JMP", "loop");
  section.atoms.push_back(jmp);

  assembler.AddSection(section);
  assembler.SetSymbolTable(&symbols);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(jmp->encoded_bytes.size(), 3UL);
  EXPECT_EQ(jmp->encoded_bytes[0], 0x4C); // JMP absolute opcode
  EXPECT_EQ(jmp->encoded_bytes[1], 0x00); // Low byte of $8000
  EXPECT_EQ(jmp->encoded_bytes[2], 0x80); // High byte of $8000
}

// Test 18: Multiple label references
TEST(AssemblerTest, MultipleLabelReferences) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);
  ConcreteSymbolTable symbols;

  // Define multiple labels
  symbols.Define("start", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8000));
  symbols.Define("loop", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8010));
  symbols.Define("end", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8020));

  // Reference them in instructions
  auto jmp1 = std::make_shared<InstructionAtom>("JMP", "start");
  auto jmp2 = std::make_shared<InstructionAtom>("JMP", "loop");
  auto jmp3 = std::make_shared<InstructionAtom>("JMP", "end");
  section.atoms.push_back(jmp1);
  section.atoms.push_back(jmp2);
  section.atoms.push_back(jmp3);

  assembler.AddSection(section);
  assembler.SetSymbolTable(&symbols);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);

  // Verify first JMP to "start" ($8000)
  EXPECT_EQ(jmp1->encoded_bytes[0], 0x4C);
  EXPECT_EQ(jmp1->encoded_bytes[1], 0x00);
  EXPECT_EQ(jmp1->encoded_bytes[2], 0x80);

  // Verify second JMP to "loop" ($8010)
  EXPECT_EQ(jmp2->encoded_bytes[0], 0x4C);
  EXPECT_EQ(jmp2->encoded_bytes[1], 0x10);
  EXPECT_EQ(jmp2->encoded_bytes[2], 0x80);

  // Verify third JMP to "end" ($8020)
  EXPECT_EQ(jmp3->encoded_bytes[0], 0x4C);
  EXPECT_EQ(jmp3->encoded_bytes[1], 0x20);
  EXPECT_EQ(jmp3->encoded_bytes[2], 0x80);
}

// Test 19: Undefined label - should encode with address 0
TEST(AssemblerTest, UndefinedLabel) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);
  ConcreteSymbolTable symbols;

  // Don't define the label, just reference it
  auto jmp = std::make_shared<InstructionAtom>("JMP", "undefined_label");
  section.atoms.push_back(jmp);

  assembler.AddSection(section);
  assembler.SetSymbolTable(&symbols);
  AssemblerResult result = assembler.Assemble();

  // Should succeed (single pass, undefined label gets value 0)
  EXPECT_TRUE(result.success);
  EXPECT_EQ(jmp->encoded_bytes.size(), 3UL);
  EXPECT_EQ(jmp->encoded_bytes[0], 0x4C); // JMP absolute opcode
  EXPECT_EQ(jmp->encoded_bytes[1], 0x00); // Low byte of $0000
  EXPECT_EQ(jmp->encoded_bytes[2], 0x00); // High byte of $0000
}

// Test 20: LDA with label operand
TEST(AssemblerTest, LDAWithLabelOperand) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);
  ConcreteSymbolTable symbols;

  // Define a data label
  symbols.Define("data_addr", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x0200));

  // LDA data_addr (should resolve to LDA $0200)
  auto lda = std::make_shared<InstructionAtom>("LDA", "data_addr");
  section.atoms.push_back(lda);

  assembler.AddSection(section);
  assembler.SetSymbolTable(&symbols);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda->encoded_bytes.size(), 3UL);
  EXPECT_EQ(lda->encoded_bytes[0], 0xAD); // LDA absolute opcode
  EXPECT_EQ(lda->encoded_bytes[1], 0x00); // Low byte of $0200
  EXPECT_EQ(lda->encoded_bytes[2], 0x02); // High byte of $0200
}

// Test 21: Internal label extraction
TEST(AssemblerTest, InternalLabelExtraction) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // Add instructions and labels (simulating parsed assembly)
  auto start_label =
      std::make_shared<LabelAtom>("start", 0); // Address will be calculated
  auto jmp1 = std::make_shared<InstructionAtom>("JMP", "forward");
  auto backward_label = std::make_shared<LabelAtom>("backward", 0);
  auto nop = std::make_shared<InstructionAtom>("NOP", "");
  auto rts = std::make_shared<InstructionAtom>("RTS", "");
  auto forward_label = std::make_shared<LabelAtom>("forward", 0);
  auto jmp2 = std::make_shared<InstructionAtom>("JMP", "backward");

  section.atoms.push_back(start_label);
  section.atoms.push_back(jmp1);
  section.atoms.push_back(backward_label);
  section.atoms.push_back(nop);
  section.atoms.push_back(rts);
  section.atoms.push_back(forward_label);
  section.atoms.push_back(jmp2);

  assembler.AddSection(section);
  // Note: Not calling SetSymbolTable - assembler should extract labels
  // internally
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);

  // Verify label addresses were calculated correctly
  EXPECT_EQ(start_label->address, 0x8000ULL);
  EXPECT_EQ(backward_label->address, 0x8003ULL);
  EXPECT_EQ(forward_label->address, 0x8005ULL);

  // Verify instructions encoded with correct addresses
  EXPECT_EQ(jmp1->encoded_bytes.size(), 3UL);
  EXPECT_EQ(jmp1->encoded_bytes[0], 0x4C); // JMP opcode
  EXPECT_EQ(jmp1->encoded_bytes[1], 0x05); // Low byte of $8005
  EXPECT_EQ(jmp1->encoded_bytes[2], 0x80); // High byte of $8005

  EXPECT_EQ(jmp2->encoded_bytes.size(), 3UL);
  EXPECT_EQ(jmp2->encoded_bytes[0], 0x4C); // JMP opcode
  EXPECT_EQ(jmp2->encoded_bytes[1], 0x03); // Low byte of $8003
  EXPECT_EQ(jmp2->encoded_bytes[2], 0x80); // High byte of $8003
}

// Test 22: Forward reference - label used before definition
TEST(AssemblerTest, ForwardReference) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // First instruction references a label that will be defined later
  auto jmp = std::make_shared<InstructionAtom>("JMP", "forward_label");
  section.atoms.push_back(jmp);

  // Add some NOPs to advance the address
  section.atoms.push_back(std::make_shared<InstructionAtom>("NOP", ""));
  section.atoms.push_back(std::make_shared<InstructionAtom>("NOP", ""));

  // Define the label AFTER it's used (forward reference)
  auto label = std::make_shared<LabelAtom>("forward_label", 0x8005);
  section.atoms.push_back(label);

  assembler.AddSection(section);

  // Create symbol table and populate from section atoms
  ConcreteSymbolTable symbols;
  // In a real assembler, labels would be extracted from atoms
  // For this test, manually define the label
  symbols.Define("forward_label", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8005));

  assembler.SetSymbolTable(&symbols);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(jmp->encoded_bytes.size(), 3UL);
  EXPECT_EQ(jmp->encoded_bytes[0], 0x4C); // JMP absolute opcode
  EXPECT_EQ(jmp->encoded_bytes[1], 0x05); // Low byte of $8005
  EXPECT_EQ(jmp->encoded_bytes[2], 0x80); // High byte of $8005
}
// ============================================================================
// Group 1: Accumulator Addressing Mode Tests
// ============================================================================

// Test 23: ASL with accumulator addressing mode
TEST(AssemblerTest, ASLAccumulatorMode) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // ASL A (accumulator addressing, 1 byte: 0A)
  auto asl = std::make_shared<InstructionAtom>("ASL", "A");
  section.atoms.push_back(asl);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(asl->encoded_bytes.size(), 1UL);
  EXPECT_EQ(asl->encoded_bytes[0], 0x0A); // ASL accumulator opcode
}

// Test 24: LSR with accumulator addressing mode
TEST(AssemblerTest, LSRAccumulatorMode) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LSR A (accumulator addressing, 1 byte: 4A)
  auto lsr = std::make_shared<InstructionAtom>("LSR", "A");
  section.atoms.push_back(lsr);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lsr->encoded_bytes.size(), 1UL);
  EXPECT_EQ(lsr->encoded_bytes[0], 0x4A); // LSR accumulator opcode
}
// ============================================================================
// Group 2: Indexed Addressing Modes Tests (,X and ,Y)
// ============================================================================

// Test 25: LDA with ZeroPageX addressing
TEST(AssemblerTest, LDAZeroPageX) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LDA $80,X (ZeroPageX addressing, 2 bytes: B5 80)
  auto lda = std::make_shared<InstructionAtom>("LDA", "$80,X");
  section.atoms.push_back(lda);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda->encoded_bytes.size(), 2UL);
  EXPECT_EQ(lda->encoded_bytes[0], 0xB5); // LDA zero page,X opcode
  EXPECT_EQ(lda->encoded_bytes[1], 0x80); // Zero page address
}

// Test 26: STA with AbsoluteX addressing
TEST(AssemblerTest, STAAbsoluteX) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // STA $1234,X (AbsoluteX addressing, 3 bytes: 9D 34 12)
  auto sta = std::make_shared<InstructionAtom>("STA", "$1234,X");
  section.atoms.push_back(sta);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(sta->encoded_bytes.size(), 3UL);
  EXPECT_EQ(sta->encoded_bytes[0], 0x9D); // STA absolute,X opcode
  EXPECT_EQ(sta->encoded_bytes[1], 0x34); // Low byte
  EXPECT_EQ(sta->encoded_bytes[2], 0x12); // High byte
}

// Test 27: LDX with ZeroPageY addressing
TEST(AssemblerTest, LDXZeroPageY) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LDX $80,Y (ZeroPageY addressing, 2 bytes: B6 80)
  auto ldx = std::make_shared<InstructionAtom>("LDX", "$80,Y");
  section.atoms.push_back(ldx);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(ldx->encoded_bytes.size(), 2UL);
  EXPECT_EQ(ldx->encoded_bytes[0], 0xB6); // LDX zero page,Y opcode
  EXPECT_EQ(ldx->encoded_bytes[1], 0x80); // Zero page address
}

// Test 28: LDA with AbsoluteY addressing
TEST(AssemblerTest, LDAAbsoluteY) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LDA $1234,Y (AbsoluteY addressing, 3 bytes: B9 34 12)
  auto lda = std::make_shared<InstructionAtom>("LDA", "$1234,Y");
  section.atoms.push_back(lda);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda->encoded_bytes.size(), 3UL);
  EXPECT_EQ(lda->encoded_bytes[0], 0xB9); // LDA absolute,Y opcode
  EXPECT_EQ(lda->encoded_bytes[1], 0x34); // Low byte
  EXPECT_EQ(lda->encoded_bytes[2], 0x12); // High byte
}

// Test 29: LDA with whitespace in indexed mode
TEST(AssemblerTest, LDAIndexedWithWhitespace) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LDA $80, X (whitespace tolerance, should work like $80,X)
  auto lda = std::make_shared<InstructionAtom>("LDA", "$80, X");
  section.atoms.push_back(lda);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda->encoded_bytes.size(), 2UL);
  EXPECT_EQ(lda->encoded_bytes[0], 0xB5); // LDA zero page,X opcode
  EXPECT_EQ(lda->encoded_bytes[1], 0x80); // Zero page address
}
// ============================================================================
// Group 3: Indirect Addressing Mode Tests
// ============================================================================

// Test 30: JMP with Indirect addressing
TEST(AssemblerTest, JMPIndirect) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // JMP ($1234) (Indirect addressing, 3 bytes: 6C 34 12)
  auto jmp = std::make_shared<InstructionAtom>("JMP", "($1234)");
  section.atoms.push_back(jmp);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(jmp->encoded_bytes.size(), 3UL);
  EXPECT_EQ(jmp->encoded_bytes[0], 0x6C); // JMP indirect opcode
  EXPECT_EQ(jmp->encoded_bytes[1], 0x34); // Low byte
  EXPECT_EQ(jmp->encoded_bytes[2], 0x12); // High byte
}

// Test 31: JMP Indirect with whitespace
TEST(AssemblerTest, JMPIndirectWithWhitespace) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // JMP ( $1234 ) (whitespace tolerance)
  auto jmp = std::make_shared<InstructionAtom>("JMP", "( $1234 )");
  section.atoms.push_back(jmp);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(jmp->encoded_bytes.size(), 3UL);
  EXPECT_EQ(jmp->encoded_bytes[0], 0x6C); // JMP indirect opcode
  EXPECT_EQ(jmp->encoded_bytes[1], 0x34); // Low byte
  EXPECT_EQ(jmp->encoded_bytes[2], 0x12); // High byte
}
// ============================================================================
// Group 4: Complex Indirect Addressing Modes (IndexedIndirect and
// IndirectIndexed)
// ============================================================================

// Test 32: LDA with IndexedIndirect (pre-indexed)
TEST(AssemblerTest, LDAIndexedIndirect) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LDA ($80,X) (IndexedIndirect, 2 bytes: A1 80)
  auto lda = std::make_shared<InstructionAtom>("LDA", "($80,X)");
  section.atoms.push_back(lda);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda->encoded_bytes.size(), 2UL);
  EXPECT_EQ(lda->encoded_bytes[0], 0xA1); // LDA indexed indirect opcode
  EXPECT_EQ(lda->encoded_bytes[1], 0x80); // Zero page address
}

// Test 33: STA with IndexedIndirect
TEST(AssemblerTest, STAIndexedIndirect) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // STA ($40,X) (IndexedIndirect, 2 bytes: 81 40)
  auto sta = std::make_shared<InstructionAtom>("STA", "($40,X)");
  section.atoms.push_back(sta);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(sta->encoded_bytes.size(), 2UL);
  EXPECT_EQ(sta->encoded_bytes[0], 0x81); // STA indexed indirect opcode
  EXPECT_EQ(sta->encoded_bytes[1], 0x40); // Zero page address
}

// Test 34: LDA with IndirectIndexed (post-indexed)
TEST(AssemblerTest, LDAIndirectIndexed) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LDA ($80),Y (IndirectIndexed, 2 bytes: B1 80)
  auto lda = std::make_shared<InstructionAtom>("LDA", "($80),Y");
  section.atoms.push_back(lda);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda->encoded_bytes.size(), 2UL);
  EXPECT_EQ(lda->encoded_bytes[0], 0xB1); // LDA indirect indexed opcode
  EXPECT_EQ(lda->encoded_bytes[1], 0x80); // Zero page address
}

// Test 35: STA with IndirectIndexed
TEST(AssemblerTest, STAIndirectIndexed) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // STA ($40),Y (IndirectIndexed, 2 bytes: 91 40)
  auto sta = std::make_shared<InstructionAtom>("STA", "($40),Y");
  section.atoms.push_back(sta);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(sta->encoded_bytes.size(), 2UL);
  EXPECT_EQ(sta->encoded_bytes[0], 0x91); // STA indirect indexed opcode
  EXPECT_EQ(sta->encoded_bytes[1], 0x40); // Zero page address
}

// Test 36: IndexedIndirect with whitespace
TEST(AssemblerTest, IndexedIndirectWithWhitespace) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LDA ( $80 , X ) (whitespace tolerance)
  auto lda = std::make_shared<InstructionAtom>("LDA", "( $80 , X )");
  section.atoms.push_back(lda);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda->encoded_bytes.size(), 2UL);
  EXPECT_EQ(lda->encoded_bytes[0], 0xA1); // LDA indexed indirect opcode
  EXPECT_EQ(lda->encoded_bytes[1], 0x80); // Zero page address
}

// Test 37: IndirectIndexed with whitespace
TEST(AssemblerTest, IndirectIndexedWithWhitespace) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LDA ( $80 ) , Y (whitespace tolerance)
  auto lda = std::make_shared<InstructionAtom>("LDA", "( $80 ) , Y");
  section.atoms.push_back(lda);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda->encoded_bytes.size(), 2UL);
  EXPECT_EQ(lda->encoded_bytes[0], 0xB1); // LDA indirect indexed opcode
  EXPECT_EQ(lda->encoded_bytes[1], 0x80); // Zero page address
}
// ============================================================================
// Group 5: Integration Tests - Complete Assembly Programs
// ============================================================================

// Test 38: Zero-page indexed loop
TEST(AssemblerTest, IntegrationZeroPageIndexedLoop) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // Simple loop using zero-page indexed
  // LDX #$00
  // loop: LDA $80,X
  //       INX
  //       BNE loop
  auto ldx = std::make_shared<InstructionAtom>("LDX", "#$00");
  auto loop_label = std::make_shared<LabelAtom>("loop", 0);
  auto lda = std::make_shared<InstructionAtom>("LDA", "$80,X");
  auto inx = std::make_shared<InstructionAtom>("INX", "");
  auto bne = std::make_shared<InstructionAtom>("BNE", "loop");

  section.atoms.push_back(ldx);
  section.atoms.push_back(loop_label);
  section.atoms.push_back(lda);
  section.atoms.push_back(inx);
  section.atoms.push_back(bne);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  if (!result.success) {
    for (const auto &err : result.errors) {
      std::cerr << "Error: " << err.message << std::endl;
    }
  }
  ASSERT_TRUE(result.success)
      << "Assembly failed with " << result.errors.size() << " errors";
  ASSERT_FALSE(ldx->encoded_bytes.empty());
  ASSERT_FALSE(lda->encoded_bytes.empty());
  ASSERT_FALSE(inx->encoded_bytes.empty());
  ASSERT_FALSE(bne->encoded_bytes.empty());
  EXPECT_EQ(ldx->encoded_bytes[0], 0xA2); // LDX immediate
  EXPECT_EQ(lda->encoded_bytes[0], 0xB5); // LDA zero page,X
  EXPECT_EQ(inx->encoded_bytes[0], 0xE8); // INX
  EXPECT_EQ(bne->encoded_bytes[0], 0xD0); // BNE
}

// Test 39: Accumulator shifts
TEST(AssemblerTest, IntegrationAccumulatorShifts) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // LDA #$42
  // ASL A
  // ROL A
  // LSR A
  // ROR A
  auto lda = std::make_shared<InstructionAtom>("LDA", "#$42");
  auto asl = std::make_shared<InstructionAtom>("ASL", "A");
  auto rol = std::make_shared<InstructionAtom>("ROL", "A");
  auto lsr = std::make_shared<InstructionAtom>("LSR", "A");
  auto ror = std::make_shared<InstructionAtom>("ROR", "A");

  section.atoms.push_back(lda);
  section.atoms.push_back(asl);
  section.atoms.push_back(rol);
  section.atoms.push_back(lsr);
  section.atoms.push_back(ror);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda->encoded_bytes[0], 0xA9); // LDA immediate
  EXPECT_EQ(asl->encoded_bytes[0], 0x0A); // ASL accumulator
  EXPECT_EQ(rol->encoded_bytes[0], 0x2A); // ROL accumulator
  EXPECT_EQ(lsr->encoded_bytes[0], 0x4A); // LSR accumulator
  EXPECT_EQ(ror->encoded_bytes[0], 0x6A); // ROR accumulator
}

// Test 40: Indirect jump table
TEST(AssemblerTest, IntegrationIndirectJump) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // JMP ($1234)  ; Indirect jump through vector
  auto jmp = std::make_shared<InstructionAtom>("JMP", "($1234)");

  section.atoms.push_back(jmp);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(jmp->encoded_bytes[0], 0x6C); // JMP indirect
  EXPECT_EQ(jmp->encoded_bytes[1], 0x34);
  EXPECT_EQ(jmp->encoded_bytes[2], 0x12);
}

// Test 41: IndexedIndirect addressing (sprite rendering)
TEST(AssemblerTest, IntegrationIndexedIndirect) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // Typical sprite rendering pattern
  // LDY #$00
  // loop: LDA ($40,X)
  //       STA ($80),Y
  //       INY
  //       BNE loop
  auto ldy = std::make_shared<InstructionAtom>("LDY", "#$00");
  auto loop_label = std::make_shared<LabelAtom>("loop", 0);
  auto lda = std::make_shared<InstructionAtom>("LDA", "($40,X)");
  auto sta = std::make_shared<InstructionAtom>("STA", "($80),Y");
  auto iny = std::make_shared<InstructionAtom>("INY", "");
  auto bne = std::make_shared<InstructionAtom>("BNE", "loop");

  section.atoms.push_back(ldy);
  section.atoms.push_back(loop_label);
  section.atoms.push_back(lda);
  section.atoms.push_back(sta);
  section.atoms.push_back(iny);
  section.atoms.push_back(bne);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  if (!result.success) {
    for (const auto &err : result.errors) {
      std::cerr << "Error: " << err.message << std::endl;
    }
  }
  ASSERT_TRUE(result.success)
      << "Assembly failed with " << result.errors.size() << " errors";
  ASSERT_FALSE(ldy->encoded_bytes.empty());
  ASSERT_FALSE(lda->encoded_bytes.empty());
  ASSERT_FALSE(sta->encoded_bytes.empty());
  ASSERT_FALSE(iny->encoded_bytes.empty());
  ASSERT_FALSE(bne->encoded_bytes.empty());
  EXPECT_EQ(ldy->encoded_bytes[0], 0xA0); // LDY immediate
  EXPECT_EQ(lda->encoded_bytes[0], 0xA1); // LDA indexed indirect
  EXPECT_EQ(sta->encoded_bytes[0], 0x91); // STA indirect indexed
  EXPECT_EQ(iny->encoded_bytes[0], 0xC8); // INY
  EXPECT_EQ(bne->encoded_bytes[0], 0xD0); // BNE
}

// Test 42: Mixed addressing modes
TEST(AssemblerTest, IntegrationMixedAddressingModes) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x8000);

  // Demonstrate all major addressing modes in one program
  auto lda_imm = std::make_shared<InstructionAtom>("LDA", "#$42");  // Immediate
  auto sta_zp = std::make_shared<InstructionAtom>("STA", "$80");    // ZeroPage
  auto lda_zpx = std::make_shared<InstructionAtom>("LDA", "$80,X"); // ZeroPageX
  auto sta_abs = std::make_shared<InstructionAtom>("STA", "$1234"); // Absolute
  auto lda_absx =
      std::make_shared<InstructionAtom>("LDA", "$1234,X"); // AbsoluteX
  auto lda_absy =
      std::make_shared<InstructionAtom>("LDA", "$1234,Y");    // AbsoluteY
  auto asl_a = std::make_shared<InstructionAtom>("ASL", "A"); // Accumulator
  auto lda_indx =
      std::make_shared<InstructionAtom>("LDA", "($40,X)"); // IndexedIndirect
  auto sta_indy =
      std::make_shared<InstructionAtom>("STA", "($50),Y"); // IndirectIndexed
  auto jmp_ind =
      std::make_shared<InstructionAtom>("JMP", "($FFFC)"); // Indirect

  section.atoms.push_back(lda_imm);
  section.atoms.push_back(sta_zp);
  section.atoms.push_back(lda_zpx);
  section.atoms.push_back(sta_abs);
  section.atoms.push_back(lda_absx);
  section.atoms.push_back(lda_absy);
  section.atoms.push_back(asl_a);
  section.atoms.push_back(lda_indx);
  section.atoms.push_back(sta_indy);
  section.atoms.push_back(jmp_ind);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(lda_imm->encoded_bytes[0], 0xA9);  // LDA immediate
  EXPECT_EQ(sta_zp->encoded_bytes[0], 0x85);   // STA zero page
  EXPECT_EQ(lda_zpx->encoded_bytes[0], 0xB5);  // LDA zero page,X
  EXPECT_EQ(sta_abs->encoded_bytes[0], 0x8D);  // STA absolute
  EXPECT_EQ(lda_absx->encoded_bytes[0], 0xBD); // LDA absolute,X
  EXPECT_EQ(lda_absy->encoded_bytes[0], 0xB9); // LDA absolute,Y
  EXPECT_EQ(asl_a->encoded_bytes[0], 0x0A);    // ASL accumulator
  EXPECT_EQ(lda_indx->encoded_bytes[0], 0xA1); // LDA indexed indirect
  EXPECT_EQ(sta_indy->encoded_bytes[0], 0x91); // STA indirect indexed
  EXPECT_EQ(jmp_ind->encoded_bytes[0], 0x6C);  // JMP indirect
}

// ============================================================================
// Group 6: Branch Relaxation Tests
// ============================================================================

// Test 43: Long branch that needs relaxation (currently FAILS)
TEST(AssemblerTest, LongBranchNeedsRelaxation) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // Create a branch with target out of range (+512 bytes, max is +127)
  auto beq = std::make_shared<InstructionAtom>("BEQ", "far_target");

  // Add padding (255 NOPs = 255 bytes, makes target > 127 bytes away)
  section.atoms.push_back(beq);
  for (int i = 0; i < 255; i++) {
    section.atoms.push_back(std::make_shared<InstructionAtom>("NOP", ""));
  }

  auto far_label = std::make_shared<LabelAtom>("far_target", 0);
  section.atoms.push_back(far_label);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  // Currently this throws an error - should succeed with relaxation
  if (!result.success) {
    for (const auto &err : result.errors) {
      std::cerr << "Error: " << err.message << std::endl;
    }
  }
  ASSERT_TRUE(result.success)
      << "Assembly failed with " << result.errors.size() << " errors";

  // Relaxed branch should be 5 bytes: BNE *+5; JMP target
  ASSERT_FALSE(beq->encoded_bytes.empty());
  EXPECT_EQ(beq->encoded_bytes.size(), 5UL);
  EXPECT_EQ(beq->encoded_bytes[0], 0xD0); // BNE (complement of BEQ)
  EXPECT_EQ(beq->encoded_bytes[1], 0x03); // Skip 3 bytes (JMP instruction)
  EXPECT_EQ(beq->encoded_bytes[2], 0x4C); // JMP opcode
}

// Test 44: Short branch that doesn't need relaxation
TEST(AssemblerTest, ShortBranchNoRelaxation) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // Create a short branch (within +127 bytes)
  auto beq = std::make_shared<InstructionAtom>("BEQ", "near_target");

  // Add a few NOPs (10 bytes, well within range)
  section.atoms.push_back(beq);
  for (int i = 0; i < 10; i++) {
    section.atoms.push_back(std::make_shared<InstructionAtom>("NOP", ""));
  }

  auto near_label = std::make_shared<LabelAtom>("near_target", 0);
  section.atoms.push_back(near_label);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  if (!result.success) {
    for (const auto &err : result.errors) {
      std::cerr << "Error: " << err.message << std::endl;
    }
  }
  ASSERT_TRUE(result.success)
      << "Assembly failed with " << result.errors.size() << " errors";

  // Short branch should be 2 bytes: BEQ offset
  ASSERT_FALSE(beq->encoded_bytes.empty());
  EXPECT_EQ(beq->encoded_bytes.size(), 2UL);
  EXPECT_EQ(beq->encoded_bytes[0], 0xF0); // BEQ opcode
  EXPECT_EQ(beq->encoded_bytes[1], 0x0A); // Offset = +10
}

// Test 45: Backward branch (negative offset)
TEST(AssemblerTest, BackwardBranch) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // Create a backward branch
  auto loop_label = std::make_shared<LabelAtom>("loop", 0);
  section.atoms.push_back(loop_label);

  // Add a few instructions
  section.atoms.push_back(std::make_shared<InstructionAtom>("NOP", ""));
  section.atoms.push_back(std::make_shared<InstructionAtom>("DEX", ""));
  auto bne = std::make_shared<InstructionAtom>("BNE", "loop");
  section.atoms.push_back(bne);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  if (!result.success) {
    for (const auto &err : result.errors) {
      std::cerr << "Error: " << err.message << std::endl;
    }
  }
  ASSERT_TRUE(result.success)
      << "Assembly failed with " << result.errors.size() << " errors";

  // Backward branch should be 2 bytes: BNE offset
  ASSERT_FALSE(bne->encoded_bytes.empty());
  EXPECT_EQ(bne->encoded_bytes.size(), 2UL);
  EXPECT_EQ(bne->encoded_bytes[0], 0xD0); // BNE opcode
  // Offset should be -4 (back to loop label)
  EXPECT_EQ(bne->encoded_bytes[1], 0xFC); // -4 in two's complement
}

// ============================================================================
// Group 7: Dynamic Cast Safety Tests (C2 Critical Issue)
// ============================================================================

// Test 46: OrgAtom cast failure detection (Pass 1)
TEST(AssemblerTest, OrgAtomCastFailure_Pass1) {
  // This test creates a corrupt atom to verify error handling
  // In practice, this should never happen, but defensive programming requires
  // checking

  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // Create a base Atom with Org type (simulates corruption)
  auto corrupt_atom = std::make_shared<Atom>(AtomType::Org);
  section.atoms.push_back(corrupt_atom);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  // Should fail with error about cast failure
  EXPECT_FALSE(result.success);
  EXPECT_GT(result.errors.size(), 0UL);

  // Error message should mention the cast failure
  bool found_cast_error = false;
  for (const auto &error : result.errors) {
    if (error.message.find("cast") != std::string::npos &&
        error.message.find("OrgAtom") != std::string::npos) {
      found_cast_error = true;
      break;
    }
  }
  EXPECT_TRUE(found_cast_error);
}

// Test 47: InstructionAtom cast failure detection (Pass 1)
TEST(AssemblerTest, InstructionAtomCastFailure_Pass1) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // Create a base Atom with Instruction type (simulates corruption)
  auto corrupt_atom = std::make_shared<Atom>(AtomType::Instruction);
  section.atoms.push_back(corrupt_atom);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  // Should fail with error about cast failure
  EXPECT_FALSE(result.success);
  EXPECT_GT(result.errors.size(), 0UL);

  // Error message should mention the cast failure
  bool found_cast_error = false;
  for (const auto &error : result.errors) {
    if (error.message.find("cast") != std::string::npos &&
        error.message.find("InstructionAtom") != std::string::npos) {
      found_cast_error = true;
      break;
    }
  }
  EXPECT_TRUE(found_cast_error);
}

// Test 48: LabelAtom cast failure detection (Pass 2)
TEST(AssemblerTest, LabelAtomCastFailure_Pass2) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // Create a base Atom with Label type (simulates corruption)
  auto corrupt_atom = std::make_shared<Atom>(AtomType::Label);
  section.atoms.push_back(corrupt_atom);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  // Should fail with error about cast failure
  EXPECT_FALSE(result.success);
  EXPECT_GT(result.errors.size(), 0UL);

  // Error message should mention the cast failure
  bool found_cast_error = false;
  for (const auto &error : result.errors) {
    if (error.message.find("cast") != std::string::npos &&
        error.message.find("LabelAtom") != std::string::npos) {
      found_cast_error = true;
      break;
    }
  }
  EXPECT_TRUE(found_cast_error);
}

// Test 49: OrgAtom cast failure detection (Pass 2)
TEST(AssemblerTest, OrgAtomCastFailure_Pass2) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // Add a valid instruction first to trigger Pass 2
  section.atoms.push_back(std::make_shared<InstructionAtom>("NOP", ""));

  // Create a base Atom with Org type (simulates corruption)
  auto corrupt_atom = std::make_shared<Atom>(AtomType::Org);
  section.atoms.push_back(corrupt_atom);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  // Should fail with error about cast failure
  EXPECT_FALSE(result.success);
  EXPECT_GT(result.errors.size(), 0UL);

  // Error message should mention the cast failure
  bool found_cast_error = false;
  for (const auto &error : result.errors) {
    if (error.message.find("cast") != std::string::npos &&
        error.message.find("OrgAtom") != std::string::npos) {
      found_cast_error = true;
      break;
    }
  }
  EXPECT_TRUE(found_cast_error);
}

// Test 50: InstructionAtom cast failure detection (Pass 2)
TEST(AssemblerTest, InstructionAtomCastFailure_Pass2) {
  Assembler assembler;
  Cpu6502 cpu;
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // Add a valid label first to trigger Pass 2
  section.atoms.push_back(std::make_shared<LabelAtom>("start", 0x1000));

  // Create a base Atom with Instruction type (simulates corruption)
  auto corrupt_atom = std::make_shared<Atom>(AtomType::Instruction);
  section.atoms.push_back(corrupt_atom);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  // Should fail with error about cast failure
  EXPECT_FALSE(result.success);
  EXPECT_GT(result.errors.size(), 0UL);

  // Error message should mention the cast failure
  bool found_cast_error = false;
  for (const auto &error : result.errors) {
    if (error.message.find("cast") != std::string::npos &&
        error.message.find("InstructionAtom") != std::string::npos) {
      found_cast_error = true;
      break;
    }
  }
  EXPECT_TRUE(found_cast_error);
}

// Test PHB - Push Data Bank Register (65816)
TEST(AssemblerTest, PHB_65816) {
  Assembler assembler;
  Cpu6502 cpu;
  cpu.SetCpuMode(CpuMode::Cpu65816);
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // PHB -> 8B
  auto instr = std::make_shared<InstructionAtom>("PHB", "");
  section.atoms.push_back(instr);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  ASSERT_EQ(instr->encoded_bytes.size(), 1UL);
  EXPECT_EQ(instr->encoded_bytes[0], 0x8B); // PHB opcode
}

// Test PLB - Pull Data Bank Register (65816)
TEST(AssemblerTest, PLB_65816) {
  Assembler assembler;
  Cpu6502 cpu;
  cpu.SetCpuMode(CpuMode::Cpu65816);
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // PLB -> AB
  auto instr = std::make_shared<InstructionAtom>("PLB", "");
  section.atoms.push_back(instr);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  ASSERT_EQ(instr->encoded_bytes.size(), 1UL);
  EXPECT_EQ(instr->encoded_bytes[0], 0xAB); // PLB opcode
}

// Test PHX - Push X Register (65C02+)
TEST(AssemblerTest, PHX_65C02) {
  Assembler assembler;
  Cpu6502 cpu;
  cpu.SetCpuMode(CpuMode::Cpu65C02);
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // PHX -> DA
  auto instr = std::make_shared<InstructionAtom>("PHX", "");
  section.atoms.push_back(instr);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  ASSERT_EQ(instr->encoded_bytes.size(), 1UL);
  EXPECT_EQ(instr->encoded_bytes[0], 0xDA); // PHX opcode
}

// Test PLX - Pull X Register (65C02+)
TEST(AssemblerTest, PLX_65C02) {
  Assembler assembler;
  Cpu6502 cpu;
  cpu.SetCpuMode(CpuMode::Cpu65C02);
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // PLX -> FA
  auto instr = std::make_shared<InstructionAtom>("PLX", "");
  section.atoms.push_back(instr);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  ASSERT_EQ(instr->encoded_bytes.size(), 1UL);
  EXPECT_EQ(instr->encoded_bytes[0], 0xFA); // PLX opcode
}

// Test PHY - Push Y Register (65C02+)
TEST(AssemblerTest, PHY_65C02) {
  Assembler assembler;
  Cpu6502 cpu;
  cpu.SetCpuMode(CpuMode::Cpu65C02);
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // PHY -> 5A
  auto instr = std::make_shared<InstructionAtom>("PHY", "");
  section.atoms.push_back(instr);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  ASSERT_EQ(instr->encoded_bytes.size(), 1UL);
  EXPECT_EQ(instr->encoded_bytes[0], 0x5A); // PHY opcode
}

// Test PLY - Pull Y Register (65C02+)
TEST(AssemblerTest, PLY_65C02) {
  Assembler assembler;
  Cpu6502 cpu;
  cpu.SetCpuMode(CpuMode::Cpu65C02);
  assembler.SetCpuPlugin(&cpu);

  Section section(".text", static_cast<uint32_t>(SectionAttributes::Code),
                  0x1000);

  // PLY -> 7A
  auto instr = std::make_shared<InstructionAtom>("PLY", "");
  section.atoms.push_back(instr);

  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();

  EXPECT_TRUE(result.success);
  ASSERT_EQ(instr->encoded_bytes.size(), 1UL);
  EXPECT_EQ(instr->encoded_bytes[0], 0x7A); // PLY opcode
}
