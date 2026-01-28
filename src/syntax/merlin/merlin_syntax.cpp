// MerlinSyntax - Merlin assembly syntax parser implementation
// Phases 1-3: Foundation, Local Labels, DUM Blocks

#include "xasm++/syntax/merlin_syntax.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace xasm {

// ============================================================================
// Constructor
// ============================================================================

MerlinSyntaxParser::MerlinSyntaxParser()
    : in_dum_block_(false)
    , dum_address_(0)
    , current_address_(0)
    , current_file_("<stdin>")
    , current_line_(0) {
}

// ============================================================================
// Helper Functions
// ============================================================================

// Strip comments: * in column 1 or ; anywhere
std::string MerlinSyntaxParser::StripComments(const std::string& line) {
    // Empty line
    if (line.empty()) {
        return "";
    }

    // * in column 1 is a full-line comment
    if (line[0] == '*') {
        return "";
    }

    // ; anywhere marks start of inline comment
    size_t comment_pos = line.find(';');
    if (comment_pos != std::string::npos) {
        return line.substr(0, comment_pos);
    }

    return line;
}

// Trim leading and trailing whitespace
std::string MerlinSyntaxParser::Trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

// Convert string to uppercase
std::string MerlinSyntaxParser::ToUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c) { return std::toupper(c); });
    return result;
}

// Format error message with source location
std::string MerlinSyntaxParser::FormatError(const std::string& message) const {
    std::ostringstream oss;
    oss << current_file_ << ":" << current_line_ << ": error: " << message;
    return oss.str();
}

// ============================================================================
// Number Parsing
// ============================================================================

// Parse number in various formats: $hex, %binary, decimal
uint32_t MerlinSyntaxParser::ParseNumber(const std::string& str) {
    if (str.empty()) {
        return 0;
    }

    // Hex: $FFFF
    if (str[0] == '$') {
        std::string hex_part = str.substr(1);
        if (hex_part.empty()) {
            throw std::runtime_error(FormatError("Invalid hex number: '" + str + "' (no digits after $)"));
        }
        try {
            return std::stoul(hex_part, nullptr, 16);
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error(FormatError("Invalid hex number: '" + str + "' - " + e.what()));
        }
    }

    // Binary: %11110000
    if (str[0] == '%') {
        std::string bin_part = str.substr(1);
        if (bin_part.empty()) {
            throw std::runtime_error(FormatError("Invalid binary number: '" + str + "' (no digits after %)"));
        }
        try {
            return std::stoul(bin_part, nullptr, 2);
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error(FormatError("Invalid binary number: '" + str + "' - " + e.what()));
        }
    }

    // Decimal: 42
    try {
        return std::stoul(str, nullptr, 10);
    } catch (const std::invalid_argument& e) {
        throw std::runtime_error(FormatError("Invalid decimal number: '" + str + "' - " + e.what()));
    }
}

// Parse expression (for now, just handle simple numbers and basic operators)
std::shared_ptr<Expression> MerlinSyntaxParser::ParseExpression(
    const std::string& str, ConcreteSymbolTable& symbols) {

    // For now, handle simple cases:
    // - Pure number: $100
    // - Simple addition: $100+$20, VALUE+5
    // - Symbol reference: LABEL
    // - Negative number: -1
    // - Low byte operator: <ADDRESS
    // - High byte operator: >ADDRESS
    // - Multiplication: 24*30

    std::string expr = Trim(str);

    // Check for character literal: "x" or 'x'
    if (!expr.empty() && (expr[0] == '"' || expr[0] == '\'')) {
        // Character literal - extract the character
        if (expr.length() >= 3 && expr[expr.length()-1] == expr[0]) {
            // Proper quote-enclosed character: "j" or 'j'
            char ch = expr[1];
            return std::make_shared<LiteralExpr>(static_cast<uint8_t>(ch));
        } else if (expr.length() == 1) {
            // Just a quote character (like in SPECIALK.S weird case)
            // Treat as the ASCII value of the quote itself
            return std::make_shared<LiteralExpr>(static_cast<uint8_t>(expr[0]));
        } else {
            // Malformed character literal - treat as 0
            return std::make_shared<LiteralExpr>(0);
        }
    }

    // Check for low byte operator (< or #)
    if (!expr.empty() && (expr[0] == '<' || expr[0] == '#')) {
        if (expr.length() < 2) {
            throw std::runtime_error(FormatError("Low byte operator (</#) requires an operand"));
        }
        std::string operand = Trim(expr.substr(1));
        if (operand.empty()) {
            throw std::runtime_error(FormatError("Low byte operator (</#) has empty operand"));
        }
        // Recursively parse the operand (might be expression like SHIFT0-$80)
        auto operand_expr = ParseExpression(operand, symbols);
        int64_t value = operand_expr->Evaluate(symbols);
        return std::make_shared<LiteralExpr>(value & 0xFF);  // Low byte
    }

    // Check for high byte operator (>)
    if (!expr.empty() && expr[0] == '>') {
        if (expr.length() < 2) {
            throw std::runtime_error(FormatError("High byte operator (>) requires an operand"));
        }
        std::string operand = Trim(expr.substr(1));
        if (operand.empty()) {
            throw std::runtime_error(FormatError("High byte operator (>) has empty operand"));
        }
        // Recursively parse the operand (might be expression like SHIFT0-$80)
        auto operand_expr = ParseExpression(operand, symbols);
        int64_t value = operand_expr->Evaluate(symbols);
        return std::make_shared<LiteralExpr>((value >> 8) & 0xFF);  // High byte
    }

    // Check for addition/subtraction first (lower precedence than multiplication)
    // We need to handle expressions like BASE+OFFSET*2 properly
    size_t plus_pos = expr.find('+');
    size_t minus_pos = expr.find('-', 1);  // Skip first char (could be negative sign)

    if (plus_pos != std::string::npos) {
        // Addition: BASE+OFFSET*2
        std::string left = Trim(expr.substr(0, plus_pos));
        std::string right = Trim(expr.substr(plus_pos + 1));
        
        // Recursively parse left side (might be complex)
        auto left_expr = ParseExpression(left, symbols);
        int64_t left_val = left_expr->Evaluate(symbols);
        
        // Recursively parse right side (might contain multiplication)
        auto right_expr = ParseExpression(right, symbols);
        int64_t right_val = right_expr->Evaluate(symbols);
        
        return std::make_shared<LiteralExpr>(left_val + right_val);
    } else if (minus_pos != std::string::npos) {
        // Subtraction: $200-$10 or VALUE-5
        std::string left = Trim(expr.substr(0, minus_pos));
        std::string right = Trim(expr.substr(minus_pos + 1));
        
        // Recursively parse left side
        auto left_expr = ParseExpression(left, symbols);
        int64_t left_val = left_expr->Evaluate(symbols);
        
        // Recursively parse right side
        auto right_expr = ParseExpression(right, symbols);
        int64_t right_val = right_expr->Evaluate(symbols);
        
        return std::make_shared<LiteralExpr>(left_val - right_val);
    }
    
    // Check for multiplication (higher precedence, parsed after +/-)
    size_t mult_pos = expr.find('*');
    if (mult_pos != std::string::npos) {
        std::string left = Trim(expr.substr(0, mult_pos));
        std::string right = Trim(expr.substr(mult_pos + 1));

        // Parse left side (could be number or symbol)
        int64_t left_val = 0;
        if (symbols.IsDefined(left)) {
            auto sym_expr = std::make_shared<SymbolExpr>(left);
            left_val = sym_expr->Evaluate(symbols);
        } else {
            left_val = ParseNumber(left);
        }

        // Parse right side (could be number or symbol)
        int64_t right_val = 0;
        if (symbols.IsDefined(right)) {
            auto sym_expr = std::make_shared<SymbolExpr>(right);
            right_val = sym_expr->Evaluate(symbols);
        } else {
            right_val = ParseNumber(right);
        }
        
        return std::make_shared<LiteralExpr>(left_val * right_val);
    }
    
    // Check for negative number
    if (!expr.empty() && expr[0] == '-') {
        // Negative number: -1, -128
        int32_t value = std::stoi(expr);
        return std::make_shared<LiteralExpr>(static_cast<uint32_t>(value));
    } else if (!expr.empty() && (expr[0] == '$' || expr[0] == '%' || std::isdigit(expr[0]))) {
        // Pure number
        return std::make_shared<LiteralExpr>(ParseNumber(expr));
    } else if (!expr.empty() && symbols.IsDefined(expr)) {
        // Symbol reference
        return std::make_shared<SymbolExpr>(expr);
    }

    // Unknown or empty - return literal 0 for now
    return std::make_shared<LiteralExpr>(0);
}

// ============================================================================
// Label Parsing
// ============================================================================

std::string MerlinSyntaxParser::ParseLabel(const std::string& line, size_t& pos,
                                          Section& /*section*/, ConcreteSymbolTable& /*symbols*/) {
    // Merlin labels can be:
    // 1. Global label: START (starts in column 1-9, no special prefix)
    // 2. :Local label: :LOOP (prefixed with :, scoped to last global)
    // 3. ]Variable label: ]TEMP (prefixed with ], used in DUM blocks)

    // Check if line starts with label (non-whitespace in first columns)
    if (pos == 0 && !line.empty() && !std::isspace(line[0])) {
        // Find end of label (space or end of line)
        size_t label_end = pos;
        while (label_end < line.length() && !std::isspace(line[label_end])) {
            label_end++;
        }

        std::string label = line.substr(pos, label_end - pos);
        pos = label_end;

        // Just return the label - let caller decide what to do with it
        // (EQU defines symbols without atoms, other labels create atoms)
        return label;
    }

    return "";
}

// ============================================================================
// Directive Handlers
// ============================================================================

void MerlinSyntaxParser::HandleOrg(const std::string& operand, Section& section,
                                    ConcreteSymbolTable& symbols) {
    std::string op = Trim(operand);
    uint32_t address = 0;

    // Check if operand is empty
    if (op.empty()) {
        throw std::runtime_error(FormatError("ORG directive requires an address operand"));
    }

    // Check if operand is a symbol or a number
    if (op[0] == '$' || op[0] == '%' || std::isdigit(op[0])) {
        // It's a number
        address = ParseNumber(op);
    } else {
        // It's a symbol - look it up
        int64_t value = 0;
        if (symbols.Lookup(op, value)) {
            address = static_cast<uint32_t>(value);
        } else {
            // Symbol not found - use 0 for now (forward reference issue)
            address = 0;
        }
    }
    
    section.atoms.push_back(std::make_shared<OrgAtom>(address));
    current_address_ = address;
}

void MerlinSyntaxParser::HandleEqu(const std::string& label, const std::string& operand,
                                   ConcreteSymbolTable& symbols) {
    auto expr = ParseExpression(operand, symbols);
    symbols.Define(label, SymbolType::Label, expr);
    // EQU doesn't create atoms
}

void MerlinSyntaxParser::HandleDB(const std::string& operand, Section& section,
                                  ConcreteSymbolTable& symbols) {
    std::vector<uint8_t> bytes;
    std::istringstream iss(operand);
    std::string value;

    while (std::getline(iss, value, ',')) {
        value = Trim(value);
        if (!value.empty()) {
            // Use ParseExpression to handle both numbers, symbols, and expressions
            auto expr = ParseExpression(value, symbols);
            int64_t result = expr->Evaluate(symbols);
            bytes.push_back(static_cast<uint8_t>(result & 0xFF));
        }
    }

    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    current_address_ += bytes.size();
}

void MerlinSyntaxParser::HandleDW(const std::string& operand, Section& section,
                                   ConcreteSymbolTable& symbols) {
    std::vector<uint8_t> bytes;
    std::istringstream iss(operand);
    std::string value;

    while (std::getline(iss, value, ',')) {
        value = Trim(value);
        if (!value.empty()) {
            // Use ParseExpression to handle both numbers, symbols, and expressions
            auto expr = ParseExpression(value, symbols);
            int64_t result = expr->Evaluate(symbols);
            uint32_t word = static_cast<uint32_t>(result);
            // Little-endian: low byte first
            bytes.push_back(static_cast<uint8_t>(word & 0xFF));
            bytes.push_back(static_cast<uint8_t>((word >> 8) & 0xFF));
        }
    }

    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    current_address_ += bytes.size();
}

void MerlinSyntaxParser::HandleHex(const std::string& operand, Section& section) {
    std::vector<uint8_t> bytes;
    std::string hex_str = Trim(operand);

    // Check if operand contains commas (comma-separated format)
    if (hex_str.find(',') != std::string::npos) {
        // Comma-separated format: "01,02,03" or "01, 02, 03"
        std::istringstream iss(hex_str);
        std::string token;
        
        while (std::getline(iss, token, ',')) {
            token = Trim(token);  // Remove whitespace around token
            if (!token.empty()) {
                bytes.push_back(static_cast<uint8_t>(std::stoul(token, nullptr, 16)));
            }
        }
    } else {
        // Concatenated format: "010203" or "AB CD EF"
        // Remove all spaces from hex string
        hex_str.erase(std::remove_if(hex_str.begin(), hex_str.end(), ::isspace), hex_str.end());

        // Parse pairs of hex digits
        for (size_t i = 0; i + 1 < hex_str.length(); i += 2) {
            std::string byte_str = hex_str.substr(i, 2);
            bytes.push_back(static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16)));
        }
    }

    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    current_address_ += bytes.size();
}

void MerlinSyntaxParser::HandleDS(const std::string& operand, Section& section,
                                   ConcreteSymbolTable& symbols) {
    // DS (Define Space) - reserve bytes
    // Can be a number, symbol, or expression
    std::string op = Trim(operand);
    uint32_t count = 0;

    // Check for empty operand
    if (op.empty()) {
        count = 0;
    }
    // Check for multiplication expression first (before trying to parse as simple number)
    else if (op.find('*') != std::string::npos) {
        size_t mult_pos = op.find('*');
        std::string left = Trim(op.substr(0, mult_pos));
        std::string right = Trim(op.substr(mult_pos + 1));
        
        // Check for empty operands
        if (left.empty() || right.empty()) {
            throw std::runtime_error(FormatError("DS: Multiplication requires operands on both sides"));
        }


        // Evaluate left side
        uint32_t left_val = 0;
        if (left[0] == '$' || left[0] == '%' || std::isdigit(left[0])) {
            left_val = ParseNumber(left);
        } else {
            // Symbol lookup
            int64_t value = 0;
            if (symbols.Lookup(left, value)) {
                left_val = static_cast<uint32_t>(value);
            } else {
                throw std::runtime_error(FormatError("DS: Undefined symbol: " + left));
            }
        }

        // Evaluate right side
        uint32_t right_val = 0;
        if (right[0] == '$' || right[0] == '%' || std::isdigit(right[0])) {
            right_val = ParseNumber(right);
        } else {
            // Symbol lookup
            int64_t value = 0;
            if (symbols.Lookup(right, value)) {
                right_val = static_cast<uint32_t>(value);
            } else {
                throw std::runtime_error(FormatError("DS: Undefined symbol: " + right));
            }
        }
        
        count = left_val * right_val;
    }
    // Simple number (no operators)
    else if (op[0] == '$' || op[0] == '%' || std::isdigit(op[0])) {
        count = ParseNumber(op);
    }
    // Symbol reference
    else {
        int64_t value = 0;
        if (symbols.Lookup(op, value)) {
            count = static_cast<uint32_t>(value);
        } else {
            throw std::runtime_error(FormatError("DS: Undefined symbol: " + op));
        }
    }
    
    section.atoms.push_back(std::make_shared<SpaceAtom>(count));
    
    // Update address if not in DUM block
    if (!in_dum_block_) {
        current_address_ += count;
    } else {
        dum_address_ += count;
    }
}

void MerlinSyntaxParser::HandleDum(const std::string& operand, ConcreteSymbolTable& symbols) {
    // DUM (Dummy section) - start variable definition block
    in_dum_block_ = true;
    
    std::string op = Trim(operand);
    
    // Check if operand is empty
    if (op.empty()) {
        throw std::runtime_error(FormatError("DUM directive requires an address operand"));
    }
    
    // Check if operand is a symbol or a number
    if (op[0] == '$' || op[0] == '%' || std::isdigit(op[0])) {
        // It's a number
        dum_address_ = ParseNumber(op);
    } else {
        // It's a symbol - look it up
        int64_t value = 0;
        if (symbols.Lookup(op, value)) {
            dum_address_ = static_cast<uint32_t>(value);
        } else {
            // Symbol not found - use 0 for now (forward reference issue)
            dum_address_ = 0;
        }
    }
}

void MerlinSyntaxParser::HandleDend() {
    // DEND - end dummy section
    // TODO: Phase 3 implementation
    in_dum_block_ = false;
    variable_labels_.clear();
}

void MerlinSyntaxParser::HandlePut(const std::string& operand, Section& section,
                                   ConcreteSymbolTable& symbols) {
    // PUT filename - include another source file
    std::string filename = Trim(operand);
    
    // Auto-append .S extension if no extension present
    // Check if filename contains a dot (has extension)
    if (filename.find('.') == std::string::npos) {
        filename += ".S";
    }
    
    // Check for circular includes
    for (const auto& included_file : include_stack_) {
        if (included_file == filename) {
            throw std::runtime_error(FormatError("Circular include detected: " + filename));
        }
    }
    
    // Add to include stack
    include_stack_.push_back(filename);
    
    // Try to open the file - first as given, then with /tmp/ prefix
    std::ifstream file(filename);
    std::string actual_filename = filename;
    
    if (!file.is_open() && filename[0] != '/') {
        // Try with /tmp/ prefix for relative paths
        actual_filename = "/tmp/" + filename;
        file.open(actual_filename);
    }
    
    if (!file.is_open()) {
        include_stack_.pop_back();  // Remove from stack on error
        throw std::runtime_error(FormatError("Cannot open file: " + Trim(operand)));  // Report original filename in error
    }
    
    // Read entire file content
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string file_content = buffer.str();
    file.close();
    
    // Parse the included file content
    std::istringstream iss(file_content);
    std::string line;
    while (std::getline(iss, line)) {
        ParseLine(line, section, symbols);
    }
    
    // Remove from include stack when done
    include_stack_.pop_back();
}

void MerlinSyntaxParser::HandleDo(const std::string& operand, ConcreteSymbolTable& symbols) {
    // DO condition - start conditional assembly block
    auto expr = ParseExpression(operand, symbols);
    int64_t condition = expr->Evaluate(symbols);
    
    // Determine if code should be emitted based on parent blocks and current condition
    bool parent_should_emit = conditional_stack_.empty() ? true : 
                               conditional_stack_.back().should_emit;
    bool should_emit = parent_should_emit && (condition != 0);
    
    conditional_stack_.push_back({
        condition != 0,  // condition
        false,           // in_else_block
        should_emit      // should_emit
    });
}

void MerlinSyntaxParser::HandleElse() {
    // ELSE - switch to alternative branch in conditional block
    if (conditional_stack_.empty()) {
        throw std::runtime_error(FormatError("ELSE without matching DO"));
    }
    
    ConditionalBlock& block = conditional_stack_.back();
    if (block.in_else_block) {
        throw std::runtime_error(FormatError("Multiple ELSE in same DO block"));
    }
    
    block.in_else_block = true;
    
    // Determine if code in ELSE branch should be emitted
    bool parent_should_emit = conditional_stack_.size() > 1 ? 
                               conditional_stack_[conditional_stack_.size()-2].should_emit : true;
    block.should_emit = parent_should_emit && !block.condition;
}

void MerlinSyntaxParser::HandleFin() {
    // FIN - end conditional assembly block
    if (conditional_stack_.empty()) {
        throw std::runtime_error(FormatError("FIN without matching DO"));
    }
    
    conditional_stack_.pop_back();
}

void MerlinSyntaxParser::HandleLst(const std::string& operand) {
    // LST/LST OFF - listing control directives
    // These control assembler output, not code generation
    // No atoms created
    (void)operand;  // Unused parameter
}

void MerlinSyntaxParser::HandleLstdo() {
    // LSTDO - list during DO blocks
    // Listing control directive
    // No atoms created
}

void MerlinSyntaxParser::HandleTr(const std::string& operand) {
    // TR [ADR|ON|OFF] - truncate listing
    // Listing control directive
    // No atoms created
    (void)operand;  // Unused parameter
}

void MerlinSyntaxParser::HandleAsc(const std::string& operand, Section& section) {
    // ASC 'string' or ASC "string" - ASCII string directive
    // Optionally followed by high bit modifier: ASC 'text'80
    
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
    
    // Check for high bit modifier (e.g., '80' after closing quote)
    bool set_high_bit = false;
    if (end_pos + 1 < op.length()) {
        std::string suffix = Trim(op.substr(end_pos + 1));
        if (suffix == "80") {
            set_high_bit = true;
        }
    }
    
    // Convert string to bytes
    for (size_t i = 0; i < text.length(); ++i) {
        uint8_t byte = static_cast<uint8_t>(text[i]);
        
        // Set high bit on last character if modifier present
        if (set_high_bit && i == text.length() - 1) {
            byte |= 0x80;
        }
        
        bytes.push_back(byte);
    }
    
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    current_address_ += bytes.size();
}

// ============================================================================
// Line Parsing
// ============================================================================

void MerlinSyntaxParser::ParseLine(const std::string& line, Section& section,
                                   ConcreteSymbolTable& symbols) {
    // Strip comments first
    std::string code_line = StripComments(line);
    
    // Check if line is empty after stripping comments
    std::string trimmed = Trim(code_line);
    if (trimmed.empty()) {
        return;
    }

    // Check for conditional assembly directives (DO/ELSE/FIN)
    // These must be processed even when inside a false conditional block
    std::string upper_trimmed = ToUpper(trimmed);
    if (upper_trimmed.find("DO ") == 0 || upper_trimmed == "DO") {
        // Extract operand after "DO"
        std::string operand = trimmed.length() > 3 ? Trim(trimmed.substr(3)) : "0";
        HandleDo(operand, symbols);
        return;
    } else if (upper_trimmed == "ELSE") {
        HandleElse();
        return;
    } else if (upper_trimmed == "FIN") {
        HandleFin();
        return;
    }

    // Check if we should skip this line due to conditional assembly
    if (!conditional_stack_.empty() && !conditional_stack_.back().should_emit) {
        return;  // Skip this line - we're in a false conditional block
    }

    // Parse label (if present) - use original (untrimmed) code_line
    // Labels must start in column 1, so leading whitespace means no label
    size_t pos = 0;
    std::string label = ParseLabel(code_line, pos, section, symbols);

    // Skip whitespace after label
    while (pos < code_line.length() && std::isspace(code_line[pos])) {
        pos++;
    }

    // If only label on line (no directive/instruction)
    if (pos >= code_line.length()) {
        if (!label.empty()) {
            // Create label atom and define symbol
            symbols.Define(label, SymbolType::Label,
                          std::make_shared<LiteralExpr>(current_address_));
            section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
            
            // Update current scope for local labels
            current_scope_.global_label = label;
            current_scope_.local_labels.clear();
        }
        return;
    }

    // Check for = syntax (alternate EQU) first
    // Format: LABEL = VALUE
    size_t equals_pos = code_line.find('=', pos);
    if (equals_pos != std::string::npos && !label.empty()) {
        // This is an = equate
        std::string value = Trim(code_line.substr(equals_pos + 1));
        HandleEqu(label, value, symbols);
        return;
    }

    // Extract directive/instruction and operands
    size_t space_pos = code_line.find(' ', pos);
    std::string directive;
    std::string operands;

    if (space_pos != std::string::npos) {
        directive = ToUpper(Trim(code_line.substr(pos, space_pos - pos)));
        operands = Trim(code_line.substr(space_pos + 1));
    } else {
        directive = ToUpper(Trim(code_line.substr(pos)));
        operands = "";
    }

    // Handle directives
    if (directive == "ORG") {
        HandleOrg(operands, section, symbols);
        return;
    } else if (directive == "EQU") {
        // EQU: Define symbol but don't create label atom
        HandleEqu(label, operands, symbols);
        return;
    } else if (directive == "DB" || directive == "DFB") {
        // DB/DFB (Define Byte / Define Font Byte) - emit byte data
        // DFB is Merlin's name for DB (used in Prince of Persia source)
        // Create label atom first if label present
        if (!label.empty()) {
            symbols.Define(label, SymbolType::Label,
                          std::make_shared<LiteralExpr>(current_address_));
            section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
            current_scope_.global_label = label;
            current_scope_.local_labels.clear();
        }
        HandleDB(operands, section, symbols);
        return;
    } else if (directive == "DW") {
        // Create label atom first if label present
        if (!label.empty()) {
            symbols.Define(label, SymbolType::Label,
                          std::make_shared<LiteralExpr>(current_address_));
            section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
            current_scope_.global_label = label;
            current_scope_.local_labels.clear();
        }
        HandleDW(operands, section, symbols);
        return;
    } else if (directive == "HEX") {
        // Create label atom first if label present
        if (!label.empty()) {
            symbols.Define(label, SymbolType::Label,
                          std::make_shared<LiteralExpr>(current_address_));
            section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
            current_scope_.global_label = label;
            current_scope_.local_labels.clear();
        }
        HandleHex(operands, section);
        return;
    } else if (directive == "DS") {
        // Create label atom first if label present
        if (!label.empty()) {
            symbols.Define(label, SymbolType::Label,
                          std::make_shared<LiteralExpr>(current_address_));
            section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
            current_scope_.global_label = label;
            current_scope_.local_labels.clear();
        }
        HandleDS(operands, section, symbols);
        return;
    } else if (directive == "DUM") {
        HandleDum(operands, symbols);
        return;
    } else if (directive == "DEND") {
        HandleDend();
        return;
    } else if (directive == "PUT") {
        HandlePut(operands, section, symbols);
        return;
    } else if (directive == "DO") {
        HandleDo(operands, symbols);
        return;
    } else if (directive == "ELSE") {
        HandleElse();
        return;
    } else if (directive == "FIN") {
        HandleFin();
        return;
    } else if (directive == "LST") {
        HandleLst(operands);
        return;
    } else if (directive == "LSTDO") {
        HandleLstdo();
        return;
    } else if (directive == "TR") {
        HandleTr(operands);
        return;
    } else if (directive == "ASC") {
        // Create label atom first if label present
        if (!label.empty()) {
            symbols.Define(label, SymbolType::Label,
                          std::make_shared<LiteralExpr>(current_address_));
            section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
            current_scope_.global_label = label;
            current_scope_.local_labels.clear();
        }
        HandleAsc(operands, section);
        return;
    } else if (directive == "USR") {
        // USR directive - external subroutine call (translates to JSR)
        // Create label atom first if label present
        if (!label.empty()) {
            symbols.Define(label, SymbolType::Label,
                          std::make_shared<LiteralExpr>(current_address_));
            section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
            current_scope_.global_label = label;
            current_scope_.local_labels.clear();
        }
        // Translate USR to JSR instruction
        section.atoms.push_back(std::make_shared<InstructionAtom>("JSR", operands));
        current_address_ += 3;  // JSR is 3 bytes (opcode + 2-byte address)
        return;
    } else {
        // Assume it's an instruction
        // Create label atom first if label present
        if (!label.empty()) {
            symbols.Define(label, SymbolType::Label,
                          std::make_shared<LiteralExpr>(current_address_));
            section.atoms.push_back(std::make_shared<LabelAtom>(label, current_address_));
            current_scope_.global_label = label;
            current_scope_.local_labels.clear();
        }
        section.atoms.push_back(std::make_shared<InstructionAtom>(directive, operands));
        current_address_ += 1;  // Placeholder size
    }
}

// ============================================================================
// Main Parse Function
// ============================================================================

void MerlinSyntaxParser::Parse(const std::string& source, Section& section,
                               ConcreteSymbolTable& symbols) {
    if (source.empty()) {
        return;
    }

    // Reset parser state
    current_address_ = 0;
    in_dum_block_ = false;
    dum_address_ = 0;
    current_scope_.global_label.clear();
    current_scope_.local_labels.clear();
    variable_labels_.clear();
    current_line_ = 0;  // Reset line counter

    // Split into lines and parse
    std::istringstream iss(source);
    std::string line;

    while (std::getline(iss, line)) {
        current_line_++;  // Increment line counter for each line
        ParseLine(line, section, symbols);
    }
    
    // Validate that all DO blocks are closed
    if (!conditional_stack_.empty()) {
        throw std::runtime_error(FormatError("Unmatched DO directive (missing FIN)"));
    }
}

} // namespace xasm
