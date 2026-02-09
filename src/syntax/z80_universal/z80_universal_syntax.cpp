/**
 * @file z80_universal_syntax.cpp
 * @brief Z80 Universal Syntax Parser Implementation
 */

#include "xasm++/syntax/z80_universal_syntax.h"
#include "xasm++/atom.h"
#include "xasm++/cpu/cpu_z80.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace xasm {

Z80UniversalSyntaxParser::Z80UniversalSyntaxParser()
    : in_macro_definition_(false), macro_expansion_depth_(0),
      current_address_(0), end_directive_seen_(false), current_line_(0) {}

void Z80UniversalSyntaxParser::SetCpu(CpuZ80 *cpu) { cpu_ = cpu; }

void Z80UniversalSyntaxParser::Parse(const std::string &source,
                                     Section &section,
                                     ConcreteSymbolTable &symbols) {
  // Reset state
  end_directive_seen_ = false;
  current_line_ = 0;
  current_address_ = 0;
  conditional_stack_.clear();
  current_scope_.global_label.clear();
  current_scope_.local_labels.clear();

  // Split source into lines
  std::istringstream iss(source);
  std::string line;

  while (std::getline(iss, line)) {
    current_line_++;

    // Stop processing if END directive was seen
    if (end_directive_seen_) {
      break;
    }

    // Strip comments
    line = StripComments(line);

    // Skip empty lines
    line = Trim(line);
    if (line.empty()) {
      continue;
    }

    // Parse the line
    ParseLine(line, section, symbols);
  }
}

std::string Z80UniversalSyntaxParser::StripComments(const std::string &line) {
  // Find semicolon comment
  size_t semi_pos = line.find(';');
  if (semi_pos != std::string::npos) {
    return line.substr(0, semi_pos);
  }
  return line;
}

std::string Z80UniversalSyntaxParser::Trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return "";
  }
  size_t last = str.find_last_not_of(" \t\r\n");
  return str.substr(first, last - first + 1);
}

void Z80UniversalSyntaxParser::ParseLine(const std::string &line,
                                         Section &section,
                                         ConcreteSymbolTable &symbols) {
  size_t pos = 0;

  // Check for label (ends with : or ::)
  std::string label = ParseLabel(line, pos, section, symbols);

  // Skip whitespace after label
  while (pos < line.size() && std::isspace(line[pos])) {
    pos++;
  }

  // Get mnemonic/directive
  size_t start = pos;
  while (pos < line.size() && !std::isspace(line[pos])) {
    pos++;
  }

  if (start >= line.size()) {
    // Line with only a label
    return;
  }

  std::string mnemonic = line.substr(start, pos - start);

  // Convert to uppercase for case-insensitive matching
  std::string upper_mnemonic = mnemonic;
  std::transform(upper_mnemonic.begin(), upper_mnemonic.end(),
                 upper_mnemonic.begin(), ::toupper);

  // Skip whitespace after mnemonic
  while (pos < line.size() && std::isspace(line[pos])) {
    pos++;
  }

  // Get next token to check if mnemonic is actually a label for EQU/=/SET
  std::string second_token;
  size_t second_start = pos;
  while (pos < line.size() && !std::isspace(line[pos])) {
    pos++;
  }
  if (second_start < line.size()) {
    second_token = line.substr(second_start, pos - second_start);
  }
  
  // Convert second token to uppercase
  std::string upper_second = second_token;
  std::transform(upper_second.begin(), upper_second.end(),
                 upper_second.begin(), ::toupper);

  // Check if this is label-without-colon syntax (LABEL EQU/=/SET value)
  if (label.empty() && 
      (upper_second == "EQU" || upper_second == "=" || 
       upper_second == "SET" || upper_second == "DEFL")) {
    // First token is the label, second is the directive
    label = mnemonic;
    upper_mnemonic = upper_second;
    
    // Skip whitespace after directive
    while (pos < line.size() && std::isspace(line[pos])) {
      pos++;
    }
  } else {
    // Reset pos to after first token for normal processing
    pos = second_start;
  }

  // Get operand (rest of line)
  std::string operand;
  if (pos < line.size()) {
    operand = Trim(line.substr(pos));
  }

  // Handle directives
  if (upper_mnemonic == "ORG") {
    HandleOrg(operand, section, symbols);
  } else if (upper_mnemonic == "EQU") {
    if (!label.empty()) {
      HandleEqu(label, operand, symbols);
    }
  } else if (upper_mnemonic == "=" || upper_mnemonic == "SET" ||
             upper_mnemonic == "DEFL") {
    if (!label.empty()) {
      HandleSet(label, operand, symbols);
    }
  } else if (upper_mnemonic == "DB" || upper_mnemonic == "DEFB" ||
             upper_mnemonic == "BYTE" || upper_mnemonic == "DM" ||
             upper_mnemonic == "DEFM") {
    HandleDB(operand, section, symbols);
  } else if (upper_mnemonic == "DW" || upper_mnemonic == "DEFW" ||
             upper_mnemonic == "WORD") {
    HandleDW(operand, section, symbols);
  } else if (upper_mnemonic == "DS" || upper_mnemonic == "DEFS" ||
             upper_mnemonic == "DSPACE" || upper_mnemonic == "BLOCK") {
    HandleDS(operand, section, symbols);
  } else if (upper_mnemonic == "END") {
    HandleEnd();
  } else if (upper_mnemonic == "PUBLIC" || upper_mnemonic == "GLOBAL" ||
             upper_mnemonic == "ENTRY") {
    HandlePublic(operand, symbols);
  } else if (upper_mnemonic == "EXTERN" || upper_mnemonic == "EXTRN" ||
             upper_mnemonic == "EXT") {
    HandleExtern(operand, symbols);
  } else if (upper_mnemonic == "CSEG") {
    HandleCseg(section);
  } else if (upper_mnemonic == "DSEG") {
    HandleDseg(section);
  } else if (upper_mnemonic == "ASEG") {
    HandleAseg(section);
  } else if (upper_mnemonic == "IF") {
    HandleIf(operand, symbols);
  } else if (upper_mnemonic == "ELSE") {
    HandleElse();
  } else if (upper_mnemonic == "ENDIF") {
    HandleEndif();
  } else if (upper_mnemonic == "MACRO") {
    HandleMacro(label);
  } else if (upper_mnemonic == "ENDM") {
    HandleEndm();
  } else if (upper_mnemonic == "REPT") {
    HandleRept(operand);
  } else {
    // Unknown directive/instruction - ignore for now (minimal stub)
    // In GREEN phase, we'll add CPU instruction handling
  }
}

std::string Z80UniversalSyntaxParser::ParseLabel(const std::string &line,
                                                 size_t &pos, Section &section,
                                                 ConcreteSymbolTable &symbols) {
  // Check if line starts with a label (identifier followed by : or ::)
  size_t colon_pos = line.find(':');
  if (colon_pos == std::string::npos || colon_pos == 0) {
    return ""; // No label
  }

  // Extract potential label
  std::string potential_label = Trim(line.substr(0, colon_pos));

  // Check if it's all identifier characters (no spaces)
  bool is_label = true;
  for (char c : potential_label) {
    if (!std::isalnum(c) && c != '_' && c != '$' && c != '.' && c != '?') {
      is_label = false;
      break;
    }
  }

  if (!is_label) {
    return "";
  }

  // Determine if public label (::) or private (:)
  // TODO: Track public/private distinction for symbol visibility
  if (colon_pos + 1 < line.size() && line[colon_pos + 1] == ':') {
    pos = colon_pos + 2; // Skip ::
  } else {
    pos = colon_pos + 1; // Skip :
  }

  // Create label atom
  auto label_atom = std::make_shared<LabelAtom>(potential_label, current_address_);
  section.atoms.push_back(label_atom);

  // Define symbol
  symbols.DefineLabel(potential_label, current_address_);

  // Update scope if it's a global label (not starting with . $ ?)
  if (potential_label[0] != '.' && potential_label[0] != '$' &&
      potential_label[0] != '?') {
    current_scope_.global_label = potential_label;
    current_scope_.local_labels.clear();
  } else {
    // Local label - add to current scope
    current_scope_.local_labels[potential_label] = current_address_;
  }

  return potential_label;
}

void Z80UniversalSyntaxParser::HandleOrg(const std::string &operand,
                                         Section &section,
                                         ConcreteSymbolTable &symbols) {
  (void)symbols;
  uint32_t address = ParseNumber(operand);

  auto org_atom = std::make_shared<OrgAtom>(address);
  section.atoms.push_back(org_atom);

  current_address_ = address;
}

void Z80UniversalSyntaxParser::HandleEqu(const std::string &label,
                                         const std::string &operand,
                                         ConcreteSymbolTable &symbols) {
  uint32_t value = ParseNumber(operand);
  symbols.DefineLabel(label, value);
}

void Z80UniversalSyntaxParser::HandleSet(const std::string &label,
                                         const std::string &operand,
                                         ConcreteSymbolTable &symbols) {
  uint32_t value = ParseNumber(operand);
  symbols.DefineLabel(label, value);
}

void Z80UniversalSyntaxParser::HandleDB(const std::string &operand,
                                        Section &section,
                                        ConcreteSymbolTable &symbols) {
  (void)symbols;
  std::vector<uint8_t> bytes;

  // Parse comma-separated values
  std::istringstream iss(operand);
  std::string token;
  while (std::getline(iss, token, ',')) {
    token = Trim(token);
    if (!token.empty()) {
      uint32_t value = ParseNumber(token);
      bytes.push_back(static_cast<uint8_t>(value & 0xFF));
      current_address_++;
    }
  }

  auto data_atom = std::make_shared<DataAtom>(bytes);
  section.atoms.push_back(data_atom);
}

void Z80UniversalSyntaxParser::HandleDW(const std::string &operand,
                                        Section &section,
                                        ConcreteSymbolTable &symbols) {
  (void)symbols;
  std::vector<uint8_t> bytes;

  // Parse comma-separated values
  std::istringstream iss(operand);
  std::string token;
  while (std::getline(iss, token, ',')) {
    token = Trim(token);
    if (!token.empty()) {
      uint32_t value = ParseNumber(token);
      bytes.push_back(static_cast<uint8_t>(value & 0xFF));         // Low byte
      bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFF)); // High byte
      current_address_ += 2;
    }
  }

  auto data_atom = std::make_shared<DataAtom>(bytes);
  section.atoms.push_back(data_atom);
}

void Z80UniversalSyntaxParser::HandleDS(const std::string &operand,
                                        Section &section,
                                        ConcreteSymbolTable &symbols) {
  (void)symbols;
  uint32_t size = ParseNumber(operand);

  auto space_atom = std::make_shared<SpaceAtom>(size);
  section.atoms.push_back(space_atom);

  current_address_ += size;
}

void Z80UniversalSyntaxParser::HandleEnd() { end_directive_seen_ = true; }

void Z80UniversalSyntaxParser::HandlePublic(const std::string &operand,
                                            ConcreteSymbolTable &symbols) {
  // PUBLIC/GLOBAL/ENTRY directive - mark symbols as exported
  std::vector<std::string> symbol_names = ParseSymbolList(operand);
  
  for (const std::string &symbol_name : symbol_names) {
    // Get the symbol
    Symbol *symbol = symbols.GetSymbol(symbol_name);
    if (symbol != nullptr) {
      // Mark as exported
      symbol->is_exported = true;
    } else {
      // Symbol not yet defined - this is okay, just mark it when defined
      // For now, we'll just ignore undefined symbols in PUBLIC directive
      // A more sophisticated implementation would track this and error later
    }
  }
}

void Z80UniversalSyntaxParser::HandleExtern(const std::string &operand,
                                            ConcreteSymbolTable &symbols) {
  // EXTERN/EXTRN/EXT directive - declare imported symbols
  std::vector<std::string> symbol_names = ParseSymbolList(operand);
  
  for (const std::string &symbol_name : symbol_names) {
    // Create extern symbol with value 0 (linker will resolve)
    // Check if symbol already exists
    if (!symbols.IsDefined(symbol_name)) {
      // Define as new symbol with value 0
      symbols.DefineLabel(symbol_name, 0);
    }
    
    // Mark as imported
    Symbol *symbol = symbols.GetSymbol(symbol_name);
    if (symbol != nullptr) {
      symbol->is_imported = true;
    }
  }
}

std::vector<std::string>
Z80UniversalSyntaxParser::ParseSymbolList(const std::string &operand) {
  // Parse comma-separated list of symbols
  std::vector<std::string> symbols;
  std::istringstream iss(operand);
  std::string symbol_name;
  
  while (std::getline(iss, symbol_name, ',')) {
    symbol_name = Trim(symbol_name);
    if (!symbol_name.empty()) {
      symbols.push_back(symbol_name);
    }
  }
  
  return symbols;
}

void Z80UniversalSyntaxParser::HandleCseg(Section &section) {
  (void)section;
  // CSEG directive - switch to code segment (no-op for now)
}

void Z80UniversalSyntaxParser::HandleDseg(Section &section) {
  (void)section;
  // DSEG directive - switch to data segment (no-op for now)
}

void Z80UniversalSyntaxParser::HandleAseg(Section &section) {
  (void)section;
  // ASEG directive - switch to absolute segment (no-op for now)
}

void Z80UniversalSyntaxParser::HandleIf(const std::string &operand,
                                        ConcreteSymbolTable &symbols) {
  (void)operand;
  (void)symbols;
  // IF directive - conditional assembly (no-op stub)
}

void Z80UniversalSyntaxParser::HandleElse() {
  // ELSE directive - conditional assembly (no-op stub)
}

void Z80UniversalSyntaxParser::HandleEndif() {
  // ENDIF directive - conditional assembly (no-op stub)
}

void Z80UniversalSyntaxParser::HandleMacro(const std::string &name) {
  (void)name;
  // MACRO directive - macro definition (no-op stub)
}

void Z80UniversalSyntaxParser::HandleEndm() {
  // ENDM directive - end macro definition (no-op stub)
}

void Z80UniversalSyntaxParser::HandleRept(const std::string &operand) {
  (void)operand;
  // REPT directive - repeat block (no-op stub)
}

uint32_t Z80UniversalSyntaxParser::ParseNumber(const std::string &str) {
  std::string trimmed = Trim(str);
  if (trimmed.empty()) {
    return 0;
  }

  // Hex: $FF, 0xFF, 0FFH
  if (trimmed[0] == '$') {
    return std::stoul(trimmed.substr(1), nullptr, 16);
  } else if (trimmed.size() >= 2 && trimmed[0] == '0' &&
             (trimmed[1] == 'x' || trimmed[1] == 'X')) {
    return std::stoul(trimmed.substr(2), nullptr, 16);
  } else if (trimmed.size() >= 2 &&
             (trimmed.back() == 'H' || trimmed.back() == 'h')) {
    return std::stoul(trimmed.substr(0, trimmed.size() - 1), nullptr, 16);
  }
  // Binary: 11110000B
  else if (trimmed.size() >= 2 &&
           (trimmed.back() == 'B' || trimmed.back() == 'b')) {
    return std::stoul(trimmed.substr(0, trimmed.size() - 1), nullptr, 2);
  }
  // Octal: 377O, 377Q
  else if (trimmed.size() >= 2 &&
           (trimmed.back() == 'O' || trimmed.back() == 'o' ||
            trimmed.back() == 'Q' || trimmed.back() == 'q')) {
    return std::stoul(trimmed.substr(0, trimmed.size() - 1), nullptr, 8);
  }
  // Decimal: 255, 255D
  else {
    // Remove optional D suffix
    std::string num = trimmed;
    if (num.back() == 'D' || num.back() == 'd') {
      num = num.substr(0, num.size() - 1);
    }
    return std::stoul(num, nullptr, 10);
  }
}

std::shared_ptr<Expression>
Z80UniversalSyntaxParser::ParseExpression(const std::string &str,
                                          ConcreteSymbolTable &symbols) {
  (void)str;
  (void)symbols;
  // Minimal stub for now - will implement in later phase
  return nullptr;
}

std::string
Z80UniversalSyntaxParser::FormatError(const std::string &message) const {
  std::ostringstream oss;
  if (!current_file_.empty()) {
    oss << current_file_ << ":";
  }
  oss << current_line_ << ": " << message;
  return oss.str();
}

} // namespace xasm
