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

// Shift and Rotate Instructions
std::vector<uint8_t> Cpu6809::EncodeASLA() const {
    return {0x48};
}

std::vector<uint8_t> Cpu6809::EncodeASLB() const {
    return {0x58};
}

std::vector<uint8_t> Cpu6809::EncodeASRA() const {
    return {0x47};
}

std::vector<uint8_t> Cpu6809::EncodeASRB() const {
    return {0x57};
}

std::vector<uint8_t> Cpu6809::EncodeLSRA() const {
    return {0x44};
}

std::vector<uint8_t> Cpu6809::EncodeLSRB() const {
    return {0x54};
}

std::vector<uint8_t> Cpu6809::EncodeROLA() const {
    return {0x49};
}

std::vector<uint8_t> Cpu6809::EncodeROLB() const {
    return {0x59};
}

std::vector<uint8_t> Cpu6809::EncodeRORA() const {
    return {0x46};
}

std::vector<uint8_t> Cpu6809::EncodeRORB() const {
    return {0x56};
}

// Increment/Decrement Instructions
std::vector<uint8_t> Cpu6809::EncodeINCA() const {
    return {0x4C};
}

std::vector<uint8_t> Cpu6809::EncodeINCB() const {
    return {0x5C};
}

std::vector<uint8_t> Cpu6809::EncodeDECA() const {
    return {0x4A};
}

std::vector<uint8_t> Cpu6809::EncodeDECB() const {
    return {0x5A};
}

// Test/Compare/Negate Instructions
std::vector<uint8_t> Cpu6809::EncodeTSTA() const {
    return {0x4D};
}

std::vector<uint8_t> Cpu6809::EncodeTSTB() const {
    return {0x5D};
}

std::vector<uint8_t> Cpu6809::EncodeCOMA() const {
    return {0x43};
}

std::vector<uint8_t> Cpu6809::EncodeCOMB() const {
    return {0x53};
}

std::vector<uint8_t> Cpu6809::EncodeNEGA() const {
    return {0x40};
}

std::vector<uint8_t> Cpu6809::EncodeNEGB() const {
    return {0x50};
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
        
        // Indexed addressing modes
        case AddressingMode6809::IndexedZeroOffset:
        case AddressingMode6809::Indexed5BitOffset:
        case AddressingMode6809::Indexed8BitOffset:
        case AddressingMode6809::Indexed16BitOffset:
        case AddressingMode6809::IndexedAccumA:
        case AddressingMode6809::IndexedAccumB:
        case AddressingMode6809::IndexedAccumD:
        case AddressingMode6809::IndexedAutoInc1:
        case AddressingMode6809::IndexedAutoInc2:
        case AddressingMode6809::IndexedAutoDec1:
        case AddressingMode6809::IndexedAutoDec2:
        case AddressingMode6809::IndexedPCRelative8:
        case AddressingMode6809::IndexedPCRelative16:
        case AddressingMode6809::IndexedIndirect:
        case AddressingMode6809::IndexedExtendedIndirect: {
            // LDA indexed opcode is 0xA6
            auto result = std::vector<uint8_t>{0xA6};
            // Default to X register (reg=0)
            auto postbyte = EncodeIndexedPostByte(mode, static_cast<int32_t>(operand), 0);
            result.insert(result.end(), postbyte.begin(), postbyte.end());
            return result;
        }
        
        default:
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
        
        // Indexed addressing modes
        case AddressingMode6809::IndexedZeroOffset:
        case AddressingMode6809::Indexed5BitOffset:
        case AddressingMode6809::Indexed8BitOffset:
        case AddressingMode6809::Indexed16BitOffset:
        case AddressingMode6809::IndexedAccumA:
        case AddressingMode6809::IndexedAccumB:
        case AddressingMode6809::IndexedAccumD:
        case AddressingMode6809::IndexedAutoInc1:
        case AddressingMode6809::IndexedAutoInc2:
        case AddressingMode6809::IndexedAutoDec1:
        case AddressingMode6809::IndexedAutoDec2:
        case AddressingMode6809::IndexedPCRelative8:
        case AddressingMode6809::IndexedPCRelative16:
        case AddressingMode6809::IndexedIndirect:
        case AddressingMode6809::IndexedExtendedIndirect: {
            // LDB indexed opcode is 0xE6
            auto result = std::vector<uint8_t>{0xE6};
            auto postbyte = EncodeIndexedPostByte(mode, static_cast<int32_t>(operand), 0);
            result.insert(result.end(), postbyte.begin(), postbyte.end());
            return result;
        }
        
        default:
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
        
        // Indexed addressing modes
        case AddressingMode6809::IndexedZeroOffset:
        case AddressingMode6809::Indexed5BitOffset:
        case AddressingMode6809::Indexed8BitOffset:
        case AddressingMode6809::Indexed16BitOffset:
        case AddressingMode6809::IndexedAccumA:
        case AddressingMode6809::IndexedAccumB:
        case AddressingMode6809::IndexedAccumD:
        case AddressingMode6809::IndexedAutoInc1:
        case AddressingMode6809::IndexedAutoInc2:
        case AddressingMode6809::IndexedAutoDec1:
        case AddressingMode6809::IndexedAutoDec2:
        case AddressingMode6809::IndexedPCRelative8:
        case AddressingMode6809::IndexedPCRelative16:
        case AddressingMode6809::IndexedIndirect:
        case AddressingMode6809::IndexedExtendedIndirect: {
            // LDD indexed opcode is 0xEC
            auto result = std::vector<uint8_t>{0xEC};
            auto postbyte = EncodeIndexedPostByte(mode, static_cast<int32_t>(operand), 0);
            result.insert(result.end(), postbyte.begin(), postbyte.end());
            return result;
        }
        
        default:
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
        
        // Indexed addressing modes
        case AddressingMode6809::IndexedZeroOffset:
        case AddressingMode6809::Indexed5BitOffset:
        case AddressingMode6809::Indexed8BitOffset:
        case AddressingMode6809::Indexed16BitOffset:
        case AddressingMode6809::IndexedAccumA:
        case AddressingMode6809::IndexedAccumB:
        case AddressingMode6809::IndexedAccumD:
        case AddressingMode6809::IndexedAutoInc1:
        case AddressingMode6809::IndexedAutoInc2:
        case AddressingMode6809::IndexedAutoDec1:
        case AddressingMode6809::IndexedAutoDec2:
        case AddressingMode6809::IndexedPCRelative8:
        case AddressingMode6809::IndexedPCRelative16:
        case AddressingMode6809::IndexedIndirect:
        case AddressingMode6809::IndexedExtendedIndirect: {
            // STB indexed opcode is 0xE7
            auto result = std::vector<uint8_t>{0xE7};
            auto postbyte = EncodeIndexedPostByte(mode, static_cast<int32_t>(operand), 0);
            result.insert(result.end(), postbyte.begin(), postbyte.end());
            return result;
        }
        
        default:
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
// Data Movement Instructions - 16-bit Index Register Load/Store
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeLDX(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate16: {
            auto result = std::vector<uint8_t>{0x8E};
            auto value_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), value_bytes.begin(), value_bytes.end());
            return result;
        }
        
        case AddressingMode6809::Direct:
            return {0x9E, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xBE};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeLDY(uint32_t operand, AddressingMode6809 mode) const {
    // LDY uses page 2 prefix ($10)
    switch (mode) {
        case AddressingMode6809::Immediate16: {
            auto result = std::vector<uint8_t>{0x10, 0x8E};
            auto value_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), value_bytes.begin(), value_bytes.end());
            return result;
        }
        
        case AddressingMode6809::Direct:
            return {0x10, 0x9E, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0x10, 0xBE};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeSTX(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Direct:
            return {0x9F, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xBF};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeSTY(uint32_t operand, AddressingMode6809 mode) const {
    // STY uses page 2 prefix ($10)
    switch (mode) {
        case AddressingMode6809::Direct:
            return {0x10, 0x9F, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0x10, 0xBF};
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
// Arithmetic Instructions
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeADDA(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0x8B, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0x9B, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xBB};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeADDB(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0xCB, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0xDB, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xFB};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeSUBA(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0x80, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0x90, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xB0};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            // TODO: Implement indexed modes
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeSUBB(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0xC0, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0xD0, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xF0};
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
// Branch Instructions (Relative Addressing)
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeBRA(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x20, static_cast<uint8_t>(offset & 0xFF)};
    }
    // TODO: Implement Relative16 (LBRA) in future
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBEQ(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x27, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBNE(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x26, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBCC(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x24, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBCS(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x25, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBMI(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x2B, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBPL(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x2A, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBVS(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x29, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBVC(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x28, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBGE(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x2C, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBLT(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x2D, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBGT(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x2E, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBLE(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x2F, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBHI(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x22, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBLS(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x23, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

std::vector<uint8_t> Cpu6809::EncodeBSR(int32_t offset, AddressingMode6809 mode) const {
    if (mode == AddressingMode6809::Relative8) {
        return {0x8D, static_cast<uint8_t>(offset & 0xFF)};
    }
    return {};
}

// ============================================================================
// Long Branch Instructions (16-bit relative addressing)
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeLBRA(int16_t offset) const {
    // LBRA - Long Branch Always (Opcode: 0x10 0x16)
    std::vector<uint8_t> result = {0x10, 0x16};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBRN(int16_t offset) const {
    // LBRN - Long Branch Never (Opcode: 0x10 0x21)
    std::vector<uint8_t> result = {0x10, 0x21};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBHI(int16_t offset) const {
    // LBHI - Long Branch if Higher, unsigned (Opcode: 0x10 0x22)
    std::vector<uint8_t> result = {0x10, 0x22};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBLS(int16_t offset) const {
    // LBLS - Long Branch if Lower or Same, unsigned (Opcode: 0x10 0x23)
    std::vector<uint8_t> result = {0x10, 0x23};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBCC(int16_t offset) const {
    // LBCC/LBHS - Long Branch if Carry Clear (Opcode: 0x10 0x24)
    std::vector<uint8_t> result = {0x10, 0x24};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBCS(int16_t offset) const {
    // LBCS/LBLO - Long Branch if Carry Set (Opcode: 0x10 0x25)
    std::vector<uint8_t> result = {0x10, 0x25};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBNE(int16_t offset) const {
    // LBNE - Long Branch if Not Equal (Opcode: 0x10 0x26)
    std::vector<uint8_t> result = {0x10, 0x26};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBEQ(int16_t offset) const {
    // LBEQ - Long Branch if Equal (Opcode: 0x10 0x27)
    std::vector<uint8_t> result = {0x10, 0x27};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBVC(int16_t offset) const {
    // LBVC - Long Branch if Overflow Clear (Opcode: 0x10 0x28)
    std::vector<uint8_t> result = {0x10, 0x28};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBVS(int16_t offset) const {
    // LBVS - Long Branch if Overflow Set (Opcode: 0x10 0x29)
    std::vector<uint8_t> result = {0x10, 0x29};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBPL(int16_t offset) const {
    // LBPL - Long Branch if Plus (Opcode: 0x10 0x2A)
    std::vector<uint8_t> result = {0x10, 0x2A};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBMI(int16_t offset) const {
    // LBMI - Long Branch if Minus (Opcode: 0x10 0x2B)
    std::vector<uint8_t> result = {0x10, 0x2B};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBGE(int16_t offset) const {
    // LBGE - Long Branch if Greater or Equal, signed (Opcode: 0x10 0x2C)
    std::vector<uint8_t> result = {0x10, 0x2C};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBLT(int16_t offset) const {
    // LBLT - Long Branch if Less Than, signed (Opcode: 0x10 0x2D)
    std::vector<uint8_t> result = {0x10, 0x2D};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBGT(int16_t offset) const {
    // LBGT - Long Branch if Greater Than, signed (Opcode: 0x10 0x2E)
    std::vector<uint8_t> result = {0x10, 0x2E};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBLE(int16_t offset) const {
    // LBLE - Long Branch if Less or Equal, signed (Opcode: 0x10 0x2F)
    std::vector<uint8_t> result = {0x10, 0x2F};
    auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
    result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
    return result;
}

// ============================================================================
// Comparison Instructions
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeCMPA(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0x81, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0x91, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xB1};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeCMPB(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0xC1, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0xD1, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xF1};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeCMPX(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate16: {
            auto result = std::vector<uint8_t>{0x8C};
            auto value_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), value_bytes.begin(), value_bytes.end());
            return result;
        }
        
        case AddressingMode6809::Direct:
            return {0x9C, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xBC};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeCMPY(uint32_t operand, AddressingMode6809 mode) const {
    // CMPY uses page 2 prefix ($10)
    switch (mode) {
        case AddressingMode6809::Immediate16: {
            auto result = std::vector<uint8_t>{0x10, 0x8C};
            auto value_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), value_bytes.begin(), value_bytes.end());
            return result;
        }
        
        case AddressingMode6809::Direct:
            return {0x10, 0x9C, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0x10, 0xBC};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

// ============================================================================
// Logical Operations
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeANDA(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0x84, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0x94, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xB4};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeANDB(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0xC4, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0xD4, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xF4};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeORA(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0x8A, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0x9A, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xBA};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeORB(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0xCA, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0xDA, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xFA};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeEORA(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0x88, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0x98, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xB8};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeEORB(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0xC8, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0xD8, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xF8};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

// ============================================================================
// Bit Test Operations
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeBITA(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0x85, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0x95, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xB5};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeBITB(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Immediate8:
            return {0xC5, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Direct:
            return {0xD5, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xF5};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

// ============================================================================
// Control Flow Instructions
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeJSR(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Direct:
            return {0x9D, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0xBD};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeJMP(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Direct:
            return {0x0E, static_cast<uint8_t>(operand & 0xFF)};
        
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0x7E};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeLEAX(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0x30};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        // Indexed addressing modes (LEA typically uses indexed addressing)
        case AddressingMode6809::IndexedZeroOffset:
        case AddressingMode6809::Indexed5BitOffset:
        case AddressingMode6809::Indexed8BitOffset:
        case AddressingMode6809::Indexed16BitOffset:
        case AddressingMode6809::IndexedAccumA:
        case AddressingMode6809::IndexedAccumB:
        case AddressingMode6809::IndexedAccumD:
        case AddressingMode6809::IndexedAutoInc1:
        case AddressingMode6809::IndexedAutoInc2:
        case AddressingMode6809::IndexedAutoDec1:
        case AddressingMode6809::IndexedAutoDec2:
        case AddressingMode6809::IndexedPCRelative8:
        case AddressingMode6809::IndexedPCRelative16:
        case AddressingMode6809::IndexedIndirect:
        case AddressingMode6809::IndexedExtendedIndirect: {
            // LEAX opcode is 0x30
            auto result = std::vector<uint8_t>{0x30};
            auto postbyte = EncodeIndexedPostByte(mode, static_cast<int32_t>(operand), 0);
            result.insert(result.end(), postbyte.begin(), postbyte.end());
            return result;
        }
        
        default:
            return {};
    }
}

std::vector<uint8_t> Cpu6809::EncodeLEAY(uint32_t operand, AddressingMode6809 mode) const {
    switch (mode) {
        case AddressingMode6809::Extended: {
            auto result = std::vector<uint8_t>{0x31};
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(operand));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            return result;
        }
        
        default:
            return {};
    }
}

// ============================================================================
// Stack Operations (PSHS, PULS, PSHU, PULU)
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodePSHS(uint8_t mask) const {
    return {0x34, mask};
}

std::vector<uint8_t> Cpu6809::EncodePULS(uint8_t mask) const {
    return {0x35, mask};
}

std::vector<uint8_t> Cpu6809::EncodePSHU(uint8_t mask) const {
    return {0x36, mask};
}

std::vector<uint8_t> Cpu6809::EncodePULU(uint8_t mask) const {
    return {0x37, mask};
}

// ============================================================================
// Register Transfer and Exchange (TFR, EXG)
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeTFR(uint8_t src, uint8_t dst) const {
    // Post-byte format: high nibble = source, low nibble = destination
    uint8_t postbyte = (src << 4) | dst;
    return {0x1F, postbyte};
}

std::vector<uint8_t> Cpu6809::EncodeEXG(uint8_t reg1, uint8_t reg2) const {
    // Post-byte format: high nibble = reg1, low nibble = reg2
    uint8_t postbyte = (reg1 << 4) | reg2;
    return {0x1E, postbyte};
}

// ============================================================================
// Indexed Addressing Support - Post-byte Encoding
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeIndexedPostByte(AddressingMode6809 mode,
                                                      int32_t offset,
                                                      uint8_t reg) const {
    std::vector<uint8_t> result;
    
    // Register encoding in bits 6-5:
    // 00 = X, 01 = Y, 10 = U, 11 = S
    uint8_t reg_bits = (reg & 0x03) << 5;
    
    switch (mode) {
        case AddressingMode6809::IndexedZeroOffset:
            // Post-byte: 1RR00100 (no offset, indexed)
            result.push_back(0x84 | reg_bits);
            break;
            
        case AddressingMode6809::Indexed5BitOffset: {
            // Post-byte: 0RRnnnnn (5-bit two's complement offset)
            // Offset must be in range -16 to +15
            uint8_t offset_5bit = static_cast<uint8_t>(offset & 0x1F);
            result.push_back(offset_5bit | reg_bits);
            break;
        }
            
        case AddressingMode6809::Indexed8BitOffset:
            // Post-byte: 1RR01000, then 8-bit signed offset
            result.push_back(0x88 | reg_bits);
            result.push_back(static_cast<uint8_t>(offset & 0xFF));
            break;
            
        case AddressingMode6809::Indexed16BitOffset: {
            // Post-byte: 1RR01001, then 16-bit signed offset (big-endian)
            result.push_back(0x89 | reg_bits);
            auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
            result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
            break;
        }
            
        case AddressingMode6809::IndexedAccumA:
            // Post-byte: 1RR00110 (A register offset)
            result.push_back(0x86 | reg_bits);
            break;
            
        case AddressingMode6809::IndexedAccumB:
            // Post-byte: 1RR00101 (B register offset)
            result.push_back(0x85 | reg_bits);
            break;
            
        case AddressingMode6809::IndexedAccumD:
            // Post-byte: 1RR01011 (D register offset)
            result.push_back(0x8B | reg_bits);
            break;
            
        case AddressingMode6809::IndexedAutoInc1:
            // Post-byte: 1RR00000 (auto-increment by 1)
            result.push_back(0x80 | reg_bits);
            break;
            
        case AddressingMode6809::IndexedAutoInc2:
            // Post-byte: 1RR00001 (auto-increment by 2)
            result.push_back(0x81 | reg_bits);
            break;
            
        case AddressingMode6809::IndexedAutoDec1:
            // Post-byte: 1RR00010 (auto-decrement by 1)
            result.push_back(0x82 | reg_bits);
            break;
            
        case AddressingMode6809::IndexedAutoDec2:
            // Post-byte: 1RR00011 (auto-decrement by 2)
            result.push_back(0x83 | reg_bits);
            break;
            
        case AddressingMode6809::IndexedPCRelative8:
            // Post-byte: 1RR01100 (PC-relative with 8-bit offset)
            // Note: PC-relative uses special register encoding
            result.push_back(0x8C | reg_bits);
            result.push_back(static_cast<uint8_t>(offset & 0xFF));
            break;
            
        case AddressingMode6809::IndexedPCRelative16: {
            // Post-byte: 1RR01101 (PC-relative with 16-bit offset)
            result.push_back(0x8D | reg_bits);
            auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
            result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
            break;
        }
            
        case AddressingMode6809::IndexedIndirect: {
            // Indirect indexed: set bit 7 of post-byte
            // Need to determine sub-mode based on offset
            if (offset == 0) {
                // [,R] - zero offset indirect
                result.push_back(0x94 | reg_bits);  // Bit 7=1 (indirect), base mode 0x14
            } else if (offset >= -128 && offset <= 127) {
                // [n,R] - 8-bit offset indirect
                result.push_back(0x98 | reg_bits);  // Bit 7=1 (indirect), 8-bit mode
                result.push_back(static_cast<uint8_t>(offset & 0xFF));
            } else {
                // [nn,R] - 16-bit offset indirect
                result.push_back(0x99 | reg_bits);  // Bit 7=1 (indirect), 16-bit mode
                auto offset_bytes = ToBigEndian(static_cast<uint16_t>(offset));
                result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
            }
            break;
        }
            
        case AddressingMode6809::IndexedExtendedIndirect: {
            // [$nnnn] - extended indirect
            // Post-byte: 10011111, then 16-bit address
            result.push_back(0x9F);
            auto addr_bytes = ToBigEndian(static_cast<uint16_t>(offset));
            result.insert(result.end(), addr_bytes.begin(), addr_bytes.end());
            break;
        }
            
        default:
            // Unsupported indexed mode
            break;
    }
    
    return result;
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
