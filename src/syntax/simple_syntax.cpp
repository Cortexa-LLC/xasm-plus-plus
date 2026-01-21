// SimpleSyntax - Simple assembly syntax parser implementation
// Phase 1: Minimal Viable Assembler - SimpleSyntax Plugin

#include "xasm++/syntax/simple_syntax.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace xasm {

// Helper: Parse hex value ($1234)
static uint32_t ParseHex(const std::string& str) {
  if (str.empty() || str[0] != '$') {
    return 0;
  }
  return std::stoul(str.substr(1), nullptr, 16);
}

// Helper: Trim whitespace
static std::string Trim(const std::string& str) {
  size_t start = str.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = str.find_last_not_of(" \t");
  return str.substr(start, end - start + 1);
}

// Helper: Convert to uppercase
static std::string ToUpper(const std::string& str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });
  return result;
}

// Helper: Strip comments (semicolon to end of line)
static std::string StripComments(const std::string& str) {
  size_t comment_pos = str.find(';');
  if (comment_pos != std::string::npos) {
    return str.substr(0, comment_pos);
  }
  return str;
}

// Helper: Check if character is valid identifier start
static bool IsIdentifierStart(char c) {
  return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

void SimpleSyntaxParser::Parse(const std::string& source, Section& section, ConcreteSymbolTable& symbols) {
  if (source.empty()) {
    return;
  }

  // Split into lines
  std::istringstream iss(source);
  std::string line;
  uint32_t current_address = 0;

  while (std::getline(iss, line)) {
    // Strip comments first
    line = StripComments(line);
    line = Trim(line);

    if (line.empty()) {
      continue;
    }

    // Check for label (identifier followed by colon)
    size_t colon_pos = line.find(':');
    if (colon_pos != std::string::npos) {
      // Extract label name
      std::string label_name = Trim(line.substr(0, colon_pos));

      if (!label_name.empty() && IsIdentifierStart(label_name[0])) {
        // Define symbol
        symbols.Define(label_name, SymbolType::Label,
              std::make_shared<LiteralExpr>(current_address));

        // Create LabelAtom
        section.atoms.push_back(std::make_shared<LabelAtom>(label_name, current_address));

        // Continue parsing rest of line (if any)
        if (colon_pos + 1 < line.length()) {
          line = Trim(line.substr(colon_pos + 1));
        } else {
          continue;  // Label only, no instruction
        }
      }
    }

    if (line.empty()) {
      continue;
    }

    // Parse directive
    if (line[0] == '.') {
      // Extract directive name
      size_t space_pos = line.find(' ');
      std::string directive = ToUpper(line.substr(0, space_pos));
      std::string operands;
      if (space_pos != std::string::npos) {
        operands = Trim(line.substr(space_pos + 1));
      }

      if (directive == ".ORG") {
        // Parse .org $8000
        uint32_t address = ParseHex(operands);
        section.atoms.push_back(std::make_shared<OrgAtom>(address));
        current_address = address;
      } else if (directive == ".DB") {
        // Parse .db $01, $02, $03
        std::vector<uint8_t> bytes;
        std::istringstream ops(operands);
        std::string value;

        while (std::getline(ops, value, ',')) {
          value = Trim(value);
          if (!value.empty()) {
            bytes.push_back(static_cast<uint8_t>(ParseHex(value)));
          }
        }

        section.atoms.push_back(std::make_shared<DataAtom>(bytes));
        current_address += bytes.size();
      } else if (directive == ".DW") {
        // Parse .dw $1234 (little-endian)
        std::vector<uint8_t> bytes;
        std::istringstream ops(operands);
        std::string value;

        while (std::getline(ops, value, ',')) {
          value = Trim(value);
          if (!value.empty()) {
            uint32_t word = ParseHex(value);
            bytes.push_back(static_cast<uint8_t>(word & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((word >> 8) & 0xFF)); // High byte
          }
        }

        section.atoms.push_back(std::make_shared<DataAtom>(bytes));
        current_address += bytes.size();
      }
    } else if (IsIdentifierStart(line[0])) {
      // Parse instruction: mnemonic [operands]
      size_t space_pos = line.find(' ');
      std::string mnemonic, operands;

      if (space_pos != std::string::npos) {
        mnemonic = ToUpper(Trim(line.substr(0, space_pos)));
        operands = Trim(line.substr(space_pos + 1));
      } else {
        mnemonic = ToUpper(Trim(line));
        operands = "";
      }

      section.atoms.push_back(std::make_shared<InstructionAtom>(mnemonic, operands));
      // Size will be determined during encoding phase
      current_address += 1;  // Placeholder (actual size determined later)
    }
  }
}

} // namespace xasm
