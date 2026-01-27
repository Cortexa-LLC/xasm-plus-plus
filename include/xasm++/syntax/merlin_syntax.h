// MerlinSyntax - Merlin assembly syntax parser
// Phases 1-3: Foundation, Local Labels, DUM Blocks

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/expression.h"

namespace xasm {

// MerlinSyntax parser
// Parses Merlin assembly syntax (Prince of Persia source format)
// Supports:
// - Comments: * (column 1) and ; (inline)
// - Labels: GLOBAL, :LOCAL, ]VARIABLE
// - Directives: ORG, EQU, DB, DW, HEX, DS, DUM, DEND
// - Number formats: $hex, %binary, decimal
class MerlinSyntaxParser {
public:
    MerlinSyntaxParser();

    // Parse assembly source and populate section with atoms
    void Parse(const std::string& source, Section& section, ConcreteSymbolTable& symbols);

private:
    // Label scope management for :LOCAL labels
    struct LabelScope {
        std::string global_label;  // The global label this scope belongs to
        std::unordered_map<std::string, uint32_t> local_labels;  // :label -> address
    };

    // Current label scope (for :LOCAL labels)
    LabelScope current_scope_;

    // DUM block state
    bool in_dum_block_;
    uint32_t dum_address_;  // Current address within DUM block
    std::unordered_map<std::string, uint32_t> variable_labels_;  // ]variable -> offset

    // Current address (for tracking label addresses)
    uint32_t current_address_;

    // Include file tracking (for circular include detection)
    std::vector<std::string> include_stack_;

    // Source location tracking (for error reporting)
    std::string current_file_;
    int current_line_;

    // Conditional assembly state (DO/ELSE/FIN)
    struct ConditionalBlock {
        bool condition;      // True if condition is met
        bool in_else_block;  // True if currently in ELSE block
        bool should_emit;    // True if code should be emitted
    };
    std::vector<ConditionalBlock> conditional_stack_;

    // Parsing helpers
    std::string StripComments(const std::string& line);
    std::string Trim(const std::string& str);
    std::string ToUpper(const std::string& str);

    void ParseLine(const std::string& line, Section& section, ConcreteSymbolTable& symbols);

    // Label parsing
    std::string ParseLabel(const std::string& line, size_t& pos, Section& section,
                          ConcreteSymbolTable& symbols);

    // Directive handlers
    void HandleOrg(const std::string& operand, Section& section,
                   ConcreteSymbolTable& symbols);
    void HandleEqu(const std::string& label, const std::string& operand,
                  ConcreteSymbolTable& symbols);
    void HandleDB(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
    void HandleDW(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
    void HandleHex(const std::string& operand, Section& section);
    void HandleDS(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
    void HandleDum(const std::string& operand, ConcreteSymbolTable& symbols);
    void HandleDend();
    void HandlePut(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
    void HandleLst(const std::string& operand);
    void HandleLstdo();
    void HandleTr(const std::string& operand);
    void HandleAsc(const std::string& operand, Section& section);
    void HandleDo(const std::string& operand, ConcreteSymbolTable& symbols);
    void HandleElse();
    void HandleFin();

    // Expression/number parsing
    uint32_t ParseNumber(const std::string& str);
    std::shared_ptr<Expression> ParseExpression(const std::string& str,
                                               ConcreteSymbolTable& symbols);

    // Error formatting with source location
    std::string FormatError(const std::string& message) const;
};

} // namespace xasm
