# Data Models: Parsing Component System

**Date:** 2026-02-09  
**Version:** 1.0  
**Related:** [Architecture Document](architecture.md) | [API Specification](api-spec.md)

---

## Overview

This document defines the data structures used throughout the parsing component system. These models represent tokenized lines, parsed elements, parser state, and component interactions.

---

## Core Data Structures

### 1. Source Location

**Purpose:** Track source file location for error reporting

```cpp
struct SourceLocation {
    std::string filename;      ///< Source file path
    int line_number;           ///< Line number (1-based)
    int column;                ///< Column number (0-based)
    
    std::string ToString() const {
        return filename + ":" + std::to_string(line_number) + 
               ":" + std::to_string(column);
    }
};
```

**Usage:**
```cpp
SourceLocation loc{"main.asm", 42, 0};
throw AssemblerError("Undefined symbol", loc);
// Output: "main.asm:42:0: Undefined symbol"
```

**Memory:** 32 bytes (string + 2 ints)

---

### 2. Line Tokens

**Purpose:** Represent tokenized line components

```cpp
struct LineTokens {
    std::optional<std::string> label;       ///< Label field (if present)
    std::optional<std::string> mnemonic;    ///< Instruction/directive
    std::optional<std::string> operand;     ///< Operand field
    std::optional<std::string> comment;     ///< Comment (if present)
    SourceLocation location;                ///< Source location
    
    // Convenience methods
    bool HasLabel() const { return label.has_value(); }
    bool HasMnemonic() const { return mnemonic.has_value(); }
    bool HasOperand() const { return operand.has_value(); }
    bool HasComment() const { return comment.has_value(); }
    bool IsEmptyLine() const { return !HasMnemonic(); }
};
```

**Example:**
```cpp
// Input: "START:  LD A,$42  ; Load 42"
LineTokens tokens{
    .label = "START",
    .mnemonic = "LD",
    .operand = "A,$42",
    .comment = " Load 42",
    .location = {"main.asm", 10, 0}
};
```

**Memory:** ~128 bytes (4 optionals + location)

---

### 3. Label Type

**Purpose:** Classify label scope and visibility

```cpp
enum class LabelType {
    Global,      ///< Normal global label (default scope)
    Local,       ///< Local label (scoped to previous global)
    Variable,    ///< DUM block variable (Merlin ]VAR)
    Private,     ///< Private label (assembly-unit scope)
    Public,      ///< Public label (exported, :: suffix)
};
```

**Scoping Rules:**

| Type | Scope | Visibility | Example |
|------|-------|------------|---------|
| Global | File-wide | Public by default | `MAIN`, `SUBROUTINE` |
| Local | Scoped to previous global | Private | `:loop`, `.local`, `$temp` |
| Variable | DUM block | Private | `]counter`, `]pointer` |
| Private | File-wide | Private | `_internal` |
| Public | File-wide | Public (exported) | `MAIN::`, `ENTRY::` |

---

### 4. Parsed Label

**Purpose:** Structured label information

```cpp
struct ParsedLabel {
    std::string name;         ///< Label name (without prefix/suffix)
    std::string raw_name;     ///< Original label text
    LabelType type;           ///< Label classification
    bool is_public;           ///< Public export flag
    SourceLocation location;  ///< Source location
};
```

**Examples:**

| Raw Input | name | raw_name | type | is_public |
|-----------|------|----------|------|-----------|
| `START:` | `START` | `START:` | Global | false |
| `MAIN::` | `MAIN` | `MAIN::` | Public | true |
| `:loop` | `loop` | `:loop` | Local | false |
| `.local` | `local` | `.local` | Local | false |
| `$temp` | `temp` | `$temp` | Local | false |
| `]counter` | `counter` | `]counter` | Variable | false |

**Memory:** 64 bytes (2 strings + enum + bool + location)

---

### 5. Parsed Operand

**Purpose:** Structured operand field with expressions

```cpp
struct ParsedOperand {
    std::string raw_text;                                  ///< Original operand text
    std::vector<std::string> expressions;                  ///< Comma-separated expressions
    std::unordered_map<std::string, std::string> attributes;  ///< Syntax-specific attributes
    SourceLocation location;
};
```

**Examples:**

**Simple operand:**
```cpp
// Input: "A,$42"
ParsedOperand{
    .raw_text = "A,$42",
    .expressions = {"A", "$42"},
    .attributes = {},
    .location = {/*...*/}
};
```

**Complex operand with attributes:**
```cpp
// Input: "(IX+10),B" (Z80 indexed)
ParsedOperand{
    .raw_text = "(IX+10),B",
    .expressions = {"(IX+10)", "B"},
    .attributes = {
        {"index_register", "IX"},
        {"displacement", "10"}
    },
    .location = {/*...*/}
};
```

**String operand:**
```cpp
// Input: "'Hello, World!'"
ParsedOperand{
    .raw_text = "'Hello, World!'",
    .expressions = {"'Hello, World!'"},
    .attributes = {{"is_string", "true"}},
    .location = {/*...*/}
};
```

**Memory:** 96+ bytes (string + vector + map + location)

---

### 6. Parsed Line

**Purpose:** Complete parsed line ready for processing

```cpp
struct ParsedLine {
    std::optional<ParsedLabel> label;  ///< Label (if present)
    std::string mnemonic;              ///< Instruction or directive
    ParsedOperand operand;             ///< Operand field
    SourceLocation location;
};
```

**Example:**
```cpp
// Input: "START:  ORG $8000"
ParsedLine{
    .label = ParsedLabel{
        .name = "START",
        .raw_name = "START:",
        .type = LabelType::Global,
        .is_public = false,
        .location = {/*...*/}
    },
    .mnemonic = "ORG",
    .operand = ParsedOperand{
        .raw_text = "$8000",
        .expressions = {"$8000"},
        .attributes = {},
        .location = {/*...*/}
    },
    .location = {"main.asm", 10, 0}
};
```

**Memory:** ~192 bytes (optional + string + operand + location)

---

## Component State Structures

### 7. Directive Context

**Purpose:** Provide access to all parser state for directive handlers

```cpp
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
```

**Usage:**
```cpp
void EquDirectiveHandler::Handle(const ParsedLine& line, DirectiveContext& ctx) {
    // Access components through context
    uint32_t value = EvaluateExpression(line.operand.expressions[0], 
                                       ctx.symbols, ctx.current_address);
    ctx.symbols.Define(line.label->name, value, SymbolType::Equate);
}
```

**Memory:** 48 bytes (6 references + 1 pointer)

---

### 8. Directive Info

**Purpose:** Metadata for directive validation and documentation

```cpp
struct DirectiveInfo {
    std::string name;                     ///< Primary directive name
    std::vector<std::string> aliases;     ///< Alternative names
    std::string description;              ///< Human-readable description
    bool requires_label;                  ///< True if label is required
    bool requires_operand;                ///< True if operand is required
};
```

**Example:**
```cpp
DirectiveInfo{
    .name = "DB",
    .aliases = {"DEFB", "BYTE", "DM", "DEFM"},
    .description = "Define byte(s) of data",
    .requires_label = false,
    .requires_operand = true
};
```

**Memory:** 64+ bytes (3 strings + vector + 2 bools)

---

## State Machine Structures

### 9. Macro Definition

**Purpose:** Store macro template for expansion

```cpp
struct MacroDefinition {
    std::string name;                  ///< Macro name
    std::vector<std::string> params;   ///< Parameter names (&param, %param)
    std::vector<std::string> body;     ///< Unexpanded body lines
    std::vector<std::string> locals;   ///< LOCAL symbols (unique per invocation)
};
```

**Example:**
```cpp
// Assembly source:
// DELAY MACRO cycles
//     LOCAL loop
// loop:
//     DJNZ loop
//     ENDM

MacroDefinition{
    .name = "DELAY",
    .params = {"cycles"},
    .body = {
        "loop:",
        "    DJNZ loop"
    },
    .locals = {"loop"}
};
```

**Memory:** 64+ bytes (4 vectors with strings)

---

### 10. REPT Block

**Purpose:** Store repetition block for expansion

```cpp
enum class ReptType {
    REPT,   ///< REPT count - Repeat N times
    IRP,    ///< IRP param,<list> - Iterate over values
    IRPC,   ///< IRPC param,<string> - Iterate over characters
};

struct ReptBlock {
    ReptType type;                     ///< Type of repetition
    std::vector<std::string> body;     ///< Unexpanded body lines
    int count;                         ///< For REPT: number of iterations
    std::string param_name;            ///< For IRP/IRPC: parameter name
    std::vector<std::string> values;   ///< For IRP/IRPC: values to iterate
};
```

**Examples:**

**REPT:**
```cpp
// Assembly: REPT 3
//             NOP
//           ENDM
ReptBlock{
    .type = ReptType::REPT,
    .body = {"    NOP"},
    .count = 3,
    .param_name = "",
    .values = {}
};
// Expands to: NOP / NOP / NOP
```

**IRP:**
```cpp
// Assembly: IRP reg,<A,B,C>
//             LD reg,0
//           ENDM
ReptBlock{
    .type = ReptType::IRP,
    .body = {"    LD reg,0"},
    .count = 0,
    .param_name = "reg",
    .values = {"A", "B", "C"}
};
// Expands to: LD A,0 / LD B,0 / LD C,0
```

**IRPC:**
```cpp
// Assembly: IRPC char,<ABC>
//             DB 'char'
//           ENDM
ReptBlock{
    .type = ReptType::IRPC,
    .body = {"    DB 'char'"},
    .count = 0,
    .param_name = "char",
    .values = {"A", "B", "C"}
};
// Expands to: DB 'A' / DB 'B' / DB 'C'
```

**Memory:** 64+ bytes (enum + 2 vectors + int + string)

---

### 11. Conditional Block

**Purpose:** Track IF/ELSE/ENDIF nesting

```cpp
struct ConditionalBlock {
    bool condition;        ///< True if condition is met
    bool in_else_block;    ///< True if currently in ELSE block
    bool should_emit;      ///< True if code should be emitted
};
```

**State Machine:**

```
IF condition=true
  ├─> should_emit=true, in_else_block=false
  │   [emit code]
  │   ELSE
  ├─> should_emit=false, in_else_block=true
  │   [skip code]
  │   ENDIF
  └─> (pop block)

IF condition=false
  ├─> should_emit=false, in_else_block=false
  │   [skip code]
  │   ELSE
  ├─> should_emit=true, in_else_block=true
  │   [emit code]
  │   ENDIF
  └─> (pop block)
```

**Memory:** 3 bytes (3 bools, but likely 12 due to alignment)

---

### 12. Label Scope

**Purpose:** Track current label scope for local labels

```cpp
struct LabelScope {
    std::string global_label;                           ///< Current global label
    std::unordered_map<std::string, uint32_t> local_labels;  ///< local_name -> address
};
```

**Example:**

```assembly
; Assembly source
SUBROUTINE1:        ; Enter global scope "SUBROUTINE1"
    LD A,0
:loop               ; Define local "SUBROUTINE1:loop" at 0x8002
    DJNZ :loop      ; Reference "SUBROUTINE1:loop"
    RET

SUBROUTINE2:        ; Enter global scope "SUBROUTINE2"
    LD B,10
:loop               ; Define local "SUBROUTINE2:loop" at 0x8010 (different!)
    DJNZ :loop      ; Reference "SUBROUTINE2:loop"
    RET
```

**State transitions:**

```cpp
// At "SUBROUTINE1:"
LabelScope{
    .global_label = "SUBROUTINE1",
    .local_labels = {
        {":loop", 0x8002}
    }
};

// At "SUBROUTINE2:"
LabelScope{
    .global_label = "SUBROUTINE2",
    .local_labels = {
        {":loop", 0x8010}  // New scope, reused name
    }
};
```

**Memory:** 32+ bytes (string + map)

---

### 13. Segment State

**Purpose:** Track current segment type and address

```cpp
enum class SegmentType {
    CODE,      ///< CSEG - Code segment
    DATA,      ///< DSEG - Data segment
    ABSOLUTE,  ///< ASEG - Absolute segment
    COMMON,    ///< COMMON - Common block
};

struct SegmentState {
    SegmentType type;        ///< Current segment type
    std::string name;        ///< Segment/common block name
    uint32_t address;        ///< Current address
    bool has_org;            ///< True if ORG has been set
};
```

**State Transitions:**

```assembly
; Default: CODE segment at 0x0000
CSEG            ; Explicitly set CODE segment
ORG $8000       ; Set origin to 0x8000
; Now: CODE segment at 0x8000

DSEG            ; Switch to DATA segment
ORG $C000       ; Set origin to 0xC000
; Now: DATA segment at 0xC000

ASEG            ; Switch to ABSOLUTE segment
ORG $FFFF       ; Set origin to 0xFFFF
; Now: ABSOLUTE segment at 0xFFFF
```

**Memory:** 44 bytes (enum + string + uint32 + bool)

---

## Data Flow Diagrams

### Parse Pipeline

```
┌─────────────────────────────────────────────────────────┐
│ Input: "START:  LD A,$42  ; Load 42"                    │
└────────────────────────┬────────────────────────────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │    Lexer::Tokenize     │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │      LineTokens        │
            │  {                     │
            │    label: "START",     │
            │    mnemonic: "LD",     │
            │    operand: "A,$42",   │
            │    comment: "Load 42"  │
            │  }                     │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │   Parser::ParseLabel   │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │     ParsedLabel        │
            │  {                     │
            │    name: "START",      │
            │    type: Global        │
            │  }                     │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │  Parser::ParseOperand  │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │    ParsedOperand       │
            │  {                     │
            │    expressions:        │
            │      ["A", "$42"]      │
            │  }                     │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │      ParsedLine        │
            │  {                     │
            │    label: {...},       │
            │    mnemonic: "LD",     │
            │    operand: {...}      │
            │  }                     │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │  Directive/Instruction │
            │       Handler          │
            └────────────────────────┘
```

---

### Directive Handling Flow

```
┌─────────────────────────────────────────────────────────┐
│ ParsedLine{mnemonic="ORG", operand="$8000"}             │
└────────────────────────┬────────────────────────────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │  DirectiveDispatcher   │
            │   .IsDirective("ORG")  │
            │        = true           │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │      Lookup Handler    │
            │  handlers_["ORG"]      │
            │  = OrgDirectiveHandler │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │   DirectiveContext     │
            │  {                     │
            │    section,            │
            │    symbols,            │
            │    segments,           │
            │    ...                 │
            │  }                     │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │ OrgDirectiveHandler    │
            │    .Handle(line, ctx)  │
            └────────────┬───────────┘
                         │
         ┌───────────────┼───────────────┐
         │               │               │
         ▼               ▼               ▼
   ┌─────────┐   ┌─────────────┐   ┌──────────┐
   │Evaluate │   │Update Segment│   │Add Atom  │
   │Expression│   │  Address     │   │to Section│
   └─────────┘   └─────────────┘   └──────────┘
```

---

### Macro Expansion Flow

```
┌─────────────────────────────────────────────────────────┐
│ ParsedLine{mnemonic="DELAY", operand="100"}             │
└────────────────────────┬────────────────────────────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │   MacroProcessor       │
            │  .IsMacroDefined       │
            │    ("DELAY") = true    │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │    Lookup Macro        │
            │  macros_["DELAY"]      │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │   MacroDefinition      │
            │  {                     │
            │    name: "DELAY",      │
            │    params: ["cycles"], │
            │    body: [             │
            │      "loop:",          │
            │      "  DJNZ loop"     │
            │    ],                  │
            │    locals: ["loop"]    │
            │  }                     │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │  Parse Arguments       │
            │  operand = "100"       │
            │  args = ["100"]        │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │ Substitute Parameters  │
            │  "cycles" -> "100"     │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │   Generate Locals      │
            │  "loop" -> "??0001"    │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │   Expanded Lines       │
            │  [                     │
            │    "??0001:",          │
            │    "  DJNZ ??0001"     │
            │  ]                     │
            └────────────┬───────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │   Re-parse Lines       │
            │  (recursive parsing)   │
            └────────────────────────┘
```

---

## Memory Layout Analysis

### Per-Parse Memory Usage

**Assumptions:**
- 1,000 line assembly file
- 10% labels (100 labels)
- 5% macros (50 macro invocations)
- Average 40 chars per line

**Memory Breakdown:**

| Component | Per-Item | Count | Total |
|-----------|----------|-------|-------|
| **Tokens** | 128 bytes | 1,000 | 128 KB |
| **Parsed Lines** | 192 bytes | 1,000 | 192 KB |
| **Labels** | 64 bytes | 100 | 6.4 KB |
| **Macro Definitions** | 64+ bytes | 10 | 0.6 KB |
| **Conditional Stack** | 12 bytes | 5 depth | 60 bytes |
| **Label Scope** | 32 bytes | 1 | 32 bytes |
| **Segment State** | 44 bytes | 1 | 44 bytes |
| **Component Objects** | 100 bytes | 8 | 800 bytes |
| **Total (excluding atoms)** | | | **~328 KB** |

**Notes:**
- Atoms (instructions/data) dominate memory (not counted above)
- Tokens/parsed lines are temporary (freed after processing)
- Actual runtime memory ~100 KB per parser instance
- Negligible compared to atom memory

---

## Data Validation

### Input Validation Rules

**Lexer:**
- Line length < 1024 characters
- Valid UTF-8 encoding
- No null characters in input

**Parser:**
- Label length < 256 characters
- Label matches identifier rules: `[A-Za-z_][A-Za-z0-9_]*`
- Operand length < 512 characters

**Directive Handlers:**
- Address range: 0x0000 - 0xFFFFFFFF
- Data values: 0x00 - 0xFF (byte), 0x0000 - 0xFFFF (word)
- String length < 1024 characters

**Expression Evaluator:**
- Maximum nesting depth: 64 levels
- Prevent stack overflow on deep expressions
- Detect circular symbol references

---

## Serialization (Future)

**Potential Use Cases:**
- Save/restore parser state
- Incremental compilation
- Distributed builds

**Serialization Format (JSON):**

```json
{
  "version": "1.0",
  "symbols": [
    {"name": "START", "value": 32768, "type": "label"},
    {"name": "SIZE", "value": 256, "type": "equate"}
  ],
  "macros": [
    {
      "name": "DELAY",
      "params": ["cycles"],
      "body": ["loop:", "  DJNZ loop"],
      "locals": ["loop"]
    }
  ],
  "segment": {
    "type": "CODE",
    "address": 32768
  }
}
```

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-02-09 | Initial data models specification |

---

**Status:** Proposed  
**Owner:** Architect Agent
