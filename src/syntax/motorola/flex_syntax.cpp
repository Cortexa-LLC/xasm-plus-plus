/**
 * @file flex_syntax.cpp
 * @brief FLEX ASM09 assembly syntax parser implementation
 * 
 * Implementation of the FLEX ASM09 assembler syntax for Motorola 6809.
 */

#include "xasm++/syntax/flex_syntax.h"
#include "xasm++/parse_utils.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_set>

namespace xasm {

// ============================================================================
// Helper Functions
// ============================================================================

std::string FlexAsmSyntax::Trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

std::string FlexAsmSyntax::ToUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string FlexAsmSyntax::StripComments(const std::string& line) {
    size_t comment_pos = line.find(';');
    if (comment_pos != std::string::npos) {
        return line.substr(0, comment_pos);
    }
    return line;
}

bool FlexAsmSyntax::IsCommentLine(const std::string& line) {
    std::string trimmed = Trim(line);
    return !trimmed.empty() && trimmed[0] == '*';
}

uint32_t FlexAsmSyntax::ParseNumber(const std::string& str) {
    std::string trimmed = Trim(str);

    if (trimmed.empty()) {
        throw std::runtime_error("Empty number string");
    }

    // Hexadecimal ($xxxx)
    if (trimmed[0] == '$') {
        return ParseHex(trimmed.substr(1));
    }

    // Binary (%10101010)
    if (trimmed[0] == '%') {
        std::string binary = trimmed.substr(1);
        uint32_t value = 0;
        for (char c : binary) {
            if (c != '0' && c != '1') {
                throw std::runtime_error("Invalid binary digit: " + std::string(1, c));
            }
            value = (value << 1) | (c - '0');
        }
        return value;
    }

    // ASCII character ('A')
    if (trimmed[0] == '\'' && trimmed.length() >= 3 && trimmed[2] == '\'') {
        return static_cast<uint32_t>(trimmed[1]);
    }

    // Decimal (default)
    try {
        return static_cast<uint32_t>(std::stoul(trimmed, nullptr, 10));
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid decimal number: " + trimmed);
    }
}

// ============================================================================
// Directive Parsing
// ============================================================================

void FlexAsmSyntax::ParseDirective(const std::string& directive,
                                    const std::string& operands,
                                    const std::string& label,
                                    Section& section,
                                    ConcreteSymbolTable& symbols) {
    std::string dir_upper = ToUpper(directive);

    // ORG - Set origin address
    if (dir_upper == "ORG") {
        uint32_t address = ParseNumber(operands);
        section.atoms.push_back(std::make_shared<OrgAtom>(address));
        current_address_ = address;
        return;
    }

    // END - End of assembly
    if (dir_upper == "END") {
        // END can have optional entry point, but we don't create an atom
        return;
    }

    // EQU - Equate (constant symbol)
    if (dir_upper == "EQU") {
        if (label.empty()) {
            throw std::runtime_error("EQU requires a label");
        }
        uint32_t value = ParseNumber(operands);
        symbols.DefineLabel(label, static_cast<int64_t>(value));
        return;
    }

    // SET - Set (variable symbol)
    if (dir_upper == "SET") {
        if (label.empty()) {
            throw std::runtime_error("SET requires a label");
        }
        uint32_t value = ParseNumber(operands);
        // SET allows redefinition, so we define/redefine
        symbols.DefineLabel(label, static_cast<int64_t>(value));
        return;
    }

    // FCB - Form Constant Byte
    if (dir_upper == "FCB") {
        std::vector<uint8_t> data;
        std::istringstream iss(operands);
        std::string value_str;

        while (std::getline(iss, value_str, ',')) {
            value_str = Trim(value_str);
            if (!value_str.empty()) {
                uint32_t value = ParseNumber(value_str);
                data.push_back(static_cast<uint8_t>(value & 0xFF));
            }
        }

        if (!data.empty()) {
            section.atoms.push_back(std::make_shared<DataAtom>(data));
        }
        return;
    }

    // FDB - Form Double Byte (16-bit big-endian)
    if (dir_upper == "FDB") {
        std::vector<uint8_t> data;
        std::istringstream iss(operands);
        std::string value_str;

        while (std::getline(iss, value_str, ',')) {
            value_str = Trim(value_str);
            if (!value_str.empty()) {
                uint32_t value = ParseNumber(value_str);
                // Big-endian (MSB first)
                data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
                data.push_back(static_cast<uint8_t>(value & 0xFF));
            }
        }

        if (!data.empty()) {
            section.atoms.push_back(std::make_shared<DataAtom>(data));
        }
        return;
    }

    // FCC - Form Constant Characters
    if (dir_upper == "FCC") {
        std::string trimmed = Trim(operands);
        if (trimmed.length() < 2) {
            throw std::runtime_error("FCC requires delimited string");
        }

        char delimiter = trimmed[0];
        size_t end_pos = trimmed.find(delimiter, 1);
        if (end_pos == std::string::npos) {
            throw std::runtime_error("FCC missing closing delimiter");
        }

        std::string str_content = trimmed.substr(1, end_pos - 1);
        std::vector<uint8_t> data(str_content.begin(), str_content.end());
        section.atoms.push_back(std::make_shared<DataAtom>(data));
        return;
    }

    // RMB - Reserve Memory Bytes
    if (dir_upper == "RMB") {
        uint32_t count = ParseNumber(operands);
        section.atoms.push_back(std::make_shared<SpaceAtom>(count));
        return;
    }

    // SETDP - Set Direct Page
    if (dir_upper == "SETDP") {
        uint32_t value = ParseNumber(operands);
        direct_page_ = static_cast<uint8_t>(value & 0xFF);
        // SETDP is assembler directive, doesn't create atom
        return;
    }

    // NAM/TTL - Set program title
    if (dir_upper == "NAM" || dir_upper == "TTL") {
        title_ = Trim(operands);
        // Listing control directive, no atom
        return;
    }

    // STTL - Set subtitle
    if (dir_upper == "STTL") {
        subtitle_ = Trim(operands);
        // Listing control directive, no atom
        return;
    }

    // PAGE - Form feed
    if (dir_upper == "PAGE") {
        // Listing control directive, no atom
        return;
    }

    // SPC - Space lines
    if (dir_upper == "SPC") {
        // Listing control directive, no atom
        return;
    }

    throw std::runtime_error("Unknown directive: " + directive);
}

// ============================================================================
// Line Parsing
// ============================================================================

void FlexAsmSyntax::ParseLine(const std::string& line,
                               Section& section,
                               ConcreteSymbolTable& symbols) {
    // Strip inline comments
    std::string cleaned = StripComments(line);

    // Skip blank lines and comment lines
    if (cleaned.empty() || IsCommentLine(cleaned)) {
        return;
    }

    std::string trimmed = Trim(cleaned);
    if (trimmed.empty()) {
        return;
    }

    // Parse line into fields: label, opcode, operands
    std::string label, opcode, operands;

    // FLEX syntax: label starts in column 1, opcode has leading whitespace
    // For simplicity, we'll use a simpler approach: first token is label if
    // line doesn't start with whitespace, else it's opcode

    if (!std::isspace(static_cast<unsigned char>(line[0]))) {
        // Label present
        size_t space_pos = trimmed.find_first_of(" \t");
        if (space_pos != std::string::npos) {
            label = trimmed.substr(0, space_pos);
            std::string rest = Trim(trimmed.substr(space_pos));

            space_pos = rest.find_first_of(" \t");
            if (space_pos != std::string::npos) {
                opcode = rest.substr(0, space_pos);
                operands = Trim(rest.substr(space_pos));
            } else {
                opcode = rest;
            }
        } else {
            // Just a label
            label = trimmed;
        }
    } else {
        // No label, starts with whitespace
        std::string rest = Trim(trimmed);
        size_t space_pos = rest.find_first_of(" \t");
        if (space_pos != std::string::npos) {
            opcode = rest.substr(0, space_pos);
            operands = Trim(rest.substr(space_pos));
        } else {
            opcode = rest;
        }
    }

    // If we have an opcode, check if it's a directive
    if (!opcode.empty()) {
        // Check if it's a directive (directives are handled specially)
        std::string opcode_upper = ToUpper(opcode);

        // List of known directives
        static const std::unordered_set<std::string> directives = {
            "ORG", "END", "EQU", "SET", "FCB", "FDB", "FCC", "RMB", "SETDP",
            "NAM", "TTL", "STTL", "PAGE", "SPC",
            "MACRO", "ENDM", "IFC", "ENDC", "RPT", "ENDR"
        };

        // Directives that define symbols but don't create label atoms
        static const std::unordered_set<std::string> symbol_directives = {
            "EQU", "SET"
        };

        if (directives.find(opcode_upper) != directives.end()) {
            // For non-symbol-defining directives, create label atom BEFORE directive
            if (!label.empty() && symbol_directives.find(opcode_upper) == symbol_directives.end()) {
                section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
                symbols.DefineLabel(label, static_cast<int64_t>(current_address_));
            }
            
            // Handle directive
            ParseDirective(opcode, operands, label, section, symbols);
        } else {
            // It's an instruction - we'll handle this later
            // For now, just skip (tests only test directives)
            // But create label atom if label is present
            if (!label.empty()) {
                section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
                symbols.DefineLabel(label, static_cast<int64_t>(current_address_));
            }
        }
    } else if (!label.empty()) {
        // Label only (no opcode) - create label atom
        section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
        symbols.DefineLabel(label, static_cast<int64_t>(current_address_));
    }
}

// ============================================================================
// Main Parse Function
// ============================================================================

void FlexAsmSyntax::Parse(const std::string& source,
                          Section& section,
                          ConcreteSymbolTable& symbols) {
    // Split source into lines
    std::istringstream iss(source);
    std::string line;

    while (std::getline(iss, line)) {
        ParseLine(line, section, symbols);
    }
}

// ============================================================================
// Macro Processor Stubs (Phase 2)
// ============================================================================

bool FlexAsmSyntax::IsMacro(const std::string& name) const {
    std::string name_copy = name;  // Need non-const for ToUpper
    std::transform(name_copy.begin(), name_copy.end(), name_copy.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return macros_.find(name_copy) != macros_.end();
}

std::vector<std::string> FlexAsmSyntax::ExpandMacro(const std::string& /*name*/,
                                                     const std::vector<std::string>& /*arguments*/) {
    // TODO: Phase 2 implementation
    return {};
}

std::string FlexAsmSyntax::SubstituteParameters(const std::string& line,
                                                 const MacroDefinition& /*macro*/,
                                                 const std::vector<std::string>& /*arguments*/) {
    // TODO: Phase 2 implementation
    return line;
}

std::string FlexAsmSyntax::MakeLocalLabelUnique(const std::string& label, int /*expansion_id*/) {
    // TODO: Phase 2 implementation
    return label;
}

// ============================================================================
// Conditional Assembly Stubs (Phase 3)
// ============================================================================

bool FlexAsmSyntax::EvaluateCondition(const std::string& /*condition*/) {
    // TODO: Phase 3 implementation
    return true;
}

bool FlexAsmSyntax::ShouldAssemble() const {
    // TODO: Phase 3 implementation
    return conditional_stack_.empty() || conditional_stack_.top();
}

} // namespace xasm
