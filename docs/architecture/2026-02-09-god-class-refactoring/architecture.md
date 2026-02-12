# Architecture Design: God-Class Refactoring for Syntax Parsers

**Date:** 2026-02-09  
**Status:** Proposed  
**Priority:** P1 - High (Technical Debt)

---

## Executive Summary

This document proposes a comprehensive refactoring strategy for three syntax parser classes that exhibit severe god-class anti-patterns: `Z80UniversalSyntaxParser` (1,842 LOC), `MerlinSyntaxParser` (1,764 LOC), and `ScmasmSyntaxParser` (1,330 LOC). These classes violate the Single Responsibility Principle by handling multiple distinct concerns including parsing, directive processing, macro expansion, expression evaluation, and state management.

**Refactoring Goal:** Decompose god-classes into focused, cohesive components following SOLID principles while maintaining backward compatibility and test coverage.

**Expected Outcome:**
- Reduced class size: Target <400 LOC per class
- Improved testability: Each component independently testable
- Enhanced maintainability: Single responsibility per class
- Code reuse: Shared components across all syntax parsers
- No breaking changes to public API

---

## Architecture Overview

### Current Architecture (God-Class Anti-Pattern)

```
┌─────────────────────────────────────────────────────────────┐
│         Z80UniversalSyntaxParser (1,842 LOC)                │
│  ┌───────────────────────────────────────────────────────┐  │
│  │ • Parse() - Main entry point                          │  │
│  │ • ParseLine() - Line-level parsing                    │  │
│  │ • ParseLabel() - Label extraction                     │  │
│  │ • StripComments() - Comment handling                  │  │
│  │ • Handle*() - 23 directive handlers                   │  │
│  │ • ExpandMacro() - Macro expansion                     │  │
│  │ • ParseExpression() - 9 expression parsing methods    │  │
│  │ • ParseNumber() - Number format parsing               │  │
│  │ • SegmentManager - Segment switching                  │  │
│  │ • ConditionalStack - IF/ELSE/ENDIF                    │  │
│  │ • MacroState - Macro definition tracking              │  │
│  │ • LabelScope - Local label scoping                    │  │
│  │ • ReptBlock - REPT/IRP/IRPC state                     │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘

Similar structure for:
- MerlinSyntaxParser (1,764 LOC) - 30 directive handlers
- ScmasmSyntaxParser (1,330 LOC) - 13 directive handlers
```

### Proposed Architecture (SOLID Principles)

```
┌──────────────────────────────────────────────────────────────────────┐
│                    Syntax Parser (Coordinating Layer)                │
│                            Target: <300 LOC                          │
│  ┌────────────────────────────────────────────────────────────────┐  │
│  │ • Parse() - Orchestrate parsing pipeline                      │  │
│  │ • ParseLine() - Delegate to appropriate handlers              │  │
│  │ • Compose components via dependency injection                 │  │
│  └────────────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────────────┘
                                    │
           ┌────────────────────────┼────────────────────────┐
           │                        │                        │
    ┌──────▼──────┐         ┌──────▼───────┐        ┌──────▼──────┐
    │   Lexer     │         │   Parser     │        │  Directive  │
    │  Component  │         │  Component   │        │  Dispatcher │
    │  ~150 LOC   │         │  ~200 LOC    │        │  ~100 LOC   │
    └──────┬──────┘         └──────┬───────┘        └──────┬──────┘
           │                        │                        │
    • Tokenization           • Label parsing         • Route to handlers
    • Comment stripping      • Operand extraction    • Validate directives
    • Line normalization     • Syntax validation     
                                                             │
                    ┌────────────────────────────────────────┘
                    │
    ┌───────────────┴────────────────────┬──────────────────────┐
    │                                    │                      │
┌───▼────────────┐            ┌─────────▼────────┐   ┌─────────▼────────┐
│   Directive    │            │    Expression    │   │     Macro        │
│   Handlers     │            │    Evaluator     │   │   Processor      │
│  ~600 LOC      │            │    ~300 LOC      │   │   ~400 LOC       │
│  (Registry)    │            └──────────────────┘   └──────────────────┘
└────────────────┘                     │                      │
│                                      │                      │
├─ OrgHandler                  • Operator precedence    • Macro definition
├─ EquHandler                  • Symbol resolution      • Parameter subst.
├─ DataHandler (DB/DW/...)     • Expression tree        • Local symbols
├─ MacroHandler                • Arithmetic ops         • REPT/IRP/IRPC
├─ ConditionalHandler          • Bitwise ops            • Expansion
└─ SegmentHandler              • Functions (HIGH/LOW)   • Recursion guard

    ┌─────────────────┬─────────────────┬─────────────────┐
    │                 │                 │                 │
┌───▼─────────┐  ┌────▼─────────┐  ┌───▼──────────┐  ┌──▼───────────┐
│   Segment   │  │ Conditional  │  │ Label Scope  │  │   Symbol     │
│   Manager   │  │   Stack      │  │   Manager    │  │   Helper     │
│  ~150 LOC   │  │   ~100 LOC   │  │   ~100 LOC   │  │   ~80 LOC    │
└─────────────┘  └──────────────┘  └──────────────┘  └──────────────┘
│                │                 │                 │
• CSEG/DSEG/ASEG • IF/ELSE/ENDIF  • Global/local   • Public/extern
• ORG tracking   • Nesting depth   • Scoping rules  • Symbol creation
• Address calc   • Condition eval  • Label lookup   • Type tracking
```

---

## System Context

### Systems Involved

**Existing Systems:**
- **Core Assembler Engine** - Multi-pass resolution, symbol table management
- **CPU Plugin Interface** - Instruction encoding and sizing
- **Section Management** - Code/data/bss section tracking
- **Expression System** - Expression tree evaluation

**New Systems (Proposed):**
- **Directive Registry** - Pluggable directive handler system
- **Parser Component System** - Modular parsing pipeline
- **Shared Utilities** - Common parsing utilities across all syntax parsers

### External Dependencies

**Existing:**
- `xasm++/expression.h` - Expression tree classes
- `xasm++/section.h` - Section and atom management
- `xasm++/symbol.h` - Symbol table interface
- `xasm++/cpu/*.h` - CPU plugin interfaces

**New:**
- `xasm++/parsing/*.h` - Parsing component interfaces
- `xasm++/directives/*.h` - Directive handler interfaces

---

## Component Architecture

### 1. Lexer Component

**Responsibility:** Tokenization and preprocessing

**Interface:**
```cpp
class ILexer {
public:
    virtual ~ILexer() = default;
    
    // Tokenize a line into components
    virtual LineTokens Tokenize(const std::string& line) = 0;
    
    // Strip comments according to syntax rules
    virtual std::string StripComments(const std::string& line) = 0;
    
    // Normalize whitespace
    virtual std::string Normalize(const std::string& line) = 0;
    
    // Check if line is empty/comment-only
    virtual bool IsEmptyLine(const std::string& line) = 0;
};

struct LineTokens {
    std::optional<std::string> label;      // Label if present
    std::optional<std::string> mnemonic;   // Instruction/directive
    std::optional<std::string> operand;    // Operand field
    std::optional<std::string> comment;    // Comment if present
    SourceLocation location;               // File, line, column
};

// Concrete implementations
class Z80Lexer : public ILexer { /* ... */ };
class MerlinLexer : public ILexer { /* ... */ };
class ScmasmLexer : public ILexer { /* ... */ };
```

**Why Separate:**
- Different assemblers have different comment styles (`;`, `*`, `.COMMENT`)
- Line number handling (SCMASM)
- Local label prefix rules (`.`, `:`, `$`, `?`)
- Can be tested independently with string inputs

**Size Estimate:** 150-200 LOC per implementation

---

### 2. Parser Component

**Responsibility:** Parse tokens into structural elements

**Interface:**
```cpp
class IParser {
public:
    virtual ~IParser() = default;
    
    // Parse label (handles global, local, variable labels)
    virtual std::optional<ParsedLabel> ParseLabel(
        const std::string& label_text,
        const SourceLocation& location) = 0;
    
    // Parse operand field (extract expressions, addressing modes)
    virtual ParsedOperand ParseOperand(
        const std::string& operand_text) = 0;
    
    // Validate syntax-specific rules
    virtual void ValidateLine(const LineTokens& tokens) = 0;
};

struct ParsedLabel {
    std::string name;
    LabelType type;      // Global, Local, Variable, etc.
    bool is_public;      // Double colon (::) in some syntaxes
};

struct ParsedOperand {
    std::vector<std::string> expressions;  // Comma-separated
    std::unordered_map<std::string, std::string> attributes;
};
```

**Why Separate:**
- Label scoping rules vary (`:local`, `.local`, `$local`, `]variable`)
- Operand syntax varies (parentheses, brackets, addressing modes)
- Can be unit tested with mocked inputs

**Size Estimate:** 200-250 LOC per implementation

---

### 3. Directive Dispatcher

**Responsibility:** Route directive calls to appropriate handlers

**Interface:**
```cpp
class DirectiveDispatcher {
public:
    // Register a directive handler
    void RegisterDirective(
        const std::string& name,
        std::unique_ptr<IDirectiveHandler> handler);
    
    // Register multiple aliases for same handler
    void RegisterAliases(
        const std::vector<std::string>& names,
        std::unique_ptr<IDirectiveHandler> handler);
    
    // Check if mnemonic is a directive
    bool IsDirective(const std::string& mnemonic) const;
    
    // Dispatch directive to handler
    void HandleDirective(
        const std::string& directive,
        const ParsedLine& line,
        DirectiveContext& context);
    
private:
    std::unordered_map<std::string, std::unique_ptr<IDirectiveHandler>> handlers_;
};
```

**Why Separate:**
- Centralizes directive routing logic
- Enables runtime registration of directives
- Reduces coupling between parser and directive implementations

**Size Estimate:** 100-150 LOC

---

### 4. Directive Handlers (Strategy Pattern)

**Responsibility:** Implement individual directive behavior

**Base Interface:**
```cpp
class IDirectiveHandler {
public:
    virtual ~IDirectiveHandler() = default;
    
    // Handle the directive
    virtual void Handle(
        const ParsedLine& line,
        DirectiveContext& context) = 0;
    
    // Get directive metadata
    virtual DirectiveInfo GetInfo() const = 0;
};

struct DirectiveContext {
    Section& section;
    ConcreteSymbolTable& symbols;
    SegmentManager& segments;
    LabelScopeManager& scopes;
    MacroProcessor& macros;
    ConditionalStack& conditionals;
    uint32_t& current_address;
};

struct ParsedLine {
    std::optional<std::string> label;
    std::string directive;
    std::string operand;
    SourceLocation location;
};
```

**Concrete Handlers:**
```cpp
// Data definition directives
class DbDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

class DwDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

class DsDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

// Symbol directives
class EquDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

class SetDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

// Segment directives
class OrgDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

class CsegDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

// Macro directives
class MacroDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

class EndmDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

// Conditional directives
class IfDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

class ElseDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};

class EndifDirectiveHandler : public IDirectiveHandler {
    void Handle(const ParsedLine& line, DirectiveContext& ctx) override;
};
```

**Why Separate:**
- Each directive is independently testable
- Adding new directives doesn't require modifying parser
- Directive logic is cohesive and focused
- Reduces cyclomatic complexity of main parser

**Size Estimate:** 
- Simple directives (ORG, EQU): 30-50 LOC each
- Complex directives (DB/DW with expressions): 80-120 LOC
- Macro directives: 150-200 LOC
- Total directive handler code: 600-800 LOC (was embedded in 1,800 LOC god-class)

---

### 5. Expression Evaluator (Shared Component)

**Responsibility:** Parse and evaluate expressions with operator precedence

**Interface:**
```cpp
class ExpressionEvaluator {
public:
    // Parse expression string into expression tree
    std::shared_ptr<Expression> Parse(
        const std::string& expr,
        ConcreteSymbolTable& symbols);
    
    // Evaluate expression to value
    int64_t Evaluate(
        const std::shared_ptr<Expression>& expr,
        ConcreteSymbolTable& symbols,
        uint32_t current_address);
    
    // Check if expression is constant (no symbols)
    bool IsConstant(const std::shared_ptr<Expression>& expr);
    
private:
    // Operator precedence parsing
    std::shared_ptr<Expression> ParseLogicalOr(const std::string& str, size_t& pos);
    std::shared_ptr<Expression> ParseLogicalAnd(const std::string& str, size_t& pos);
    std::shared_ptr<Expression> ParseBitwiseOr(const std::string& str, size_t& pos);
    std::shared_ptr<Expression> ParseBitwiseXor(const std::string& str, size_t& pos);
    std::shared_ptr<Expression> ParseBitwiseAnd(const std::string& str, size_t& pos);
    std::shared_ptr<Expression> ParseComparison(const std::string& str, size_t& pos);
    std::shared_ptr<Expression> ParseShift(const std::string& str, size_t& pos);
    std::shared_ptr<Expression> ParseAddSubtract(const std::string& str, size_t& pos);
    std::shared_ptr<Expression> ParseMultiplyDivide(const std::string& str, size_t& pos);
    std::shared_ptr<Expression> ParseUnary(const std::string& str, size_t& pos);
    std::shared_ptr<Expression> ParsePrimary(const std::string& str, size_t& pos);
    
    ConcreteSymbolTable* symbols_;
};
```

**Why Shared:**
- Expression parsing logic is identical across all syntax parsers
- Currently duplicated 3 times (9 methods × 3 parsers = 27 duplicated methods!)
- Operator precedence is universal (not syntax-specific)
- Number format parsing can be parameterized

**Size Estimate:** 300-400 LOC (eliminates ~900 LOC of duplication)

---

### 6. Macro Processor

**Responsibility:** Handle macro definition, expansion, and parameter substitution

**Interface:**
```cpp
class MacroProcessor {
public:
    // Begin macro definition
    void BeginMacro(
        const std::string& name,
        const std::vector<std::string>& params);
    
    // Add line to current macro body
    void AddMacroLine(const std::string& line);
    
    // End macro definition
    void EndMacro();
    
    // Check if macro exists
    bool IsMacroDefined(const std::string& name) const;
    
    // Expand macro with arguments
    std::vector<std::string> ExpandMacro(
        const std::string& name,
        const std::vector<std::string>& args);
    
    // REPT/IRP/IRPC support
    void BeginRept(int count);
    void BeginIrp(const std::string& param, const std::vector<std::string>& values);
    void BeginIrpc(const std::string& param, const std::string& str);
    std::vector<std::string> ExpandRept();
    
    // State queries
    bool IsInMacroDefinition() const;
    bool IsInReptDefinition() const;
    int GetExpansionDepth() const;
    
private:
    struct MacroDefinition {
        std::string name;
        std::vector<std::string> params;
        std::vector<std::string> body;
        std::vector<std::string> locals;  // LOCAL symbols
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
    
    // Parameter substitution
    std::string SubstituteParams(
        const std::string& line,
        const std::vector<std::string>& params,
        const std::vector<std::string>& args);
    
    // Generate unique local labels
    std::string GenerateLocalLabel(const std::string& name, int invocation_id);
};
```

**Why Separate:**
- Macro expansion is complex state machine
- Currently tangled with parsing logic
- Can be tested independently with string inputs/outputs
- Syntax-agnostic (parameter substitution rules vary but core logic same)

**Size Estimate:** 400-500 LOC (was ~300 LOC embedded in each parser)

---

### 7. Segment Manager

**Responsibility:** Track segment state (CSEG/DSEG/ASEG/COMMON)

**Interface:**
```cpp
class SegmentManager {
public:
    // Segment operations
    void SetCodeSegment();
    void SetDataSegment();
    void SetAbsoluteSegment();
    void SetCommonBlock(const std::string& name);
    void SetOrigin(uint32_t address);
    
    // State queries
    SegmentType GetCurrentSegmentType() const;
    std::string GetCurrentSegmentName() const;
    uint32_t GetCurrentAddress() const;
    bool IsAbsoluteSegment() const;
    
    // Address tracking
    void AdvanceAddress(size_t bytes);
    void Reset();
    
private:
    enum class SegmentType { CODE, DATA, ABSOLUTE, COMMON };
    
    SegmentType current_type_ = SegmentType::CODE;
    std::string current_name_;
    uint32_t current_address_ = 0;
    bool has_org_ = false;
};
```

**Why Separate:**
- Segment state is independent concern
- Currently embedded in Z80 parser only (not in Merlin/SCMASM)
- Can be reused across all parsers
- Simplifies address calculation logic

**Size Estimate:** 150-200 LOC

---

### 8. Conditional Stack

**Responsibility:** Track IF/ELSE/ENDIF nesting and evaluation

**Interface:**
```cpp
class ConditionalStack {
public:
    // Push new conditional block
    void PushIf(bool condition);
    
    // Handle ELSE in current block
    void HandleElse();
    
    // Pop conditional block
    void PopEndif();
    
    // Query current state
    bool ShouldEmitCode() const;
    bool IsInConditional() const;
    int GetNestingDepth() const;
    
    // Validate structure
    void ValidateEmpty() const;  // At end of file
    
private:
    struct ConditionalBlock {
        bool condition;
        bool in_else_block;
        bool should_emit;
    };
    
    std::vector<ConditionalBlock> stack_;
};
```

**Why Separate:**
- Conditional nesting is self-contained logic
- Currently duplicated across parsers
- Can be unit tested independently
- Simpler than embedding in parser

**Size Estimate:** 100-120 LOC

---

### 9. Label Scope Manager

**Responsibility:** Handle global/local/private label scoping

**Interface:**
```cpp
class LabelScopeManager {
public:
    // Scope operations
    void EnterGlobalLabel(const std::string& label);
    void DefineLocalLabel(const std::string& label, uint32_t address);
    std::string ResolveLocalLabel(const std::string& label) const;
    
    // Label type detection
    bool IsLocalLabel(const std::string& label) const;
    bool IsVariableLabel(const std::string& label) const;
    
    // Get current scope
    std::string GetCurrentGlobalLabel() const;
    
private:
    struct LabelScope {
        std::string global_label;
        std::unordered_map<std::string, uint32_t> local_labels;
    };
    
    LabelScope current_scope_;
};
```

**Why Separate:**
- Scoping rules vary by syntax (`:local`, `.local`, `$local`, `]var`)
- Currently duplicated with slight variations
- Can be parameterized for different syntaxes
- Reduces parser complexity

**Size Estimate:** 100-150 LOC per syntax variant

---

### 10. Coordinating Syntax Parser (Facade)

**Responsibility:** Orchestrate components, maintain public API

**Interface:**
```cpp
class Z80UniversalSyntaxParser {
public:
    Z80UniversalSyntaxParser();
    
    // Public API (unchanged for backward compatibility)
    void SetCpu(CpuZ80* cpu);
    void Parse(const std::string& source, Section& section, ConcreteSymbolTable& symbols);
    std::shared_ptr<Expression> ParseExpression(const std::string& str, ConcreteSymbolTable& symbols);
    
private:
    // Components (injected via constructor)
    std::unique_ptr<ILexer> lexer_;
    std::unique_ptr<IParser> parser_;
    std::unique_ptr<DirectiveDispatcher> directive_dispatcher_;
    std::unique_ptr<ExpressionEvaluator> expression_evaluator_;
    std::unique_ptr<MacroProcessor> macro_processor_;
    std::unique_ptr<SegmentManager> segment_manager_;
    std::unique_ptr<ConditionalStack> conditional_stack_;
    std::unique_ptr<LabelScopeManager> label_scope_manager_;
    
    // CPU plugin (non-owning)
    CpuZ80* cpu_ = nullptr;
    
    // Parsing pipeline
    void ParseLine(const std::string& line, Section& section, ConcreteSymbolTable& symbols);
    void ProcessTokens(const LineTokens& tokens, Section& section, ConcreteSymbolTable& symbols);
    void HandleLabel(const ParsedLabel& label, Section& section, ConcreteSymbolTable& symbols);
    void HandleDirective(const std::string& directive, const ParsedLine& line,
                        Section& section, ConcreteSymbolTable& symbols);
    void HandleInstruction(const std::string& mnemonic, const std::string& operand,
                          Section& section, ConcreteSymbolTable& symbols);
};
```

**Why Keep:**
- Maintains backward compatibility
- Provides single entry point for clients
- Coordinates component interactions
- Handles high-level parse flow

**Size Estimate:** 250-300 LOC (down from 1,842!)

---

## Data Architecture

### Component Dependencies

```
┌──────────────────────────────────────────────────────────┐
│              Z80UniversalSyntaxParser                    │
│                  (Coordination)                          │
└──────────────────────┬───────────────────────────────────┘
                       │ owns
          ┌────────────┼────────────┬─────────────┐
          │            │            │             │
     ┌────▼────┐  ┌────▼────┐  ┌───▼──────┐  ┌──▼────────┐
     │ Lexer   │  │ Parser  │  │Directive │  │Expression │
     │         │  │         │  │Dispatcher│  │Evaluator  │
     └─────────┘  └─────────┘  └────┬─────┘  └───────────┘
                                     │ delegates
                         ┌───────────┴────────────┐
                         │                        │
                    ┌────▼─────┐           ┌─────▼─────┐
                    │Directive │           │  Macro    │
                    │Handlers  │           │ Processor │
                    └──────────┘           └───────────┘
```

### Data Flow

```
Source File
    │
    ▼
┌───────────────────────────────────────────────────┐
│ Parse() - Main entry point                        │
│ 1. Split into lines                               │
│ 2. For each line:                                 │
└───────────────────────────────────────────────────┘
    │
    ▼
┌───────────────────────────────────────────────────┐
│ ParseLine()                                        │
│ 1. Check macro/rept state                         │
│ 2. Check conditional state                        │
│ 3. Tokenize line                                  │
└───────────────────────────────────────────────────┘
    │
    ▼
┌───────────────────────────────────────────────────┐
│ ProcessTokens()                                    │
│ 1. Handle label (if present)                      │
│ 2. Dispatch directive OR instruction              │
└───────────────────────────────────────────────────┘
    │
    ├─────────────┬──────────────┐
    │             │              │
    ▼             ▼              ▼
┌───────┐  ┌──────────┐  ┌──────────────┐
│Handle │  │ Handle   │  │   Handle     │
│Label  │  │Directive │  │ Instruction  │
└───────┘  └──────────┘  └──────────────┘
               │
               ▼
        ┌──────────────┐
        │  Directive   │
        │   Handler    │
        │  (Strategy)  │
        └──────────────┘
```

---

## Integration Architecture

### Component Communication Patterns

#### 1. Dependency Injection (Constructor)

```cpp
Z80UniversalSyntaxParser::Z80UniversalSyntaxParser()
    : lexer_(std::make_unique<Z80Lexer>()),
      parser_(std::make_unique<Z80Parser>()),
      directive_dispatcher_(std::make_unique<DirectiveDispatcher>()),
      expression_evaluator_(std::make_unique<ExpressionEvaluator>()),
      macro_processor_(std::make_unique<MacroProcessor>()),
      segment_manager_(std::make_unique<SegmentManager>()),
      conditional_stack_(std::make_unique<ConditionalStack>()),
      label_scope_manager_(std::make_unique<LabelScopeManager>())
{
    // Register Z80-specific directives
    RegisterDirectives();
}
```

**Benefits:**
- Components can be mocked for testing
- Clear ownership model
- No hidden dependencies

#### 2. Context Object Pattern

```cpp
struct DirectiveContext {
    Section& section;
    ConcreteSymbolTable& symbols;
    SegmentManager& segments;
    LabelScopeManager& scopes;
    MacroProcessor& macros;
    ConditionalStack& conditionals;
    uint32_t& current_address;
};

// Pass to directive handlers
void DirectiveDispatcher::HandleDirective(
    const std::string& directive,
    const ParsedLine& line,
    DirectiveContext& context)
{
    auto handler = handlers_.find(directive);
    handler->second->Handle(line, context);
}
```

**Benefits:**
- Avoids long parameter lists
- Easy to extend with new context
- Clear what resources are available

#### 3. Strategy Pattern (Directive Handlers)

```cpp
// Register handlers at construction
void Z80UniversalSyntaxParser::RegisterDirectives() {
    directive_dispatcher_->RegisterDirective("ORG", 
        std::make_unique<OrgDirectiveHandler>());
    directive_dispatcher_->RegisterDirective("EQU", 
        std::make_unique<EquDirectiveHandler>());
    directive_dispatcher_->RegisterAliases({"DB", "DEFB", "BYTE"},
        std::make_unique<DbDirectiveHandler>());
    // ... etc
}
```

**Benefits:**
- Open/Closed Principle: Add new directives without modifying parser
- Each handler independently testable
- Syntax-specific directives can be easily added

---

## Non-Functional Architecture

### Performance

**Current Performance:**
- God-class approach: All logic inline, no virtual calls
- Typical assembly file: 1,000-10,000 lines
- Parse time: ~50-200ms

**Refactored Performance:**
- Component approach: Virtual calls for directive dispatch, expression evaluation
- Virtual call overhead: ~5-10ns per call
- Expected overhead: <5% increase in parse time
- Trade-off: Acceptable for massive maintainability improvement

**Mitigation Strategies:**
1. Use `final` on concrete implementations to enable devirtualization
2. Inline hot paths (lexer, simple directives)
3. Profile-guided optimization (PGO) in release builds
4. Minimize virtual call depth (max 2 levels)

### Memory

**Current Memory:**
- God-class: Single large object per parser (~500 bytes state)
- Atoms: Dominant memory use (linked list)

**Refactored Memory:**
- Multiple component objects: ~100 bytes each × 8 components = 800 bytes
- Increase: ~300 bytes per parser instance (negligible)
- Atoms: Unchanged (still dominant)

**Mitigation:**
- Parsers are short-lived (per-file)
- Memory increase is one-time per parse, not per-line

### Testability

**Current Testability:**
- God-class: Hard to test directives in isolation
- Mock dependencies: Difficult (tightly coupled)
- Test coverage: ~60% (complex interactions hard to test)

**Refactored Testability:**
- Component isolation: Each testable independently
- Mock components: Easy with interfaces
- Test coverage target: 85%+

**Test Strategy:**
```cpp
// Example: Unit test for ORG directive handler
TEST(OrgDirectiveHandler, SetsAddressCorrectly) {
    // Arrange
    MockSection section;
    MockSymbolTable symbols;
    MockSegmentManager segments;
    // ... other mocks
    
    DirectiveContext ctx{section, symbols, segments, /* ... */};
    ParsedLine line{.directive="ORG", .operand="$8000"};
    
    OrgDirectiveHandler handler;
    
    // Act
    handler.Handle(line, ctx);
    
    // Assert
    EXPECT_EQ(segments.GetCurrentAddress(), 0x8000);
}
```

### Security

**Input Validation:**
- Lexer: Validate line structure, detect malformed input
- Parser: Validate label/operand syntax
- Directive handlers: Validate operand ranges, types
- Expression evaluator: Prevent stack overflow on deep expressions

**Error Handling:**
- All components throw `AssemblerError` on invalid input
- No silent failures
- Clear error messages with source location

---

## Technology Choices

### C++17 Features Utilized

**std::variant:**
```cpp
// In LineTokens
std::variant<std::string, std::monostate> label;
```

**std::optional:**
```cpp
// For optional fields
std::optional<std::string> label;
std::optional<ParsedLabel> ParseLabel(const std::string& text);
```

**std::unique_ptr / std::shared_ptr:**
```cpp
// Component ownership
std::unique_ptr<IDirectiveHandler> handler;

// Expression trees (shared ownership)
std::shared_ptr<Expression> expr;
```

**std::unordered_map:**
```cpp
// Fast directive/macro lookup
std::unordered_map<std::string, std::unique_ptr<IDirectiveHandler>> handlers_;
```

---

## Architecture Decision Records (ADRs)

### ADR-001: Component-Based Refactoring Over Template Approach

**Context:**
Two approaches considered:
1. Component-based: Extract to separate classes with virtual interfaces
2. Template-based: Use CRTP to extract components while avoiding virtual calls

**Decision:** Component-based with virtual interfaces

**Rationale:**
- Clarity: Easier to understand and maintain
- Testability: Can mock components
- Flexibility: Can change implementations at runtime
- Performance: Virtual call overhead negligible for assembler workload
- Team familiarity: Virtual interfaces are more widely understood

**Consequences:**
- Positive: Better testability, clearer architecture
- Negative: ~5% performance overhead (acceptable)
- Risk: None (proven pattern)

---

### ADR-002: Shared Expression Evaluator Over Per-Parser Implementation

**Context:**
Expression evaluation currently duplicated across all three parsers with nearly identical implementations.

**Decision:** Extract to shared `ExpressionEvaluator` component

**Rationale:**
- DRY principle: Eliminate 900+ LOC of duplication
- Consistency: Same operator precedence across all syntaxes
- Maintainability: Fix bugs once, not three times
- Extensibility: Easy to add new operators/functions

**Consequences:**
- Positive: Massive code reduction, easier maintenance
- Negative: Must parameterize number format parsing (syntax-specific)
- Risk: Low (expression evaluation is well-defined)

**Implementation Note:**
Number format parsing remains syntax-specific (injected via callback or subclass).

---

### ADR-003: Strategy Pattern for Directive Handlers Over Switch Statement

**Context:**
Current implementation uses massive switch/if-else chains to dispatch directives.

**Decision:** Use Strategy pattern with registry of directive handlers

**Rationale:**
- Open/Closed Principle: Add directives without modifying parser
- Single Responsibility: Each directive handler is focused
- Testability: Test each directive in isolation
- Clarity: Directive logic is self-contained

**Consequences:**
- Positive: Much cleaner architecture, easier to extend
- Negative: Slight indirection (one map lookup per directive)
- Risk: None (proven pattern, used in vasm-ext)

---

### ADR-004: Maintain Public API for Backward Compatibility

**Context:**
Existing code depends on current `Parse()` method signature.

**Decision:** Keep public API unchanged, refactor internals only

**Rationale:**
- Backward compatibility: No breaking changes for clients
- Safe refactoring: Can verify behavior with existing tests
- Gradual migration: Can refactor one parser at a time

**Consequences:**
- Positive: Zero client code changes required
- Negative: Public API may not reflect new architecture
- Risk: None (Facade pattern handles translation)

---

## Refactoring Strategy

### Phase 1: Extract Shared Components (2-3 days)

**Goal:** Create reusable components without breaking existing parsers

**Steps:**
1. Extract `ExpressionEvaluator` to `src/parsing/expression_evaluator.cpp`
   - Copy expression parsing methods from Z80 parser
   - Make number parsing pluggable (interface or callback)
   - Unit test with all three syntaxes' number formats
   
2. Extract `ConditionalStack` to `src/parsing/conditional_stack.cpp`
   - Implement IF/ELSE/ENDIF logic
   - Unit test with nested conditionals
   
3. Extract `LabelScopeManager` to `src/parsing/label_scope_manager.cpp`
   - Parameterize local label prefixes
   - Unit test with different scoping rules
   
4. Extract `SegmentManager` to `src/parsing/segment_manager.cpp`
   - Implement CSEG/DSEG/ASEG/COMMON logic
   - Unit test segment switching

**Verification:**
- All new components have 90%+ test coverage
- No changes to existing parser code yet
- Build succeeds, all tests pass

---

### Phase 2: Refactor Z80UniversalSyntaxParser (3-4 days)

**Goal:** Refactor largest parser first as proof of concept

**Steps:**
1. Create component interfaces (`ILexer`, `IParser`, `IDirectiveHandler`)
2. Implement `Z80Lexer` (extract tokenization logic)
3. Implement `Z80Parser` (extract parsing logic)
4. Implement `DirectiveDispatcher` (registry pattern)
5. Implement directive handlers (23 handlers for Z80)
6. Wire components via dependency injection
7. Remove old code, verify tests pass

**Verification:**
- All existing Z80 tests pass (regression suite)
- New unit tests for each component (90%+ coverage)
- Code size reduced from 1,842 LOC to ~1,500 LOC total (split across files)
- Binary compatibility tests pass

---

### Phase 3: Refactor MerlinSyntaxParser (2-3 days)

**Goal:** Apply pattern to second parser, identify reusable components

**Steps:**
1. Implement `MerlinLexer` (Merlin-specific tokenization)
2. Implement `MerlinParser` (Merlin-specific parsing)
3. Implement Merlin directive handlers (30 handlers)
   - Reuse data handlers (DB/DW) where possible
   - Implement Merlin-specific (HEX, ASC, DCI, INV, FLS)
4. Wire components
5. Remove old code, verify tests pass

**Verification:**
- All existing Merlin tests pass
- Binary compatibility tests pass
- Code size reduced from 1,764 LOC to ~1,400 LOC total

---

### Phase 4: Refactor ScmasmSyntaxParser (2-3 days)

**Goal:** Complete refactoring, maximize code reuse

**Steps:**
1. Implement `ScmasmLexer` (line number handling, comment styles)
2. Implement `ScmasmParser` (local label `.0-.9`, variable labels `]var`)
3. Implement SCMASM directive handlers (13 handlers)
   - Reuse expression evaluator
   - Implement SCMASM-specific (`.HS`, `.BS`, `.AS`, `.AT`, `.AZ`)
4. Wire components
5. Remove old code, verify tests pass

**Verification:**
- All existing SCMASM tests pass
- Binary compatibility tests pass
- Code size reduced from 1,330 LOC to ~1,100 LOC total

---

### Phase 5: Consolidate and Document (1-2 days)

**Goal:** Clean up, optimize, document

**Steps:**
1. Identify common directive handlers to extract to base classes
2. Optimize hot paths (profile-guided)
3. Update architecture documentation
4. Create migration guide for other parsers
5. Performance benchmark (ensure <5% overhead)

**Verification:**
- All tests pass across all parsers
- Performance within acceptable limits
- Documentation complete

---

## Estimated Effort

| Phase | Description | Effort | Risk |
|-------|-------------|--------|------|
| Phase 1 | Extract shared components | 2-3 days | Low |
| Phase 2 | Refactor Z80 parser | 3-4 days | Medium |
| Phase 3 | Refactor Merlin parser | 2-3 days | Low |
| Phase 4 | Refactor SCMASM parser | 2-3 days | Low |
| Phase 5 | Consolidate & document | 1-2 days | Low |
| **Total** | **Complete refactoring** | **10-15 days** | **Low-Medium** |

**Risk Factors:**
- Medium risk on Phase 2 (first refactoring, potential unforeseen issues)
- Low risk on Phase 3-4 (pattern established, lessons learned)
- Overall: Low-Medium (well-defined refactoring, comprehensive tests)

---

## Success Metrics

### Quantitative Metrics

**Code Size:**
- Z80UniversalSyntaxParser: 1,842 LOC → ~300 LOC (83% reduction)
- MerlinSyntaxParser: 1,764 LOC → ~300 LOC (83% reduction)
- ScmasmSyntaxParser: 1,330 LOC → ~250 LOC (81% reduction)
- Total parser code: 4,936 LOC → ~2,500 LOC (including shared components)
- Net reduction: ~2,400 LOC (49% reduction)

**Test Coverage:**
- Before: ~60% (difficult to test god-classes)
- After: 85%+ target (component isolation enables better testing)

**Cyclomatic Complexity:**
- Before: `ParseLine()` complexity ~50-80 per parser
- After: Max complexity <15 per method

**Performance:**
- Overhead: <5% increase in parse time
- Acceptable trade-off for maintainability

### Qualitative Metrics

**Maintainability:**
- ✅ Single Responsibility Principle: Each class has one focused responsibility
- ✅ Open/Closed Principle: Add directives without modifying parser
- ✅ Testability: Each component independently testable
- ✅ Clarity: Code intent is clear

**Extensibility:**
- ✅ New directives: Add handler, register in dispatcher
- ✅ New syntax: Implement lexer/parser, reuse components
- ✅ New features: Modify focused component, not entire parser

**Code Reuse:**
- ✅ Expression evaluator: Shared across all parsers
- ✅ Conditional stack: Shared across all parsers
- ✅ Segment manager: Shared across all parsers
- ✅ Common directive handlers: Reused where applicable

---

## Risks and Mitigation

### Risk 1: Performance Degradation

**Probability:** Low  
**Impact:** Medium

**Mitigation:**
- Benchmark before/after refactoring
- Profile hot paths with real assembly files
- Use `final` keyword to enable devirtualization
- Inline small, frequently-called methods
- Accept <5% overhead as acceptable trade-off

**Contingency:**
- If overhead >5%, identify hot paths via profiling
- Optimize specific components (e.g., inline lexer methods)
- Consider CRTP for hot paths if necessary

---

### Risk 2: Regression in Behavior

**Probability:** Medium  
**Impact:** High

**Mitigation:**
- Comprehensive regression test suite (existing tests)
- Binary compatibility tests (compare output with original)
- Refactor one parser at a time (isolate issues)
- No changes to public API (backward compatible)
- Code review of each component

**Contingency:**
- If regression found, git bisect to identify issue
- Fix component or revert specific change
- Comprehensive test prevents shipping broken code

---

### Risk 3: Incomplete Refactoring

**Probability:** Low  
**Impact:** Medium

**Mitigation:**
- Phased approach (complete one parser before starting next)
- Clear success criteria per phase
- Tests must pass before proceeding to next phase

**Contingency:**
- If time constrained, keep refactored parsers and leave others
- Parsers are independent (no coupling)
- Document which parsers are refactored vs. legacy

---

### Risk 4: Unforeseen Complexity

**Probability:** Medium  
**Impact:** Medium

**Mitigation:**
- Start with largest parser (Z80) to identify issues early
- Spike/prototype component extraction before full implementation
- Time-box investigation (if blocked, document and escalate)

**Contingency:**
- If component extraction is too complex, keep logic in parser
- Document rationale in ADR
- Revisit in future refactoring

---

## Related Documents

**Architecture:**
- [xasm++ Architecture Overview](../00-overview.md) - Overall system design
- [Syntax Plugin Interface](../03-syntax-plugin-interface.md) - Plugin architecture

**ADRs:**
- [ADR-001: Component-Based Refactoring](../../adr/002-component-based-refactoring.md)
- [ADR-002: Shared Expression Evaluator](../../adr/003-shared-expression-evaluator.md)

**Implementation:**
- [God-Class Investigation Task](.ai/tasks/2026-02-09_god-class-investigation/) - Original investigation
- [Architectural Review](.ai/tasks/2026-02-09_architectural-review-patterns/) - Broader architectural patterns

---

## Appendix: Code Size Breakdown

### Current Implementation (God-Class)

| Parser | Header | Implementation | Total | Methods |
|--------|--------|----------------|-------|---------|
| Z80UniversalSyntaxParser | 420 LOC | 1,842 LOC | 2,262 LOC | 50 |
| MerlinSyntaxParser | 265 LOC | 1,764 LOC | 2,029 LOC | 44 |
| ScmasmSyntaxParser | 450 LOC | 1,330 LOC | 1,780 LOC | 30 |
| **Total** | **1,135 LOC** | **4,936 LOC** | **6,071 LOC** | **124** |

### Proposed Implementation (Component-Based)

| Component | LOC | Shared? |
|-----------|-----|---------|
| **Shared Components** | | |
| ExpressionEvaluator | 350 | ✓ |
| ConditionalStack | 120 | ✓ |
| SegmentManager | 180 | ✓ |
| DirectiveDispatcher | 120 | ✓ |
| **Subtotal (Shared)** | **770** | |
| | | |
| **Z80-Specific** | | |
| Z80Lexer | 180 | ✗ |
| Z80Parser | 220 | ✗ |
| Z80UniversalSyntaxParser (facade) | 280 | ✗ |
| Z80 Directive Handlers (23 handlers) | 680 | ✗ |
| **Subtotal (Z80)** | **1,360** | |
| | | |
| **Merlin-Specific** | | |
| MerlinLexer | 160 | ✗ |
| MerlinParser | 200 | ✗ |
| MerlinSyntaxParser (facade) | 260 | ✗ |
| Merlin Directive Handlers (30 handlers) | 780 | ✗ |
| **Subtotal (Merlin)** | **1,400** | |
| | | |
| **SCMASM-Specific** | | |
| ScmasmLexer | 150 | ✗ |
| ScmasmParser | 180 | ✗ |
| ScmasmSyntaxParser (facade) | 240 | ✗ |
| SCMASM Directive Handlers (13 handlers) | 530 | ✗ |
| **Subtotal (SCMASM)** | **1,100** | |
| | | |
| **Grand Total** | **4,630 LOC** | |

**Net Reduction:** 6,071 - 4,630 = **1,441 LOC saved** (24% reduction)

**Maintainability Gain:**
- Largest class: 350 LOC (ExpressionEvaluator) vs. 1,842 LOC (Z80 god-class)
- Average class size: 140 LOC vs. 1,645 LOC
- Classes with SRP: 100% vs. 0%

---

**Document Version:** 1.0  
**Last Updated:** 2026-02-09  
**Status:** Ready for Review  
**Owner:** Architect Agent
