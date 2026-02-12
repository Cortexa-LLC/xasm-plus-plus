/**
 * @file merlin_directives.cpp
 * @brief Implementation of Merlin-specific directive handlers
 *
 * Extracted from MerlinSyntaxParser as part of God-Class Phase 6c.1 refactoring.
 */

#include "xasm++/directives/merlin_directives.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/syntax/directive_registry.h"
#include "xasm++/atom.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/expression.h"
#include "xasm++/syntax/merlin_syntax.h"
#include "xasm++/util/string_utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace xasm {

using xasm::util::ToUpper;
using xasm::util::Trim;

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Format error message with source location
 *
 * Internal helper for creating error messages. If parser context is available,
 * includes file and line information.
 *
 * @param message Error message
 * @param file Source file name (empty if not available)
 * @param line Source line number (0 if not available)
 * @return Formatted error message
 */
static std::string FormatError(const std::string &message,
                               const std::string &file = "",
                               int line = 0) {
  if (!file.empty() && line > 0) {
    std::ostringstream oss;
    oss << file << ":" << line << ": error: " << message;
    return oss.str();
  }
  return message;
}

// ============================================================================
// Hex Data Directive
// ============================================================================

void HandleHexDirective(const std::string &operand, Section &section,
                        uint32_t &current_address) {
  std::vector<uint8_t> bytes;
  std::string hex_str = Trim(operand);

  // Check if operand contains commas (comma-separated format)
  if (hex_str.find(',') != std::string::npos) {
    // Comma-separated format: "01,02,03" or "01, 02, 03"
    std::istringstream iss(hex_str);
    std::string token;

    while (std::getline(iss, token, ',')) {
      token = Trim(token); // Remove whitespace around token
      if (!token.empty()) {
        // Validate hex digits before calling stoul
        for (char c : token) {
          if (!std::isxdigit(static_cast<unsigned char>(c))) {
            throw std::runtime_error(
                FormatError("Invalid hex digit '" + std::string(1, c) +
                            "' in HEX directive: '" + token + "'"));
          }
        }
        try {
          bytes.push_back(static_cast<uint8_t>(std::stoul(token, nullptr, 16)));
        } catch (const std::exception &e) {
          throw std::runtime_error(
              FormatError("Invalid hex value in HEX directive: '" + token +
                          "' - " + e.what()));
        }
      }
    }
  } else {
    // Concatenated format: "010203" or "AB CD EF"
    // Remove all spaces from hex string
    hex_str.erase(std::remove_if(hex_str.begin(), hex_str.end(), ::isspace),
                  hex_str.end());

    // Validate all characters are hex digits
    for (char c : hex_str) {
      if (!std::isxdigit(static_cast<unsigned char>(c))) {
        throw std::runtime_error(
            FormatError("Invalid hex digit '" + std::string(1, c) +
                        "' in HEX directive: '" + operand + "'"));
      }
    }

    // Parse pairs of hex digits
    for (size_t i = 0; i + 1 < hex_str.length(); i += 2) {
      std::string byte_str = hex_str.substr(i, 2);
      try {
        bytes.push_back(
            static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16)));
      } catch (const std::exception &e) {
        throw std::runtime_error(
            FormatError("Invalid hex value in HEX directive: '" + byte_str +
                        "' - " + e.what()));
      }
    }
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address += bytes.size();
}

// ============================================================================
// String Directives
// ============================================================================

void HandleAscDirective(const std::string &operand, Section &section,
                        uint32_t &current_address) {
  // ASC 'string' or ASC "string" - ASCII string directive
  // Apple II/Merlin standard: Sets high bit on ALL characters (0x80 | char)

  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    // No quote found - empty string
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find closing quote
  size_t end_pos = op.find(quote, start_pos);
  if (end_pos == std::string::npos) {
    // No closing quote - treat rest as string
    end_pos = op.length();
  }

  // Extract string content
  std::string text = op.substr(start_pos, end_pos - start_pos);

  // Convert string to bytes with high bit set (Apple II standard)
  for (size_t i = 0; i < text.length(); ++i) {
    uint8_t byte = static_cast<uint8_t>(text[i]);

    // Set high bit on ALL characters (Apple II/Merlin compatibility)
    byte |= 0x80;

    bytes.push_back(byte);
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address += bytes.size();
}

void HandleDciDirective(const std::string &operand, Section &section,
                        uint32_t &current_address) {
  // DCI 'string' - DCI string (last character with high bit set)

  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    // No quote found - empty string
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find closing quote
  size_t end_pos = op.find(quote, start_pos);
  if (end_pos == std::string::npos) {
    // No closing quote - treat rest as string
    end_pos = op.length();
  }

  // Extract string content
  std::string text = op.substr(start_pos, end_pos - start_pos);

  // Convert string to bytes, setting high bit on last character
  for (size_t i = 0; i < text.length(); ++i) {
    uint8_t byte = static_cast<uint8_t>(text[i]);

    // Set high bit on last character
    if (i == text.length() - 1) {
      byte |= 0x80;
    }

    bytes.push_back(byte);
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address += bytes.size();
}

void HandleInvDirective(const std::string &operand, Section &section,
                        uint32_t &current_address) {
  // INV 'string' - Inverse ASCII (all characters with high bit set)

  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    // No quote found - empty string
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find closing quote
  size_t end_pos = op.find(quote, start_pos);
  if (end_pos == std::string::npos) {
    // No closing quote - treat rest as string
    end_pos = op.length();
  }

  // Extract string content
  std::string text = op.substr(start_pos, end_pos - start_pos);

  // Convert string to bytes, setting high bit on all characters
  for (size_t i = 0; i < text.length(); ++i) {
    uint8_t byte = static_cast<uint8_t>(text[i]) | 0x80;
    bytes.push_back(byte);
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address += bytes.size();
}

void HandleFlsDirective(const std::string &operand, Section &section,
                        uint32_t &current_address) {
  // FLS 'string' - Flash ASCII (alternating high bit for flashing effect)

  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    // No quote found - empty string
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find closing quote
  size_t end_pos = op.find(quote, start_pos);
  if (end_pos == std::string::npos) {
    // No closing quote - treat rest as string
    end_pos = op.length();
  }

  // Extract string content
  std::string text = op.substr(start_pos, end_pos - start_pos);

  // Convert string to bytes, alternating high bit on every other character
  for (size_t i = 0; i < text.length(); ++i) {
    uint8_t byte = static_cast<uint8_t>(text[i]);

    // Set high bit on ODD-indexed characters (1, 3, 5...)
    if (i % 2 == 1) {
      byte |= 0x80;
    }

    bytes.push_back(byte);
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address += bytes.size();
}

// ============================================================================
// Data Definition Directives
// ============================================================================

void HandleDaDirective(const std::string &operand, Section &section,
                       ConcreteSymbolTable &symbols, uint32_t &current_address) {
  // DA (Define Address) - same as DW, word definitions in little-endian
  // Delegate to core DW handler
  extern void HandleDwDirective(const std::string &operand, Section &section,
                                ConcreteSymbolTable &symbols,
                                uint32_t &current_address);
  HandleDwDirective(operand, section, symbols, current_address);
}

// ============================================================================
// Control Directives
// ============================================================================

void HandleEndDirective(bool &end_directive_seen) {
  // END - mark end of source (stop processing further lines)
  end_directive_seen = true;
}

void HandleSavDirective(const std::string & /* operand */) {
  // SAV filename - Save output filename directive
  // This is a no-op for now - output filename is controlled by command-line args
  // No state changed, no atoms created
}

void HandleXcDirective(const std::string &operand, Cpu6502 *cpu) {
  // XC [ON|OFF] - Toggle 65C02 CPU instruction set

  if (!cpu) {
    // No CPU set - silently ignore (for tests that don't need CPU)
    return;
  }

  std::string op = ToUpper(Trim(operand));

  if (op.empty() || op == "ON") {
    // Enable 65C02 mode
    cpu->SetCpuMode(CpuMode::Cpu65C02);
  } else if (op == "OFF") {
    // Disable 65C02 mode (back to 6502)
    cpu->SetCpuMode(CpuMode::Cpu6502);
  } else {
    throw std::runtime_error(
        FormatError("XC: invalid operand (expected ON or OFF)"));
  }
}

void HandleMxDirective(const std::string &operand) {
  // MX mode - Set 65816 accumulator and index register widths
  // This is a directive only - tracks state but doesn't change CPU encoding

  std::string op = Trim(operand);
  if (op.empty()) {
    throw std::runtime_error(FormatError("MX directive requires an operand"));
  }

  int mode = -1;

  // Check for binary format %00-%11
  if (op[0] == '%') {
    std::string binary = op.substr(1);
    if (binary == "00")
      mode = 0;
    else if (binary == "01")
      mode = 1;
    else if (binary == "10")
      mode = 2;
    else if (binary == "11")
      mode = 3;
    else {
      throw std::runtime_error(
          FormatError("MX directive expects binary %00-%11 or decimal 0-3"));
    }
  }
  // Check for decimal format 0-3
  else if (op.length() == 1 && op[0] >= '0' && op[0] <= '3') {
    mode = op[0] - '0';
  } else {
    throw std::runtime_error(
        FormatError("MX directive expects binary %00-%11 or decimal 0-3"));
  }

  // Mode validated - in full implementation, would affect 65816 encoding
  // For now, just validate and accept
  (void)mode; // Suppress unused variable warning
}

void HandleRevDirective(const std::string &label, const std::string &operand,
                        Section &section, ConcreteSymbolTable &symbols,
                        uint32_t &current_address) {
  // REV "string" - Reverse ASCII string

  std::string op = Trim(operand);

  if (op.empty()) {
    throw std::runtime_error(FormatError("REV requires a string operand"));
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    throw std::runtime_error(FormatError("REV requires quoted string"));
  }

  // Find closing quote
  size_t end_pos = op.find(quote, start_pos);
  if (end_pos == std::string::npos) {
    // No closing quote - treat rest as string
    end_pos = op.length();
  }

  // Extract string content
  std::string text = op.substr(start_pos, end_pos - start_pos);

  if (text.empty()) {
    throw std::runtime_error(FormatError("REV requires non-empty string"));
  }

  // Create label at current address (before emitting bytes)
  if (!label.empty()) {
    symbols.Define(label, SymbolType::Label,
                   std::make_shared<LiteralExpr>(current_address));
    section.atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
  }

  // Reverse the string
  std::string reversed(text.rbegin(), text.rend());

  // Emit reversed bytes as data
  std::vector<uint8_t> bytes;
  for (char ch : reversed) {
    bytes.push_back(static_cast<uint8_t>(ch));
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address += bytes.size();
}

// ============================================================================
// Block Control Directives
// ============================================================================

void HandleDumDirective(const std::string &operand, ConcreteSymbolTable &symbols,
                        bool &in_dum_block, uint32_t &dum_address) {
  // DUM (Dummy section) - start variable definition block
  in_dum_block = true;

  std::string op = Trim(operand);

  // Check if operand is empty
  if (op.empty()) {
    throw std::runtime_error(
        FormatError("DUM directive requires an address operand"));
  }

  // Parse number (decimal, hex, or binary)
  // For simplicity, delegate to a parser's ParseNumber if needed
  // For now, handle basic cases
  if (op[0] == '$') {
    // Hex
    dum_address = std::stoul(op.substr(1), nullptr, 16);
  } else if (op[0] == '%') {
    // Binary
    dum_address = std::stoul(op.substr(1), nullptr, 2);
  } else if (std::isdigit(op[0])) {
    // Decimal
    dum_address = std::stoul(op, nullptr, 10);
  } else {
    // Symbol - look it up
    int64_t value = 0;
    if (symbols.Lookup(op, value)) {
      dum_address = static_cast<uint32_t>(value);
    } else {
      // Symbol not found - use 0 for now (forward reference issue)
      dum_address = 0;
    }
  }
}

void HandleDendDirective(bool &in_dum_block) {
  // DEND - end dummy section
  in_dum_block = false;
}

// ============================================================================
// Listing Control Directives (No-ops)
// ============================================================================

void HandleLstDirective(const std::string & /* operand */) {
  // LST/LST OFF - listing control
  // No-op for compatibility
}

void HandleLstdoDirective() {
  // LSTDO - list during DO blocks
  // No-op for compatibility
}

void HandleTrDirective(const std::string & /* operand */) {
  // TR [ADR|ON|OFF] - truncate listing
  // No-op for compatibility
}

void HandleUsrDirective() {
  // USR - user-defined subroutine
  // No-op - no atoms generated
}

// ============================================================================
// Loop Directive (Not Implemented)
// ============================================================================

void HandleLupDirective(const std::string & /* operand */) {
  // LUP count - Loop directive (repeat following code count times)
  throw std::runtime_error(
      FormatError("LUP directive not yet implemented (deferred)"));
}

// ============================================================================
// DirectiveRegistry Integration
// ============================================================================

void RegisterMerlinDirectiveHandlers(DirectiveRegistry &registry,
                                     MerlinSyntaxParser *parser) {
  // HEX - Define hex bytes
  registry.Register("HEX",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label; // HEX doesn't use label
      HandleHexDirective(operand, *ctx.section, *ctx.current_address);
    });

  // ASC - ASCII string with high bit set
  registry.Register("ASC",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label; // ASC doesn't use label (could be added later)
      HandleAscDirective(operand, *ctx.section, *ctx.current_address);
    });

  // DCI - DCI string (last char with high bit set)
  registry.Register("DCI",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      HandleDciDirective(operand, *ctx.section, *ctx.current_address);
    });

  // INV - Inverse ASCII (all chars with high bit set)
  registry.Register("INV",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      HandleInvDirective(operand, *ctx.section, *ctx.current_address);
    });

  // FLS - Flash ASCII (alternating high bit)
  registry.Register("FLS",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      HandleFlsDirective(operand, *ctx.section, *ctx.current_address);
    });

  // DA - Define address (word)
  registry.Register("DA",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      HandleDaDirective(operand, *ctx.section, *ctx.symbols, *ctx.current_address);
    });

  // END - End of source
  registry.Register("END",
    [parser](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      (void)operand;
      (void)ctx;
      if (parser) {
        HandleEndDirective(parser->end_directive_seen_);
      }
    });

  // SAV - Save output filename (no-op)
  registry.Register("SAV",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      (void)ctx;
      HandleSavDirective(operand);
    });

  // XC - Toggle 65C02 mode
  registry.Register("XC",
    [parser](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      (void)ctx;
      if (parser) {
        HandleXcDirective(operand, parser->cpu_);
      }
    });

  // MX - Set 65816 register widths
  registry.Register("MX",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      (void)ctx;
      HandleMxDirective(operand);
    });

  // REV - Reverse ASCII string
  registry.Register("REV",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      if (label.empty()) {
        throw std::runtime_error(FormatError("REV requires a label", "", 0));
      }
      HandleRevDirective(label, operand, *ctx.section, *ctx.symbols, *ctx.current_address);
    });

  // DUM - Start dummy section
  registry.Register("DUM",
    [parser](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      if (parser) {
        HandleDumDirective(operand, *ctx.symbols, parser->in_dum_block_,
                          parser->dum_address_);
      }
    });

  // DEND - End dummy section
  registry.Register("DEND",
    [parser](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      (void)operand;
      (void)ctx;
      if (parser) {
        HandleDendDirective(parser->in_dum_block_);
      }
    });

  // LST - Listing control (no-op)
  registry.Register("LST",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      (void)ctx;
      HandleLstDirective(operand);
    });

  // LSTDO - List during DO blocks (no-op)
  registry.Register("LSTDO",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      (void)operand;
      (void)ctx;
      HandleLstdoDirective();
    });

  // TR - Truncate listing (no-op)
  registry.Register("TR",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      (void)ctx;
      HandleTrDirective(operand);
    });

  // USR - User-defined subroutine (no-op)
  registry.Register("USR",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      (void)operand;
      (void)ctx;
      HandleUsrDirective();
    });

  // LUP - Loop assembly (not implemented)
  registry.Register("LUP",
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label;
      (void)ctx;
      HandleLupDirective(operand);
    });
}

} // namespace xasm
