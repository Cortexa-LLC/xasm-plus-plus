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

// Group 8: Subroutine

// Test 67: JSR absolute
TEST(Cpu6502Test, JSR_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeJSR(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x20);  // JSR absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// ============================================================================
// Phase 2.3: Complete 6502 Instruction Set
// ============================================================================

// Group 1: BIT - Test Bits

// Test 68: BIT zero page
TEST(Cpu6502Test, BIT_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBIT(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x24);  // BIT zero page opcode
    EXPECT_EQ(result[1], 0x80);  // Zero page address
}

// Test 69: BIT absolute
TEST(Cpu6502Test, BIT_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBIT(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x2C);  // BIT absolute opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Group 2: Shift Instructions - ASL, LSR

// Test 70: ASL accumulator
TEST(Cpu6502Test, ASL_Accumulator) {
    Cpu6502 cpu;
    auto result = cpu.EncodeASL(0, AddressingMode::Accumulator);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x0A);  // ASL A opcode
}

// Test 71: ASL zero page
TEST(Cpu6502Test, ASL_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeASL(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x06);  // ASL zp opcode
    EXPECT_EQ(result[1], 0x80);  // Zero page address
}

// Test 72: ASL zero page,X
TEST(Cpu6502Test, ASL_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeASL(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x16);  // ASL zp,X opcode
    EXPECT_EQ(result[1], 0x80);  // Zero page address
}

// Test 73: ASL absolute
TEST(Cpu6502Test, ASL_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeASL(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x0E);  // ASL abs opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 74: ASL absolute,X
TEST(Cpu6502Test, ASL_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeASL(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x1E);  // ASL abs,X opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 75: LSR accumulator
TEST(Cpu6502Test, LSR_Accumulator) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLSR(0, AddressingMode::Accumulator);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x4A);  // LSR A opcode
}

// Test 76: LSR zero page
TEST(Cpu6502Test, LSR_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLSR(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x46);  // LSR zp opcode
    EXPECT_EQ(result[1], 0x80);  // Zero page address
}

// Test 77: LSR zero page,X
TEST(Cpu6502Test, LSR_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLSR(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x56);  // LSR zp,X opcode
    EXPECT_EQ(result[1], 0x80);  // Zero page address
}

// Test 78: LSR absolute
TEST(Cpu6502Test, LSR_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLSR(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x4E);  // LSR abs opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 79: LSR absolute,X
TEST(Cpu6502Test, LSR_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLSR(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x5E);  // LSR abs,X opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Group 3: Rotate Instructions - ROL, ROR

// Test 80: ROL accumulator
TEST(Cpu6502Test, ROL_Accumulator) {
    Cpu6502 cpu;
    auto result = cpu.EncodeROL(0, AddressingMode::Accumulator);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x2A);  // ROL A opcode
}

// Test 81: ROL zero page
TEST(Cpu6502Test, ROL_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeROL(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x26);  // ROL zp opcode
    EXPECT_EQ(result[1], 0x80);  // Zero page address
}

// Test 82: ROL zero page,X
TEST(Cpu6502Test, ROL_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeROL(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x36);  // ROL zp,X opcode
    EXPECT_EQ(result[1], 0x80);  // Zero page address
}

// Test 83: ROL absolute
TEST(Cpu6502Test, ROL_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeROL(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x2E);  // ROL abs opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 84: ROL absolute,X
TEST(Cpu6502Test, ROL_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeROL(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x3E);  // ROL abs,X opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 85: ROR accumulator
TEST(Cpu6502Test, ROR_Accumulator) {
    Cpu6502 cpu;
    auto result = cpu.EncodeROR(0, AddressingMode::Accumulator);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x6A);  // ROR A opcode
}

// Test 86: ROR zero page
TEST(Cpu6502Test, ROR_ZeroPage) {
    Cpu6502 cpu;
    auto result = cpu.EncodeROR(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x66);  // ROR zp opcode
    EXPECT_EQ(result[1], 0x80);  // Zero page address
}

// Test 87: ROR zero page,X
TEST(Cpu6502Test, ROR_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeROR(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x76);  // ROR zp,X opcode
    EXPECT_EQ(result[1], 0x80);  // Zero page address
}

// Test 88: ROR absolute
TEST(Cpu6502Test, ROR_Absolute) {
    Cpu6502 cpu;
    auto result = cpu.EncodeROR(0x1234, AddressingMode::Absolute);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x6E);  // ROR abs opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Test 89: ROR absolute,X
TEST(Cpu6502Test, ROR_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeROR(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x7E);  // ROR abs,X opcode
    EXPECT_EQ(result[1], 0x34);  // Low byte
    EXPECT_EQ(result[2], 0x12);  // High byte
}

// Group 4: Interrupt Instructions - RTI, BRK

// Test 90: RTI implied
TEST(Cpu6502Test, RTI_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeRTI();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x40);  // RTI opcode
}

// Test 91: BRK implied
TEST(Cpu6502Test, BRK_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeBRK();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x00);  // BRK opcode
}

// Group 5: Flag Operations

// Test 92: CLC implied
TEST(Cpu6502Test, CLC_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCLC();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x18);  // CLC opcode
}

// Test 93: SEC implied
TEST(Cpu6502Test, SEC_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSEC();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x38);  // SEC opcode
}

// Test 94: CLD implied
TEST(Cpu6502Test, CLD_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCLD();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0xD8);  // CLD opcode
}

// Test 95: SED implied
TEST(Cpu6502Test, SED_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSED();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0xF8);  // SED opcode
}

// Test 96: CLI implied
TEST(Cpu6502Test, CLI_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCLI();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x58);  // CLI opcode
}

// Test 97: SEI implied
TEST(Cpu6502Test, SEI_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSEI();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x78);  // SEI opcode
}

// Test 98: CLV implied
TEST(Cpu6502Test, CLV_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCLV();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0xB8);  // CLV opcode
}

// Group 6: Transfer Instructions

// Test 99: TSX implied
TEST(Cpu6502Test, TSX_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeTSX();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0xBA);  // TSX opcode
}

// Test 100: TXS implied
TEST(Cpu6502Test, TXS_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeTXS();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x9A);  // TXS opcode
}

// Test 101: TAX implied
TEST(Cpu6502Test, TAX_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeTAX();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0xAA);  // TAX opcode
}

// Test 102: TAY implied
TEST(Cpu6502Test, TAY_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeTAY();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0xA8);  // TAY opcode
}

// Test 103: TXA implied
TEST(Cpu6502Test, TXA_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeTXA();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x8A);  // TXA opcode
}

// Test 104: TYA implied
TEST(Cpu6502Test, TYA_Implied) {
    Cpu6502 cpu;
    auto result = cpu.EncodeTYA();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x98);  // TYA opcode
}

// Group 7: Indexed Addressing Modes for Existing Instructions

// Test 105: LDA zero page,X
TEST(Cpu6502Test, LDA_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDA(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xB5);  // LDA zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 106: LDA absolute,X
TEST(Cpu6502Test, LDA_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDA(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xBD);  // LDA abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 107: LDA absolute,Y
TEST(Cpu6502Test, LDA_AbsoluteY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDA(0x1234, AddressingMode::AbsoluteY);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xB9);  // LDA abs,Y opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 108: STA zero page,X
TEST(Cpu6502Test, STA_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTA(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x95);  // STA zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 109: STA absolute,X
TEST(Cpu6502Test, STA_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTA(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x9D);  // STA abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 110: STA absolute,Y
TEST(Cpu6502Test, STA_AbsoluteY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTA(0x1234, AddressingMode::AbsoluteY);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x99);  // STA abs,Y opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 111: LDX zero page,Y
TEST(Cpu6502Test, LDX_ZeroPageY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDX(0x80, AddressingMode::ZeroPageY);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xB6);  // LDX zp,Y opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 112: LDX absolute,Y
TEST(Cpu6502Test, LDX_AbsoluteY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDX(0x1234, AddressingMode::AbsoluteY);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xBE);  // LDX abs,Y opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 113: STX zero page,Y
TEST(Cpu6502Test, STX_ZeroPageY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTX(0x80, AddressingMode::ZeroPageY);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x96);  // STX zp,Y opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 114: LDY zero page,X
TEST(Cpu6502Test, LDY_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDY(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xB4);  // LDY zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 115: LDY absolute,X
TEST(Cpu6502Test, LDY_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDY(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xBC);  // LDY abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 116: STY zero page,X
TEST(Cpu6502Test, STY_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTY(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x94);  // STY zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 117: AND zero page,X
TEST(Cpu6502Test, AND_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeAND(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x35);  // AND zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 118: AND absolute,X
TEST(Cpu6502Test, AND_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeAND(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x3D);  // AND abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 119: AND absolute,Y
TEST(Cpu6502Test, AND_AbsoluteY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeAND(0x1234, AddressingMode::AbsoluteY);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x39);  // AND abs,Y opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 120: ORA zero page,X
TEST(Cpu6502Test, ORA_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeORA(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x15);  // ORA zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 121: ORA absolute,X
TEST(Cpu6502Test, ORA_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeORA(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x1D);  // ORA abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 122: ORA absolute,Y
TEST(Cpu6502Test, ORA_AbsoluteY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeORA(0x1234, AddressingMode::AbsoluteY);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x19);  // ORA abs,Y opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 123: EOR zero page,X
TEST(Cpu6502Test, EOR_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeEOR(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x55);  // EOR zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 124: EOR absolute,X
TEST(Cpu6502Test, EOR_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeEOR(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x5D);  // EOR abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 125: EOR absolute,Y
TEST(Cpu6502Test, EOR_AbsoluteY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeEOR(0x1234, AddressingMode::AbsoluteY);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x59);  // EOR abs,Y opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 126: ADC zero page,X
TEST(Cpu6502Test, ADC_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeADC(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x75);  // ADC zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 127: ADC absolute,X
TEST(Cpu6502Test, ADC_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeADC(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x7D);  // ADC abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 128: ADC absolute,Y
TEST(Cpu6502Test, ADC_AbsoluteY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeADC(0x1234, AddressingMode::AbsoluteY);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x79);  // ADC abs,Y opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 129: SBC zero page,X
TEST(Cpu6502Test, SBC_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSBC(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xF5);  // SBC zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 130: SBC absolute,X
TEST(Cpu6502Test, SBC_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSBC(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xFD);  // SBC abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 131: SBC absolute,Y
TEST(Cpu6502Test, SBC_AbsoluteY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSBC(0x1234, AddressingMode::AbsoluteY);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xF9);  // SBC abs,Y opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 132: CMP zero page,X
TEST(Cpu6502Test, CMP_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCMP(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xD5);  // CMP zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 133: CMP absolute,X
TEST(Cpu6502Test, CMP_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCMP(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xDD);  // CMP abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 134: CMP absolute,Y
TEST(Cpu6502Test, CMP_AbsoluteY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCMP(0x1234, AddressingMode::AbsoluteY);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xD9);  // CMP abs,Y opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 135: INC zero page,X
TEST(Cpu6502Test, INC_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeINC(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xF6);  // INC zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 136: INC absolute,X
TEST(Cpu6502Test, INC_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeINC(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xFE);  // INC abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 137: DEC zero page,X
TEST(Cpu6502Test, DEC_ZeroPageX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeDEC(0x80, AddressingMode::ZeroPageX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xD6);  // DEC zp,X opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 138: DEC absolute,X
TEST(Cpu6502Test, DEC_AbsoluteX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeDEC(0x1234, AddressingMode::AbsoluteX);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0xDE);  // DEC abs,X opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Group 8: Indirect Addressing Modes

// Test 139: JMP indirect
TEST(Cpu6502Test, JMP_Indirect) {
    Cpu6502 cpu;
    auto result = cpu.EncodeJMP(0x1234, AddressingMode::Indirect);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 0x6C);  // JMP (ind) opcode
    EXPECT_EQ(result[1], 0x34);
    EXPECT_EQ(result[2], 0x12);
}

// Test 140: LDA indexed indirect (zp,X)
TEST(Cpu6502Test, LDA_IndirectX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDA(0x80, AddressingMode::IndirectX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xA1);  // LDA (zp,X) opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 141: LDA indirect indexed (zp),Y
TEST(Cpu6502Test, LDA_IndirectY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeLDA(0x80, AddressingMode::IndirectY);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xB1);  // LDA (zp),Y opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 142: STA indexed indirect (zp,X)
TEST(Cpu6502Test, STA_IndirectX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTA(0x80, AddressingMode::IndirectX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x81);  // STA (zp,X) opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 143: STA indirect indexed (zp),Y
TEST(Cpu6502Test, STA_IndirectY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSTA(0x80, AddressingMode::IndirectY);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x91);  // STA (zp),Y opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 144: CMP indexed indirect (zp,X)
TEST(Cpu6502Test, CMP_IndirectX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCMP(0x80, AddressingMode::IndirectX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xC1);  // CMP (zp,X) opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 145: CMP indirect indexed (zp),Y
TEST(Cpu6502Test, CMP_IndirectY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeCMP(0x80, AddressingMode::IndirectY);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xD1);  // CMP (zp),Y opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 146: AND indexed indirect (zp,X)
TEST(Cpu6502Test, AND_IndirectX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeAND(0x80, AddressingMode::IndirectX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x21);  // AND (zp,X) opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 147: AND indirect indexed (zp),Y
TEST(Cpu6502Test, AND_IndirectY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeAND(0x80, AddressingMode::IndirectY);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x31);  // AND (zp),Y opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 148: ORA indexed indirect (zp,X)
TEST(Cpu6502Test, ORA_IndirectX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeORA(0x80, AddressingMode::IndirectX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x01);  // ORA (zp,X) opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 149: ORA indirect indexed (zp),Y
TEST(Cpu6502Test, ORA_IndirectY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeORA(0x80, AddressingMode::IndirectY);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x11);  // ORA (zp),Y opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 150: EOR indexed indirect (zp,X)
TEST(Cpu6502Test, EOR_IndirectX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeEOR(0x80, AddressingMode::IndirectX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x41);  // EOR (zp,X) opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 151: EOR indirect indexed (zp),Y
TEST(Cpu6502Test, EOR_IndirectY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeEOR(0x80, AddressingMode::IndirectY);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x51);  // EOR (zp),Y opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 152: ADC indexed indirect (zp,X)
TEST(Cpu6502Test, ADC_IndirectX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeADC(0x80, AddressingMode::IndirectX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x61);  // ADC (zp,X) opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 153: ADC indirect indexed (zp),Y
TEST(Cpu6502Test, ADC_IndirectY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeADC(0x80, AddressingMode::IndirectY);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x71);  // ADC (zp),Y opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 154: SBC indexed indirect (zp,X)
TEST(Cpu6502Test, SBC_IndirectX) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSBC(0x80, AddressingMode::IndirectX);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xE1);  // SBC (zp,X) opcode
    EXPECT_EQ(result[1], 0x80);
}

// Test 155: SBC indirect indexed (zp),Y
TEST(Cpu6502Test, SBC_IndirectY) {
    Cpu6502 cpu;
    auto result = cpu.EncodeSBC(0x80, AddressingMode::IndirectY);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0xF1);  // SBC (zp),Y opcode
    EXPECT_EQ(result[1], 0x80);
}

// ============================================================================
// Phase 2.5: 65C02 and 65816 Support
// Group 1: CPU Mode Infrastructure
// ============================================================================

// Test 156: Default CPU mode is 6502
TEST(Cpu6502Test, DefaultCpuModeIs6502) {
    Cpu6502 cpu;
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);
}

// Test 157: Set CPU mode to 65C02
TEST(Cpu6502Test, SetCpuModeTo65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);
}

// Test 158: Set CPU mode to 65C02-Rockwell
TEST(Cpu6502Test, SetCpuModeTo65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02Rock);
}

// Test 159: Set CPU mode to 65816
TEST(Cpu6502Test, SetCpuModeTo65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65816);
}

// Test 160: CPU mode can be changed multiple times
TEST(Cpu6502Test, CpuModeCanBeChangedMultipleTimes) {
    Cpu6502 cpu;

    // Start with default (6502)
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

    // Switch to 65C02
    cpu.SetCpuMode(CpuMode::Cpu65C02);
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65C02);

    // Switch to 65816
    cpu.SetCpuMode(CpuMode::Cpu65816);
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu65816);

    // Switch back to 6502
    cpu.SetCpuMode(CpuMode::Cpu6502);
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);
}

// ============================================================================
// Group 2: 65C02 Stack Operations
// ============================================================================

// Test 161: PHX (Push X to stack) - 65C02
TEST(Cpu6502Test, PHX_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // PHX -> DA
    auto bytes = cpu.EncodePHX();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xDA);  // PHX opcode
}

// Test 162: PLX (Pull X from stack) - 65C02
TEST(Cpu6502Test, PLX_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // PLX -> FA
    auto bytes = cpu.EncodePLX();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xFA);  // PLX opcode
}

// Test 163: PHY (Push Y to stack) - 65C02
TEST(Cpu6502Test, PHY_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // PHY -> 5A
    auto bytes = cpu.EncodePHY();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x5A);  // PHY opcode
}

// Test 164: PLY (Pull Y from stack) - 65C02
TEST(Cpu6502Test, PLY_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // PLY -> 7A
    auto bytes = cpu.EncodePLY();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x7A);  // PLY opcode
}

// Test 165: PHX should return empty in 6502 mode (not available)
TEST(Cpu6502Test, PHX_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

    // PHX not available in 6502 mode - should return empty
    auto bytes = cpu.EncodePHX();
    EXPECT_EQ(bytes.size(), 0);  // Empty = not supported in this mode
}

// ============================================================================
// Group 3: 65C02 Store Zero (STZ)
// ============================================================================

// Test 166: STZ zero page - 65C02
TEST(Cpu6502Test, STZ_ZeroPage_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // STZ $80 -> 64 80
    auto bytes = cpu.EncodeSTZ(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x64);  // STZ zp opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 167: STZ zero page,X - 65C02
TEST(Cpu6502Test, STZ_ZeroPageX_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // STZ $80,X -> 74 80
    auto bytes = cpu.EncodeSTZ(0x80, AddressingMode::ZeroPageX);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x74);  // STZ zp,X opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 168: STZ absolute - 65C02
TEST(Cpu6502Test, STZ_Absolute_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // STZ $1234 -> 9C 34 12
    auto bytes = cpu.EncodeSTZ(0x1234, AddressingMode::Absolute);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x9C);  // STZ abs opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// Test 169: STZ absolute,X - 65C02
TEST(Cpu6502Test, STZ_AbsoluteX_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // STZ $1234,X -> 9E 34 12
    auto bytes = cpu.EncodeSTZ(0x1234, AddressingMode::AbsoluteX);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x9E);  // STZ abs,X opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// Test 170: STZ should return empty in 6502 mode (not available)
TEST(Cpu6502Test, STZ_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

    // STZ not available in 6502 mode - should return empty
    auto bytes = cpu.EncodeSTZ(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(bytes.size(), 0);  // Empty = not supported in this mode
}

// ============================================================================
// Group 4: 65C02 Bit Test (TRB/TSB)
// ============================================================================

// Test 171: TRB zero page - 65C02
TEST(Cpu6502Test, TRB_ZeroPage_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // TRB $80 -> 14 80
    auto bytes = cpu.EncodeTRB(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x14);  // TRB zp opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 172: TRB absolute - 65C02
TEST(Cpu6502Test, TRB_Absolute_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // TRB $1234 -> 1C 34 12
    auto bytes = cpu.EncodeTRB(0x1234, AddressingMode::Absolute);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x1C);  // TRB abs opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// Test 173: TSB zero page - 65C02
TEST(Cpu6502Test, TSB_ZeroPage_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // TSB $80 -> 04 80
    auto bytes = cpu.EncodeTSB(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x04);  // TSB zp opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 174: TSB absolute - 65C02
TEST(Cpu6502Test, TSB_Absolute_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // TSB $1234 -> 0C 34 12
    auto bytes = cpu.EncodeTSB(0x1234, AddressingMode::Absolute);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x0C);  // TSB abs opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// Test 175: TRB should return empty in 6502 mode (not available)
TEST(Cpu6502Test, TRB_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

    // TRB not available in 6502 mode - should return empty
    auto bytes = cpu.EncodeTRB(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(bytes.size(), 0);  // Empty = not supported in this mode
}

// ============================================================================
// Group 5: 65C02 Branch Always (BRA)
// ============================================================================

// Test 176: BRA forward branch - 65C02
TEST(Cpu6502Test, BRA_ForwardBranch_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // BRA +10 -> 80 0A (forward branch of 10 bytes)
    auto bytes = cpu.EncodeBRA(0x0A, AddressingMode::Relative);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x80);  // BRA opcode
    EXPECT_EQ(bytes[1], 0x0A);  // Relative offset
}

// Test 177: BRA backward branch - 65C02
TEST(Cpu6502Test, BRA_BackwardBranch_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // BRA -10 -> 80 F6 (backward branch, two's complement)
    auto bytes = cpu.EncodeBRA(0xF6, AddressingMode::Relative);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x80);  // BRA opcode
    EXPECT_EQ(bytes[1], 0xF6);  // Relative offset (negative)
}

// Test 178: BRA should return empty in 6502 mode (not available)
TEST(Cpu6502Test, BRA_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

    // BRA not available in 6502 mode - should return empty
    auto bytes = cpu.EncodeBRA(0x0A, AddressingMode::Relative);
    EXPECT_EQ(bytes.size(), 0);  // Empty = not supported in this mode
}

// ============================================================================
// Group 6: 65C02 Enhanced Addressing Modes
// ============================================================================

// Test 179: LDA indirect (no Y) - 65C02 enhancement
// Note: Original 6502 only has ($80),Y. 65C02 adds ($80) without Y index.
TEST(Cpu6502Test, LDA_Indirect_NoY_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // LDA ($80) -> B2 80 (65C02 new addressing mode)
    auto bytes = cpu.EncodeLDA(0x80, AddressingMode::IndirectZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xB2);  // LDA (zp) opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 180: JMP indexed indirect - 65C02 enhancement
// Note: Original 6502 only has JMP ($1234). 65C02 adds JMP ($1234,X).
TEST(Cpu6502Test, JMP_IndexedIndirect_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // JMP ($1234,X) -> 7C 34 12 (65C02 new addressing mode)
    auto bytes = cpu.EncodeJMP(0x1234, AddressingMode::AbsoluteIndexedIndirect);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x7C);  // JMP (abs,X) opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// Test 181: BIT immediate - 65C02 enhancement
// Note: Original 6502 only has BIT zp and BIT abs. 65C02 adds BIT #$80.
TEST(Cpu6502Test, BIT_Immediate_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // BIT #$80 -> 89 80 (65C02 new addressing mode)
    auto bytes = cpu.EncodeBIT(0x80, AddressingMode::Immediate);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x89);  // BIT imm opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 182: BIT zero page,X - 65C02 enhancement
TEST(Cpu6502Test, BIT_ZeroPageX_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // BIT $80,X -> 34 80 (65C02 new addressing mode)
    auto bytes = cpu.EncodeBIT(0x80, AddressingMode::ZeroPageX);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x34);  // BIT zp,X opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 183: BIT absolute,X - 65C02 enhancement
TEST(Cpu6502Test, BIT_AbsoluteX_65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    // BIT $1234,X -> 3C 34 12 (65C02 new addressing mode)
    auto bytes = cpu.EncodeBIT(0x1234, AddressingMode::AbsoluteX);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x3C);  // BIT abs,X opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// Test 184: Enhanced addressing modes should fail in 6502 mode
TEST(Cpu6502Test, EnhancedModes_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502
    EXPECT_EQ(cpu.GetCpuMode(), CpuMode::Cpu6502);

    // LDA (zp) - not available in 6502 mode
    auto bytes1 = cpu.EncodeLDA(0x80, AddressingMode::IndirectZeroPage);
    EXPECT_EQ(bytes1.size(), 0);  // Empty = not supported

    // JMP (abs,X) - not available in 6502 mode
    auto bytes2 = cpu.EncodeJMP(0x1234, AddressingMode::AbsoluteIndexedIndirect);
    EXPECT_EQ(bytes2.size(), 0);  // Empty = not supported

    // BIT immediate - not available in 6502 mode
    auto bytes3 = cpu.EncodeBIT(0x80, AddressingMode::Immediate);
    EXPECT_EQ(bytes3.size(), 0);  // Empty = not supported
}

// ============================================================================
// Group 8: 65816 MX Directive Infrastructure
// ============================================================================

// Test 185: Default MX state in 65816 mode (emulation mode = 8-bit)
TEST(Cpu6502Test, MX_DefaultStateIs8Bit) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // Default should be MX %11 (both 8-bit, emulation mode)
    EXPECT_TRUE(cpu.IsAccumulator8Bit());
    EXPECT_TRUE(cpu.IsIndex8Bit());
}

// Test 186: SetMX %00 - Both 16-bit
TEST(Cpu6502Test, MX_SetBoth16Bit) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // MX %00 - A is 16-bit, X/Y are 16-bit
    cpu.SetMX(false, false);  // m=0 (16-bit A), x=0 (16-bit X/Y)

    EXPECT_FALSE(cpu.IsAccumulator8Bit());
    EXPECT_FALSE(cpu.IsIndex8Bit());
}

// Test 187: SetMX %01 - A=16-bit, X/Y=8-bit
TEST(Cpu6502Test, MX_SetA16Index8) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // MX %01 - A is 16-bit, X/Y are 8-bit
    cpu.SetMX(false, true);  // m=0 (16-bit A), x=1 (8-bit X/Y)

    EXPECT_FALSE(cpu.IsAccumulator8Bit());
    EXPECT_TRUE(cpu.IsIndex8Bit());
}

// Test 188: SetMX %10 - A=8-bit, X/Y=16-bit
TEST(Cpu6502Test, MX_SetA8Index16) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // MX %10 - A is 8-bit, X/Y are 16-bit
    cpu.SetMX(true, false);  // m=1 (8-bit A), x=0 (16-bit X/Y)

    EXPECT_TRUE(cpu.IsAccumulator8Bit());
    EXPECT_FALSE(cpu.IsIndex8Bit());
}

// Test 189: SetMX %11 - Both 8-bit (emulation mode)
TEST(Cpu6502Test, MX_SetBoth8Bit) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // MX %11 - A is 8-bit, X/Y are 8-bit (like 6502)
    cpu.SetMX(true, true);  // m=1 (8-bit A), x=1 (8-bit X/Y)

    EXPECT_TRUE(cpu.IsAccumulator8Bit());
    EXPECT_TRUE(cpu.IsIndex8Bit());
}

// Test 190: MX state can be changed multiple times
TEST(Cpu6502Test, MX_CanBeChangedMultipleTimes) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // Start with 8-bit (default)
    EXPECT_TRUE(cpu.IsAccumulator8Bit());
    EXPECT_TRUE(cpu.IsIndex8Bit());

    // Switch to 16-bit
    cpu.SetMX(false, false);
    EXPECT_FALSE(cpu.IsAccumulator8Bit());
    EXPECT_FALSE(cpu.IsIndex8Bit());

    // Switch to mixed (A=16, X/Y=8)
    cpu.SetMX(false, true);
    EXPECT_FALSE(cpu.IsAccumulator8Bit());
    EXPECT_TRUE(cpu.IsIndex8Bit());

    // Switch back to 8-bit
    cpu.SetMX(true, true);
    EXPECT_TRUE(cpu.IsAccumulator8Bit());
    EXPECT_TRUE(cpu.IsIndex8Bit());
}

// ============================================================================
// Group 9: 65816 Long Addressing Modes (24-bit)
// ============================================================================

// Test 191: LDA absolute long - 65816
TEST(Cpu6502Test, LDA_AbsoluteLong_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // LDA $123456 -> AF 56 34 12 (24-bit address, little-endian)
    auto bytes = cpu.EncodeLDA(0x123456, AddressingMode::AbsoluteLong);
    ASSERT_EQ(bytes.size(), 4);
    EXPECT_EQ(bytes[0], 0xAF);  // LDA long opcode
    EXPECT_EQ(bytes[1], 0x56);  // Low byte
    EXPECT_EQ(bytes[2], 0x34);  // Middle byte
    EXPECT_EQ(bytes[3], 0x12);  // High byte (bank)
}

// Test 192: LDA indirect long - 65816
TEST(Cpu6502Test, LDA_IndirectLong_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // LDA [$80] -> A7 80 (24-bit pointer at zero page $80)
    auto bytes = cpu.EncodeLDA(0x80, AddressingMode::IndirectLong);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xA7);  // LDA [dp] opcode
    EXPECT_EQ(bytes[1], 0x80);  // Zero page address
}

// Test 193: LDA indirect long indexed Y - 65816
TEST(Cpu6502Test, LDA_IndirectLongIndexedY_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // LDA [$80],Y -> B7 80 (24-bit pointer + Y)
    auto bytes = cpu.EncodeLDA(0x80, AddressingMode::IndirectLongIndexedY);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xB7);  // LDA [dp],Y opcode
    EXPECT_EQ(bytes[1], 0x80);  // Zero page address
}

// Test 194: Long addressing modes not available in 6502/65C02 mode
TEST(Cpu6502Test, LongAddressing_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502

    // LDA long not available
    auto bytes1 = cpu.EncodeLDA(0x123456, AddressingMode::AbsoluteLong);
    EXPECT_EQ(bytes1.size(), 0);

    // Switch to 65C02 - still not available
    cpu.SetCpuMode(CpuMode::Cpu65C02);
    auto bytes2 = cpu.EncodeLDA(0x123456, AddressingMode::AbsoluteLong);
    EXPECT_EQ(bytes2.size(), 0);
}

// ============================================================================
// Group 10: 65816 Stack Relative Addressing
// ============================================================================

// Test 195: LDA stack relative - 65816
TEST(Cpu6502Test, LDA_StackRelative_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // LDA $03,S -> A3 03 (access data relative to stack pointer)
    auto bytes = cpu.EncodeLDA(0x03, AddressingMode::StackRelative);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xA3);  // LDA sr opcode
    EXPECT_EQ(bytes[1], 0x03);  // Stack offset
}

// Test 196: LDA stack relative indirect indexed - 65816
TEST(Cpu6502Test, LDA_StackRelativeIndirectIndexedY_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // LDA ($03,S),Y -> B3 03 (indirect through stack pointer, then Y)
    auto bytes = cpu.EncodeLDA(0x03, AddressingMode::StackRelativeIndirectIndexedY);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xB3);  // LDA (sr,S),Y opcode
    EXPECT_EQ(bytes[1], 0x03);  // Stack offset
}

// Test 197: Stack relative modes not available in 6502/65C02
TEST(Cpu6502Test, StackRelative_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502

    // Not available in 6502
    auto bytes1 = cpu.EncodeLDA(0x03, AddressingMode::StackRelative);
    EXPECT_EQ(bytes1.size(), 0);

    // Not available in 65C02
    cpu.SetCpuMode(CpuMode::Cpu65C02);
    auto bytes2 = cpu.EncodeLDA(0x03, AddressingMode::StackRelative);
    EXPECT_EQ(bytes2.size(), 0);
}

// ============================================================================
// Group 11: 65816 Bank Operations
// ============================================================================

// Test 198: PHB - Push Data Bank Register - 65816
TEST(Cpu6502Test, PHB_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // PHB -> 8B
    auto bytes = cpu.EncodePHB();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x8B);  // PHB opcode
}

// Test 199: PLB - Pull Data Bank Register - 65816
TEST(Cpu6502Test, PLB_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // PLB -> AB
    auto bytes = cpu.EncodePLB();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xAB);  // PLB opcode
}

// Test 200: PHK - Push Program Bank Register - 65816
TEST(Cpu6502Test, PHK_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // PHK -> 4B
    auto bytes = cpu.EncodePHK();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x4B);  // PHK opcode
}

// Test 201: PHD - Push Direct Page Register - 65816
TEST(Cpu6502Test, PHD_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // PHD -> 0B
    auto bytes = cpu.EncodePHD();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x0B);  // PHD opcode
}

// Test 202: PLD - Pull Direct Page Register - 65816
TEST(Cpu6502Test, PLD_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // PLD -> 2B
    auto bytes = cpu.EncodePLD();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x2B);  // PLD opcode
}

// Test 203: Bank operations not available in 6502/65C02
TEST(Cpu6502Test, BankOps_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502

    auto bytes1 = cpu.EncodePHB();
    EXPECT_EQ(bytes1.size(), 0);

    cpu.SetCpuMode(CpuMode::Cpu65C02);
    auto bytes2 = cpu.EncodePHB();
    EXPECT_EQ(bytes2.size(), 0);
}

// ============================================================================
// Group 12: 65816 Transfer Operations
// ============================================================================

// Test 204: TCD - Transfer C to Direct Page - 65816
TEST(Cpu6502Test, TCD_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // TCD -> 5B (Transfer 16-bit accumulator to Direct Page register)
    auto bytes = cpu.EncodeTCD();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x5B);  // TCD opcode
}

// Test 205: TDC - Transfer Direct Page to C - 65816
TEST(Cpu6502Test, TDC_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // TDC -> 7B (Transfer Direct Page register to 16-bit accumulator)
    auto bytes = cpu.EncodeTDC();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x7B);  // TDC opcode
}

// Test 206: TCS - Transfer C to Stack Pointer - 65816
TEST(Cpu6502Test, TCS_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // TCS -> 1B (Transfer 16-bit accumulator to Stack Pointer)
    auto bytes = cpu.EncodeTCS();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x1B);  // TCS opcode
}

// Test 207: TSC - Transfer Stack Pointer to C - 65816
TEST(Cpu6502Test, TSC_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // TSC -> 3B (Transfer Stack Pointer to 16-bit accumulator)
    auto bytes = cpu.EncodeTSC();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x3B);  // TSC opcode
}

// Test 208: Transfer operations not available in 6502/65C02
TEST(Cpu6502Test, Transfers_NotAvailableIn6502Mode) {
    Cpu6502 cpu;

    auto bytes1 = cpu.EncodeTCD();
    EXPECT_EQ(bytes1.size(), 0);

    cpu.SetCpuMode(CpuMode::Cpu65C02);
    auto bytes2 = cpu.EncodeTCS();
    EXPECT_EQ(bytes2.size(), 0);
}

// ============================================================================
// Phase 2.5 - Group 13: 65816 Long Jumps
// ============================================================================

// Test 209: JML - Jump Long (65816)
TEST(Cpu6502Test, JML_65816_AbsoluteLong) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    auto bytes = cpu.EncodeJML(0x123456, AddressingMode::AbsoluteLong);
    ASSERT_EQ(bytes.size(), 4);
    EXPECT_EQ(bytes[0], 0x5C);  // JML opcode
    EXPECT_EQ(bytes[1], 0x56);  // Low byte
    EXPECT_EQ(bytes[2], 0x34);  // Middle byte
    EXPECT_EQ(bytes[3], 0x12);  // Bank byte
}

// Test 210: JSL - Jump Subroutine Long (65816)
TEST(Cpu6502Test, JSL_65816_AbsoluteLong) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    auto bytes = cpu.EncodeJSL(0x023456, AddressingMode::AbsoluteLong);
    ASSERT_EQ(bytes.size(), 4);
    EXPECT_EQ(bytes[0], 0x22);  // JSL opcode
    EXPECT_EQ(bytes[1], 0x56);  // Low byte
    EXPECT_EQ(bytes[2], 0x34);  // Middle byte
    EXPECT_EQ(bytes[3], 0x02);  // Bank byte
}

// Test 211: RTL - Return from Subroutine Long (65816)
TEST(Cpu6502Test, RTL_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    auto bytes = cpu.EncodeRTL();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x6B);  // RTL opcode
}

// Test 212: Long jumps not available in 6502/65C02
TEST(Cpu6502Test, LongJumps_NotAvailableIn6502Mode) {
    Cpu6502 cpu;

    auto bytes1 = cpu.EncodeJML(0x123456, AddressingMode::AbsoluteLong);
    EXPECT_EQ(bytes1.size(), 0);

    cpu.SetCpuMode(CpuMode::Cpu65C02);
    auto bytes2 = cpu.EncodeJSL(0x123456, AddressingMode::AbsoluteLong);
    EXPECT_EQ(bytes2.size(), 0);

    auto bytes3 = cpu.EncodeRTL();
    EXPECT_EQ(bytes3.size(), 0);
}

// ============================================================================
// Phase 2.5 - Group 14: 65816 Miscellaneous Opcodes
// ============================================================================

// Test 213: PEA - Push Effective Address (65816)
TEST(Cpu6502Test, PEA_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    auto bytes = cpu.EncodePEA(0x1234, AddressingMode::Immediate);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0xF4);  // PEA opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// Test 214: PEI - Push Effective Indirect Address (65816)
TEST(Cpu6502Test, PEI_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    auto bytes = cpu.EncodePEI(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xD4);  // PEI opcode
    EXPECT_EQ(bytes[1], 0x80);  // Zero page address
}

// Test 215: PER - Push Effective PC Relative Address (65816)
TEST(Cpu6502Test, PER_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    auto bytes = cpu.EncodePER(0x1234, AddressingMode::Relative);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x62);  // PER opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// Test 216: MVN - Block Move Negative (65816)
TEST(Cpu6502Test, MVN_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // MVN srcbank, destbank
    auto bytes = cpu.EncodeMVN(0x12, 0x34);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x54);  // MVN opcode
    EXPECT_EQ(bytes[1], 0x12);  // Source bank
    EXPECT_EQ(bytes[2], 0x34);  // Destination bank
}

// Test 217: MVP - Block Move Positive (65816)
TEST(Cpu6502Test, MVP_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // MVP srcbank, destbank
    auto bytes = cpu.EncodeMVP(0x56, 0x78);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x44);  // MVP opcode
    EXPECT_EQ(bytes[1], 0x56);  // Source bank
    EXPECT_EQ(bytes[2], 0x78);  // Destination bank
}

// Test 218: COP - Coprocessor (65816)
TEST(Cpu6502Test, COP_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    auto bytes = cpu.EncodeCOP(0x42, AddressingMode::Immediate);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x02);  // COP opcode
    EXPECT_EQ(bytes[1], 0x42);  // Signature byte
}

// Test 219: WDM - Reserved (65816)
TEST(Cpu6502Test, WDM_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    auto bytes = cpu.EncodeWDM(0x99, AddressingMode::Immediate);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x42);  // WDM opcode
    EXPECT_EQ(bytes[1], 0x99);  // Reserved byte
}

// Test 220: XBA - Exchange B and A (65816)
TEST(Cpu6502Test, XBA_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    auto bytes = cpu.EncodeXBA();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xEB);  // XBA opcode
}

// Test 221: XCE - Exchange Carry and Emulation (65816)
TEST(Cpu6502Test, XCE_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    auto bytes = cpu.EncodeXCE();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xFB);  // XCE opcode
}

// Test 222: Miscellaneous opcodes not available in 6502/65C02
TEST(Cpu6502Test, MiscOpcodes_NotAvailableIn6502Mode) {
    Cpu6502 cpu;

    auto bytes1 = cpu.EncodePEA(0x1234, AddressingMode::Immediate);
    EXPECT_EQ(bytes1.size(), 0);

    auto bytes2 = cpu.EncodeXBA();
    EXPECT_EQ(bytes2.size(), 0);

    cpu.SetCpuMode(CpuMode::Cpu65C02);
    auto bytes3 = cpu.EncodeXCE();
    EXPECT_EQ(bytes3.size(), 0);

    auto bytes4 = cpu.EncodeMVN(0x12, 0x34);
    EXPECT_EQ(bytes4.size(), 0);
}

// ============================================================================
// Branch Relaxation Tests (Long Branch Support)
// ============================================================================

// Test 223: Branch in range - no relaxation needed (+127)
TEST(Cpu6502Test, BranchRelaxation_InRange_Positive127) {
    Cpu6502 cpu;
    
    // BEQ with offset +127 (maximum positive offset)
    // Should NOT relax - still fits in 8-bit signed range
    auto bytes = cpu.EncodeBEQ(0x7F, AddressingMode::Relative);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xF0);  // BEQ opcode (not relaxed)
    EXPECT_EQ(bytes[1], 0x7F);  // Offset +127
}

// Test 224: Branch in range - no relaxation needed (-128)
TEST(Cpu6502Test, BranchRelaxation_InRange_Negative128) {
    Cpu6502 cpu;
    
    // BEQ with offset -128 (minimum negative offset)
    // Should NOT relax - still fits in 8-bit signed range
    auto bytes = cpu.EncodeBEQ(0x80, AddressingMode::Relative);  // 0x80 = -128 in signed
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xF0);  // BEQ opcode (not relaxed)
    EXPECT_EQ(bytes[1], 0x80);  // Offset -128
}

// Test 225: Branch out of range - relaxation needed (+128)
// BEQ target → BNE *+5; JMP target
TEST(Cpu6502Test, BranchRelaxation_OutOfRange_Positive128) {
    Cpu6502 cpu;
    
    // BEQ to address 0x1082 from address 0x1000
    // Offset = 0x1082 - (0x1000 + 2) = 0x80 = +128 (out of range)
    // Should relax to: BNE *+5; JMP $1082
    auto bytes = cpu.EncodeBranchWithRelaxation(0xF0, 0x1000, 0x1082);
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0xD0);  // BNE opcode (complement of BEQ)
    EXPECT_EQ(bytes[1], 0x03);  // Skip 3 bytes (JMP instruction)
    EXPECT_EQ(bytes[2], 0x4C);  // JMP opcode
    EXPECT_EQ(bytes[3], 0x82);  // Target low byte
    EXPECT_EQ(bytes[4], 0x10);  // Target high byte
}

// Test 226: Branch out of range - relaxation needed (-129)
// BEQ target → BNE *+5; JMP target
TEST(Cpu6502Test, BranchRelaxation_OutOfRange_Negative129) {
    Cpu6502 cpu;
    
    // BEQ to address 0x0F7F from address 0x1000
    // Offset = 0x0F7F - (0x1000 + 2) = -131 (out of range)
    // Should relax to: BNE *+5; JMP $0F7F
    auto bytes = cpu.EncodeBranchWithRelaxation(0xF0, 0x1000, 0x0F7F);
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0xD0);  // BNE opcode (complement of BEQ)
    EXPECT_EQ(bytes[1], 0x03);  // Skip 3 bytes (JMP instruction)
    EXPECT_EQ(bytes[2], 0x4C);  // JMP opcode
    EXPECT_EQ(bytes[3], 0x7F);  // Target low byte
    EXPECT_EQ(bytes[4], 0x0F);  // Target high byte
}

// Test 227: BNE out of range - should relax to BEQ + JMP
TEST(Cpu6502Test, BranchRelaxation_BNE_OutOfRange) {
    Cpu6502 cpu;
    
    // BNE to address 0x1200 from address 0x1000
    // Offset = 0x1200 - (0x1000 + 2) = +510 (out of range)
    // Should relax to: BEQ *+5; JMP $1200
    auto bytes = cpu.EncodeBranchWithRelaxation(0xD0, 0x1000, 0x1200);
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0xF0);  // BEQ opcode (complement of BNE)
    EXPECT_EQ(bytes[1], 0x03);  // Skip 3 bytes
    EXPECT_EQ(bytes[2], 0x4C);  // JMP opcode
    EXPECT_EQ(bytes[3], 0x00);  // Target low byte
    EXPECT_EQ(bytes[4], 0x12);  // Target high byte
}

// Test 228: BCC out of range - should relax to BCS + JMP
TEST(Cpu6502Test, BranchRelaxation_BCC_OutOfRange) {
    Cpu6502 cpu;
    
    // BCC to address 0x1200 from address 0x1000
    // Should relax to: BCS *+5; JMP $1200
    auto bytes = cpu.EncodeBranchWithRelaxation(0x90, 0x1000, 0x1200);
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0xB0);  // BCS opcode (complement of BCC)
    EXPECT_EQ(bytes[1], 0x03);  // Skip 3 bytes
    EXPECT_EQ(bytes[2], 0x4C);  // JMP opcode
    EXPECT_EQ(bytes[3], 0x00);  // Target low byte
    EXPECT_EQ(bytes[4], 0x12);  // Target high byte
}

// Test 229: BCS out of range - should relax to BCC + JMP
TEST(Cpu6502Test, BranchRelaxation_BCS_OutOfRange) {
    Cpu6502 cpu;
    
    // BCS to address 0x1200 from address 0x1000
    // Should relax to: BCC *+5; JMP $1200
    auto bytes = cpu.EncodeBranchWithRelaxation(0xB0, 0x1000, 0x1200);
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0x90);  // BCC opcode (complement of BCS)
    EXPECT_EQ(bytes[1], 0x03);  // Skip 3 bytes
    EXPECT_EQ(bytes[2], 0x4C);  // JMP opcode
    EXPECT_EQ(bytes[3], 0x00);  // Target low byte
    EXPECT_EQ(bytes[4], 0x12);  // Target high byte
}

// Test 230: BPL out of range - should relax to BMI + JMP
TEST(Cpu6502Test, BranchRelaxation_BPL_OutOfRange) {
    Cpu6502 cpu;
    
    // BPL to address 0x1200 from address 0x1000
    // Should relax to: BMI *+5; JMP $1200
    auto bytes = cpu.EncodeBranchWithRelaxation(0x10, 0x1000, 0x1200);
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0x30);  // BMI opcode (complement of BPL)
    EXPECT_EQ(bytes[1], 0x03);  // Skip 3 bytes
    EXPECT_EQ(bytes[2], 0x4C);  // JMP opcode
    EXPECT_EQ(bytes[3], 0x00);  // Target low byte
    EXPECT_EQ(bytes[4], 0x12);  // Target high byte
}

// Test 231: BMI out of range - should relax to BPL + JMP
TEST(Cpu6502Test, BranchRelaxation_BMI_OutOfRange) {
    Cpu6502 cpu;
    
    // BMI to address 0x1200 from address 0x1000
    // Should relax to: BPL *+5; JMP $1200
    auto bytes = cpu.EncodeBranchWithRelaxation(0x30, 0x1000, 0x1200);
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0x10);  // BPL opcode (complement of BMI)
    EXPECT_EQ(bytes[1], 0x03);  // Skip 3 bytes
    EXPECT_EQ(bytes[2], 0x4C);  // JMP opcode
    EXPECT_EQ(bytes[3], 0x00);  // Target low byte
    EXPECT_EQ(bytes[4], 0x12);  // Target high byte
}

// Test 232: BVC out of range - should relax to BVS + JMP
TEST(Cpu6502Test, BranchRelaxation_BVC_OutOfRange) {
    Cpu6502 cpu;
    
    // BVC to address 0x1200 from address 0x1000
    // Should relax to: BVS *+5; JMP $1200
    auto bytes = cpu.EncodeBranchWithRelaxation(0x50, 0x1000, 0x1200);
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0x70);  // BVS opcode (complement of BVC)
    EXPECT_EQ(bytes[1], 0x03);  // Skip 3 bytes
    EXPECT_EQ(bytes[2], 0x4C);  // JMP opcode
    EXPECT_EQ(bytes[3], 0x00);  // Target low byte
    EXPECT_EQ(bytes[4], 0x12);  // Target high byte
}

// Test 233: BVS out of range - should relax to BVC + JMP
TEST(Cpu6502Test, BranchRelaxation_BVS_OutOfRange) {
    Cpu6502 cpu;
    
    // BVS to address 0x1200 from address 0x1000
    // Should relax to: BVC *+5; JMP $1200
    auto bytes = cpu.EncodeBranchWithRelaxation(0x70, 0x1000, 0x1200);
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0x50);  // BVC opcode (complement of BVS)
    EXPECT_EQ(bytes[1], 0x03);  // Skip 3 bytes
    EXPECT_EQ(bytes[2], 0x4C);  // JMP opcode
    EXPECT_EQ(bytes[3], 0x00);  // Target low byte
    EXPECT_EQ(bytes[4], 0x12);  // Target high byte
}

// Test 234: Test opcode complement function (XOR 0x20)
TEST(Cpu6502Test, BranchRelaxation_OpcodeComplement) {
    Cpu6502 cpu;
    
    // Test that all branch opcodes complement correctly via XOR 0x20
    EXPECT_EQ(cpu.GetComplementaryBranchOpcode(0x10), 0x30);  // BPL → BMI
    EXPECT_EQ(cpu.GetComplementaryBranchOpcode(0x30), 0x10);  // BMI → BPL
    EXPECT_EQ(cpu.GetComplementaryBranchOpcode(0x50), 0x70);  // BVC → BVS
    EXPECT_EQ(cpu.GetComplementaryBranchOpcode(0x70), 0x50);  // BVS → BVC
    EXPECT_EQ(cpu.GetComplementaryBranchOpcode(0x90), 0xB0);  // BCC → BCS
    EXPECT_EQ(cpu.GetComplementaryBranchOpcode(0xB0), 0x90);  // BCS → BCC
    EXPECT_EQ(cpu.GetComplementaryBranchOpcode(0xD0), 0xF0);  // BNE → BEQ
    EXPECT_EQ(cpu.GetComplementaryBranchOpcode(0xF0), 0xD0);  // BEQ → BNE
}

// Test 235: Test branch range detection
TEST(Cpu6502Test, BranchRelaxation_NeedsBranchRelaxation) {
    Cpu6502 cpu;
    
    // Test positive edge cases
    // From 0x1000, to reach 0x1081: offset = 0x1081 - (0x1000 + 2) = 0x7F = +127 (in range)
    EXPECT_FALSE(cpu.NeedsBranchRelaxation(0x1000, 0x1081));  // +127 - in range
    // From 0x1000, to reach 0x1082: offset = 0x1082 - (0x1000 + 2) = 0x80 = +128 (out of range)
    EXPECT_TRUE(cpu.NeedsBranchRelaxation(0x1000, 0x1082));   // +128 - out of range
    
    // Test negative edge cases
    // From 0x1000, to reach 0x0F82: offset = 0x0F82 - (0x1000 + 2) = -128 (in range)
    EXPECT_FALSE(cpu.NeedsBranchRelaxation(0x1000, 0x0F82));  // -128 - in range
    // From 0x1000, to reach 0x0F81: offset = 0x0F81 - (0x1000 + 2) = -129 (out of range)
    EXPECT_TRUE(cpu.NeedsBranchRelaxation(0x1000, 0x0F81));   // -129 - out of range
    
    // Test large offsets
    EXPECT_TRUE(cpu.NeedsBranchRelaxation(0x1000, 0x1200));   // +510 - out of range
    EXPECT_TRUE(cpu.NeedsBranchRelaxation(0x1200, 0x1000));   // -510 - out of range
}

// Test 236: Large offset (Prince of Persia scale - 3017 bytes)
TEST(Cpu6502Test, BranchRelaxation_LargeOffset_PoP) {
    Cpu6502 cpu;
    
    // Simulate FRAMEADV.S case: BCC from $12FE to $1EC9 (offset = 3017 bytes)
    // Should relax to: BCS *+5; JMP $1EC9
    auto bytes = cpu.EncodeBranchWithRelaxation(0x90, 0x12FE, 0x1EC9);
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0xB0);  // BCS opcode (complement of BCC)
    EXPECT_EQ(bytes[1], 0x03);  // Skip 3 bytes
    EXPECT_EQ(bytes[2], 0x4C);  // JMP opcode
    EXPECT_EQ(bytes[3], 0xC9);  // Target low byte
    EXPECT_EQ(bytes[4], 0x1E);  // Target high byte
}

// ============================================================================
// Phase 2.6: 65C02 Rockwell Extensions
// Group 1: WAI/STP - Processor Control
// ============================================================================

// Test 237: WAI (Wait for Interrupt) - 65C02 Rockwell
TEST(Cpu6502Test, WAI_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    // WAI -> CB
    auto bytes = cpu.EncodeWAI();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xCB);  // WAI opcode
}

// Test 238: STP (Stop Processor) - 65C02 Rockwell
TEST(Cpu6502Test, STP_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    // STP -> DB
    auto bytes = cpu.EncodeSTP();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xDB);  // STP opcode
}

// Test 239: WAI not available in 6502 mode
TEST(Cpu6502Test, WAI_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502
    
    auto bytes = cpu.EncodeWAI();
    EXPECT_EQ(bytes.size(), 0);  // Empty = not supported
}

// Test 240: WAI not available in standard 65C02 mode (Rockwell-only)
TEST(Cpu6502Test, WAI_NotAvailableInStandard65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);  // Standard 65C02, not Rockwell
    
    auto bytes = cpu.EncodeWAI();
    EXPECT_EQ(bytes.size(), 0);  // Empty = Rockwell-only instruction
}

// Test 241: STP not available in 6502 mode
TEST(Cpu6502Test, STP_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502
    
    auto bytes = cpu.EncodeSTP();
    EXPECT_EQ(bytes.size(), 0);  // Empty = not supported
}

// Test 242: STP not available in standard 65C02 mode (Rockwell-only)
TEST(Cpu6502Test, STP_NotAvailableInStandard65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);  // Standard 65C02, not Rockwell
    
    auto bytes = cpu.EncodeSTP();
    EXPECT_EQ(bytes.size(), 0);  // Empty = Rockwell-only instruction
}

// ============================================================================
// Phase 2.6: 65C02 Rockwell Extensions
// Group 2: RMB/SMB - Reset/Set Memory Bit
// ============================================================================

// Test 243: RMB0 (Reset Memory Bit 0) - 65C02 Rockwell
TEST(Cpu6502Test, RMB0_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    // RMB0 $80 -> 07 80
    auto bytes = cpu.EncodeRMB0(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x07);  // RMB0 opcode
    EXPECT_EQ(bytes[1], 0x80);  // Zero page address
}

// Test 244: RMB1 - 65C02 Rockwell
TEST(Cpu6502Test, RMB1_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    // RMB1 $42 -> 17 42
    auto bytes = cpu.EncodeRMB1(0x42, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x17);  // RMB1 opcode
    EXPECT_EQ(bytes[1], 0x42);
}

// Test 245: RMB2 - 65C02 Rockwell
TEST(Cpu6502Test, RMB2_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    // RMB2 $FF -> 27 FF
    auto bytes = cpu.EncodeRMB2(0xFF, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x27);
    EXPECT_EQ(bytes[1], 0xFF);
}

// Test 246: RMB3 - 65C02 Rockwell
TEST(Cpu6502Test, RMB3_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeRMB3(0x00, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x37);
    EXPECT_EQ(bytes[1], 0x00);
}

// Test 247: RMB4 - 65C02 Rockwell
TEST(Cpu6502Test, RMB4_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeRMB4(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x47);
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 248: RMB5 - 65C02 Rockwell
TEST(Cpu6502Test, RMB5_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeRMB5(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x57);
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 249: RMB6 - 65C02 Rockwell
TEST(Cpu6502Test, RMB6_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeRMB6(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x67);
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 250: RMB7 - 65C02 Rockwell
TEST(Cpu6502Test, RMB7_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeRMB7(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x77);
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 251: SMB0 (Set Memory Bit 0) - 65C02 Rockwell
TEST(Cpu6502Test, SMB0_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    // SMB0 $80 -> 87 80
    auto bytes = cpu.EncodeSMB0(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x87);
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 252: SMB1 - 65C02 Rockwell
TEST(Cpu6502Test, SMB1_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeSMB1(0x42, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x97);
    EXPECT_EQ(bytes[1], 0x42);
}

// Test 253: SMB2 - 65C02 Rockwell
TEST(Cpu6502Test, SMB2_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeSMB2(0xFF, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xA7);
    EXPECT_EQ(bytes[1], 0xFF);
}

// Test 254: SMB3 - 65C02 Rockwell
TEST(Cpu6502Test, SMB3_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeSMB3(0x00, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xB7);
    EXPECT_EQ(bytes[1], 0x00);
}

// Test 255: SMB4 - 65C02 Rockwell
TEST(Cpu6502Test, SMB4_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeSMB4(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xC7);
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 256: SMB5 - 65C02 Rockwell
TEST(Cpu6502Test, SMB5_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeSMB5(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xD7);
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 257: SMB6 - 65C02 Rockwell
TEST(Cpu6502Test, SMB6_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeSMB6(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xE7);
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 258: SMB7 - 65C02 Rockwell
TEST(Cpu6502Test, SMB7_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeSMB7(0x80, AddressingMode::ZeroPage);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xF7);
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 259: RMB not available in 6502 mode
TEST(Cpu6502Test, RMB_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    
    auto bytes = cpu.EncodeRMB0(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(bytes.size(), 0);
}

// Test 260: SMB not available in standard 65C02 mode
TEST(Cpu6502Test, SMB_NotAvailableInStandard65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);
    
    auto bytes = cpu.EncodeSMB0(0x80, AddressingMode::ZeroPage);
    EXPECT_EQ(bytes.size(), 0);
}

// ============================================================================
// Phase 2.6: 65C02 Rockwell Extensions
// Group 3: BBR/BBS - Branch on Bit Reset/Set
// ============================================================================

// Test 261: BBR0 (Branch if Bit 0 Reset) - 65C02 Rockwell
TEST(Cpu6502Test, BBR0_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    // BBR0 $80, $10 -> 0F 80 10
    // Zero page address $80, relative branch offset $10
    auto bytes = cpu.EncodeBBR0(0x80, 0x10);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x0F);  // BBR0 opcode
    EXPECT_EQ(bytes[1], 0x80);  // Zero page address
    EXPECT_EQ(bytes[2], 0x10);  // Relative offset
}

// Test 262: BBR1 - 65C02 Rockwell
TEST(Cpu6502Test, BBR1_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBR1(0x42, 0x20);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x1F);
    EXPECT_EQ(bytes[1], 0x42);
    EXPECT_EQ(bytes[2], 0x20);
}

// Test 263: BBR2 - 65C02 Rockwell
TEST(Cpu6502Test, BBR2_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBR2(0xFF, 0xFE);  // Negative offset
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x2F);
    EXPECT_EQ(bytes[1], 0xFF);
    EXPECT_EQ(bytes[2], 0xFE);
}

// Test 264: BBR3 - 65C02 Rockwell
TEST(Cpu6502Test, BBR3_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBR3(0x00, 0x7F);  // Max positive offset
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x3F);
    EXPECT_EQ(bytes[1], 0x00);
    EXPECT_EQ(bytes[2], 0x7F);
}

// Test 265: BBR4 - 65C02 Rockwell
TEST(Cpu6502Test, BBR4_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBR4(0x80, 0x00);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x4F);
    EXPECT_EQ(bytes[1], 0x80);
    EXPECT_EQ(bytes[2], 0x00);
}

// Test 266: BBR5 - 65C02 Rockwell
TEST(Cpu6502Test, BBR5_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBR5(0x80, 0x10);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x5F);
    EXPECT_EQ(bytes[1], 0x80);
    EXPECT_EQ(bytes[2], 0x10);
}

// Test 267: BBR6 - 65C02 Rockwell
TEST(Cpu6502Test, BBR6_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBR6(0x80, 0x10);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x6F);
    EXPECT_EQ(bytes[1], 0x80);
    EXPECT_EQ(bytes[2], 0x10);
}

// Test 268: BBR7 - 65C02 Rockwell
TEST(Cpu6502Test, BBR7_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBR7(0x80, 0x10);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x7F);
    EXPECT_EQ(bytes[1], 0x80);
    EXPECT_EQ(bytes[2], 0x10);
}

// Test 269: BBS0 (Branch if Bit 0 Set) - 65C02 Rockwell
TEST(Cpu6502Test, BBS0_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    // BBS0 $80, $10 -> 8F 80 10
    auto bytes = cpu.EncodeBBS0(0x80, 0x10);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x8F);
    EXPECT_EQ(bytes[1], 0x80);
    EXPECT_EQ(bytes[2], 0x10);
}

// Test 270: BBS1 - 65C02 Rockwell
TEST(Cpu6502Test, BBS1_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBS1(0x42, 0x20);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x9F);
    EXPECT_EQ(bytes[1], 0x42);
    EXPECT_EQ(bytes[2], 0x20);
}

// Test 271: BBS2 - 65C02 Rockwell
TEST(Cpu6502Test, BBS2_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBS2(0xFF, 0xFE);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0xAF);
    EXPECT_EQ(bytes[1], 0xFF);
    EXPECT_EQ(bytes[2], 0xFE);
}

// Test 272: BBS3 - 65C02 Rockwell
TEST(Cpu6502Test, BBS3_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBS3(0x00, 0x7F);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0xBF);
    EXPECT_EQ(bytes[1], 0x00);
    EXPECT_EQ(bytes[2], 0x7F);
}

// Test 273: BBS4 - 65C02 Rockwell
TEST(Cpu6502Test, BBS4_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBS4(0x80, 0x00);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0xCF);
    EXPECT_EQ(bytes[1], 0x80);
    EXPECT_EQ(bytes[2], 0x00);
}

// Test 274: BBS5 - 65C02 Rockwell
TEST(Cpu6502Test, BBS5_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBS5(0x80, 0x10);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0xDF);
    EXPECT_EQ(bytes[1], 0x80);
    EXPECT_EQ(bytes[2], 0x10);
}

// Test 275: BBS6 - 65C02 Rockwell
TEST(Cpu6502Test, BBS6_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBS6(0x80, 0x10);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0xEF);
    EXPECT_EQ(bytes[1], 0x80);
    EXPECT_EQ(bytes[2], 0x10);
}

// Test 276: BBS7 - 65C02 Rockwell
TEST(Cpu6502Test, BBS7_65C02Rockwell) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02Rock);

    auto bytes = cpu.EncodeBBS7(0x80, 0x10);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0xFF);
    EXPECT_EQ(bytes[1], 0x80);
    EXPECT_EQ(bytes[2], 0x10);
}

// Test 277: BBR not available in 6502 mode
TEST(Cpu6502Test, BBR_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    
    auto bytes = cpu.EncodeBBR0(0x80, 0x10);
    EXPECT_EQ(bytes.size(), 0);
}

// Test 278: BBS not available in standard 65C02 mode
TEST(Cpu6502Test, BBS_NotAvailableInStandard65C02) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);
    
    auto bytes = cpu.EncodeBBS0(0x80, 0x10);
    EXPECT_EQ(bytes.size(), 0);
}
