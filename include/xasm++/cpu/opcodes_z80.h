// Z80 Opcode Constants
// Replaces magic numbers with descriptive named constants

#pragma once

#include <cstdint>

namespace xasm {

// ============================================================================
// Z80 Mnemonic String Constants
// ============================================================================
// Eliminates magic strings code smell by providing named constants for
// instruction mnemonics used in string comparisons throughout parsers.
namespace Z80Mnemonics {

// Load/Store Instructions
constexpr const char* LD = "LD";
constexpr const char* PUSH = "PUSH";
constexpr const char* POP = "POP";

// Arithmetic Instructions
constexpr const char* ADD = "ADD";
constexpr const char* ADC = "ADC";
constexpr const char* SUB = "SUB";
constexpr const char* SBC = "SBC";
constexpr const char* INC = "INC";
constexpr const char* DEC = "DEC";

// Logical Instructions
constexpr const char* AND = "AND";
constexpr const char* OR = "OR";
constexpr const char* XOR = "XOR";
constexpr const char* CP = "CP";

// Branch/Jump Instructions
constexpr const char* JP = "JP";
constexpr const char* JR = "JR";
constexpr const char* CALL = "CALL";
constexpr const char* RET = "RET";
constexpr const char* RST = "RST";
constexpr const char* DJNZ = "DJNZ";

// Rotate/Shift Instructions
constexpr const char* RLCA = "RLCA";
constexpr const char* RRCA = "RRCA";
constexpr const char* RLA = "RLA";
constexpr const char* RRA = "RRA";
constexpr const char* RLC = "RLC";
constexpr const char* RRC = "RRC";
constexpr const char* RL = "RL";
constexpr const char* RR = "RR";
constexpr const char* SLA = "SLA";
constexpr const char* SRA = "SRA";
constexpr const char* SRL = "SRL";

// Bit Manipulation Instructions
constexpr const char* BIT = "BIT";
constexpr const char* SET = "SET";
constexpr const char* RES = "RES";

// Miscellaneous Instructions
constexpr const char* NOP = "NOP";
constexpr const char* HALT = "HALT";
constexpr const char* DI = "DI";
constexpr const char* EI = "EI";
constexpr const char* NEG = "NEG";
constexpr const char* CPL = "CPL";
constexpr const char* CCF = "CCF";
constexpr const char* SCF = "SCF";
constexpr const char* DAA = "DAA";

// Block Transfer/Search Instructions
constexpr const char* LDI = "LDI";
constexpr const char* LDIR = "LDIR";
constexpr const char* LDD = "LDD";
constexpr const char* LDDR = "LDDR";
constexpr const char* CPI = "CPI";
constexpr const char* CPIR = "CPIR";
constexpr const char* CPD = "CPD";
constexpr const char* CPDR = "CPDR";

// Input/Output Instructions
constexpr const char* IN = "IN";
constexpr const char* OUT = "OUT";
constexpr const char* INI = "INI";
constexpr const char* INIR = "INIR";
constexpr const char* IND = "IND";
constexpr const char* INDR = "INDR";
constexpr const char* OUTI = "OUTI";
constexpr const char* OTIR = "OTIR";
constexpr const char* OUTD = "OUTD";
constexpr const char* OTDR = "OTDR";

// Exchange Instructions
constexpr const char* EX = "EX";
constexpr const char* EXX = "EXX";

// Interrupt Instructions
constexpr const char* RETI = "RETI";
constexpr const char* RETN = "RETN";
constexpr const char* IM = "IM";

} // namespace Z80Mnemonics

namespace Z80Opcodes {

// ============================================================================
// Load/Store Instructions - 8-bit
// ============================================================================

// LD r, n - Load register with immediate value
constexpr uint8_t LD_B_n = 0x06; // LD B, n      - Load B with immediate
constexpr uint8_t LD_C_n = 0x0E; // LD C, n      - Load C with immediate
constexpr uint8_t LD_D_n = 0x16; // LD D, n      - Load D with immediate
constexpr uint8_t LD_E_n = 0x1E; // LD E, n      - Load E with immediate
constexpr uint8_t LD_H_n = 0x26; // LD H, n      - Load H with immediate
constexpr uint8_t LD_L_n = 0x2E; // LD L, n      - Load L with immediate
constexpr uint8_t LD_A_n = 0x3E; // LD A, n      - Load A with immediate

// LD r, r' - Load register from register
constexpr uint8_t LD_B_B = 0x40; // LD B, B
constexpr uint8_t LD_B_C = 0x41; // LD B, C
constexpr uint8_t LD_B_D = 0x42; // LD B, D
constexpr uint8_t LD_B_E = 0x43; // LD B, E
constexpr uint8_t LD_B_H = 0x44; // LD B, H
constexpr uint8_t LD_B_L = 0x45; // LD B, L
constexpr uint8_t LD_B_HL = 0x46; // LD B, (HL)
constexpr uint8_t LD_B_A = 0x47; // LD B, A

constexpr uint8_t LD_C_B = 0x48; // LD C, B
constexpr uint8_t LD_C_C = 0x49; // LD C, C
constexpr uint8_t LD_C_D = 0x4A; // LD C, D
constexpr uint8_t LD_C_E = 0x4B; // LD C, E
constexpr uint8_t LD_C_H = 0x4C; // LD C, H
constexpr uint8_t LD_C_L = 0x4D; // LD C, L
constexpr uint8_t LD_C_HL = 0x4E; // LD C, (HL)
constexpr uint8_t LD_C_A = 0x4F; // LD C, A

constexpr uint8_t LD_D_B = 0x50; // LD D, B
constexpr uint8_t LD_D_C = 0x51; // LD D, C
constexpr uint8_t LD_D_D = 0x52; // LD D, D
constexpr uint8_t LD_D_E = 0x53; // LD D, E
constexpr uint8_t LD_D_H = 0x54; // LD D, H
constexpr uint8_t LD_D_L = 0x55; // LD D, L
constexpr uint8_t LD_D_HL = 0x56; // LD D, (HL)
constexpr uint8_t LD_D_A = 0x57; // LD D, A

constexpr uint8_t LD_E_B = 0x58; // LD E, B
constexpr uint8_t LD_E_C = 0x59; // LD E, C
constexpr uint8_t LD_E_D = 0x5A; // LD E, D
constexpr uint8_t LD_E_E = 0x5B; // LD E, E
constexpr uint8_t LD_E_H = 0x5C; // LD E, H
constexpr uint8_t LD_E_L = 0x5D; // LD E, L
constexpr uint8_t LD_E_HL = 0x5E; // LD E, (HL)
constexpr uint8_t LD_E_A = 0x5F; // LD E, A

constexpr uint8_t LD_H_B = 0x60; // LD H, B
constexpr uint8_t LD_H_C = 0x61; // LD H, C
constexpr uint8_t LD_H_D = 0x62; // LD H, D
constexpr uint8_t LD_H_E = 0x63; // LD H, E
constexpr uint8_t LD_H_H = 0x64; // LD H, H
constexpr uint8_t LD_H_L = 0x65; // LD H, L
constexpr uint8_t LD_H_HL = 0x66; // LD H, (HL)
constexpr uint8_t LD_H_A = 0x67; // LD H, A

constexpr uint8_t LD_L_B = 0x68; // LD L, B
constexpr uint8_t LD_L_C = 0x69; // LD L, C
constexpr uint8_t LD_L_D = 0x6A; // LD L, D
constexpr uint8_t LD_L_E = 0x6B; // LD L, E
constexpr uint8_t LD_L_H = 0x6C; // LD L, H
constexpr uint8_t LD_L_L = 0x6D; // LD L, L
constexpr uint8_t LD_L_HL = 0x6E; // LD L, (HL)
constexpr uint8_t LD_L_A = 0x6F; // LD L, A

constexpr uint8_t LD_HL_B = 0x70; // LD (HL), B
constexpr uint8_t LD_HL_C = 0x71; // LD (HL), C
constexpr uint8_t LD_HL_D = 0x72; // LD (HL), D
constexpr uint8_t LD_HL_E = 0x73; // LD (HL), E
constexpr uint8_t LD_HL_H = 0x74; // LD (HL), H
constexpr uint8_t LD_HL_L = 0x75; // LD (HL), L
constexpr uint8_t HALT = 0x76;    // HALT
constexpr uint8_t LD_HL_A = 0x77; // LD (HL), A

constexpr uint8_t LD_A_B = 0x78; // LD A, B
constexpr uint8_t LD_A_C = 0x79; // LD A, C
constexpr uint8_t LD_A_D = 0x7A; // LD A, D
constexpr uint8_t LD_A_E = 0x7B; // LD A, E
constexpr uint8_t LD_A_H = 0x7C; // LD A, H
constexpr uint8_t LD_A_L = 0x7D; // LD A, L
constexpr uint8_t LD_A_HL = 0x7E; // LD A, (HL)
constexpr uint8_t LD_A_A = 0x7F; // LD A, A

// LD A, (addr) - Load A from memory address
constexpr uint8_t LD_A_addr = 0x3A; // LD A, (nn)   - Load A from address
constexpr uint8_t LD_addr_A = 0x32; // LD (nn), A   - Store A to address

// LD A, (BC/DE) - Load A from register pair indirect
constexpr uint8_t LD_A_BC = 0x0A; // LD A, (BC)   - Load A from address in BC
constexpr uint8_t LD_A_DE = 0x1A; // LD A, (DE)   - Load A from address in DE

// LD (BC/DE), A - Store A to register pair indirect
constexpr uint8_t LD_BC_A = 0x02; // LD (BC), A   - Store A to address in BC
constexpr uint8_t LD_DE_A = 0x12; // LD (DE), A   - Store A to address in DE

// LD (HL), n - Load immediate to memory at HL
constexpr uint8_t LD_HL_n = 0x36; // LD (HL), n   - Store immediate to address in HL

// ============================================================================
// Load/Store Instructions - 16-bit
// ============================================================================

constexpr uint8_t LD_BC_nn = 0x01; // LD BC, nn    - Load BC with immediate 16-bit
constexpr uint8_t LD_DE_nn = 0x11; // LD DE, nn    - Load DE with immediate 16-bit
constexpr uint8_t LD_HL_nn = 0x21; // LD HL, nn    - Load HL with immediate 16-bit
constexpr uint8_t LD_SP_nn = 0x31; // LD SP, nn    - Load SP with immediate 16-bit

// LD HL, (nn) / LD (nn), HL - 16-bit memory operations
constexpr uint8_t LD_HL_addr = 0x2A; // LD HL, (nn)  - Load HL from memory address
constexpr uint8_t LD_addr_HL = 0x22; // LD (nn), HL  - Store HL to memory address

// LD SP, HL - Load stack pointer from HL
constexpr uint8_t LD_SP_HL = 0xF9; // LD SP, HL    - Copy HL to SP

// ============================================================================
// Arithmetic Instructions
// ============================================================================

// ADD - Add
constexpr uint8_t ADD_A_B = 0x80; // ADD A, B
constexpr uint8_t ADD_A_C = 0x81; // ADD A, C
constexpr uint8_t ADD_A_D = 0x82; // ADD A, D
constexpr uint8_t ADD_A_E = 0x83; // ADD A, E
constexpr uint8_t ADD_A_H = 0x84; // ADD A, H
constexpr uint8_t ADD_A_L = 0x85; // ADD A, L
constexpr uint8_t ADD_A_HL = 0x86; // ADD A, (HL)
constexpr uint8_t ADD_A_A = 0x87; // ADD A, A
constexpr uint8_t ADD_A_n = 0xC6; // ADD A, n     - Add immediate to A

// ADC - Add with Carry
constexpr uint8_t ADC_A_B = 0x88; // ADC A, B
constexpr uint8_t ADC_A_C = 0x89; // ADC A, C
constexpr uint8_t ADC_A_D = 0x8A; // ADC A, D
constexpr uint8_t ADC_A_E = 0x8B; // ADC A, E
constexpr uint8_t ADC_A_H = 0x8C; // ADC A, H
constexpr uint8_t ADC_A_L = 0x8D; // ADC A, L
constexpr uint8_t ADC_A_HL = 0x8E; // ADC A, (HL)
constexpr uint8_t ADC_A_A = 0x8F; // ADC A, A
constexpr uint8_t ADC_A_n = 0xCE; // ADC A, n     - Add immediate with carry to A

// SUB - Subtract
constexpr uint8_t SUB_B = 0x90; // SUB B
constexpr uint8_t SUB_C = 0x91; // SUB C
constexpr uint8_t SUB_D = 0x92; // SUB D
constexpr uint8_t SUB_E = 0x93; // SUB E
constexpr uint8_t SUB_H = 0x94; // SUB H
constexpr uint8_t SUB_L = 0x95; // SUB L
constexpr uint8_t SUB_HL = 0x96; // SUB (HL)
constexpr uint8_t SUB_A = 0x97; // SUB A
constexpr uint8_t SUB_n = 0xD6; // SUB n        - Subtract immediate from A

// SBC - Subtract with Carry
constexpr uint8_t SBC_A_B = 0x98; // SBC A, B
constexpr uint8_t SBC_A_C = 0x99; // SBC A, C
constexpr uint8_t SBC_A_D = 0x9A; // SBC A, D
constexpr uint8_t SBC_A_E = 0x9B; // SBC A, E
constexpr uint8_t SBC_A_H = 0x9C; // SBC A, H
constexpr uint8_t SBC_A_L = 0x9D; // SBC A, L
constexpr uint8_t SBC_A_HL = 0x9E; // SBC A, (HL)
constexpr uint8_t SBC_A_A = 0x9F; // SBC A, A
constexpr uint8_t SBC_A_n = 0xDE; // SBC A, n     - Subtract immediate with carry from A

// INC - Increment
constexpr uint8_t INC_B = 0x04;  // INC B
constexpr uint8_t INC_C = 0x0C;  // INC C
constexpr uint8_t INC_D = 0x14;  // INC D
constexpr uint8_t INC_E = 0x1C;  // INC E
constexpr uint8_t INC_H = 0x24;  // INC H
constexpr uint8_t INC_L = 0x2C;  // INC L
constexpr uint8_t INC_HL = 0x34; // INC (HL)
constexpr uint8_t INC_A = 0x3C;  // INC A

constexpr uint8_t INC_BC = 0x03; // INC BC       - Increment 16-bit BC
constexpr uint8_t INC_DE = 0x13; // INC DE       - Increment 16-bit DE
constexpr uint8_t INC_HL_16 = 0x23; // INC HL    - Increment 16-bit HL
constexpr uint8_t INC_SP = 0x33; // INC SP       - Increment 16-bit SP

// DEC - Decrement
constexpr uint8_t DEC_B = 0x05;  // DEC B
constexpr uint8_t DEC_C = 0x0D;  // DEC C
constexpr uint8_t DEC_D = 0x15;  // DEC D
constexpr uint8_t DEC_E = 0x1D;  // DEC E
constexpr uint8_t DEC_H = 0x25;  // DEC H
constexpr uint8_t DEC_L = 0x2D;  // DEC L
constexpr uint8_t DEC_HL = 0x35; // DEC (HL)
constexpr uint8_t DEC_A = 0x3D;  // DEC A

constexpr uint8_t DEC_BC = 0x0B; // DEC BC       - Decrement 16-bit BC
constexpr uint8_t DEC_DE = 0x1B; // DEC DE       - Decrement 16-bit DE
constexpr uint8_t DEC_HL_16 = 0x2B; // DEC HL    - Decrement 16-bit HL
constexpr uint8_t DEC_SP = 0x3B; // DEC SP       - Decrement 16-bit SP

// ============================================================================
// Logical Instructions
// ============================================================================

// AND - Logical AND
constexpr uint8_t AND_B = 0xA0; // AND B
constexpr uint8_t AND_C = 0xA1; // AND C
constexpr uint8_t AND_D = 0xA2; // AND D
constexpr uint8_t AND_E = 0xA3; // AND E
constexpr uint8_t AND_H = 0xA4; // AND H
constexpr uint8_t AND_L = 0xA5; // AND L
constexpr uint8_t AND_HL = 0xA6; // AND (HL)
constexpr uint8_t AND_A = 0xA7; // AND A
constexpr uint8_t AND_n = 0xE6; // AND n        - AND immediate with A

// OR - Logical OR
constexpr uint8_t OR_B = 0xB0; // OR B
constexpr uint8_t OR_C = 0xB1; // OR C
constexpr uint8_t OR_D = 0xB2; // OR D
constexpr uint8_t OR_E = 0xB3; // OR E
constexpr uint8_t OR_H = 0xB4; // OR H
constexpr uint8_t OR_L = 0xB5; // OR L
constexpr uint8_t OR_HL = 0xB6; // OR (HL)
constexpr uint8_t OR_A = 0xB7; // OR A
constexpr uint8_t OR_n = 0xF6; // OR n         - OR immediate with A

// XOR - Logical XOR
constexpr uint8_t XOR_B = 0xA8; // XOR B
constexpr uint8_t XOR_C = 0xA9; // XOR C
constexpr uint8_t XOR_D = 0xAA; // XOR D
constexpr uint8_t XOR_E = 0xAB; // XOR E
constexpr uint8_t XOR_H = 0xAC; // XOR H
constexpr uint8_t XOR_L = 0xAD; // XOR L
constexpr uint8_t XOR_HL = 0xAE; // XOR (HL)
constexpr uint8_t XOR_A = 0xAF; // XOR A        - Common idiom to clear A
constexpr uint8_t XOR_n = 0xEE; // XOR n        - XOR immediate with A

// CP - Compare
constexpr uint8_t CP_B = 0xB8; // CP B
constexpr uint8_t CP_C = 0xB9; // CP C
constexpr uint8_t CP_D = 0xBA; // CP D
constexpr uint8_t CP_E = 0xBB; // CP E
constexpr uint8_t CP_H = 0xBC; // CP H
constexpr uint8_t CP_L = 0xBD; // CP L
constexpr uint8_t CP_HL = 0xBE; // CP (HL)
constexpr uint8_t CP_A = 0xBF; // CP A
constexpr uint8_t CP_n = 0xFE; // CP n         - Compare immediate with A

// ============================================================================
// Rotate and Shift Instructions
// ============================================================================

constexpr uint8_t RLCA = 0x07; // RLCA         - Rotate Left Circular Accumulator
constexpr uint8_t RRCA = 0x0F; // RRCA         - Rotate Right Circular Accumulator
constexpr uint8_t RLA = 0x17;  // RLA          - Rotate Left through carry
constexpr uint8_t RRA = 0x1F;  // RRA          - Rotate Right through carry

// ============================================================================
// Control Flow Instructions
// ============================================================================

constexpr uint8_t NOP = 0x00; // NOP          - No Operation

// JP - Jump
constexpr uint8_t JP_nn = 0xC3;   // JP nn        - Jump to address
constexpr uint8_t JP_NZ_nn = 0xC2; // JP NZ, nn   - Jump if not zero
constexpr uint8_t JP_Z_nn = 0xCA;  // JP Z, nn    - Jump if zero
constexpr uint8_t JP_NC_nn = 0xD2; // JP NC, nn   - Jump if no carry
constexpr uint8_t JP_C_nn = 0xDA;  // JP C, nn    - Jump if carry
constexpr uint8_t JP_PO_nn = 0xE2; // JP PO, nn   - Jump if parity odd
constexpr uint8_t JP_PE_nn = 0xEA; // JP PE, nn   - Jump if parity even
constexpr uint8_t JP_P_nn = 0xF2;  // JP P, nn    - Jump if positive
constexpr uint8_t JP_M_nn = 0xFA;  // JP M, nn    - Jump if minus

constexpr uint8_t JP_HL = 0xE9;    // JP (HL)     - Jump to address in HL

// JR - Jump Relative
constexpr uint8_t JR_e = 0x18;    // JR e         - Jump relative
constexpr uint8_t JR_NZ_e = 0x20; // JR NZ, e     - Jump relative if not zero
constexpr uint8_t JR_Z_e = 0x28;  // JR Z, e      - Jump relative if zero
constexpr uint8_t JR_NC_e = 0x30; // JR NC, e     - Jump relative if no carry
constexpr uint8_t JR_C_e = 0x38;  // JR C, e      - Jump relative if carry

// CALL - Call Subroutine
constexpr uint8_t CALL_nn = 0xCD;   // CALL nn      - Call subroutine
constexpr uint8_t CALL_NZ_nn = 0xC4; // CALL NZ, nn - Call if not zero
constexpr uint8_t CALL_Z_nn = 0xCC;  // CALL Z, nn  - Call if zero
constexpr uint8_t CALL_NC_nn = 0xD4; // CALL NC, nn - Call if no carry
constexpr uint8_t CALL_C_nn = 0xDC;  // CALL C, nn  - Call if carry
constexpr uint8_t CALL_PO_nn = 0xE4; // CALL PO, nn - Call if parity odd
constexpr uint8_t CALL_PE_nn = 0xEC; // CALL PE, nn - Call if parity even
constexpr uint8_t CALL_P_nn = 0xF4;  // CALL P, nn  - Call if positive
constexpr uint8_t CALL_M_nn = 0xFC;  // CALL M, nn  - Call if minus

// RET - Return
constexpr uint8_t RET = 0xC9;    // RET          - Return from subroutine
constexpr uint8_t RET_NZ = 0xC0; // RET NZ       - Return if not zero
constexpr uint8_t RET_Z = 0xC8;  // RET Z        - Return if zero
constexpr uint8_t RET_NC = 0xD0; // RET NC       - Return if no carry
constexpr uint8_t RET_C = 0xD8;  // RET C        - Return if carry
constexpr uint8_t RET_PO = 0xE0; // RET PO       - Return if parity odd
constexpr uint8_t RET_PE = 0xE8; // RET PE       - Return if parity even
constexpr uint8_t RET_P = 0xF0;  // RET P        - Return if positive
constexpr uint8_t RET_M = 0xF8;  // RET M        - Return if minus

// RST - Restart
constexpr uint8_t RST_00 = 0xC7; // RST 00H
constexpr uint8_t RST_08 = 0xCF; // RST 08H
constexpr uint8_t RST_10 = 0xD7; // RST 10H
constexpr uint8_t RST_18 = 0xDF; // RST 18H
constexpr uint8_t RST_20 = 0xE7; // RST 20H
constexpr uint8_t RST_28 = 0xEF; // RST 28H
constexpr uint8_t RST_30 = 0xF7; // RST 30H
constexpr uint8_t RST_38 = 0xFF; // RST 38H

// ============================================================================
// Stack Instructions
// ============================================================================

constexpr uint8_t PUSH_BC = 0xC5; // PUSH BC
constexpr uint8_t PUSH_DE = 0xD5; // PUSH DE
constexpr uint8_t PUSH_HL = 0xE5; // PUSH HL
constexpr uint8_t PUSH_AF = 0xF5; // PUSH AF

constexpr uint8_t POP_BC = 0xC1; // POP BC
constexpr uint8_t POP_DE = 0xD1; // POP DE
constexpr uint8_t POP_HL = 0xE1; // POP HL
constexpr uint8_t POP_AF = 0xF1; // POP AF

// ============================================================================
// Miscellaneous Instructions
// ============================================================================

constexpr uint8_t DAA = 0x27;  // DAA          - Decimal Adjust Accumulator
constexpr uint8_t CPL = 0x2F;  // CPL          - Complement Accumulator
constexpr uint8_t SCF = 0x37;  // SCF          - Set Carry Flag
constexpr uint8_t CCF = 0x3F;  // CCF          - Complement Carry Flag
constexpr uint8_t DI = 0xF3;   // DI           - Disable Interrupts
constexpr uint8_t EI = 0xFB;   // EI           - Enable Interrupts

// ============================================================================
// Input/Output Instructions
// ============================================================================

// IN/OUT - Port I/O with A register
constexpr uint8_t IN_A_n = 0xDB;  // IN A, (n)    - Input from port n to A
constexpr uint8_t OUT_n_A = 0xD3; // OUT (n), A   - Output from A to port n

// ============================================================================
// CB-Prefixed Bit Operations
// ============================================================================

constexpr uint8_t CB_PREFIX = 0xCB; // CB prefix for bit operations

namespace CB {
// RLC - Rotate Left Circular
constexpr uint8_t RLC_B = 0x00; // RLC B
constexpr uint8_t RLC_C = 0x01; // RLC C
constexpr uint8_t RLC_D = 0x02; // RLC D
constexpr uint8_t RLC_E = 0x03; // RLC E
constexpr uint8_t RLC_H = 0x04; // RLC H
constexpr uint8_t RLC_L = 0x05; // RLC L
constexpr uint8_t RLC_HL = 0x06; // RLC (HL)
constexpr uint8_t RLC_A = 0x07; // RLC A

// RRC - Rotate Right Circular
constexpr uint8_t RRC_B = 0x08; // RRC B
constexpr uint8_t RRC_C = 0x09; // RRC C
constexpr uint8_t RRC_D = 0x0A; // RRC D
constexpr uint8_t RRC_E = 0x0B; // RRC E
constexpr uint8_t RRC_H = 0x0C; // RRC H
constexpr uint8_t RRC_L = 0x0D; // RRC L
constexpr uint8_t RRC_HL = 0x0E; // RRC (HL)
constexpr uint8_t RRC_A = 0x0F; // RRC A

// RL - Rotate Left through Carry
constexpr uint8_t RL_B = 0x10; // RL B
constexpr uint8_t RL_C = 0x11; // RL C
constexpr uint8_t RL_D = 0x12; // RL D
constexpr uint8_t RL_E = 0x13; // RL E
constexpr uint8_t RL_H = 0x14; // RL H
constexpr uint8_t RL_L = 0x15; // RL L
constexpr uint8_t RL_HL = 0x16; // RL (HL)
constexpr uint8_t RL_A = 0x17; // RL A

// RR - Rotate Right through Carry
constexpr uint8_t RR_B = 0x18; // RR B
constexpr uint8_t RR_C = 0x19; // RR C
constexpr uint8_t RR_D = 0x1A; // RR D
constexpr uint8_t RR_E = 0x1B; // RR E
constexpr uint8_t RR_H = 0x1C; // RR H
constexpr uint8_t RR_L = 0x1D; // RR L
constexpr uint8_t RR_HL = 0x1E; // RR (HL)
constexpr uint8_t RR_A = 0x1F; // RR A

// SLA - Shift Left Arithmetic
constexpr uint8_t SLA_B = 0x20; // SLA B
constexpr uint8_t SLA_C = 0x21; // SLA C
constexpr uint8_t SLA_D = 0x22; // SLA D
constexpr uint8_t SLA_E = 0x23; // SLA E
constexpr uint8_t SLA_H = 0x24; // SLA H
constexpr uint8_t SLA_L = 0x25; // SLA L
constexpr uint8_t SLA_HL = 0x26; // SLA (HL)
constexpr uint8_t SLA_A = 0x27; // SLA A

// SRA - Shift Right Arithmetic
constexpr uint8_t SRA_B = 0x28; // SRA B
constexpr uint8_t SRA_C = 0x29; // SRA C
constexpr uint8_t SRA_D = 0x2A; // SRA D
constexpr uint8_t SRA_E = 0x2B; // SRA E
constexpr uint8_t SRA_H = 0x2C; // SRA H
constexpr uint8_t SRA_L = 0x2D; // SRA L
constexpr uint8_t SRA_HL = 0x2E; // SRA (HL)
constexpr uint8_t SRA_A = 0x2F; // SRA A

// SRL - Shift Right Logical
constexpr uint8_t SRL_B = 0x38; // SRL B
constexpr uint8_t SRL_C = 0x39; // SRL C
constexpr uint8_t SRL_D = 0x3A; // SRL D
constexpr uint8_t SRL_E = 0x3B; // SRL E
constexpr uint8_t SRL_H = 0x3C; // SRL H
constexpr uint8_t SRL_L = 0x3D; // SRL L
constexpr uint8_t SRL_HL = 0x3E; // SRL (HL)
constexpr uint8_t SRL_A = 0x3F; // SRL A

// BIT - Test Bit
constexpr uint8_t BIT_0_B = 0x40; // BIT 0, B
constexpr uint8_t BIT_0_C = 0x41; // BIT 0, C
constexpr uint8_t BIT_0_D = 0x42; // BIT 0, D
constexpr uint8_t BIT_0_E = 0x43; // BIT 0, E
constexpr uint8_t BIT_0_H = 0x44; // BIT 0, H
constexpr uint8_t BIT_0_L = 0x45; // BIT 0, L
constexpr uint8_t BIT_0_HL = 0x46; // BIT 0, (HL)
constexpr uint8_t BIT_0_A = 0x47; // BIT 0, A

constexpr uint8_t BIT_1_B = 0x48; // BIT 1, B
constexpr uint8_t BIT_1_C = 0x49; // BIT 1, C
constexpr uint8_t BIT_1_D = 0x4A; // BIT 1, D
constexpr uint8_t BIT_1_E = 0x4B; // BIT 1, E
constexpr uint8_t BIT_1_H = 0x4C; // BIT 1, H
constexpr uint8_t BIT_1_L = 0x4D; // BIT 1, L
constexpr uint8_t BIT_1_HL = 0x4E; // BIT 1, (HL)
constexpr uint8_t BIT_1_A = 0x4F; // BIT 1, A

constexpr uint8_t BIT_2_B = 0x50; // BIT 2, B
constexpr uint8_t BIT_2_C = 0x51; // BIT 2, C
constexpr uint8_t BIT_2_D = 0x52; // BIT 2, D
constexpr uint8_t BIT_2_E = 0x53; // BIT 2, E
constexpr uint8_t BIT_2_H = 0x54; // BIT 2, H
constexpr uint8_t BIT_2_L = 0x55; // BIT 2, L
constexpr uint8_t BIT_2_HL = 0x56; // BIT 2, (HL)
constexpr uint8_t BIT_2_A = 0x57; // BIT 2, A

constexpr uint8_t BIT_7_A = 0x7F; // BIT 7, A

// RES - Reset Bit
constexpr uint8_t RES_0_B = 0x80; // RES 0, B
constexpr uint8_t RES_0_C = 0x81; // RES 0, C
constexpr uint8_t RES_0_D = 0x82; // RES 0, D
constexpr uint8_t RES_0_E = 0x83; // RES 0, E
constexpr uint8_t RES_0_H = 0x84; // RES 0, H
constexpr uint8_t RES_0_L = 0x85; // RES 0, L
constexpr uint8_t RES_0_HL = 0x86; // RES 0, (HL)
constexpr uint8_t RES_0_A = 0x87; // RES 0, A

constexpr uint8_t RES_3_B = 0x98; // RES 3, B
constexpr uint8_t RES_3_C = 0x99; // RES 3, C
constexpr uint8_t RES_3_D = 0x9A; // RES 3, D
constexpr uint8_t RES_3_E = 0x9B; // RES 3, E
constexpr uint8_t RES_3_H = 0x9C; // RES 3, H
constexpr uint8_t RES_3_L = 0x9D; // RES 3, L
constexpr uint8_t RES_3_HL = 0x9E; // RES 3, (HL)
constexpr uint8_t RES_3_A = 0x9F; // RES 3, A

// SET - Set Bit
constexpr uint8_t SET_0_B = 0xC0; // SET 0, B
constexpr uint8_t SET_0_C = 0xC1; // SET 0, C
constexpr uint8_t SET_0_D = 0xC2; // SET 0, D
constexpr uint8_t SET_0_E = 0xC3; // SET 0, E
constexpr uint8_t SET_0_H = 0xC4; // SET 0, H
constexpr uint8_t SET_0_L = 0xC5; // SET 0, L
constexpr uint8_t SET_0_HL = 0xC6; // SET 0, (HL)
constexpr uint8_t SET_0_A = 0xC7; // SET 0, A

constexpr uint8_t SET_7_B = 0xF8; // SET 7, B
constexpr uint8_t SET_7_C = 0xF9; // SET 7, C
constexpr uint8_t SET_7_D = 0xFA; // SET 7, D
constexpr uint8_t SET_7_E = 0xFB; // SET 7, E
constexpr uint8_t SET_7_H = 0xFC; // SET 7, H
constexpr uint8_t SET_7_L = 0xFD; // SET 7, L
constexpr uint8_t SET_7_HL = 0xFE; // SET 7, (HL)
constexpr uint8_t SET_7_A = 0xFF; // SET 7, A

} // namespace CB

// ============================================================================
// ED-Prefixed Extended Instructions
// ============================================================================

constexpr uint8_t ED_PREFIX = 0xED; // ED prefix for extended instructions

namespace ED {
// LD I/R operations
constexpr uint8_t LD_I_A = 0x47; // LD I, A      - Load Interrupt vector
constexpr uint8_t LD_R_A = 0x4F; // LD R, A      - Load Refresh register
constexpr uint8_t LD_A_I = 0x57; // LD A, I      - Load A from Interrupt vector
constexpr uint8_t LD_A_R = 0x5F; // LD A, R      - Load A from Refresh register

// Block Transfer operations
constexpr uint8_t LDI = 0xA0;   // LDI          - Load and Increment
constexpr uint8_t LDIR = 0xB0;  // LDIR         - Load, Increment, Repeat
constexpr uint8_t LDD = 0xA8;   // LDD          - Load and Decrement
constexpr uint8_t LDDR = 0xB8;  // LDDR         - Load, Decrement, Repeat

// Block Compare operations
constexpr uint8_t CPI = 0xA1;   // CPI          - Compare and Increment
constexpr uint8_t CPIR = 0xB1;  // CPIR         - Compare, Increment, Repeat
constexpr uint8_t CPD = 0xA9;   // CPD          - Compare and Decrement
constexpr uint8_t CPDR = 0xB9;  // CPDR         - Compare, Decrement, Repeat

// Digit rotate operations
constexpr uint8_t RLD = 0x6F;   // RLD          - Rotate Left Digit (A and (HL))
constexpr uint8_t RRD = 0x67;   // RRD          - Rotate Right Digit (A and (HL))

// IN/OUT operations (port C)
constexpr uint8_t IN_B_C = 0x40;  // IN B, (C)    - Input from port C to B
constexpr uint8_t IN_C_C = 0x48;  // IN C, (C)    - Input from port C to C
constexpr uint8_t IN_D_C = 0x50;  // IN D, (C)    - Input from port C to D
constexpr uint8_t IN_E_C = 0x58;  // IN E, (C)    - Input from port C to E
constexpr uint8_t IN_H_C = 0x60;  // IN H, (C)    - Input from port C to H
constexpr uint8_t IN_L_C = 0x68;  // IN L, (C)    - Input from port C to L
constexpr uint8_t IN_A_C = 0x78;  // IN A, (C)    - Input from port C to A

constexpr uint8_t OUT_C_B = 0x41; // OUT (C), B   - Output from B to port C
constexpr uint8_t OUT_C_C = 0x49; // OUT (C), C   - Output from C to port C
constexpr uint8_t OUT_C_D = 0x51; // OUT (C), D   - Output from D to port C
constexpr uint8_t OUT_C_E = 0x59; // OUT (C), E   - Output from E to port C
constexpr uint8_t OUT_C_H = 0x61; // OUT (C), H   - Output from H to port C
constexpr uint8_t OUT_C_L = 0x69; // OUT (C), L   - Output from L to port C
constexpr uint8_t OUT_C_A = 0x79; // OUT (C), A   - Output from A to port C

// Negate and Interrupts
constexpr uint8_t NEG = 0x44;   // NEG          - Negate accumulator (2's complement)
constexpr uint8_t RETN = 0x45;  // RETN         - Return from NMI
constexpr uint8_t RETI = 0x4D;  // RETI         - Return from interrupt

// Interrupt Mode
constexpr uint8_t IM_0 = 0x46;  // IM 0         - Interrupt Mode 0
constexpr uint8_t IM_1 = 0x56;  // IM 1         - Interrupt Mode 1
constexpr uint8_t IM_2 = 0x5E;  // IM 2         - Interrupt Mode 2

// 16-bit arithmetic
constexpr uint8_t ADC_HL_BC = 0x4A; // ADC HL, BC   - Add with carry BC to HL
constexpr uint8_t ADC_HL_DE = 0x5A; // ADC HL, DE   - Add with carry DE to HL
constexpr uint8_t ADC_HL_HL = 0x6A; // ADC HL, HL   - Add with carry HL to HL
constexpr uint8_t ADC_HL_SP = 0x7A; // ADC HL, SP   - Add with carry SP to HL

constexpr uint8_t SBC_HL_BC = 0x42; // SBC HL, BC   - Subtract with carry BC from HL
constexpr uint8_t SBC_HL_DE = 0x52; // SBC HL, DE   - Subtract with carry DE from HL
constexpr uint8_t SBC_HL_HL = 0x62; // SBC HL, HL   - Subtract with carry HL from HL
constexpr uint8_t SBC_HL_SP = 0x72; // SBC HL, SP   - Subtract with carry SP from HL

} // namespace ED

// ============================================================================
// DD-Prefixed IX Register Instructions
// ============================================================================

constexpr uint8_t DD_PREFIX = 0xDD; // DD prefix for IX register operations

namespace DD {
constexpr uint8_t LD_IX_nn = 0x21;   // LD IX, nn    - Load IX with immediate
constexpr uint8_t LD_A_IX_d = 0x7E;  // LD A, (IX+d) - Load A from IX+displacement
constexpr uint8_t ADD_IX_BC = 0x09;  // ADD IX, BC   - Add BC to IX
constexpr uint8_t ADD_IX_DE = 0x19;  // ADD IX, DE   - Add DE to IX
constexpr uint8_t ADD_IX_IX = 0x29;  // ADD IX, IX   - Add IX to IX (double IX)
constexpr uint8_t ADD_IX_SP = 0x39;  // ADD IX, SP   - Add SP to IX
constexpr uint8_t INC_IX = 0x23;     // INC IX       - Increment IX
constexpr uint8_t DEC_IX = 0x2B;     // DEC IX       - Decrement IX
} // namespace DD

// ============================================================================
// FD-Prefixed IY Register Instructions
// ============================================================================

constexpr uint8_t FD_PREFIX = 0xFD; // FD prefix for IY register operations

namespace FD {
constexpr uint8_t LD_IY_nn = 0x21;   // LD IY, nn    - Load IY with immediate
constexpr uint8_t LD_A_IY_d = 0x7E;  // LD A, (IY+d) - Load A from IY+displacement
constexpr uint8_t ADD_IY_BC = 0x09;  // ADD IY, BC   - Add BC to IY
constexpr uint8_t ADD_IY_DE = 0x19;  // ADD IY, DE   - Add DE to IY
constexpr uint8_t ADD_IY_IY = 0x29;  // ADD IY, IY   - Add IY to IY (double IY)
constexpr uint8_t ADD_IY_SP = 0x39;  // ADD IY, SP   - Add SP to IY
constexpr uint8_t INC_IY = 0x23;     // INC IY       - Increment IY
constexpr uint8_t DEC_IY = 0x2B;     // DEC IY       - Decrement IY
} // namespace FD

} // namespace Z80Opcodes
} // namespace xasm
