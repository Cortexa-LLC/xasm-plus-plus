// 6502 CPU Plugin tests
// Phase 1: Minimal Viable Assembler - 6502 CPU Plugin

#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>

using namespace xasm;

// Test 1: CPU creation
TEST(Cpu6502Test, Creation) {
    Cpu6502 cpu;

    EXPECT_EQ(cpu.GetName(), "6502");
}

// Test 2: LDA Immediate encoding (#$42)
TEST(Cpu6502Test, LDA_Immediate) {
    Cpu6502 cpu;
    ConcreteSymbolTable symbols;

    // LDA #$42 -> A9 42
    auto bytes = cpu.EncodeLDA(0x42, AddressingMode::Immediate);

    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xA9);  // LDA immediate opcode
    EXPECT_EQ(bytes[1], 0x42);  // Operand
}

// Test 3: LDA Zero Page encoding ($80)
TEST(Cpu6502Test, LDA_ZeroPage) {
    Cpu6502 cpu;

    // LDA $80 -> A5 80
    auto bytes = cpu.EncodeLDA(0x80, AddressingMode::ZeroPage);

    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xA5);  // LDA zero page opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 4: LDA Absolute encoding ($1234)
TEST(Cpu6502Test, LDA_Absolute) {
    Cpu6502 cpu;

    // LDA $1234 -> AD 34 12 (little-endian)
    auto bytes = cpu.EncodeLDA(0x1234, AddressingMode::Absolute);

    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0xAD);  // LDA absolute opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// Test 5: STA Zero Page encoding
TEST(Cpu6502Test, STA_ZeroPage) {
    Cpu6502 cpu;

    // STA $80 -> 85 80
    auto bytes = cpu.EncodeSTA(0x80, AddressingMode::ZeroPage);

    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x85);  // STA zero page opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 6: STA Absolute encoding
TEST(Cpu6502Test, STA_Absolute) {
    Cpu6502 cpu;

    // STA $1234 -> 8D 34 12
    auto bytes = cpu.EncodeSTA(0x1234, AddressingMode::Absolute);

    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x8D);  // STA absolute opcode
    EXPECT_EQ(bytes[1], 0x34);
    EXPECT_EQ(bytes[2], 0x12);
}

// Test 7: JMP Absolute encoding
TEST(Cpu6502Test, JMP_Absolute) {
    Cpu6502 cpu;

    // JMP $8000 -> 4C 00 80
    auto bytes = cpu.EncodeJMP(0x8000, AddressingMode::Absolute);

    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x4C);  // JMP absolute opcode
    EXPECT_EQ(bytes[1], 0x00);
    EXPECT_EQ(bytes[2], 0x80);
}

// Test 8: NOP encoding
TEST(Cpu6502Test, NOP) {
    Cpu6502 cpu;

    // NOP -> EA
    auto bytes = cpu.EncodeNOP();

    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xEA);  // NOP opcode
}

// Test 9: RTS encoding
TEST(Cpu6502Test, RTS) {
    Cpu6502 cpu;

    // RTS -> 60
    auto bytes = cpu.EncodeRTS();

    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x60);  // RTS opcode
}

// Test 10: AddressingMode enum
TEST(Cpu6502Test, AddressingModes) {
    AddressingMode imm = AddressingMode::Immediate;
    AddressingMode zp = AddressingMode::ZeroPage;
    AddressingMode abs = AddressingMode::Absolute;

    EXPECT_NE(imm, zp);
    EXPECT_NE(zp, abs);
    EXPECT_NE(imm, abs);
}

// Test 11: Instruction size calculation
TEST(Cpu6502Test, CalculateSize) {
    Cpu6502 cpu;

    EXPECT_EQ(cpu.CalculateInstructionSize(AddressingMode::Immediate), 2);
    EXPECT_EQ(cpu.CalculateInstructionSize(AddressingMode::ZeroPage), 2);
    EXPECT_EQ(cpu.CalculateInstructionSize(AddressingMode::Absolute), 3);
    EXPECT_EQ(cpu.CalculateInstructionSize(AddressingMode::Implied), 1);
}

// Test 12: Multiple instructions
TEST(Cpu6502Test, MultipleInstructions) {
    Cpu6502 cpu;

    auto lda = cpu.EncodeLDA(0x42, AddressingMode::Immediate);
    auto sta = cpu.EncodeSTA(0x80, AddressingMode::ZeroPage);
    auto nop = cpu.EncodeNOP();
    auto rts = cpu.EncodeRTS();

    EXPECT_EQ(lda[0], 0xA9);
    EXPECT_EQ(sta[0], 0x85);
    EXPECT_EQ(nop[0], 0xEA);
    EXPECT_EQ(rts[0], 0x60);
}

// Group 1: Arithmetic Instructions

// Test 13: ADC immediate
TEST(Cpu6502Test, ADC_Immediate) {
    Cpu6502 cpu;
    auto result = cpu.EncodeADC(0x42, AddressingMode::Immediate);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x69);  // ADC immediate opcode
    EXPECT_EQ(result[1], 0x42);  // Operand
}

// Test 14: ADC zero page
TEST(Cpu6502Test, ADC_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeADC(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x65);  // ADC zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 15: ADC absolute
TEST(Cpu6502Test, ADC_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeADC(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x6D);  // ADC absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 16: SBC immediate
TEST(Cpu6502Test, SBC_Immediate) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSBC(0x42, AddressingMode::Immediate);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xE9);  // SBC immediate opcode
    EXPECT_EQ(result[1], 0x42);  // Operand
}

// Test 17: SBC zero page
TEST(Cpu6502Test, SBC_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSBC(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xE5);  // SBC zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 18: SBC absolute
TEST(Cpu6502Test, SBC_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSBC(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xED);  // SBC absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Group 2: Logic Instructions

// Test 19: AND immediate
TEST(Cpu6502Test, AND_Immediate) {
    Cpu6502 cpu;
    auto result = cpu.EncodeAND(0x42, AddressingMode::Immediate);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x29);  // AND immediate opcode
    EXPECT_EQ(result[1], 0x42);  // Operand
}

// Test 20: AND zero page
TEST(Cpu6502Test, AND_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeAND(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x25);  // AND zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 21: AND absolute
TEST(Cpu6502Test, AND_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeAND(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x2D);  // AND absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 22: ORA immediate
TEST(Cpu6502Test, ORA_Immediate) {
    Cpu6502 cpu;
    auto result = cpu.EncodeORA(0x42, AddressingMode::Immediate);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x09);  // ORA immediate opcode
    EXPECT_EQ(result[1], 0x42);  // Operand
}

// Test 23: ORA zero page
TEST(Cpu6502Test, ORA_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeORA(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x05);  // ORA zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 24: ORA absolute
TEST(Cpu6502Test, ORA_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeORA(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x0D);  // ORA absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 25: EOR immediate
TEST(Cpu6502Test, EOR_Immediate) {
    Cpu6502 cpu;
    auto result = cpu.EncodeEOR(0x42, AddressingMode::Immediate);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x49);  // EOR immediate opcode
    EXPECT_EQ(result[1], 0x42);  // Operand
}

// Test 26: EOR zero page
TEST(Cpu6502Test, EOR_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeEOR(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x45);  // EOR zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 27: EOR absolute
TEST(Cpu6502Test, EOR_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeEOR(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x4D);  // EOR absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Group 3: Additional Loads/Stores

// Test 28: LDX immediate
TEST(Cpu6502Test, LDX_Immediate) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDX(0x42, AddressingMode::Immediate);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xA2);  // LDX immediate opcode
    EXPECT_EQ(result[1], 0x42);  // Operand
}

// Test 29: LDX zero page
TEST(Cpu6502Test, LDX_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDX(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xA6);  // LDX zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 30: LDX absolute
TEST(Cpu6502Test, LDX_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDX(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xAE);  // LDX absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 31: LDY immediate
TEST(Cpu6502Test, LDY_Immediate) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDY(0x42, AddressingMode::Immediate);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xA0);  // LDY immediate opcode
    EXPECT_EQ(result[1], 0x42);  // Operand
}

// Test 32: LDY zero page
TEST(Cpu6502Test, LDY_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDY(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xA4);  // LDY zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 33: LDY absolute
TEST(Cpu6502Test, LDY_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDY(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xAC);  // LDY absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 34: STX zero page
TEST(Cpu6502Test, STX_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTX(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x86);  // STX zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 35: STX absolute
TEST(Cpu6502Test, STX_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTX(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x8E);  // STX absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 36: STY zero page
TEST(Cpu6502Test, STY_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTY(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x84);  // STY zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 37: STY absolute
TEST(Cpu6502Test, STY_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTY(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x8C);  // STY absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Group 4: Comparisons

// Test 38: CMP immediate
TEST(Cpu6502Test, CMP_Immediate) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCMP(0x42, AddressingMode::Immediate);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xC9);  // CMP immediate opcode
    EXPECT_EQ(result[1], 0x42);  // Operand
}

// Test 39: CMP zero page
TEST(Cpu6502Test, CMP_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCMP(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xC5);  // CMP zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 40: CMP absolute
TEST(Cpu6502Test, CMP_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCMP(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xCD);  // CMP absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 41: CPX immediate
TEST(Cpu6502Test, CPX_Immediate) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCPX(0x42, AddressingMode::Immediate);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xE0);  // CPX immediate opcode
    EXPECT_EQ(result[1], 0x42);  // Operand
}

// Test 42: CPX zero page
TEST(Cpu6502Test, CPX_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCPX(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xE4);  // CPX zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 43: CPX absolute
TEST(Cpu6502Test, CPX_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCPX(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xEC);  // CPX absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 44: CPY immediate
TEST(Cpu6502Test, CPY_Immediate) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCPY(0x42, AddressingMode::Immediate);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xC0);  // CPY immediate opcode
    EXPECT_EQ(result[1], 0x42);  // Operand
}

// Test 45: CPY zero page
TEST(Cpu6502Test, CPY_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCPY(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xC4);  // CPY zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 46: CPY absolute
TEST(Cpu6502Test, CPY_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCPY(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xCC);  // CPY absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Group 5: Branch Instructions

// Test 47: BEQ relative
TEST(Cpu6502Test, BEQ_Relative) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBEQ(0x10, AddressingMode::Relative);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xF0);  // BEQ opcode
    EXPECT_EQ(result[1], 0x10);  // Relative offset
}

// Test 48: BNE relative
TEST(Cpu6502Test, BNE_Relative) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBNE(0x10, AddressingMode::Relative);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xD0);  // BNE opcode
    EXPECT_EQ(result[1], 0x10);  // Relative offset
}

// Test 49: BCC relative
TEST(Cpu6502Test, BCC_Relative) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBCC(0x10, AddressingMode::Relative);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x90);  // BCC opcode
    EXPECT_EQ(result[1], 0x10);  // Relative offset
}

// Test 50: BCS relative
TEST(Cpu6502Test, BCS_Relative) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBCS(0x10, AddressingMode::Relative);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xB0);  // BCS opcode
    EXPECT_EQ(result[1], 0x10);  // Relative offset
}

// Test 51: BMI relative
TEST(Cpu6502Test, BMI_Relative) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBMI(0x10, AddressingMode::Relative);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x30);  // BMI opcode
    EXPECT_EQ(result[1], 0x10);  // Relative offset
}

// Test 52: BPL relative
TEST(Cpu6502Test, BPL_Relative) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBPL(0x10, AddressingMode::Relative);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x10);  // BPL opcode
    EXPECT_EQ(result[1], 0x10);  // Relative offset
}

// Test 53: BVC relative
TEST(Cpu6502Test, BVC_Relative) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBVC(0x10, AddressingMode::Relative);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x50);  // BVC opcode
    EXPECT_EQ(result[1], 0x10);  // Relative offset
}

// Test 54: BVS relative
TEST(Cpu6502Test, BVS_Relative) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBVS(0x10, AddressingMode::Relative);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x70);  // BVS opcode
    EXPECT_EQ(result[1], 0x10);  // Relative offset
}

// Group 6: Inc/Dec Instructions

// Test 55: INX implied
TEST(Cpu6502Test, INX_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeINX();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0xE8);  // INX opcode
}

// Test 56: INY implied
TEST(Cpu6502Test, INY_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeINY();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0xC8);  // INY opcode
}

// Test 57: DEX implied
TEST(Cpu6502Test, DEX_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeDEX();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0xCA);  // DEX opcode
}

// Test 58: DEY implied
TEST(Cpu6502Test, DEY_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeDEY();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x88);  // DEY opcode
}

// Test 59: INC zero page
TEST(Cpu6502Test, INC_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeINC(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xE6);  // INC zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 60: INC absolute
TEST(Cpu6502Test, INC_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeINC(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xEE);  // INC absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 61: DEC zero page
TEST(Cpu6502Test, DEC_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeDEC(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xC6);  // DEC zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Address
}

// Test 62: DEC absolute
TEST(Cpu6502Test, DEC_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeDEC(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xCE);  // DEC absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Group 7: Stack Operations

// Test 63: PHA implied
TEST(Cpu6502Test, PHA_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodePHA();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x48);  // PHA opcode
}

// Test 64: PLA implied
TEST(Cpu6502Test, PLA_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodePLA();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x68);  // PLA opcode
}

// Test 65: PHP implied
TEST(Cpu6502Test, PHP_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodePHP();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x08);  // PHP opcode
}

// Test 66: PLP implied
TEST(Cpu6502Test, PLP_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodePLP();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x28);  // PLP opcode
}
