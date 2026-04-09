/**
 * @file mnemonics_6809.h
 * @brief Mnemonic enum and parse-time mapping for Motorola 6809 instructions
 *
 * Replaces the prior `constexpr const char*` namespace with an `enum class`
 * so that all mnemonic dispatch uses a single `unordered_map` lookup at
 * parse time and an O(1), compiler-checked `switch` at encode time.
 *
 * @section usage Usage Example
 * @code
 * // Parse once at instruction-decode time:
 * M6809Mnemonic mn = ParseM6809Mnemonic(str);
 *
 * // Switch in encoder (compiler warns on unhandled cases):
 * switch (mn) {
 *   case M6809Mnemonic::LDA: ...
 *   ...
 * }
 * @endcode
 */

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace xasm {

// ============================================================================
// Mnemonic enum — one enumerator per M6809 instruction
// ============================================================================

enum class M6809Mnemonic {
  Unknown = 0,

  // Load / Store — 8-bit accumulators
  LDA, STA,
  LDB, STB,

  // Load / Store — 16-bit registers
  LDD, STD,
  LDX, STX,
  LDY, STY,
  LDU, STU,
  LDS, STS,

  // Arithmetic — Accumulator A
  ADDA, ADCA, SUBA, SBCA, CMPA,
  // Arithmetic — Accumulator B
  ADDB, ADCB, SUBB, SBCB, CMPB,
  // Arithmetic — 16-bit
  ADDD, SUBD, CMPX, CMPY, CMPU, CMPS, CMPD,

  // Logical — Accumulator A
  ANDA, ORA, EORA, BITA,
  // Logical — Accumulator B
  ANDB, ORB, EORB, BITB,

  // Register-flag operations
  ANDCC, ORCC,

  // Inherent — Accumulator A only
  CLRA, ASLA, ASRA, LSRA, ROLA, RORA, INCA, DECA, TSTA, NEGA, COMA, DAA,
  // Inherent — Accumulator B only
  CLRB, ASLB, ASRB, LSRB, ROLB, RORB, INCB, DECB, TSTB, NEGB, COMB,

  // Memory-addressed single-operand
  CLR, ASL, ASR, LSR, ROL, ROR, INC, DEC, TST, NEG,

  // Transfer / Exchange / Stack
  EXG, TFR,
  PSHS, PULS,
  PSHU, PULU,

  // Inherent (no operand)
  NOP, RTS, RTI, MUL, SEX, SWI, SWI2, SWI3, SYNC, CWAI, ABX,

  // Jump / Call
  JMP, JSR,

  // Load Effective Address
  LEAX, LEAY, LEAS, LEAU,

  // Short Branch (8-bit relative)
  BRA, BRN, BSR,
  BEQ, BNE,
  BCS, BCC, BLO, BHS,
  BMI, BPL, BVS, BVC,
  BHI, BLS, BGE, BGT, BLE, BLT,

  // Long Branch (16-bit relative)
  LBRA, LBRN, LBSR,
  LBEQ, LBNE,
  LBCS, LBCC, LBLO, LBHS,
  LBMI, LBPL, LBVS, LBVC,
  LBHI, LBLS, LBGE, LBGT, LBLE, LBLT,
};

// ============================================================================
// Parse-time string -> M6809Mnemonic mapping
// ============================================================================

/**
 * @brief Convert an uppercase mnemonic string to M6809Mnemonic.
 *
 * Returns M6809Mnemonic::Unknown for any unrecognised string.
 * The map is built once (function-local static).
 */
inline M6809Mnemonic ParseM6809Mnemonic(std::string_view s) {
  // clang-format off
  static const std::unordered_map<std::string, M6809Mnemonic> kMap = {
    // Load / Store
    { "LDA",  M6809Mnemonic::LDA  }, { "STA",  M6809Mnemonic::STA  },
    { "LDB",  M6809Mnemonic::LDB  }, { "STB",  M6809Mnemonic::STB  },
    { "LDD",  M6809Mnemonic::LDD  }, { "STD",  M6809Mnemonic::STD  },
    { "LDX",  M6809Mnemonic::LDX  }, { "STX",  M6809Mnemonic::STX  },
    { "LDY",  M6809Mnemonic::LDY  }, { "STY",  M6809Mnemonic::STY  },
    { "LDU",  M6809Mnemonic::LDU  }, { "STU",  M6809Mnemonic::STU  },
    { "LDS",  M6809Mnemonic::LDS  }, { "STS",  M6809Mnemonic::STS  },

    // Arithmetic
    { "ADDA", M6809Mnemonic::ADDA }, { "ADCA", M6809Mnemonic::ADCA },
    { "SUBA", M6809Mnemonic::SUBA }, { "SBCA", M6809Mnemonic::SBCA },
    { "CMPA", M6809Mnemonic::CMPA },
    { "ADDB", M6809Mnemonic::ADDB }, { "ADCB", M6809Mnemonic::ADCB },
    { "SUBB", M6809Mnemonic::SUBB }, { "SBCB", M6809Mnemonic::SBCB },
    { "CMPB", M6809Mnemonic::CMPB },
    { "ADDD", M6809Mnemonic::ADDD }, { "SUBD", M6809Mnemonic::SUBD },
    { "CMPX", M6809Mnemonic::CMPX }, { "CMPY", M6809Mnemonic::CMPY },
    { "CMPU", M6809Mnemonic::CMPU }, { "CMPS", M6809Mnemonic::CMPS },
    { "CMPD", M6809Mnemonic::CMPD },

    // Logical
    { "ANDA", M6809Mnemonic::ANDA }, { "ORA",  M6809Mnemonic::ORA  },
    { "EORA", M6809Mnemonic::EORA }, { "BITA", M6809Mnemonic::BITA },
    { "ANDB", M6809Mnemonic::ANDB }, { "ORB",  M6809Mnemonic::ORB  },
    { "EORB", M6809Mnemonic::EORB }, { "BITB", M6809Mnemonic::BITB },
    { "ANDCC",M6809Mnemonic::ANDCC}, { "ORCC", M6809Mnemonic::ORCC },

    // Inherent accumulator A
    { "CLRA", M6809Mnemonic::CLRA }, { "ASLA", M6809Mnemonic::ASLA },
    { "ASRA", M6809Mnemonic::ASRA }, { "LSRA", M6809Mnemonic::LSRA },
    { "ROLA", M6809Mnemonic::ROLA }, { "RORA", M6809Mnemonic::RORA },
    { "INCA", M6809Mnemonic::INCA }, { "DECA", M6809Mnemonic::DECA },
    { "TSTA", M6809Mnemonic::TSTA }, { "NEGA", M6809Mnemonic::NEGA },
    { "COMA", M6809Mnemonic::COMA }, { "DAA",  M6809Mnemonic::DAA  },

    // Inherent accumulator B
    { "CLRB", M6809Mnemonic::CLRB }, { "ASLB", M6809Mnemonic::ASLB },
    { "ASRB", M6809Mnemonic::ASRB }, { "LSRB", M6809Mnemonic::LSRB },
    { "ROLB", M6809Mnemonic::ROLB }, { "RORB", M6809Mnemonic::RORB },
    { "INCB", M6809Mnemonic::INCB }, { "DECB", M6809Mnemonic::DECB },
    { "TSTB", M6809Mnemonic::TSTB }, { "NEGB", M6809Mnemonic::NEGB },
    { "COMB", M6809Mnemonic::COMB },

    // Memory-addressed single-operand
    { "CLR",  M6809Mnemonic::CLR  }, { "ASL",  M6809Mnemonic::ASL  },
    { "ASR",  M6809Mnemonic::ASR  }, { "LSR",  M6809Mnemonic::LSR  },
    { "ROL",  M6809Mnemonic::ROL  }, { "ROR",  M6809Mnemonic::ROR  },
    { "INC",  M6809Mnemonic::INC  }, { "DEC",  M6809Mnemonic::DEC  },
    { "TST",  M6809Mnemonic::TST  }, { "NEG",  M6809Mnemonic::NEG  },

    // Transfer / Exchange / Stack
    { "EXG",  M6809Mnemonic::EXG  }, { "TFR",  M6809Mnemonic::TFR  },
    { "PSHS", M6809Mnemonic::PSHS }, { "PULS", M6809Mnemonic::PULS },
    { "PSHU", M6809Mnemonic::PSHU }, { "PULU", M6809Mnemonic::PULU },

    // Inherent (no operand)
    { "NOP",  M6809Mnemonic::NOP  }, { "RTS",  M6809Mnemonic::RTS  },
    { "RTI",  M6809Mnemonic::RTI  }, { "MUL",  M6809Mnemonic::MUL  },
    { "SEX",  M6809Mnemonic::SEX  }, { "SWI",  M6809Mnemonic::SWI  },
    { "SWI2", M6809Mnemonic::SWI2 }, { "SWI3", M6809Mnemonic::SWI3 },
    { "SYNC", M6809Mnemonic::SYNC }, { "CWAI", M6809Mnemonic::CWAI },
    { "ABX",  M6809Mnemonic::ABX  },

    // Jump / Call
    { "JMP",  M6809Mnemonic::JMP  }, { "JSR",  M6809Mnemonic::JSR  },

    // Load Effective Address
    { "LEAX", M6809Mnemonic::LEAX }, { "LEAY", M6809Mnemonic::LEAY },
    { "LEAS", M6809Mnemonic::LEAS }, { "LEAU", M6809Mnemonic::LEAU },

    // Short Branch
    { "BRA",  M6809Mnemonic::BRA  }, { "BRN",  M6809Mnemonic::BRN  },
    { "BSR",  M6809Mnemonic::BSR  },
    { "BEQ",  M6809Mnemonic::BEQ  }, { "BNE",  M6809Mnemonic::BNE  },
    { "BCS",  M6809Mnemonic::BCS  }, { "BCC",  M6809Mnemonic::BCC  },
    { "BLO",  M6809Mnemonic::BLO  }, { "BHS",  M6809Mnemonic::BHS  },
    { "BMI",  M6809Mnemonic::BMI  }, { "BPL",  M6809Mnemonic::BPL  },
    { "BVS",  M6809Mnemonic::BVS  }, { "BVC",  M6809Mnemonic::BVC  },
    { "BHI",  M6809Mnemonic::BHI  }, { "BLS",  M6809Mnemonic::BLS  },
    { "BGE",  M6809Mnemonic::BGE  }, { "BGT",  M6809Mnemonic::BGT  },
    { "BLE",  M6809Mnemonic::BLE  }, { "BLT",  M6809Mnemonic::BLT  },

    // Long Branch
    { "LBRA", M6809Mnemonic::LBRA }, { "LBRN", M6809Mnemonic::LBRN },
    { "LBSR", M6809Mnemonic::LBSR },
    { "LBEQ", M6809Mnemonic::LBEQ }, { "LBNE", M6809Mnemonic::LBNE },
    { "LBCS", M6809Mnemonic::LBCS }, { "LBCC", M6809Mnemonic::LBCC },
    { "LBLO", M6809Mnemonic::LBLO }, { "LBHS", M6809Mnemonic::LBHS },
    { "LBMI", M6809Mnemonic::LBMI }, { "LBPL", M6809Mnemonic::LBPL },
    { "LBVS", M6809Mnemonic::LBVS }, { "LBVC", M6809Mnemonic::LBVC },
    { "LBHI", M6809Mnemonic::LBHI }, { "LBLS", M6809Mnemonic::LBLS },
    { "LBGE", M6809Mnemonic::LBGE }, { "LBGT", M6809Mnemonic::LBGT },
    { "LBLE", M6809Mnemonic::LBLE }, { "LBLT", M6809Mnemonic::LBLT },
  };
  // clang-format on

  auto it = kMap.find(std::string(s));
  return it != kMap.end() ? it->second : M6809Mnemonic::Unknown;
}

} // namespace xasm
