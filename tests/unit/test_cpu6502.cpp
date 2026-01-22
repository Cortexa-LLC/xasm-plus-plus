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
