# API Specification: Parsing Component Interfaces

**Date:** 2026-02-09  
**Version:** 1.0  
**Related:** [Architecture Document](architecture.md)

---

## Overview

This document specifies the API contracts for the parsing component system that will replace the god-class syntax parsers. These interfaces enable component-based parsing with clear separation of concerns and testability.

---

## Core Interfaces

### 1. ILexer Interface

**Purpose:** Tokenize source lines into structured components

**Header:** `include/xasm++/parsing/lexer.h`

```cpp
#pragma once

#include <optional>
#include <string>
#include <vector>

namespace xasm {
namespace parsing {

/**
 * @brief Source location for error reporting
 */
struct SourceLocation {
    std::string filename;
    int line_number;
    int column;
    
    std::string ToString() const;
};

/**
 * @brief Tokenized line components
 */
struct LineTokens {
    std::optional<std::string> label;       ///< Label field (if present)
    std::optional<std::string> mnemonic;    ///< Instruction/directive
    std::optional<std::string> operand;     ///< Operand field
    std::optional<std::string> comment;     ///< Comment (if present)
    SourceLocation location;                ///< Source location
    
    bool HasLabel() const { return label.has_value(); }
    bool HasMnemonic() const { return mnemonic.has_value(); }
    bool HasOperand() const { return operand.has_value(); }
    bool IsEmptyLine() const { return !HasMnemonic(); }
};

/**
 * @brief Lexer interface for syntax-specific tokenization
 * 
 * Implementations handle syntax-specific rules:
 * - Comment styles (; * .COMMENT)
 * - Label formats (LABEL:, LABEL::, :LOCAL, $LOCAL, ]VAR)
 * - Line number prefixes (SCMASM)
 * - Whitespace rules (fixed-column vs. free-form)
 */
class ILexer {
public:
    virtual ~ILexer() = default;
    
    /**
     * @brief Tokenize a line into structured components
     * 
     * @param line Raw source line
     * @param location Source location for error reporting
     * @return Tokenized components
     * @throws AssemblerError on malformed input
     */
    virtual LineTokens Tokenize(const std::string& line, 
                               const SourceLocation& location) = 0;
    
    /**
     * @brief Strip comments from line (preprocessing)
     * 
     * @param line Raw source line
     * @return Line with comments removed
     */
    virtual std::string StripComments(const std::string& line) = 0;
    
    /**
     * @brief Normalize whitespace and formatting
     * 
     * @param line Input line
     * @return Normalized line
     */
    virtual std::string Normalize(const std::string& line) = 0;
    
    /**
     * @brief Check if line is empty or comment-only
     * 
     * @param line Input line
     * @return true if line should be skipped
     */
    virtual bool IsEmptyLine(const std::string& line) = 0;
    
    /**
     * @brief Get comment character for this syntax
     * 
     * @return Primary comment character (e.g., ';')
     */
    virtual char GetCommentChar() const = 0;
};

}} // namespace xasm::parsing
```

---

### 2. IParser Interface

**Purpose:** Parse tokenized lines into structured elements

**Header:** `include/xasm++/parsing/parser.h`

```cpp
#pragma once

#include "lexer.h"
#include <optional>
#include <string>
#include <vector>

namespace xasm {
namespace parsing {

/**
 * @brief Label type classification
 */
enum class LabelType {
    Global,      ///< Normal global label
    Local,       ///< Local label (scoped to previous global)
    Variable,    ///< DUM block variable (Merlin ]VAR)
    Private,     ///< Private label (assembly-unit scope)
    Public,      ///< Public label (exported)
};

/**
 * @brief Parsed label information
 */
struct ParsedLabel {
    std::string name;         ///< Label name (without prefix/suffix)
    std::string raw_name;     ///< Original label text
    LabelType type;           ///< Label classification
    bool is_public;           ///< Public export flag (:: suffix)
    SourceLocation location;  ///< Source location
};

/**
 * @brief Parsed operand information
 */
struct ParsedOperand {
    std::string raw_text;                           ///< Original operand text
    std::vector<std::string> expressions;           ///< Comma-separated expressions
    std::unordered_map<std::string, std::string> attributes;  ///< Syntax-specific attributes
    SourceLocation location;
};

/**
 * @brief Complete parsed line
 */
struct ParsedLine {
    std::optional<ParsedLabel> label;
    std::string mnemonic;           ///< Instruction or directive
    ParsedOperand operand;
    SourceLocation location;
};

/**
 * @brief Parser interface for syntax-specific parsing
 */
class IParser {
public:
    virtual ~IParser() = default;
    
    /**
     * @brief Parse label field
     * 
     * Handles syntax-specific label formats:
     * - Global: LABEL, LABEL:, LABEL::
     * - Local: :LOCAL, .LOCAL, $LOCAL, ?TEMP
     * - Variable: ]VAR
     * 
     * @param label_text Label field from tokenizer
     * @param location Source location
     * @return Parsed label information
     * @throws AssemblerError on invalid label format
     */
    virtual ParsedLabel ParseLabel(const std::string& label_text,
                                   const SourceLocation& location) = 0;
    
    /**
     * @brief Parse operand field
     * 
     * Splits comma-separated expressions, handles quotes,
     * detects addressing modes (if applicable).
     * 
     * @param operand_text Operand field from tokenizer
     * @param location Source location
     * @return Parsed operand information
     * @throws AssemblerError on invalid operand format
     */
    virtual ParsedOperand ParseOperand(const std::string& operand_text,
                                      const SourceLocation& location) = 0;
    
    /**
     * @brief Validate syntax-specific rules
     * 
     * Check label/mnemonic combinations, operand requirements, etc.
     * 
     * @param line Parsed line to validate
     * @throws AssemblerError on validation failure
     */
    virtual void ValidateLine(const ParsedLine& line) = 0;
    
    /**
     * @brief Check if label is local format
     * 
     * @param label_text Label to check
     * @return true if local label format
     */
    virtual bool IsLocalLabel(const std::string& label_text) const = 0;
    
    /**
     * @brief Check if label is variable format
     * 
     * @param label_text Label to check
     * @return true if variable label format (Merlin ]VAR)
     */
    virtual bool IsVariableLabel(const std::string& label_text) const = 0;
};

}} // namespace xasm::parsing
```

---

### 3. IDirectiveHandler Interface

**Purpose:** Handle individual directive processing

**Header:** `include/xasm++/parsing/directive_handler.h`

```cpp
#pragma once

#include "parser.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <string>
#include <vector>

namespace xasm {
namespace parsing {

// Forward declarations
class SegmentManager;
class LabelScopeManager;
class MacroProcessor;
class ConditionalStack;

/**
 * @brief Directive execution context
 * 
 * Provides access to all parser state needed by directive handlers.
 */
struct DirectiveContext {
    Section& section;                     ///< Current section
    ConcreteSymbolTable& symbols;         ///< Symbol table
    SegmentManager& segments;             ///< Segment state manager
    LabelScopeManager& scopes;            ///< Label scoping
    MacroProcessor& macros;               ///< Macro processor
    ConditionalStack& conditionals;       ///< Conditional assembly stack
    uint32_t& current_address;            ///< Current address (mutable)
    
    // Optional CPU plugin (for mode switching, etc.)
    void* cpu_plugin = nullptr;           ///< CPU-specific plugin (cast as needed)
};

/**
 * @brief Directive metadata
 */
struct DirectiveInfo {
    std::string name;                     ///< Primary directive name
    std::vector<std::string> aliases;     ///< Alternative names
    std::string description;              ///< Human-readable description
    bool requires_label;                  ///< True if label is required
    bool requires_operand;                ///< True if operand is required
};

/**
 * @brief Base interface for directive handlers (Strategy Pattern)
 * 
 * Each directive is implemented as a separate handler class.
 * This enables:
 * - Single Responsibility Principle
 * - Open/Closed Principle (add directives without modifying parser)
 * - Independent testing of each directive
 */
class IDirectiveHandler {
public:
    virtual ~IDirectiveHandler() = default;
    
    /**
     * @brief Execute directive
     * 
     * @param line Parsed line with label, operand
     * @param context Execution context (section, symbols, etc.)
     * @throws AssemblerError on directive-specific errors
     */
    virtual void Handle(const ParsedLine& line, 
                       DirectiveContext& context) = 0;
    
    /**
     * @brief Get directive metadata
     * 
     * @return Directive information for documentation/validation
     */
    virtual DirectiveInfo GetInfo() const = 0;
};

/**
 * @brief Directive dispatcher (registry pattern)
 * 
 * Routes directive calls to appropriate handlers.
 */
class DirectiveDispatcher {
public:
    /**
     * @brief Register a directive handler
     * 
     * @param name Directive name (case-insensitive)
     * @param handler Handler implementation (takes ownership)
     */
    void RegisterDirective(const std::string& name,
                          std::unique_ptr<IDirectiveHandler> handler);
    
    /**
     * @brief Register multiple aliases for same handler
     * 
     * Example: DB, DEFB, BYTE all map to same handler
     * 
     * @param names List of directive names/aliases
     * @param handler Handler implementation (takes ownership)
     */
    void RegisterAliases(const std::vector<std::string>& names,
                        std::unique_ptr<IDirectiveHandler> handler);
    
    /**
     * @brief Check if mnemonic is a directive
     * 
     * @param mnemonic Potential directive name
     * @return true if registered as directive
     */
    bool IsDirective(const std::string& mnemonic) const;
    
    /**
     * @brief Dispatch directive to handler
     * 
     * @param line Parsed line with directive and operand
     * @param context Execution context
     * @throws AssemblerError if directive not found or handler fails
     */
    void HandleDirective(const ParsedLine& line,
                        DirectiveContext& context);
    
    /**
     * @brief Get list of registered directives
     * 
     * @return Vector of directive names (for help/documentation)
     */
    std::vector<std::string> GetRegisteredDirectives() const;
    
private:
    std::unordered_map<std::string, std::unique_ptr<IDirectiveHandler>> handlers_;
};

}} // namespace xasm::parsing
```

---

### 4. Expression Evaluator API

**Purpose:** Parse and evaluate expressions (shared across all parsers)

**Header:** `include/xasm++/parsing/expression_evaluator.h`

```cpp
#pragma once

#include "xasm++/expression.h"
#include "xasm++/symbol.h"
#include <functional>
#include <memory>
#include <string>

namespace xasm {
namespace parsing {

/**
 * @brief Number format parser callback
 * 
 * Allows syntax-specific number parsing while reusing expression logic.
 */
using NumberParser = std::function<uint32_t(const std::string&)>;

/**
 * @brief Expression evaluator (shared component)
 * 
 * Parses expressions with operator precedence and evaluates them.
 * Syntax-agnostic except for number format parsing (injected).
 */
class ExpressionEvaluator {
public:
    /**
     * @brief Constructor
     * 
     * @param number_parser Syntax-specific number parser
     */
    explicit ExpressionEvaluator(NumberParser number_parser);
    
    /**
     * @brief Parse expression string into expression tree
     * 
     * Handles:
     * - Arithmetic: +, -, *, /, %
     * - Bitwise: &, |, ^, <<, >>
     * - Logical: &&, ||
     * - Comparison: ==, !=, <, >, <=, >=
     * - Unary: -, ~, !
     * - Functions: <, >, ^ (lo/hi/bank byte)
     * - Parentheses for grouping
     * 
     * @param expr Expression string
     * @param symbols Symbol table for symbol references
     * @return Expression tree
     * @throws AssemblerError on parse error
     */
    std::shared_ptr<Expression> Parse(const std::string& expr,
                                     ConcreteSymbolTable& symbols);
    
    /**
     * @brief Evaluate expression to value
     * 
     * @param expr Expression tree
     * @param symbols Symbol table for resolution
     * @param current_address Current address (for $ symbol)
     * @return Evaluated value
     * @throws AssemblerError on undefined symbol or evaluation error
     */
    int64_t Evaluate(const std::shared_ptr<Expression>& expr,
                    ConcreteSymbolTable& symbols,
                    uint32_t current_address);
    
    /**
     * @brief Check if expression is constant (no symbols)
     * 
     * @param expr Expression tree
     * @return true if expression has no symbol references
     */
    bool IsConstant(const std::shared_ptr<Expression>& expr) const;
    
    /**
     * @brief Parse and evaluate in one call (convenience)
     * 
     * @param expr Expression string
     * @param symbols Symbol table
     * @param current_address Current address
     * @return Evaluated value
     * @throws AssemblerError on parse or evaluation error
     */
    int64_t ParseAndEvaluate(const std::string& expr,
                            ConcreteSymbolTable& symbols,
                            uint32_t current_address);
    
private:
    NumberParser number_parser_;  ///< Syntax-specific number parsing
    
    // Operator precedence parsing (recursive descent)
    std::shared_ptr<Expression> ParseLogicalOr(const std::string& str, size_t& pos,
                                              ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseLogicalAnd(const std::string& str, size_t& pos,
                                               ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseBitwiseOr(const std::string& str, size_t& pos,
                                              ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseBitwiseXor(const std::string& str, size_t& pos,
                                               ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseBitwiseAnd(const std::string& str, size_t& pos,
                                               ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseComparison(const std::string& str, size_t& pos,
                                               ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseShift(const std::string& str, size_t& pos,
                                          ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseAddSubtract(const std::string& str, size_t& pos,
                                                ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseMultiplyDivide(const std::string& str, size_t& pos,
                                                   ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseUnary(const std::string& str, size_t& pos,
                                          ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParsePrimary(const std::string& str, size_t& pos,
                                            ConcreteSymbolTable& symbols);
    
    // Helper: Skip whitespace
    void SkipWhitespace(const std::string& str, size_t& pos);
};

}} // namespace xasm::parsing
```

---

### 5. Macro Processor API

**Purpose:** Handle macro definition, expansion, and parameter substitution

**Header:** `include/xasm++/parsing/macro_processor.h`

```cpp
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace xasm {
namespace parsing {

/**
 * @brief Repetition block type
 */
enum class ReptType {
    REPT,   ///< REPT count - Repeat N times
    IRP,    ///< IRP param,<list> - Iterate over values
    IRPC,   ///< IRPC param,<string> - Iterate over characters
};

/**
 * @brief Macro processor (state machine for macro handling)
 */
class MacroProcessor {
public:
    MacroProcessor();
    
    /**
     * @brief Begin macro definition
     * 
     * @param name Macro name
     * @param params Parameter names
     * @throws AssemblerError if already in macro definition
     */
    void BeginMacro(const std::string& name,
                   const std::vector<std::string>& params);
    
    /**
     * @brief Add line to current macro body
     * 
     * @param line Source line (unexpanded)
     * @throws AssemblerError if not in macro definition
     */
    void AddMacroLine(const std::string& line);
    
    /**
     * @brief Declare LOCAL symbols in current macro
     * 
     * LOCAL symbols are unique per macro invocation.
     * 
     * @param symbols Local symbol names
     * @throws AssemblerError if not in macro definition
     */
    void DeclareLocalSymbols(const std::vector<std::string>& symbols);
    
    /**
     * @brief End macro definition
     * 
     * @throws AssemblerError if not in macro definition
     */
    void EndMacro();
    
    /**
     * @brief Check if macro is defined
     * 
     * @param name Macro name (case-insensitive)
     * @return true if macro exists
     */
    bool IsMacroDefined(const std::string& name) const;
    
    /**
     * @brief Expand macro with arguments
     * 
     * Substitutes parameters and generates unique local labels.
     * 
     * @param name Macro name
     * @param args Argument values (must match parameter count)
     * @return Expanded lines ready for parsing
     * @throws AssemblerError if macro not found or arg count mismatch
     */
    std::vector<std::string> ExpandMacro(const std::string& name,
                                        const std::vector<std::string>& args);
    
    /**
     * @brief Begin REPT block (repeat N times)
     * 
     * @param count Number of repetitions
     * @throws AssemblerError if already in definition
     */
    void BeginRept(int count);
    
    /**
     * @brief Begin IRP block (iterate over value list)
     * 
     * @param param Parameter name to substitute
     * @param values Values to iterate over
     * @throws AssemblerError if already in definition
     */
    void BeginIrp(const std::string& param,
                 const std::vector<std::string>& values);
    
    /**
     * @brief Begin IRPC block (iterate over string characters)
     * 
     * @param param Parameter name to substitute
     * @param str String to iterate over
     * @throws AssemblerError if already in definition
     */
    void BeginIrpc(const std::string& param,
                  const std::string& str);
    
    /**
     * @brief Add line to current REPT/IRP/IRPC body
     * 
     * @param line Source line (unexpanded)
     * @throws AssemblerError if not in rept definition
     */
    void AddReptLine(const std::string& line);
    
    /**
     * @brief End and expand REPT/IRP/IRPC block
     * 
     * @return Expanded lines ready for parsing
     * @throws AssemblerError if not in rept definition
     */
    std::vector<std::string> EndRept();
    
    /**
     * @brief Handle EXITM (early exit from macro)
     * 
     * Sets flag to stop macro expansion.
     */
    void ExitMacro();
    
    /**
     * @brief Check if EXITM was called
     * 
     * @return true if macro expansion should stop
     */
    bool IsExitRequested() const;
    
    /**
     * @brief Clear EXITM flag (after macro expansion)
     */
    void ClearExitFlag();
    
    /**
     * @brief State queries
     */
    bool IsInMacroDefinition() const;
    bool IsInReptDefinition() const;
    int GetExpansionDepth() const;
    
    /**
     * @brief Reset all state (for new file)
     */
    void Reset();
    
private:
    struct MacroDefinition {
        std::string name;
        std::vector<std::string> params;
        std::vector<std::string> body;
        std::vector<std::string> locals;
    };
    
    struct ReptBlock {
        ReptType type;
        std::vector<std::string> body;
        int count;
        std::string param_name;
        std::vector<std::string> values;
    };
    
    std::unordered_map<std::string, MacroDefinition> macros_;
    MacroDefinition* current_macro_ = nullptr;
    ReptBlock* current_rept_ = nullptr;
    int expansion_depth_ = 0;
    int macro_invocation_counter_ = 0;
    bool exitm_requested_ = false;
    
    // Maximum recursion depth
    static constexpr int MAX_EXPANSION_DEPTH = 100;
    
    // Parameter substitution
    std::string SubstituteParams(const std::string& line,
                                const std::vector<std::string>& params,
                                const std::vector<std::string>& args);
    
    // Generate unique local label name
    std::string GenerateLocalLabel(const std::string& name, int invocation_id);
};

}} // namespace xasm::parsing
```

---

### 6. Supporting Components

#### Segment Manager

**Header:** `include/xasm++/parsing/segment_manager.h`

```cpp
#pragma once

#include <string>

namespace xasm {
namespace parsing {

enum class SegmentType {
    CODE,      ///< CSEG - Code segment
    DATA,      ///< DSEG - Data segment
    ABSOLUTE,  ///< ASEG - Absolute segment
    COMMON,    ///< COMMON - Common block
};

class SegmentManager {
public:
    void SetCodeSegment();
    void SetDataSegment();
    void SetAbsoluteSegment();
    void SetCommonBlock(const std::string& name);
    void SetOrigin(uint32_t address);
    
    SegmentType GetCurrentSegmentType() const;
    std::string GetCurrentSegmentName() const;
    uint32_t GetCurrentAddress() const;
    bool IsAbsoluteSegment() const;
    
    void AdvanceAddress(size_t bytes);
    void Reset();
    
private:
    SegmentType current_type_ = SegmentType::CODE;
    std::string current_name_;
    uint32_t current_address_ = 0;
    bool has_org_ = false;
};

}} // namespace xasm::parsing
```

#### Conditional Stack

**Header:** `include/xasm++/parsing/conditional_stack.h`

```cpp
#pragma once

#include <vector>

namespace xasm {
namespace parsing {

class ConditionalStack {
public:
    void PushIf(bool condition);
    void HandleElse();
    void PopEndif();
    
    bool ShouldEmitCode() const;
    bool IsInConditional() const;
    int GetNestingDepth() const;
    
    void ValidateEmpty() const;  // At end of file
    
private:
    struct ConditionalBlock {
        bool condition;
        bool in_else_block;
        bool should_emit;
    };
    
    std::vector<ConditionalBlock> stack_;
};

}} // namespace xasm::parsing
```

#### Label Scope Manager

**Header:** `include/xasm++/parsing/label_scope_manager.h`

```cpp
#pragma once

#include <string>
#include <unordered_map>
#include <functional>

namespace xasm {
namespace parsing {

/**
 * @brief Label scope manager
 * 
 * Manages global/local label scoping with syntax-specific rules.
 */
class LabelScopeManager {
public:
    /**
     * @brief Constructor
     * 
     * @param is_local_label Predicate to test if label is local format
     */
    explicit LabelScopeManager(std::function<bool(const std::string&)> is_local_label);
    
    void EnterGlobalLabel(const std::string& label);
    void DefineLocalLabel(const std::string& label, uint32_t address);
    std::string ResolveLocalLabel(const std::string& label) const;
    
    bool IsLocalLabel(const std::string& label) const;
    std::string GetCurrentGlobalLabel() const;
    
    void Reset();
    
private:
    struct LabelScope {
        std::string global_label;
        std::unordered_map<std::string, uint32_t> local_labels;
    };
    
    LabelScope current_scope_;
    std::function<bool(const std::string&)> is_local_label_;
};

}} // namespace xasm::parsing
```

---

## Example Usage

### Implementing Z80 Lexer

```cpp
#include "xasm++/parsing/lexer.h"

namespace xasm {
namespace parsing {

class Z80Lexer : public ILexer {
public:
    LineTokens Tokenize(const std::string& line, 
                       const SourceLocation& location) override {
        LineTokens tokens;
        tokens.location = location;
        
        // Strip comments first
        std::string clean_line = StripComments(line);
        
        // Parse label (if present)
        size_t pos = 0;
        if (!IsWhitespace(clean_line[0])) {
            // Label starts in column 1
            size_t colon = clean_line.find(':');
            if (colon != std::string::npos) {
                tokens.label = clean_line.substr(0, colon);
                pos = colon + 1;
                
                // Check for double colon (public label)
                if (pos < clean_line.size() && clean_line[pos] == ':') {
                    tokens.label = *tokens.label + ":";  // Mark as public
                    pos++;
                }
            }
        }
        
        // Skip whitespace
        while (pos < clean_line.size() && IsWhitespace(clean_line[pos])) {
            pos++;
        }
        
        // Parse mnemonic
        size_t mnemonic_start = pos;
        while (pos < clean_line.size() && !IsWhitespace(clean_line[pos])) {
            pos++;
        }
        if (pos > mnemonic_start) {
            tokens.mnemonic = clean_line.substr(mnemonic_start, pos - mnemonic_start);
        }
        
        // Skip whitespace
        while (pos < clean_line.size() && IsWhitespace(clean_line[pos])) {
            pos++;
        }
        
        // Parse operand (rest of line)
        if (pos < clean_line.size()) {
            tokens.operand = clean_line.substr(pos);
        }
        
        return tokens;
    }
    
    std::string StripComments(const std::string& line) override {
        size_t semi = line.find(';');
        if (semi != std::string::npos) {
            return line.substr(0, semi);
        }
        return line;
    }
    
    std::string Normalize(const std::string& line) override {
        // Trim trailing whitespace
        size_t end = line.find_last_not_of(" \t\r\n");
        if (end == std::string::npos) return "";
        return line.substr(0, end + 1);
    }
    
    bool IsEmptyLine(const std::string& line) override {
        std::string clean = StripComments(line);
        return Normalize(clean).empty();
    }
    
    char GetCommentChar() const override {
        return ';';
    }
    
private:
    bool IsWhitespace(char c) const {
        return c == ' ' || c == '\t';
    }
};

}} // namespace xasm::parsing
```

### Implementing ORG Directive Handler

```cpp
#include "xasm++/parsing/directive_handler.h"

namespace xasm {
namespace parsing {

class OrgDirectiveHandler : public IDirectiveHandler {
public:
    void Handle(const ParsedLine& line, DirectiveContext& context) override {
        // Validate operand exists
        if (line.operand.expressions.empty()) {
            throw AssemblerError("ORG directive requires address operand",
                               line.location);
        }
        
        // Parse and evaluate address expression
        ExpressionEvaluator evaluator(/* number parser */);
        uint32_t address = evaluator.ParseAndEvaluate(
            line.operand.expressions[0],
            context.symbols,
            context.current_address
        );
        
        // Update segment manager
        context.segments.SetOrigin(address);
        context.current_address = address;
        
        // Add ORG atom to section
        context.section.AddAtom(std::make_shared<OrgAtom>(address, line.location));
    }
    
    DirectiveInfo GetInfo() const override {
        return DirectiveInfo{
            .name = "ORG",
            .aliases = {},
            .description = "Set origin address",
            .requires_label = false,
            .requires_operand = true
        };
    }
};

}} // namespace xasm::parsing
```

### Using Components in Parser

```cpp
class Z80UniversalSyntaxParser {
public:
    Z80UniversalSyntaxParser() 
        : lexer_(std::make_unique<Z80Lexer>()),
          parser_(std::make_unique<Z80Parser>()),
          directive_dispatcher_(std::make_unique<DirectiveDispatcher>()),
          expression_evaluator_(std::make_unique<ExpressionEvaluator>(
              [this](const std::string& s) { return ParseZ80Number(s); })),
          macro_processor_(std::make_unique<MacroProcessor>()),
          segment_manager_(std::make_unique<SegmentManager>()),
          conditional_stack_(std::make_unique<ConditionalStack>()),
          label_scope_manager_(std::make_unique<LabelScopeManager>(
              [](const std::string& l) { return l[0] == '.' || l[0] == '$'; }))
    {
        RegisterDirectives();
    }
    
    void Parse(const std::string& source, Section& section, 
              ConcreteSymbolTable& symbols) {
        // Split into lines
        std::istringstream iss(source);
        std::string line;
        int line_number = 0;
        
        while (std::getline(iss, line)) {
            line_number++;
            SourceLocation location{"<input>", line_number, 0};
            
            // Tokenize
            if (lexer_->IsEmptyLine(line)) continue;
            LineTokens tokens = lexer_->Tokenize(line, location);
            
            // Check conditional state
            if (!conditional_stack_->ShouldEmitCode()) {
                // Still process conditional directives
                if (tokens.mnemonic && IsConditionalDirective(*tokens.mnemonic)) {
                    ProcessTokens(tokens, section, symbols);
                }
                continue;
            }
            
            // Check macro/rept state
            if (macro_processor_->IsInMacroDefinition()) {
                if (tokens.mnemonic && *tokens.mnemonic == "ENDM") {
                    macro_processor_->EndMacro();
                } else {
                    macro_processor_->AddMacroLine(line);
                }
                continue;
            }
            
            // Process line
            ProcessTokens(tokens, section, symbols);
        }
    }
    
private:
    void ProcessTokens(const LineTokens& tokens, Section& section,
                      ConcreteSymbolTable& symbols) {
        // Handle label
        if (tokens.label) {
            ParsedLabel label = parser_->ParseLabel(*tokens.label, tokens.location);
            HandleLabel(label, section, symbols);
        }
        
        // Handle mnemonic
        if (tokens.mnemonic) {
            ParsedLine line;
            line.location = tokens.location;
            line.mnemonic = *tokens.mnemonic;
            if (tokens.operand) {
                line.operand = parser_->ParseOperand(*tokens.operand, tokens.location);
            }
            
            if (directive_dispatcher_->IsDirective(line.mnemonic)) {
                DirectiveContext ctx{section, symbols, *segment_manager_, 
                                   /* ... other components ... */};
                directive_dispatcher_->HandleDirective(line, ctx);
            } else {
                HandleInstruction(line, section, symbols);
            }
        }
    }
    
    void RegisterDirectives() {
        directive_dispatcher_->RegisterDirective("ORG",
            std::make_unique<OrgDirectiveHandler>());
        directive_dispatcher_->RegisterDirective("EQU",
            std::make_unique<EquDirectiveHandler>());
        directive_dispatcher_->RegisterAliases({"DB", "DEFB", "BYTE"},
            std::make_unique<DbDirectiveHandler>());
        // ... register all Z80 directives
    }
    
    // Components
    std::unique_ptr<ILexer> lexer_;
    std::unique_ptr<IParser> parser_;
    std::unique_ptr<DirectiveDispatcher> directive_dispatcher_;
    std::unique_ptr<ExpressionEvaluator> expression_evaluator_;
    std::unique_ptr<MacroProcessor> macro_processor_;
    std::unique_ptr<SegmentManager> segment_manager_;
    std::unique_ptr<ConditionalStack> conditional_stack_;
    std::unique_ptr<LabelScopeManager> label_scope_manager_;
};
```

---

## Error Handling

All components throw `AssemblerError` on failures:

```cpp
class AssemblerError : public std::runtime_error {
public:
    AssemblerError(const std::string& message, 
                  const SourceLocation& location)
        : std::runtime_error(FormatError(message, location)),
          location_(location) {}
    
    const SourceLocation& GetLocation() const { return location_; }
    
private:
    SourceLocation location_;
    
    static std::string FormatError(const std::string& msg,
                                  const SourceLocation& loc) {
        return loc.ToString() + ": " + msg;
    }
};
```

---

## Testing Strategy

### Unit Testing Components

Each component is independently testable:

```cpp
TEST(Z80Lexer, TokenizesSimpleInstruction) {
    Z80Lexer lexer;
    SourceLocation loc{"test.asm", 10, 0};
    
    LineTokens tokens = lexer.Tokenize("START:  LD A,$42  ; Load 42", loc);
    
    EXPECT_EQ(tokens.label, "START");
    EXPECT_EQ(tokens.mnemonic, "LD");
    EXPECT_EQ(tokens.operand, "A,$42");
    EXPECT_FALSE(tokens.comment.has_value());
}

TEST(OrgDirectiveHandler, SetsAddress) {
    MockSection section;
    MockSymbolTable symbols;
    MockSegmentManager segments;
    // ... other mocks
    
    DirectiveContext ctx{section, symbols, segments, /* ... */};
    ParsedLine line{
        .mnemonic = "ORG",
        .operand = {.expressions = {"$8000"}},
        .location = {"test.asm", 1, 0}
    };
    
    OrgDirectiveHandler handler;
    handler.Handle(line, ctx);
    
    EXPECT_EQ(segments.GetCurrentAddress(), 0x8000);
}
```

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-02-09 | Initial API specification |

---

**Status:** Proposed  
**Owner:** Architect Agent
