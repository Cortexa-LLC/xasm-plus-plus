// 6502 CPU implementation

#include "xasm++/cpu/cpu_6502.h"

namespace xasm {

// LDA - Load Accumulator
std::vector<uint8_t> Cpu6502::EncodeLDA(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0xA9);  // LDA #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xA5);  // LDA zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xAD);  // LDA abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            // For Phase 1, only support these modes
            break;
    }

    return bytes;
}

// STA - Store Accumulator
std::vector<uint8_t> Cpu6502::EncodeSTA(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(0x85);  // STA zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x8D);  // STA abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
            break;

        default:
            break;
    }

    return bytes;
}

// JMP - Jump
std::vector<uint8_t> Cpu6502::EncodeJMP(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::Absolute) {
        bytes.push_back(0x4C);  // JMP abs
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    }

    return bytes;
}

// NOP - No Operation
std::vector<uint8_t> Cpu6502::EncodeNOP() const {
    return {0xEA};
}

// RTS - Return from Subroutine
std::vector<uint8_t> Cpu6502::EncodeRTS() const {
    return {0x60};
}

// Phase 2.2: Arithmetic Instructions

// ADC - Add with Carry
std::vector<uint8_t> Cpu6502::EncodeADC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0x69);  // ADC #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x65);  // ADC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x6D);  // ADC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// SBC - Subtract with Carry
std::vector<uint8_t> Cpu6502::EncodeSBC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0xE9);  // SBC #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xE5);  // SBC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xED);  // SBC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// Phase 2.2: Logic Instructions

// AND - Logical AND
std::vector<uint8_t> Cpu6502::EncodeAND(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0x29);  // AND #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x25);  // AND zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x2D);  // AND abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// ORA - Logical OR
std::vector<uint8_t> Cpu6502::EncodeORA(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0x09);  // ORA #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x05);  // ORA zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x0D);  // ORA abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// EOR - Exclusive OR
std::vector<uint8_t> Cpu6502::EncodeEOR(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0x49);  // EOR #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x45);  // EOR zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x4D);  // EOR abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// Phase 2.2: Additional Loads/Stores

// LDX - Load X Register
std::vector<uint8_t> Cpu6502::EncodeLDX(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0xA2);  // LDX #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xA6);  // LDX zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xAE);  // LDX abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// LDY - Load Y Register
std::vector<uint8_t> Cpu6502::EncodeLDY(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0xA0);  // LDY #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xA4);  // LDY zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xAC);  // LDY abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// STX - Store X Register
std::vector<uint8_t> Cpu6502::EncodeSTX(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(0x86);  // STX zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x8E);  // STX abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// STY - Store Y Register
std::vector<uint8_t> Cpu6502::EncodeSTY(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(0x84);  // STY zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x8C);  // STY abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// Phase 2.2: Comparisons

// CMP - Compare Accumulator
std::vector<uint8_t> Cpu6502::EncodeCMP(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0xC9);  // CMP #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xC5);  // CMP zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xCD);  // CMP abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// CPX - Compare X Register
std::vector<uint8_t> Cpu6502::EncodeCPX(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0xE0);  // CPX #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xE4);  // CPX zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xEC);  // CPX abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// CPY - Compare Y Register
std::vector<uint8_t> Cpu6502::EncodeCPY(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0xC0);  // CPY #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xC4);  // CPY zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xCC);  // CPY abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// Phase 2.2: Branch Instructions

// BEQ - Branch if Equal
std::vector<uint8_t> Cpu6502::EncodeBEQ(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0xF0);  // BEQ opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BNE - Branch if Not Equal
std::vector<uint8_t> Cpu6502::EncodeBNE(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0xD0);  // BNE opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BCC - Branch if Carry Clear
std::vector<uint8_t> Cpu6502::EncodeBCC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x90);  // BCC opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BCS - Branch if Carry Set
std::vector<uint8_t> Cpu6502::EncodeBCS(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0xB0);  // BCS opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BMI - Branch if Minus
std::vector<uint8_t> Cpu6502::EncodeBMI(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x30);  // BMI opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BPL - Branch if Plus
std::vector<uint8_t> Cpu6502::EncodeBPL(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x10);  // BPL opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BVC - Branch if Overflow Clear
std::vector<uint8_t> Cpu6502::EncodeBVC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x50);  // BVC opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BVS - Branch if Overflow Set
std::vector<uint8_t> Cpu6502::EncodeBVS(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x70);  // BVS opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// Phase 2.2: Inc/Dec Instructions

// INX - Increment X Register
std::vector<uint8_t> Cpu6502::EncodeINX() const {
    return {0xE8};
}

// INY - Increment Y Register
std::vector<uint8_t> Cpu6502::EncodeINY() const {
    return {0xC8};
}

// DEX - Decrement X Register
std::vector<uint8_t> Cpu6502::EncodeDEX() const {
    return {0xCA};
}

// DEY - Decrement Y Register
std::vector<uint8_t> Cpu6502::EncodeDEY() const {
    return {0x88};
}

// INC - Increment Memory
std::vector<uint8_t> Cpu6502::EncodeINC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(0xE6);  // INC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xEE);  // INC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// DEC - Decrement Memory
std::vector<uint8_t> Cpu6502::EncodeDEC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(0xC6);  // DEC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xCE);  // DEC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// Phase 2.2: Stack Operations

// PHA - Push Accumulator
std::vector<uint8_t> Cpu6502::EncodePHA() const {
    return {0x48};
}

// PLA - Pull Accumulator
std::vector<uint8_t> Cpu6502::EncodePLA() const {
    return {0x68};
}

// PHP - Push Processor Status
std::vector<uint8_t> Cpu6502::EncodePHP() const {
    return {0x08};
}

// PLP - Pull Processor Status
std::vector<uint8_t> Cpu6502::EncodePLP() const {
    return {0x28};
}

// Phase 2.2: Subroutine

// JSR - Jump to Subroutine
std::vector<uint8_t> Cpu6502::EncodeJSR(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::Absolute) {
        bytes.push_back(0x20);  // JSR abs
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
    }

    return bytes;
}

// Calculate instruction size based on addressing mode
size_t Cpu6502::CalculateInstructionSize(AddressingMode mode) const {
    switch (mode) {
        case AddressingMode::Implied:
            return 1;

        case AddressingMode::Immediate:
        case AddressingMode::ZeroPage:
        case AddressingMode::ZeroPageX:
        case AddressingMode::ZeroPageY:
        case AddressingMode::IndirectX:
        case AddressingMode::IndirectY:
        case AddressingMode::Relative:
            return 2;

        case AddressingMode::Absolute:
        case AddressingMode::AbsoluteX:
        case AddressingMode::AbsoluteY:
        case AddressingMode::Indirect:
            return 3;

        default:
            return 0;
    }
}

} // namespace xasm
