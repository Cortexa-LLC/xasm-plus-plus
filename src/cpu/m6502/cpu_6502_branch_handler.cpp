// 6502 Branch Relaxation Handler implementation
// Extracted from Cpu6502 Large class refactoring - improving SRP compliance (M6.1)

#include "xasm++/cpu/cpu_6502_branch_handler.h"
#include "xasm++/cpu/opcodes_6502.h"

namespace xasm {

bool Cpu6502BranchHandler::NeedsBranchRelaxation(uint16_t current_addr, uint16_t target_addr) const {
    // Calculate offset: target - (PC + 2)
    // PC + 2 because branch instruction is 2 bytes (opcode + offset)
    int16_t offset = static_cast<int16_t>(target_addr) - static_cast<int16_t>(current_addr + 2);
    
    // Check if offset fits in 8-bit signed range (-128 to +127)
    return (offset < -128 || offset > 127);
}

uint8_t Cpu6502BranchHandler::GetComplementaryBranchOpcode(uint8_t branch_opcode) const {
    return branch_opcode ^ Opcodes::BRANCH_COMPLEMENT_MASK;
}

std::vector<uint8_t> Cpu6502BranchHandler::EncodeBranchWithRelaxation(uint8_t branch_opcode,
                                                                        uint16_t current_addr,
                                                                        uint16_t target_addr) const {
    std::vector<uint8_t> bytes;
    
    if (!NeedsBranchRelaxation(current_addr, target_addr)) {
        // Branch is in range - emit normal 2-byte branch
        int16_t offset = static_cast<int16_t>(target_addr) - static_cast<int16_t>(current_addr + 2);
        bytes.push_back(branch_opcode);
        bytes.push_back(static_cast<uint8_t>(offset & 0xFF));
    } else {
        // Branch is out of range - emit relaxed 5-byte sequence
        // Format: [B!cc] [0x03] [JMP] [target_lo] [target_hi]
        
        // 1. Emit complementary branch (inverted condition)
        uint8_t complement = GetComplementaryBranchOpcode(branch_opcode);
        bytes.push_back(complement);
        
        // 2. Emit offset of +3 (skip over the 3-byte JMP instruction)
        bytes.push_back(Opcodes::BRANCH_RELAXATION_OFFSET);
        
        // 3. Emit JMP absolute to target
        bytes.push_back(Opcodes::JMP_ABS);  // JMP opcode
        bytes.push_back(static_cast<uint8_t>(target_addr & 0xFF));        // Low byte
        bytes.push_back(static_cast<uint8_t>((target_addr >> 8) & 0xFF)); // High byte
    }
    
    return bytes;
}

} // namespace xasm
