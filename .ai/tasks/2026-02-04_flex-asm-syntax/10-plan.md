# Implementation Plan: FLEX ASM Syntax Processor

**Task:** xasm++-k5s
**Date:** 2026-02-04
**Workflow:** Feature (New Syntax Mode)

---

## Planning Phase

### Workflow Selection

**Chosen:** `.ai-pack/workflows/feature.md`

**Rationale:** This is a significant new feature adding macro support and conditional assembly - capabilities not present in existing syntax modes.

### Architecture Approach

**Pattern:** Extend MotorolaBaseSyntax (Template Method Pattern)

**Code Reuse:** ~60% from existing Motorola base

**New Components:**
1. MacroProcessor - Macro definition storage and expansion
2. ConditionalStack - Conditional assembly state tracking
3. FlexAsmSyntax - Main parser extending MotorolaBaseSyntax

---

## Phase 0: Architecture Design (Optional but Recommended)

### Design Decisions

**Q1: Where to implement macro expansion?**
- **Option A:** Preprocessor (before parsing)
- **Option B:** During parsing (inline expansion)
- **Decision:** Option A - Preprocessor approach
- **Rationale:** Cleaner separation, easier debugging, matches FLEX behavior

**Q2: How to handle local labels in macros?**
- **Option A:** Prefix with unique ID (`.LOOP` → `.LOOP_001`)
- **Option B:** Maintain separate symbol table per macro
- **Decision:** Option A - Prefix approach
- **Rationale:** Simpler, matches common assembler behavior

**Q3: Macro parameter limit?**
- **Decision:** 8 parameters (matches FLEX spec)
- **Rationale:** FLEX standard, reasonable for 6809 programming

### Architecture Document

Create `docs/architecture/flex-asm-architecture.md` documenting:
- Macro processor design
- Conditional assembly strategy
- Integration with existing infrastructure

---

## Phase 1: Basic Directives Implementation

**Goal:** Get basic FLEX directives working without macros/conditionals

### Step 1.1: Create FlexAsmSyntax Skeleton

**Files:**
- `include/xasm++/syntax/flex_syntax.h`
- `src/syntax/flex/flex_syntax.cpp`

**Implementation:**
```cpp
class FlexAsmSyntax : public MotorolaBaseSyntax {
public:
    FlexAsmSyntax();

    std::string GetName() const override;
    std::string GetDescription() const override;

protected:
    // Override directive handling for FLEX-specific behavior
    void HandleDirective(const std::string& directive,
                        const std::string& args,
                        ParserState& state) override;

    // FLEX-specific helpers
    void HandleNAM(const std::string& title);
    void HandleTTL(const std::string& title);
    void HandleSTTL(const std::string& subtitle);
    void HandlePAG();
    void HandleSPC(int lines);
};
```

**Tests:**
- `tests/unit/test_flex_basic.cpp`
- Verify inheritance works
- Test directive routing

**Time:** 3-4 hours

### Step 1.2: Implement Basic Directives

Implement handlers for:
- ORG, END, SETDP (reuse from MotorolaBaseSyntax)
- EQU, SET (EQU exists, add SET with reassignment)
- FCB, FDB, FCC, RMB (reuse from MotorolaBaseSyntax)
- NAM/TTL, STTL, PAG, SPC (new - listing control)

**Key Difference:** SET allows reassignment (EQU does not)

```cpp
void FlexAsmSyntax::HandleSET(const std::string& symbol,
                               int64_t value) {
    // Allow redefinition
    if (symbol_table_->Exists(symbol)) {
        symbol_table_->Update(symbol, value);
    } else {
        symbol_table_->Define(symbol, value, SymbolType::Variable);
    }
}
```

**Tests:**
- Each directive with valid parameters
- SET reassignment test
- Error cases (invalid syntax)

**Time:** 5-6 hours

### Step 1.3: Register with Syntax Registry

Update `src/syntax/syntax_registry.cpp`:

```cpp
#include "xasm++/syntax/flex_syntax.h"

void RegisterBuiltinSyntaxes() {
    // Existing registrations...

    SyntaxRegistry::Instance().Register("flex",
        []() { return std::make_unique<FlexAsmSyntax>(); });
}
```

Update CLI:
```cpp
// In cli_parser.cpp
app.add_option("--syntax", opts.syntax, "Syntax mode")
    ->check(CLI::IsMember({"simple", "merlin", "scmasm", "edtasm", "flex"}));
```

**Tests:**
- CLI recognizes `--syntax flex`
- Can create FlexAsmSyntax instance
- Basic assembly works

**Time:** 1-2 hours

### Step 1.4: Integration Testing

Create test programs in `tests/integration/flex/`:
- `simple.asm` - Basic directives only
- `data.asm` - FCB, FDB, FCC, RMB
- `listing.asm` - NAM, TTL, STTL, PAG, SPC

**Verify:**
- Correct binary output
- Symbol table correct
- Listing control works (if implemented)

**Time:** 2-3 hours

**Phase 1 Total:** 10-15 hours

---

## Phase 2: Macro Processor Implementation

**Goal:** Enable macro definition and expansion

### Step 2.1: Create MacroProcessor Class

**Files:**
- `include/xasm++/syntax/macro_processor.h`
- `src/syntax/flex/macro_processor.cpp`

**Design:**
```cpp
class MacroProcessor {
public:
    struct MacroDefinition {
        std::string name;
        std::vector<std::string> parameters;
        std::vector<std::string> body;    // Raw source lines
        int definition_line;              // For error reporting
    };

    // Define a new macro
    void DefineMacro(const std::string& name,
                    const std::vector<std::string>& parameters,
                    const std::vector<std::string>& body);

    // Check if macro exists
    bool IsMacro(const std::string& name) const;

    // Expand macro with arguments
    std::vector<std::string> ExpandMacro(
        const std::string& name,
        const std::vector<std::string>& arguments,
        int invocation_line);

private:
    std::unordered_map<std::string, MacroDefinition> macros_;
    int expansion_counter_ = 0;  // For unique local labels

    std::string SubstituteParameters(
        const std::string& line,
        const MacroDefinition& macro,
        const std::vector<std::string>& arguments);

    std::string MakeLocalLabelUnique(
        const std::string& label,
        int expansion_id);
};
```

**Tests:**
- `tests/unit/test_macro_processor.cpp`
- Define simple macro
- Expand with parameters
- Local label uniquification

**Time:** 4-5 hours

### Step 2.2: Parse MACRO/ENDM Directives

Add to FlexAsmSyntax:

```cpp
enum class ParserMode {
    Normal,
    InMacroDefinition,
    // ... other modes
};

class FlexAsmSyntax : public MotorolaBaseSyntax {
private:
    MacroProcessor macro_processor_;
    ParserMode mode_ = ParserMode::Normal;
    MacroDefinition current_macro_;  // During definition

    void HandleMACRO(const std::string& name,
                    const std::vector<std::string>& params);
    void HandleENDM();
    void CollectMacroLine(const std::string& line);
};
```

**Implementation Flow:**
1. Encounter MACRO directive → Enter InMacroDefinition mode
2. Collect lines until ENDM
3. Store in macro_processor_
4. Return to Normal mode

**Tests:**
- Parse simple macro definition
- Parse macro with parameters
- Error: ENDM without MACRO
- Error: Nested MACRO (not allowed)

**Time:** 3-4 hours

### Step 2.3: Implement Macro Expansion

During normal parsing, check if line is macro invocation:

```cpp
std::vector<std::shared_ptr<Atom>> FlexAsmSyntax::ParseLine(
    const std::string& line) {

    auto tokens = Tokenize(line);
    if (tokens.empty()) return {};

    // Check if operation field is a macro
    if (macro_processor_.IsMacro(tokens[0])) {
        // Parse arguments
        std::vector<std::string> args = ParseMacroArguments(tokens);

        // Expand macro
        auto expanded_lines = macro_processor_.ExpandMacro(
            tokens[0], args, current_line_number_);

        // Parse expanded lines recursively
        std::vector<std::shared_ptr<Atom>> atoms;
        for (const auto& expanded : expanded_lines) {
            auto expanded_atoms = ParseLine(expanded);
            atoms.insert(atoms.end(),
                        expanded_atoms.begin(),
                        expanded_atoms.end());
        }
        return atoms;
    }

    // Normal instruction/directive handling
    return MotorolaBaseSyntax::ParseLine(line);
}
```

**Parameter Substitution:**
```cpp
std::string MacroProcessor::SubstituteParameters(
    const std::string& line,
    const MacroDefinition& macro,
    const std::vector<std::string>& arguments) {

    std::string result = line;

    // Replace each parameter with its argument
    for (size_t i = 0; i < macro.parameters.size(); ++i) {
        std::string param = macro.parameters[i];
        std::string arg = (i < arguments.size()) ? arguments[i] : "";

        // Use regex or manual replacement
        result = ReplaceAll(result, param, arg);
    }

    return result;
}
```

**Local Label Handling:**
```cpp
std::string MacroProcessor::MakeLocalLabelUnique(
    const std::string& label,
    int expansion_id) {

    if (label[0] == '.') {
        // Local label: .LOOP → .LOOP_001
        return label + "_" + std::to_string(expansion_id);
    }
    return label;  // Global label unchanged
}
```

**Tests:**
- Expand simple macro (no parameters)
- Expand with 1 parameter
- Expand with multiple parameters
- Local label uniqueness
- Nested macro calls (macro calls another macro)

**Time:** 5-6 hours

### Step 2.4: Integration Testing

Create test programs:
- `macro_simple.asm` - Simple macros
- `macro_params.asm` - Macros with parameters
- `macro_local.asm` - Local labels in macros
- `macro_nested.asm` - Nested macro calls

**Verify:**
- Correct expansion
- Parameters substituted correctly
- Local labels unique per invocation
- Binary output matches hand-assembled version

**Time:** 3-5 hours

**Phase 2 Total:** 15-20 hours

---

## Phase 3: Conditional Assembly Implementation

**Goal:** Enable IFC/ENDC directives

### Step 3.1: Create ConditionalStack Class

**Implementation:**
```cpp
class ConditionalStack {
public:
    void Push(bool condition);
    void Pop();
    bool ShouldAssemble() const;  // All conditions must be true
    bool IsEmpty() const;

private:
    std::stack<bool> conditions_;
};
```

**Logic:** Only assemble if all conditions on stack are true.

**Tests:**
- Push true, should assemble
- Push false, should skip
- Nested conditions (true inside false → skip)

**Time:** 2-3 hours

### Step 3.2: Implement IFC/ENDC Directives

Add to FlexAsmSyntax:

```cpp
class FlexAsmSyntax : public MotorolaBaseSyntax {
private:
    ConditionalStack conditional_stack_;

    void HandleIFC(const std::string& condition);
    void HandleENDC();
    bool EvaluateCondition(const std::string& condition);
};
```

**IFC Evaluation:**
```cpp
void FlexAsmSyntax::HandleIFC(const std::string& condition) {
    bool result = EvaluateCondition(condition);
    conditional_stack_.Push(result);
}

bool FlexAsmSyntax::EvaluateCondition(const std::string& condition) {
    // Simple: Check if symbol is defined
    // Advanced: Evaluate expression (condition != 0 is true)
    if (symbol_table_->IsDefined(condition)) {
        return true;
    }

    // Try evaluating as expression
    try {
        int64_t value = expression_evaluator_.Evaluate(condition);
        return value != 0;
    } catch (...) {
        return false;
    }
}
```

**Parsing with Conditionals:**
```cpp
std::vector<std::shared_ptr<Atom>> FlexAsmSyntax::ParseLine(
    const std::string& line) {

    // Check for conditional directives first
    if (IsDirective(line, "IFC")) {
        HandleIFC(ExtractCondition(line));
        return {};  // Directive doesn't generate atoms
    }

    if (IsDirective(line, "ENDC")) {
        HandleENDC();
        return {};
    }

    // Skip line if condition is false
    if (!conditional_stack_.ShouldAssemble()) {
        return {};  // Skip this line
    }

    // Normal parsing
    return MotorolaBaseSyntax::ParseLine(line);
}
```

**Tests:**
- IFC with defined symbol (should assemble)
- IFC with undefined symbol (should skip)
- Nested IFC/ENDC
- Error: ENDC without IFC

**Time:** 4-5 hours

### Step 3.3: Integration Testing

Create test programs:
- `conditional_simple.asm` - Basic IFC/ENDC
- `conditional_nested.asm` - Nested conditionals
- `conditional_expression.asm` - Expression-based conditions

**Verify:**
- Code included when condition true
- Code skipped when condition false
- Nested conditionals work correctly

**Time:** 2-3 hours

**Phase 3 Total:** 8-10 hours

---

## Phase 4: Advanced Directives Implementation

**Goal:** Implement remaining FLEX directives

### Step 4.1: LIB (Library Inclusion)

**Implementation:**
```cpp
void FlexAsmSyntax::HandleLIB(const std::string& filename) {
    std::string path = FindLibraryFile(filename);
    if (path.empty()) {
        ReportError("Library file not found: " + filename);
        return;
    }

    // Recursively parse the library file
    auto library_atoms = ParseFile(path);
    current_section_->atoms.insert(
        current_section_->atoms.end(),
        library_atoms.begin(),
        library_atoms.end()
    );
}
```

**Library Search Path:**
- Current directory
- Library paths (configurable)

**Time:** 3-4 hours

### Step 4.2: REG (Register Lists)

**Implementation:**
```cpp
void FlexAsmSyntax::HandleREG(const std::string& name,
                               const std::string& registers) {
    // Store register list as a macro-like expansion
    reg_lists_[name] = ParseRegisterList(registers);
}

std::vector<std::string> ParseRegisterList(const std::string& list) {
    // Parse comma-separated list: A,B,X,Y
    // Return vector of register names
}
```

**Usage:** Allow register list name in PSHS/PULS:
```asm
REGS    REG     A,B,X,Y
        PSHS    REGS        ; Expands to PSHS A,B,X,Y
```

**Time:** 2-3 hours

### Step 4.3: OPT (Assembler Options)

**Implementation:**
```cpp
void FlexAsmSyntax::HandleOPT(const std::string& options) {
    for (char opt : options) {
        switch (opt) {
            case 'C':  // Cycle counting
                enable_cycle_counting_ = true;
                break;
            case 'L':  // Listing
                enable_listing_ = true;
                break;
            case 'P':  // Pagination
                enable_pagination_ = true;
                break;
            // ... other options
        }
    }
}
```

**Time:** 2-3 hours

### Step 4.4: ERR (Force Error)

**Implementation:**
```cpp
void FlexAsmSyntax::HandleERR(const std::string& message) {
    ReportError(message);
}
```

**Simple but useful for validation.**

**Time:** 1 hour

### Step 4.5: RPT/ENDR (Repeat Blocks)

**Implementation:**
```cpp
void FlexAsmSyntax::HandleRPT(int count) {
    repeat_stack_.push({count, {}});
    mode_ = ParserMode::InRepeatBlock;
}

void FlexAsmSyntax::HandleENDR() {
    auto repeat_info = repeat_stack_.top();
    repeat_stack_.pop();

    // Expand the block 'count' times
    for (int i = 0; i < repeat_info.count; ++i) {
        for (const auto& line : repeat_info.lines) {
            ParseLine(line);
        }
    }

    mode_ = ParserMode::Normal;
}
```

**Time:** 3-4 hours

### Step 4.6: Testing

Create test programs for each advanced directive.

**Time:** 2-3 hours

**Phase 4 Total:** 10-15 hours

---

## Phase 5: Documentation and Polish

### Step 5.1: Update User Documentation

**Files to Update:**
- `docs/content/intro.md` - Add FLEX to supported syntaxes
- `docs/content/getting-started/quick-start.md` - Add FLEX example
- `docs/content/reference/flex/flex-asm-syntax-reference.md` - Already created ✅

**New Content:**
- FLEX ASM programming guide (examples, best practices)

**Time:** 2-3 hours

### Step 5.2: Update Developer Documentation

**Create:**
- `docs/architecture/flex-macro-processor.md` - Macro processor design
- `docs/api/flex-syntax-api.md` - FlexAsmSyntax API documentation

**Time:** 2-3 hours

### Step 5.3: Final Integration Testing

- Run full test suite
- Test with complex FLEX programs
- Validate output against expected results

**Time:** 2-3 hours

**Phase 5 Total:** 6-9 hours

---

## Execution Strategy

### Parallel vs Sequential

**Sequential Execution** (Recommended for this task):
1. Phase 1 → Foundation for all other phases
2. Phase 2 → Macros build on basic directives
3. Phase 3 → Conditionals are independent but benefit from testing infrastructure
4. Phase 4 → Advanced directives use macro processor
5. Phase 5 → Documentation after features complete

**Rationale:** Each phase builds on previous phases. Parallel execution would require coordination overhead.

---

## Risk Mitigation

### Risk 1: Macro Processor Complexity

**Mitigation Strategy:**
- Start with simplest macros (no parameters)
- Add parameters incrementally
- Extensive unit testing at each step
- Reference EDTASM macro implementation (if exists) or other assemblers

**Checkpoint:** After Step 2.2, evaluate complexity. If too complex, consider:
- Shipping without macros in v1.0
- Adding macros in v1.1

### Risk 2: Testing Without Real FLEX Code

**Mitigation Strategy:**
- Create comprehensive synthetic test cases
- Base tests on examples in FLEX manual
- Use FLEX ASM reference manual examples
- Search for FLEX source code online (if available)

**Checkpoint:** After Phase 1, search for real FLEX programs to test with.

### Risk 3: Performance of Macro Expansion

**Mitigation Strategy:**
- Profile macro expansion in Step 2.3
- Optimize if needed (caching, better string handling)
- Set reasonable limits (max macro depth, max expansions)

**Checkpoint:** After Step 2.4 integration testing, measure performance.

---

## Checkpoints and Decision Points

### Checkpoint 1: After Phase 1 (Basic Directives)

**Questions:**
- Do basic directives work correctly?
- Is inheritance from MotorolaBaseSyntax working well?
- Are we achieving ~60% code reuse?

**Decision:** Continue to Phase 2 or refactor base if needed.

### Checkpoint 2: After Step 2.2 (Macro Definition Parsing)

**Questions:**
- Is macro definition parsing clean?
- Are we comfortable with the design?
- Is testing infrastructure adequate?

**Decision:** Continue to expansion or redesign if needed.

### Checkpoint 3: After Phase 2 (Macro Processor Complete)

**Questions:**
- Does macro expansion work reliably?
- Is performance acceptable?
- Do we have confidence in the implementation?

**Decision:** Continue to Phase 3 or polish Phase 2.

### Checkpoint 4: After Phase 3 (Conditional Assembly)

**Questions:**
- Do conditionals work correctly?
- Are nested conditionals handled well?
- Is error handling adequate?

**Decision:** Continue to Phase 4 or add more tests.

### Final Checkpoint: After Phase 4

**Questions:**
- Are all features implemented?
- Do all tests pass?
- Is documentation complete?

**Decision:** Ship or add more polish.

---

## Success Metrics

### Code Quality
- [ ] All unit tests pass (target: 100+ tests)
- [ ] Code coverage >80%
- [ ] Zero compiler warnings
- [ ] Passes Reviewer validation

### Functionality
- [ ] Can assemble simple FLEX programs
- [ ] Macros work with parameters
- [ ] Conditional assembly works
- [ ] Advanced directives implemented

### Performance
- [ ] Assembly time comparable to EDTASM
- [ ] Macro expansion is efficient

### Documentation
- [ ] User guide complete
- [ ] API documentation complete
- [ ] Examples work

---

## Timeline Estimate

**Minimum (Best Case):** 45 hours (9 working days at 5 hours/day)

**Expected (Typical):** 52 hours (10-11 working days)

**Maximum (With Issues):** 60 hours (12 working days)

**Recommendation:** Plan for 2-3 weeks of calendar time (accounting for other work).

---

## Next Steps

1. **Review this plan** - Ensure approach is sound
2. **Architecture design** - Document macro processor design
3. **Begin Phase 1** - Implement basic directives
4. **Iterate** - Follow plan, adjust as needed

**Status:** ✅ Plan Complete - Ready to Execute
