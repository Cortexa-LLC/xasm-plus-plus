// Assembler tests
// Phase 1: Minimal Viable Assembler - Assembler Engine

#include "xasm++/assembler.h"
#include "xasm++/section.h"
#include "xasm++/atom.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>

using namespace xasm;

// Test 1: Assembler creation
TEST(AssemblerTest, Creation) {
    Assembler assembler;

    EXPECT_TRUE(true);  // Just verify it compiles
}

// Test 2: Add section to assembler
TEST(AssemblerTest, AddSection) {
    Assembler assembler;

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
    assembler.AddSection(section);

    EXPECT_EQ(assembler.GetSectionCount(), 1);
}

// Test 3: Add multiple sections
TEST(AssemblerTest, AddMultipleSections) {
    Assembler assembler;

    Section text(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
    Section data(".data", static_cast<uint32_t>(SectionAttributes::Data), 0x9000);

    assembler.AddSection(text);
    assembler.AddSection(data);

    EXPECT_EQ(assembler.GetSectionCount(), 2);
}

// Test 4: Assemble with no sections (should succeed with 0 passes)
TEST(AssemblerTest, AssembleEmpty) {
    Assembler assembler;

    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.pass_count, 0);
    EXPECT_EQ(result.errors.size(), 0);
}

// Test 5: Assemble with single section
TEST(AssemblerTest, AssembleSingleSection) {
    Assembler assembler;

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
    auto label = std::make_shared<LabelAtom>("start", 0x8000);
    section.atoms.push_back(label);

    assembler.AddSection(section);
    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);
    EXPECT_GE(result.pass_count, 1);  // At least one pass
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
    EXPECT_GE(result.pass_count, 1);
    EXPECT_LE(result.pass_count, 500);  // MAX_PASSES limit
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
    EXPECT_LE(result.pass_count, 10);
}

// Test 8: Error accumulation
TEST(AssemblerTest, ErrorAccumulation) {
    Assembler assembler;

    // For now, just verify the result has an errors vector
    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code));
    assembler.AddSection(section);

    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.errors.size(), 0);  // No errors for valid assembly
}

// Test 9: Get pass count limits
TEST(AssemblerTest, PassLimits) {
    EXPECT_EQ(Assembler::FAST_PHASE_LIMIT, 50);
    EXPECT_EQ(Assembler::MAX_PASSES, 500);
}

// Test 10: Reset assembler
TEST(AssemblerTest, Reset) {
    Assembler assembler;

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code));
    assembler.AddSection(section);

    EXPECT_EQ(assembler.GetSectionCount(), 1);

    assembler.Reset();

    EXPECT_EQ(assembler.GetSectionCount(), 0);
}

// Test 11: Assemble result structure
TEST(AssemblerTest, ResultStructure) {
    AssemblerResult result;
    result.success = true;
    result.pass_count = 5;

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.pass_count, 5);
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

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);

    // Add NOP instruction (implied addressing, 1 byte: EA)
    auto nop = std::make_shared<InstructionAtom>("NOP", "");
    section.atoms.push_back(nop);

    assembler.AddSection(section);
    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);
    // Verify encoded_bytes was populated
    EXPECT_FALSE(nop->encoded_bytes.empty());
    EXPECT_EQ(nop->encoded_bytes.size(), 1);
    EXPECT_EQ(nop->encoded_bytes[0], 0xEA);  // NOP opcode
}

// Test 14: LDA immediate encoding
TEST(AssemblerTest, LDAImmediateEncoding) {
    Assembler assembler;
    Cpu6502 cpu;
    assembler.SetCpuPlugin(&cpu);

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);

    // LDA #$42 (immediate addressing, 2 bytes: A9 42)
    auto lda = std::make_shared<InstructionAtom>("LDA", "#$42");
    section.atoms.push_back(lda);

    assembler.AddSection(section);
    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);
    EXPECT_EQ(lda->encoded_bytes.size(), 2);
    EXPECT_EQ(lda->encoded_bytes[0], 0xA9);  // LDA immediate opcode
    EXPECT_EQ(lda->encoded_bytes[1], 0x42);  // Operand
}

// Test 15: STA absolute encoding
TEST(AssemblerTest, STAAbsoluteEncoding) {
    Assembler assembler;
    Cpu6502 cpu;
    assembler.SetCpuPlugin(&cpu);

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);

    // STA $1234 (absolute addressing, 3 bytes: 8D 34 12)
    auto sta = std::make_shared<InstructionAtom>("STA", "$1234");
    section.atoms.push_back(sta);

    assembler.AddSection(section);
    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);
    EXPECT_EQ(sta->encoded_bytes.size(), 3);
    EXPECT_EQ(sta->encoded_bytes[0], 0x8D);  // STA absolute opcode
    EXPECT_EQ(sta->encoded_bytes[1], 0x34);  // Low byte
    EXPECT_EQ(sta->encoded_bytes[2], 0x12);  // High byte
}

// Test 16: Symbol resolution - label as operand
TEST(AssemblerTest, LabelAsOperand) {
    Assembler assembler;
    Cpu6502 cpu;
    assembler.SetCpuPlugin(&cpu);

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
    ConcreteSymbolTable symbols;

    // Define a label at address $8005
    symbols.Define("target", SymbolType::Label, std::make_shared<LiteralExpr>(0x8005));

    // JMP target (should resolve to JMP $8005)
    auto jmp = std::make_shared<InstructionAtom>("JMP", "target");
    section.atoms.push_back(jmp);

    assembler.AddSection(section);
    assembler.SetSymbolTable(&symbols);
    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);
    EXPECT_EQ(jmp->encoded_bytes.size(), 3);
    EXPECT_EQ(jmp->encoded_bytes[0], 0x4C);  // JMP absolute opcode
    EXPECT_EQ(jmp->encoded_bytes[1], 0x05);  // Low byte of $8005
    EXPECT_EQ(jmp->encoded_bytes[2], 0x80);  // High byte of $8005
}

// Test 17: Backward reference - label defined before use
TEST(AssemblerTest, BackwardReference) {
    Assembler assembler;
    Cpu6502 cpu;
    assembler.SetCpuPlugin(&cpu);

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
    ConcreteSymbolTable symbols;

    // Define label first at $8000
    symbols.Define("loop", SymbolType::Label, std::make_shared<LiteralExpr>(0x8000));

    // Then reference it later (backward reference)
    auto jmp = std::make_shared<InstructionAtom>("JMP", "loop");
    section.atoms.push_back(jmp);

    assembler.AddSection(section);
    assembler.SetSymbolTable(&symbols);
    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);
    EXPECT_EQ(jmp->encoded_bytes.size(), 3);
    EXPECT_EQ(jmp->encoded_bytes[0], 0x4C);  // JMP absolute opcode
    EXPECT_EQ(jmp->encoded_bytes[1], 0x00);  // Low byte of $8000
    EXPECT_EQ(jmp->encoded_bytes[2], 0x80);  // High byte of $8000
}

// Test 18: Multiple label references
TEST(AssemblerTest, MultipleLabelReferences) {
    Assembler assembler;
    Cpu6502 cpu;
    assembler.SetCpuPlugin(&cpu);

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
    ConcreteSymbolTable symbols;

    // Define multiple labels
    symbols.Define("start", SymbolType::Label, std::make_shared<LiteralExpr>(0x8000));
    symbols.Define("loop", SymbolType::Label, std::make_shared<LiteralExpr>(0x8010));
    symbols.Define("end", SymbolType::Label, std::make_shared<LiteralExpr>(0x8020));

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

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
    ConcreteSymbolTable symbols;

    // Don't define the label, just reference it
    auto jmp = std::make_shared<InstructionAtom>("JMP", "undefined_label");
    section.atoms.push_back(jmp);

    assembler.AddSection(section);
    assembler.SetSymbolTable(&symbols);
    AssemblerResult result = assembler.Assemble();

    // Should succeed (single pass, undefined label gets value 0)
    EXPECT_TRUE(result.success);
    EXPECT_EQ(jmp->encoded_bytes.size(), 3);
    EXPECT_EQ(jmp->encoded_bytes[0], 0x4C);  // JMP absolute opcode
    EXPECT_EQ(jmp->encoded_bytes[1], 0x00);  // Low byte of $0000
    EXPECT_EQ(jmp->encoded_bytes[2], 0x00);  // High byte of $0000
}

// Test 20: LDA with label operand
TEST(AssemblerTest, LDAWithLabelOperand) {
    Assembler assembler;
    Cpu6502 cpu;
    assembler.SetCpuPlugin(&cpu);

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
    ConcreteSymbolTable symbols;

    // Define a data label
    symbols.Define("data_addr", SymbolType::Label, std::make_shared<LiteralExpr>(0x0200));

    // LDA data_addr (should resolve to LDA $0200)
    auto lda = std::make_shared<InstructionAtom>("LDA", "data_addr");
    section.atoms.push_back(lda);

    assembler.AddSection(section);
    assembler.SetSymbolTable(&symbols);
    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);
    EXPECT_EQ(lda->encoded_bytes.size(), 3);
    EXPECT_EQ(lda->encoded_bytes[0], 0xAD);  // LDA absolute opcode
    EXPECT_EQ(lda->encoded_bytes[1], 0x00);  // Low byte of $0200
    EXPECT_EQ(lda->encoded_bytes[2], 0x02);  // High byte of $0200
}

// Test 21: Internal label extraction
TEST(AssemblerTest, InternalLabelExtraction) {
    Assembler assembler;
    Cpu6502 cpu;
    assembler.SetCpuPlugin(&cpu);

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);

    // Add instructions and labels (simulating parsed assembly)
    auto start_label = std::make_shared<LabelAtom>("start", 0);  // Address will be calculated
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
    // Note: Not calling SetSymbolTable - assembler should extract labels internally
    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);

    // Verify label addresses were calculated correctly
    EXPECT_EQ(start_label->address, 0x8000);
    EXPECT_EQ(backward_label->address, 0x8003);
    EXPECT_EQ(forward_label->address, 0x8005);

    // Verify instructions encoded with correct addresses
    EXPECT_EQ(jmp1->encoded_bytes.size(), 3);
    EXPECT_EQ(jmp1->encoded_bytes[0], 0x4C);  // JMP opcode
    EXPECT_EQ(jmp1->encoded_bytes[1], 0x05);  // Low byte of $8005
    EXPECT_EQ(jmp1->encoded_bytes[2], 0x80);  // High byte of $8005

    EXPECT_EQ(jmp2->encoded_bytes.size(), 3);
    EXPECT_EQ(jmp2->encoded_bytes[0], 0x4C);  // JMP opcode
    EXPECT_EQ(jmp2->encoded_bytes[1], 0x03);  // Low byte of $8003
    EXPECT_EQ(jmp2->encoded_bytes[2], 0x80);  // High byte of $8003
}

// Test 22: Forward reference - label used before definition
TEST(AssemblerTest, ForwardReference) {
    Assembler assembler;
    Cpu6502 cpu;
    assembler.SetCpuPlugin(&cpu);

    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);

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
    symbols.Define("forward_label", SymbolType::Label, std::make_shared<LiteralExpr>(0x8005));

    assembler.SetSymbolTable(&symbols);
    AssemblerResult result = assembler.Assemble();

    EXPECT_TRUE(result.success);
    EXPECT_EQ(jmp->encoded_bytes.size(), 3);
    EXPECT_EQ(jmp->encoded_bytes[0], 0x4C);  // JMP absolute opcode
    EXPECT_EQ(jmp->encoded_bytes[1], 0x05);  // Low byte of $8005
    EXPECT_EQ(jmp->encoded_bytes[2], 0x80);  // High byte of $8005
}
