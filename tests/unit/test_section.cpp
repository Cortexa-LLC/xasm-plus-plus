// Section tests
// Phase 1: Minimal Viable Assembler - Core Abstractions

#include "xasm++/section.h"
#include "xasm++/atom.h"
#include <gtest/gtest.h>

using namespace xasm;

// Test 1: SectionAttributes enum values
TEST(SectionTest, AttributeEnumValues) {
    EXPECT_EQ(static_cast<uint32_t>(SectionAttributes::Code), 0x01);
    EXPECT_EQ(static_cast<uint32_t>(SectionAttributes::Data), 0x02);
    EXPECT_EQ(static_cast<uint32_t>(SectionAttributes::Bss), 0x04);
    EXPECT_EQ(static_cast<uint32_t>(SectionAttributes::Read), 0x08);
    EXPECT_EQ(static_cast<uint32_t>(SectionAttributes::Write), 0x10);
    EXPECT_EQ(static_cast<uint32_t>(SectionAttributes::Execute), 0x20);
}

// Test 2: Basic section creation
TEST(SectionTest, BasicCreation) {
    Section section;
    section.name = ".text";
    section.attributes = static_cast<uint32_t>(SectionAttributes::Code);
    section.org = 0;

    EXPECT_EQ(section.name, ".text");
    EXPECT_EQ(section.attributes, 0x01);
    EXPECT_EQ(section.org, 0);
    EXPECT_EQ(section.current_offset, 0);
    EXPECT_TRUE(section.atoms.empty());
}

// Test 3: Section with constructor
TEST(SectionTest, ConstructorCreation) {
    Section section(".data", static_cast<uint32_t>(SectionAttributes::Data), 0x2000);

    EXPECT_EQ(section.name, ".data");
    EXPECT_EQ(section.attributes, 0x02);
    EXPECT_EQ(section.org, 0x2000);
    EXPECT_EQ(section.current_offset, 0);
}

// Test 4: Section with default org
TEST(SectionTest, DefaultOrg) {
    Section section;
    section.name = ".data";
    section.attributes = static_cast<uint32_t>(SectionAttributes::Data);

    EXPECT_EQ(section.org, 0);
}

// Test 5: Section with custom org
TEST(SectionTest, CustomOrg) {
    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);

    EXPECT_EQ(section.org, 0x8000);
}

// Test 6: Adding atoms to section
TEST(SectionTest, AddAtoms) {
    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code));

    auto label = std::make_shared<LabelAtom>("start", 0x8000);
    auto instr = std::make_shared<InstructionAtom>("LDA", "#$00");

    section.atoms.push_back(label);
    section.atoms.push_back(instr);

    EXPECT_EQ(section.atoms.size(), 2);
    EXPECT_EQ(section.atoms[0], label);
    EXPECT_EQ(section.atoms[1], instr);
}

// Test 7: Offset tracking
TEST(SectionTest, OffsetTracking) {
    Section section(".text", static_cast<uint32_t>(SectionAttributes::Code));

    auto instr = std::make_shared<InstructionAtom>("LDA", "#$00");
    section.atoms.push_back(instr);
    section.current_offset += 2;  // Assume 2-byte instruction

    EXPECT_EQ(section.current_offset, 2);

    auto instr2 = std::make_shared<InstructionAtom>("STA", "$1000");
    section.atoms.push_back(instr2);
    section.current_offset += 3;  // Assume 3-byte instruction

    EXPECT_EQ(section.current_offset, 5);
}

// Test 8: Multiple sections
TEST(SectionTest, MultipleSections) {
    Section text_section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);
    Section data_section(".data", static_cast<uint32_t>(SectionAttributes::Data), 0x9000);

    EXPECT_EQ(text_section.name, ".text");
    EXPECT_EQ(text_section.org, 0x8000);
    EXPECT_EQ(data_section.name, ".data");
    EXPECT_EQ(data_section.org, 0x9000);
}

// Test 9: Bitwise attribute flags
TEST(SectionTest, BitwiseAttributeFlags) {
    uint32_t attrs = static_cast<uint32_t>(SectionAttributes::Code) |
                     static_cast<uint32_t>(SectionAttributes::Read) |
                     static_cast<uint32_t>(SectionAttributes::Execute);

    Section section(".text", attrs, 0x8000);

    EXPECT_EQ(section.attributes, 0x01 | 0x08 | 0x20);
    EXPECT_EQ(section.attributes, 0x29);

    // Verify individual flags are set
    EXPECT_TRUE(section.attributes & static_cast<uint32_t>(SectionAttributes::Code));
    EXPECT_TRUE(section.attributes & static_cast<uint32_t>(SectionAttributes::Read));
    EXPECT_TRUE(section.attributes & static_cast<uint32_t>(SectionAttributes::Execute));
    EXPECT_FALSE(section.attributes & static_cast<uint32_t>(SectionAttributes::Write));
}

// Test 10: BSS section
TEST(SectionTest, BssSection) {
    uint32_t attrs = static_cast<uint32_t>(SectionAttributes::Bss) |
                     static_cast<uint32_t>(SectionAttributes::Read) |
                     static_cast<uint32_t>(SectionAttributes::Write);

    Section section(".bss", attrs, 0xA000);

    EXPECT_EQ(section.name, ".bss");
    EXPECT_EQ(section.org, 0xA000);
    EXPECT_TRUE(section.attributes & static_cast<uint32_t>(SectionAttributes::Bss));
    EXPECT_TRUE(section.attributes & static_cast<uint32_t>(SectionAttributes::Read));
    EXPECT_TRUE(section.attributes & static_cast<uint32_t>(SectionAttributes::Write));
}

// Test 11: Data section with atoms
TEST(SectionTest, DataSectionWithAtoms) {
    uint32_t attrs = static_cast<uint32_t>(SectionAttributes::Data) |
                     static_cast<uint32_t>(SectionAttributes::Read) |
                     static_cast<uint32_t>(SectionAttributes::Write);

    Section section(".data", attrs, 0x9000);

    auto data1 = std::make_shared<DataAtom>(std::vector<uint8_t>{0x01, 0x02, 0x03});
    auto data2 = std::make_shared<DataAtom>(std::vector<uint8_t>{0xFF, 0xFE});

    section.atoms.push_back(data1);
    section.current_offset += 3;

    section.atoms.push_back(data2);
    section.current_offset += 2;

    EXPECT_EQ(section.atoms.size(), 2);
    EXPECT_EQ(section.current_offset, 5);
}

// Test 12: Empty section initialization
TEST(SectionTest, EmptySection) {
    Section section;

    EXPECT_TRUE(section.name.empty());
    EXPECT_EQ(section.attributes, 0);
    EXPECT_EQ(section.org, 0);
    EXPECT_EQ(section.current_offset, 0);
    EXPECT_TRUE(section.atoms.empty());
}
