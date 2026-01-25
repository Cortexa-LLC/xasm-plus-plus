// 6502 CPU implementation - 65816 extensions

#include "xasm++/cpu/cpu_6502.h"

namespace xasm {

// ============================================================================

// Set CPU mode (6502, 65C02, 65C02-Rockwell, 65816)
void Cpu6502::SetCpuMode(CpuMode mode) {
    cpu_mode_ = mode;
}

// Get current CPU mode
CpuMode Cpu6502::GetCpuMode() const {
    return cpu_mode_;
}

// Group 11: 65816 Bank Operations
// ============================================================================

// PHB - Push Data Bank Register (65816)
std::vector<uint8_t> Cpu6502::EncodePHB() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x8B};
}

// PLB - Pull Data Bank Register (65816)
std::vector<uint8_t> Cpu6502::EncodePLB() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0xAB};
}

// PHK - Push Program Bank Register (65816)
std::vector<uint8_t> Cpu6502::EncodePHK() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x4B};
}

// PHD - Push Direct Page Register (65816)
std::vector<uint8_t> Cpu6502::EncodePHD() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x0B};
}

// PLD - Pull Direct Page Register (65816)
std::vector<uint8_t> Cpu6502::EncodePLD() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x2B};
}

// ============================================================================
// Group 12: 65816 Transfer Operations
// ============================================================================

// TCD - Transfer C to Direct Page (65816)
std::vector<uint8_t> Cpu6502::EncodeTCD() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x5B};
}

// TDC - Transfer Direct Page to C (65816)
std::vector<uint8_t> Cpu6502::EncodeTDC() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x7B};
}

// TCS - Transfer C to Stack Pointer (65816)
std::vector<uint8_t> Cpu6502::EncodeTCS() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x1B};
}

// TSC - Transfer Stack Pointer to C (65816)
std::vector<uint8_t> Cpu6502::EncodeTSC() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x3B};
}

// ============================================================================
// Group 13: 65816 Long Jumps
// ============================================================================

// JML - Jump Long (65816)
std::vector<uint8_t> Cpu6502::EncodeJML(uint32_t operand, AddressingMode mode) const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }

    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::AbsoluteLong) {
        bytes.push_back(0x5C);  // JML opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));          // Low byte
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));   // Middle byte
        bytes.push_back(static_cast<uint8_t>((operand >> 16) & 0xFF));  // Bank byte
    }

    return bytes;
}

// JSL - Jump Subroutine Long (65816)
std::vector<uint8_t> Cpu6502::EncodeJSL(uint32_t operand, AddressingMode mode) const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }

    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::AbsoluteLong) {
        bytes.push_back(0x22);  // JSL opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));          // Low byte
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));   // Middle byte
        bytes.push_back(static_cast<uint8_t>((operand >> 16) & 0xFF));  // Bank byte
    }

    return bytes;
}

// RTL - Return from Subroutine Long (65816)
std::vector<uint8_t> Cpu6502::EncodeRTL() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x6B};
}

// ============================================================================
// Group 14: 65816 Miscellaneous Opcodes
// ============================================================================

// PEA - Push Effective Address (65816)
std::vector<uint8_t> Cpu6502::EncodePEA(uint16_t operand, AddressingMode mode) const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }

    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::Immediate) {
        bytes.push_back(0xF4);  // PEA opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
    }

    return bytes;
}

// PEI - Push Effective Indirect Address (65816)
std::vector<uint8_t> Cpu6502::EncodePEI(uint8_t operand, AddressingMode mode) const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }

    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::ZeroPage) {
        bytes.push_back(0xD4);  // PEI opcode
        bytes.push_back(operand);  // Zero page address
    }

    return bytes;
}

// PER - Push Effective PC Relative Address (65816)
std::vector<uint8_t> Cpu6502::EncodePER(uint16_t operand, AddressingMode mode) const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }

    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x62);  // PER opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
    }

    return bytes;
}

// MVN - Block Move Negative (65816)
std::vector<uint8_t> Cpu6502::EncodeMVN(uint8_t srcbank, uint8_t destbank) const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x54, srcbank, destbank};
}

// MVP - Block Move Positive (65816)
std::vector<uint8_t> Cpu6502::EncodeMVP(uint8_t srcbank, uint8_t destbank) const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0x44, srcbank, destbank};
}

// COP - Coprocessor (65816)
std::vector<uint8_t> Cpu6502::EncodeCOP(uint8_t operand, AddressingMode mode) const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }

    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::Immediate) {
        bytes.push_back(0x02);  // COP opcode
        bytes.push_back(operand);  // Signature byte
    }

    return bytes;
}

// WDM - Reserved (65816)
std::vector<uint8_t> Cpu6502::EncodeWDM(uint8_t operand, AddressingMode mode) const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }

    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::Immediate) {
        bytes.push_back(0x42);  // WDM opcode
        bytes.push_back(operand);  // Reserved byte
    }

    return bytes;
}

// XBA - Exchange B and A (65816)
std::vector<uint8_t> Cpu6502::EncodeXBA() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0xEB};
}

// XCE - Exchange Carry and Emulation (65816)
std::vector<uint8_t> Cpu6502::EncodeXCE() const {
    // Only available in 65816
    if (cpu_mode_ != CpuMode::Cpu65816) {
        return {};  // Not supported in 6502/65C02 mode
    }
    return {0xFB};
}

// ============================================================================
// Group 8: 65816 MX Directive (Register Width Control)
// ============================================================================

// Set MX flags (register width control for 65816)
// m_flag: true = 8-bit accumulator, false = 16-bit accumulator
// x_flag: true = 8-bit index, false = 16-bit index
void Cpu6502::SetMX(bool m_flag, bool x_flag) {
    m_flag_ = m_flag;
    x_flag_ = x_flag;
}

// Check if accumulator is 8-bit
bool Cpu6502::IsAccumulator8Bit() const {
    return m_flag_;  // true = 8-bit, false = 16-bit
}

// Check if index registers (X/Y) are 8-bit
bool Cpu6502::IsIndex8Bit() const {
    return x_flag_;  // true = 8-bit, false = 16-bit
}


} // namespace xasm
