// 6502 CPU implementation - 65C02 extensions

#include "xasm++/cpu/cpu_6502.h"

namespace xasm {

// Group 2: 65C02 Stack Operations
// ============================================================================

// PHX - Push X to stack (65C02+)
std::vector<uint8_t> Cpu6502::EncodePHX() const {
    // Only available in 65C02 and later
    if (cpu_mode_ == CpuMode::Cpu6502) {
        return {};  // Not supported in 6502 mode
    }
    return {0xDA};
}

// PLX - Pull X from stack (65C02+)
std::vector<uint8_t> Cpu6502::EncodePLX() const {
    // Only available in 65C02 and later
    if (cpu_mode_ == CpuMode::Cpu6502) {
        return {};  // Not supported in 6502 mode
    }
    return {0xFA};
}

// PHY - Push Y to stack (65C02+)
std::vector<uint8_t> Cpu6502::EncodePHY() const {
    // Only available in 65C02 and later
    if (cpu_mode_ == CpuMode::Cpu6502) {
        return {};  // Not supported in 6502 mode
    }
    return {0x5A};
}

// PLY - Pull Y from stack (65C02+)
std::vector<uint8_t> Cpu6502::EncodePLY() const {
    // Only available in 65C02 and later
    if (cpu_mode_ == CpuMode::Cpu6502) {
        return {};  // Not supported in 6502 mode
    }
    return {0x7A};
}

// ============================================================================
// Group 3: 65C02 Store Zero
// ============================================================================

// STZ - Store Zero (65C02+)
std::vector<uint8_t> Cpu6502::EncodeSTZ(uint16_t operand, AddressingMode mode) const {
    // Only available in 65C02 and later
    if (cpu_mode_ == CpuMode::Cpu6502) {
        return {};  // Not supported in 6502 mode
    }

    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(0x64);  // STZ zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x74);  // STZ zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x9C);  // STZ abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0x9E);  // STZ abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// ============================================================================
// Group 4: 65C02 Bit Test
// ============================================================================

// TRB - Test and Reset Bits (65C02+)
std::vector<uint8_t> Cpu6502::EncodeTRB(uint16_t operand, AddressingMode mode) const {
    // Only available in 65C02 and later
    if (cpu_mode_ == CpuMode::Cpu6502) {
        return {};  // Not supported in 6502 mode
    }

    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(0x14);  // TRB zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x1C);  // TRB abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// TSB - Test and Set Bits (65C02+)
std::vector<uint8_t> Cpu6502::EncodeTSB(uint16_t operand, AddressingMode mode) const {
    // Only available in 65C02 and later
    if (cpu_mode_ == CpuMode::Cpu6502) {
        return {};  // Not supported in 6502 mode
    }

    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(0x04);  // TSB zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x0C);  // TSB abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// ============================================================================
// Group 5: 65C02 Branch Always
// ============================================================================

// BRA - Branch Always (65C02+)
std::vector<uint8_t> Cpu6502::EncodeBRA(uint16_t operand, AddressingMode mode) const {
    // Only available in 65C02 and later
    if (cpu_mode_ == CpuMode::Cpu6502) {
        return {};  // Not supported in 6502 mode
    }

    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x80);  // BRA opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));  // Relative offset
    }

    return bytes;
}

} // namespace xasm
