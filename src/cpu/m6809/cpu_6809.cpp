/**
 * @file cpu_6809.cpp
 * @brief Motorola 6809 CPU Plugin implementation
 * 
 * Implements instruction encoding for the Motorola 6809 processor.
 * The 6809 is one of the most sophisticated 8-bit CPUs, featuring
 * advanced indexed addressing and 16-bit operations.
 */

#include "xasm++/cpu/cpu_6809.h"
#include <stdexcept>

namespace xasm {

// ============================================================================
// Direct Page Register Management
// ============================================================================

void Cpu6809::SetDirectPage(uint8_t dp) {
    direct_page_ = dp;
}

uint8_t Cpu6809::GetDirectPage() const {
    return direct_page_;
}

// ============================================================================
// Byte Order Utilities (Big-Endian)
// ============================================================================

std::vector<uint8_t> Cpu6809::ToBigEndian(uint16_t value) const {
    // 6809 uses big-endian: MSB first, LSB second
    return {
        static_cast<uint8_t>((value >> 8) & 0xFF),  // High byte
        static_cast<uint8_t>(value & 0xFF)           // Low byte
    };
}

// ============================================================================
// Inherent Instructions (1 byte, no operand)
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeNOP() const {
    return {0x12};  // NOP opcode
}

std::vector<uint8_t> Cpu6809::EncodeRTS() const {
    return {0x39};  // RTS opcode
}

std::vector<uint8_t> Cpu6809::EncodeCLRA() const {
    return {0x4F};  // CLRA opcode
}

std::vector<uint8_t> Cpu6809::EncodeCLRB() const {
    return {0x5F};  // CLRB opcode
}

// ============================================================================
// Data Movement Instructions - Load Accumulator
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeLDA(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0x86, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct: {
            return {0x96, static_cast<uint8_t>(operand & 0xFF)};
        }
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xB6};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes in Phase 5+
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeLDB(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0xC6, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0xD6, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xF6};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeLDD(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate16: {
            // LDD #$1234 -> {0xCC, 0x12, 0x34} (big-endian)
            auto result = std::vector<uint8_t>{0xCC};
            auto value_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), value_bytes.begin(), value_bytes.end());
            return result;
        }
        
        case AddressingMode6809::Direct:
            return {0xDC, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xFC};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

// ============================================================================
// Data Movement Instructions - Store Accumulator
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeSTA(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Direct:
            return {0x97, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xB7};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeSTB(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Direct:
            return {0xD7, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xF7};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeSTD(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Direct:
            return {0xDD, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xFD};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

// ============================================================================
// Indexed Addressing Support (Stub for Phase 5+)
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeIndexedPostByte(AddressingMode6809 mode,
                                                      int32_t offset,
                                                      uint8_t reg) const {
    // Silence unused parameter warnings for stub implementation
    (void)mode;
    (void)offset;
    (void)reg;

    // TODO: Implement in Phase 5+
    // This is the most complex part of the 6809 - 16 sub-modes
    return {};
}

// ============================================================================
// Instruction Size Calculation
// ============================================================================

size_t Cpu6809::CalculateInstructionSize(AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Inherent:
            return 1;
        
        case AddressingMode6809::Immediate8:
        case AddressingMode6809::Direct:
            return 2;
        
        case AddressingMode6809::Immediate16:
        case AddressingMode6809::Extended:
            return 3;
        
        case AddressingMode6809::Relative8:
            return 2;
        
        case AddressingMode6809::Relative16:
            return 3;
        
        // Indexed modes vary by sub-mode (2-5 bytes)
        case AddressingMode6809::IndexedZeroOffset:
        case AddressingMode6809::IndexedAutoInc1:
        case AddressingMode6809::IndexedAutoDec1:
            return 2;  // Opcode + post-byte
        
        case AddressingMode6809::Indexed5BitOffset:
        case AddressingMode6809::Indexed8BitOffset:
        case AddressingMode6809::IndexedAccumA:
        case AddressingMode6809::IndexedAccumB:
        case AddressingMode6809::IndexedAccumD:
        case AddressingMode6809::IndexedAutoInc2:
        case AddressingMode6809::IndexedAutoDec2:
        case AddressingMode6809::IndexedPCRelative8:
            return 3;  // Opcode + post-byte + 8-bit offset
        
        case AddressingMode6809::Indexed16BitOffset:
        case AddressingMode6809::IndexedPCRelative16:
            return 4;  // Opcode + post-byte + 16-bit offset
        
        case AddressingMode6809::IndexedIndirect:
        case AddressingMode6809::IndexedExtendedIndirect:
            return 5;  // Opcode + post-byte + 16-bit address
        
        default:
            return 0;
    }
}

} // namespace xasm
