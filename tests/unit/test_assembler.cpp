// Assembler tests
// Phase 1: Minimal Viable Assembler - Assembler Engine

#include "xasm++/assembler.h"
#include "xasm++/section.h"
#include "xasm++/atom.h"
#include "xasm++/cpu/cpu_6502.h"
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
