# Implementation Plan

**Task ID:** 2026-01-24_merlin-syntax-support
**Created:** 2026-01-24
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Based on comprehensive analysis of Prince of Persia source code (30,900 lines, 29 files), this plan prioritizes implementing Merlin features **by actual usage frequency** rather than attempting to implement all 227 directives from vasm-ext.

**Key Finding:** PoP uses a focused subset of Merlin features:
- 6502 only (no 65816)
- No macros (surprising but true!)
- Heavy reliance on: local labels (2,077), DUM/]variables (365), comments (5,391)
- Minimal strings (2 ASC directives total)

This allows us to build a **Minimal Viable Merlin (MVM)** focused on what real code actually needs.

**Key Technical Decisions:**
- **Phased implementation by usage tier** - Implement most-used features first (comments, local labels, data directives) before rarely-used features (macros, strings)
- **Leverage existing infrastructure** - Use xasm++ Assembler/Section/Symbol/Expression classes, only add Merlin-specific parsing layer
- **Port concepts from vasm-ext, not code** - Adapt C implementation patterns to C++ idioms (std::string, std::vector, smart pointers)
- **Test-driven with real-world validation** - Unit tests for each feature + integration test assembling actual PoP files

**Patterns to Use:**
- **Parser pattern** - SyntaxPlugin reads lines, delegates to directive handlers
- **Strategy pattern** - Separate handlers for each directive type (data, storage, control)
- **Symbol table extensions** - Add local label scoping (:label) and variable label tracking (]label)
- **Visitor pattern** - Expression evaluation leverages existing Expression evaluator

---

## Prince of Persia Usage Analysis

From analyzing ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/:

### Implementation Priority by Frequency

**TIER 1 - CRITICAL (>500 uses):**
| Feature | Count | Impact |
|---------|-------|--------|
| Comments (`*` and `;`) | 5,391 | Can't parse any file without this |
| Data (`db`) | 922 | Core data definition |
| Local labels (`:`) | 2,077 | Every subroutine uses these |
| Variables (`]`) | 730 | DUM blocks + references |
| ORG directive | 44 | Sets code origin |

**TIER 2 - VERY HIGH (>50 uses):**
| Feature | Count | Notes |
|---------|-------|-------|
| HEX directive | 197 | Hex literal arrays |
| DW (16-bit) | 134 | Address tables |
| DUM/DEND | 130 | Memory allocation |
| LST control | 69 | Listing on/off |
| PUT (include) | 63 | File inclusion |
| DS (reserve) | 53 | Space allocation |

**TIER 3 - IMPORTANT (20-50 uses):**
| Feature | Count | Notes |
|---------|-------|-------|
| DO/FIN conditional | 49 | Compile-time conditionals |
| DFB (signed bytes) | 44 | Special data format |
| TR (trace) | 24 | Trace mode |

**TIER 4 - OPTIONAL (<20 uses):**
| Feature | Count | Notes |
|---------|-------|-------|
| ASC (strings) | 2 | Minimal string usage |
| LSTDO | 1 | Rare directive |

**NOT USED IN PoP:**
- Macros (MAC/<<<) - 0 instances
- 65816 features (MX, LONGA, LONGI) - 0 instances
- String directives (DCI, INV, FLS, REV) - 0 instances

---

## Critical Files Identified

### Files to Create

```
1. src/syntax/merlin/merlin_syntax.h
   - Purpose: Merlin syntax plugin interface
   - Contents: Class declaration, directive handlers
   - Based on: include/xasm++/syntax/syntax_plugin.h

2. src/syntax/merlin/merlin_syntax.cpp
   - Purpose: Merlin syntax implementation
   - Contents: Parser, directive handlers, label management
   - Size estimate: ~2,000-3,000 lines
   - Reference: vasm-ext/syntax/merlin/syntax.c (4,219 lines)

3. tests/unit/test_merlin_syntax.cpp
   - Purpose: Unit tests for Merlin syntax
   - Contents: Tests for each directive, label type, feature
   - Size estimate: ~1,500-2,000 lines
   - Pattern: tests/unit/test_cpu6502.cpp (2,271 lines)

4. tests/integration/test_merlin_pop.cpp
   - Purpose: Integration test with actual PoP source
   - Contents: Assemble select PoP files, verify output
   - Files to test: Start with EQ.S, then SUBS.S, then complex files
```

### Files to Modify

```
1. src/CMakeLists.txt
   - Add: src/syntax/merlin/merlin_syntax.cpp to xasm_syntax library
   - Pattern: Same as cpu/m6502/*.cpp additions

2. tests/unit/CMakeLists.txt
   - Add: test_merlin_syntax executable

3. tests/integration/CMakeLists.txt (create if needed)
   - Add: test_merlin_pop executable with PoP source path
```

### Files to Read (for context)

```
1. include/xasm++/syntax/syntax_plugin.h
   - Current interface to understand
   - Methods to implement

2. src/syntax/simple_syntax.cpp
   - Existing basic implementation
   - Pattern to follow (or replace)

3. include/xasm++/core/assembler.h
   - Assembler API for directives
   - Symbol table interface
   - Expression evaluation

4. vasm-ext/syntax/merlin/syntax.c (reference)
   - Lines 82-194: Variable label management
   - Lines 2737-2967: Directive table
   - Lines 4109-4141: init_syntax()

5. ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/01 POP Source/Source/
   - EQ.S: Simple equates file (good starting test)
   - SUBS.S: Subroutines with local labels
   - MOVER.S: Complex file with DUM blocks
```

---

## Implementation Phases

### Phase 1: Foundation (Week 1)
**Goal:** Parse basic Merlin syntax, assemble simple programs

**Features:**
- ✅ Comment parsing (`*` and `;`)
- ✅ Basic line parsing (label, directive, operand, comment)
- ✅ Global labels (no `:` or `]` yet)
- ✅ Core directives: ORG, EQU, DB, DW, HEX
- ✅ Number formats: $hex, %binary, decimal
- ✅ Simple expressions: +, -, *, /, &, |, ^

**Test:** Assemble simple "Hello World" equivalent
```asm
* Hello World equivalent
        org $2000
start   db $00
        hex 48656C6C6F  ; "Hello"
len     equ *-start
```

**Deliverables:**
- merlin_syntax.h/cpp with basic parser
- test_merlin_syntax.cpp with Phase 1 tests
- All tests passing (100%)

---

### Phase 2: Local Labels (Week 1-2)
**Goal:** Three-tier label system working

**Features:**
- ✅ `:LOCAL` labels (scoped to last global)
- ✅ Forward/backward references
- ✅ Label scoping logic
- ✅ Anonymous labels (`:+`, `:-`) - optional

**Test:** Assemble code with subroutines using local labels
```asm
        org $2000
main    
        jsr sub1
        jsr sub2
        rts
        
sub1    ldx #0
:loop   inx
        cpx #10
        bne :loop
        rts
        
sub2    ldy #0
:loop   iny         ; Different :loop, different scope
        cpy #5
        bne :loop
        rts
```

**Deliverables:**
- Local label scoping implemented
- Symbol table extended with scope tracking
- Tests for all label scenarios
- Can assemble PoP's SUBS.S (if it has local labels)

---

### Phase 3: DUM Blocks & Variables (Week 2)
**Goal:** Memory allocation and `]` variable labels working

**Features:**
- ✅ DUM/DEND blocks (define unnamed memory)
- ✅ `]VARIABLE` labels in DUM blocks
- ✅ DS (define space) directive
- ✅ Variable label references

**Test:** Assemble memory structure definitions
```asm
        dum $f0      ; Zero-page variables
]dest   ds 2         ; Local variable
]source ds 2
]count  ds 1
        dend
        
        org $2000
        lda ]dest    ; Reference variable
        sta ]source
```

**PoP Example from analysis:**
```asm
 dum locals
]dest ds 2
]source ds 2
]endsourc ds 2
]HiPitch = 100
]rts rts
 dend
```

**Deliverables:**
- DUM/DEND implementation
- ]VARIABLE tracking (mutable labels)
- DS directive
- Tests for PoP-style patterns
- Can assemble files with DUM blocks

---

### Phase 4: File Inclusion (Week 2-3)
**Goal:** PUT directive working, can assemble multi-file projects

**Features:**
- ✅ PUT filename (include source file)
- ✅ Path resolution (relative paths)
- ✅ Circular include detection
- ✅ Nested includes

**Test:** Assemble project with includes
```asm
* main.s
        lst
        put eq       ; Include EQ.S
        lst
        put subs     ; Include SUBS.S
        org $2000
start   jmp main
```

**Deliverables:**
- PUT implementation
- File system integration
- Include path management
- Tests with multiple files
- Can assemble PoP's main files that PUT equates

---

### Phase 5: Data Directives (Week 3)
**Goal:** All data formats working

**Features:**
- ✅ DFB (define font/signed bytes)
- ✅ Multiple values per directive
- ✅ Expression evaluation in data
- ✅ Labels in data (address tables)

**Test:** Assemble data tables like PoP uses
```asm
lookup
:0      dw DoPlateDown
:1      dw DoPlateUp
:2      dw DoGateDown
endlook

maxaddr equ endlook-lookup

levels  db 33,0
        db 33,33,32
        hex 00,03,03,03
        dfb 00,00,00,00
```

**Deliverables:**
- All data directive variants
- Expression evaluation in data context
- Tests for PoP data patterns
- Can assemble PoP's SEQTABLE.S and data files

---

### Phase 6: Conditional Assembly (Week 3-4)
**Goal:** DO/FIN conditionals working

**Features:**
- ✅ DO condition (if)
- ✅ ELSE clause
- ✅ FIN (endif)
- ✅ Nested conditionals
- ✅ Compile-time expression evaluation

**Test:** Assemble with conditionals
```asm
DemoDisk = 0

        do DemoDisk
        org $8000    ; Demo version
        else
        org $2000    ; Full version
        fin
        
        do CheckTimer
min     equ 180
        else
min     equ 1090
        fin
```

**Deliverables:**
- DO/ELSE/FIN implementation
- Conditional expression evaluator
- Nested conditional handling
- Tests for PoP conditional patterns
- Can assemble PoP files with DO/FIN

---

### Phase 7: Listing Control (Week 4)
**Goal:** Assembler control directives

**Features:**
- ✅ LST (listing on)
- ✅ LST OFF (listing off)
- ✅ LSTDO (list DO blocks)
- ✅ TR (trace mode)

**Note:** These can be mostly no-ops for cross-assembly, but need to parse correctly.

**Test:** Assemble with listing directives
```asm
        lst          ; Enable listing
        put eq
        lst off      ; Disable listing
        put data
        lst          ; Re-enable
```

**Deliverables:**
- LST/LSTDO/TR parsing (implementations can be stubs)
- Tests that directives are accepted
- Can assemble all PoP files without syntax errors

---

### Phase 8: Real-World Validation (Week 4-5)
**Goal:** Assemble actual Prince of Persia files

**Target Files (in order of complexity):**

1. **EQ.S** (simplest) - Pure equates
   - Expected: ~200 lines
   - Features: Comments, EQU, simple expressions
   - Success: Produces correct symbol table

2. **GAMEEQ.S** - Game equates with conditionals
   - Expected: ~300 lines
   - Features: DO/FIN, complex expressions
   - Success: Correct values under different conditions

3. **SUBS.S** - Subroutines with local labels
   - Expected: ~1,000 lines
   - Features: ORG, local labels, simple code
   - Success: Assembles to correct binary

4. **MOVER.S** - Complex file with DUM blocks
   - Expected: ~2,174 lines
   - Features: DUM/DEND, ]variables, complex logic
   - Success: Assembles without errors

5. **MASTER.S** - Main file with includes
   - Expected: ~500 lines + includes
   - Features: PUT directives, pulls everything together
   - Success: Full project assembles

**Validation Strategy:**
- Compare binary output with known-good assembly (if available)
- Verify symbol table matches expected addresses
- Check that all directives parse without errors
- Measure coverage: what % of PoP syntax we support

**Deliverables:**
- Integration tests for each target file
- Documentation of unsupported features (if any)
- Performance metrics (assembly time)
- Coverage report
- Compatibility matrix (what works, what doesn't)

---

## Architecture Design

### Class Structure

```cpp
// merlin_syntax.h

namespace xasm {

class MerlinSyntax : public SyntaxPlugin {
public:
    MerlinSyntax();
    ~MerlinSyntax() override;
    
    // SyntaxPlugin interface
    bool ParseLine(const std::string& line, Assembler& assembler) override;
    std::string GetName() const override { return "Merlin"; }
    
private:
    // Label management
    struct LabelScope {
        std::string global_label;
        std::map<std::string, uint32_t> local_labels;  // :label → address
    };
    
    struct VariableLabel {
        std::string name;           // ]VARIABLE
        std::string unique_name;    // Internal unique identifier
        uint32_t address;
        bool defined;
    };
    
    LabelScope current_scope_;
    std::map<std::string, VariableLabel> variable_labels_;
    
    // DUM block state
    bool in_dum_block_ = false;
    uint32_t dum_address_ = 0;
    
    // Conditional assembly state
    struct ConditionalBlock {
        bool condition_met;
        bool in_else_clause;
    };
    std::stack<ConditionalBlock> conditional_stack_;
    
    // Parsing helpers
    bool ParseLabel(const std::string& label, Assembler& assembler);
    bool ParseDirective(const std::string& directive, 
                       const std::string& operand,
                       Assembler& assembler);
    
    // Directive handlers (by category)
    void HandleOrg(const std::string& operand, Assembler& assembler);
    void HandleEqu(const std::string& label, const std::string& operand, Assembler& assembler);
    void HandleDB(const std::string& operand, Assembler& assembler);
    void HandleDW(const std::string& operand, Assembler& assembler);
    void HandleHex(const std::string& operand, Assembler& assembler);
    void HandleDS(const std::string& operand, Assembler& assembler);
    
    void HandleDum(const std::string& operand, Assembler& assembler);
    void HandleDend(Assembler& assembler);
    
    void HandlePut(const std::string& filename, Assembler& assembler);
    
    void HandleDo(const std::string& condition, Assembler& assembler);
    void HandleElse(Assembler& assembler);
    void HandleFin(Assembler& assembler);
    
    void HandleLst(const std::string& operand, Assembler& assembler);
    
    // Expression evaluation
    int64_t EvaluateExpression(const std::string& expr, Assembler& assembler);
    
    // Label utilities
    std::string ResolveLabel(const std::string& label, Assembler& assembler);
    void DefineLocalLabel(const std::string& label, uint32_t address);
    void DefineVariableLabel(const std::string& label, uint32_t address);
    
    // Parsing utilities
    struct ParsedLine {
        std::string label;
        std::string directive;
        std::string operand;
        std::string comment;
    };
    
    ParsedLine ParseLineComponents(const std::string& line);
    bool IsComment(const std::string& line);
    bool IsDirective(const std::string& word);
};

} // namespace xasm
```

### Integration with Existing Code

```cpp
// In Assembler class (existing):
class Assembler {
public:
    // Already exists:
    void DefineSymbol(const std::string& name, uint64_t value);
    uint64_t LookupSymbol(const std::string& name);
    void SetOrigin(uint64_t address);
    void EmitByte(uint8_t byte);
    void EmitWord(uint16_t word);
    
    // May need to add:
    void PushScope(const std::string& scope_name);  // For :local labels
    void PopScope();
    void IncludeFile(const std::string& filename);  // For PUT
};
```

---

## Testing Strategy

### Unit Tests (test_merlin_syntax.cpp)

**Test Organization by Feature:**

```cpp
// Comment tests
TEST(MerlinSyntaxTest, AsteriskComment)
TEST(MerlinSyntaxTest, SemicolonComment)
TEST(MerlinSyntaxTest, BlankLines)

// Label tests
TEST(MerlinSyntaxTest, GlobalLabel)
TEST(MerlinSyntaxTest, LocalLabel_ForwardReference)
TEST(MerlinSyntaxTest, LocalLabel_BackwardReference)
TEST(MerlinSyntaxTest, LocalLabel_ScopeChange)
TEST(MerlinSyntaxTest, VariableLabel_Definition)
TEST(MerlinSyntaxTest, VariableLabel_Reference)
TEST(MerlinSyntaxTest, VariableLabel_InDumBlock)

// Directive tests
TEST(MerlinSyntaxTest, ORG_Directive)
TEST(MerlinSyntaxTest, EQU_Directive)
TEST(MerlinSyntaxTest, DB_SingleValue)
TEST(MerlinSyntaxTest, DB_MultipleValues)
TEST(MerlinSyntaxTest, DW_AddressTable)
TEST(MerlinSyntaxTest, HEX_HexString)
TEST(MerlinSyntaxTest, DS_ReserveSpace)

// DUM tests
TEST(MerlinSyntaxTest, DUM_BasicBlock)
TEST(MerlinSyntaxTest, DUM_WithVariables)
TEST(MerlinSyntaxTest, DUM_DEND_Pairing)

// PUT tests
TEST(MerlinSyntaxTest, PUT_IncludeFile)
TEST(MerlinSyntaxTest, PUT_RelativePath)
TEST(MerlinSyntaxTest, PUT_CircularDetection)

// Conditional tests
TEST(MerlinSyntaxTest, DO_True)
TEST(MerlinSyntaxTest, DO_False)
TEST(MerlinSyntaxTest, DO_ELSE_FIN)
TEST(MerlinSyntaxTest, DO_Nested)

// Expression tests
TEST(MerlinSyntaxTest, Expression_HexNumber)
TEST(MerlinSyntaxTest, Expression_BinaryNumber)
TEST(MerlinSyntaxTest, Expression_Arithmetic)
TEST(MerlinSyntaxTest, Expression_LabelReference)
```

### Integration Tests (test_merlin_pop.cpp)

```cpp
TEST(MerlinPopTest, Assemble_EQ_S) {
    // Assemble PoP's EQ.S file
    // Verify symbol table has expected values
}

TEST(MerlinPopTest, Assemble_SUBS_S) {
    // Assemble PoP's SUBS.S file
    // Verify binary output (if reference available)
}

TEST(MerlinPopTest, Assemble_MOVER_S) {
    // Most complex single file
    // Success = no errors
}

TEST(MerlinPopTest, Assemble_MASTER_S) {
    // Full project assembly
    // Verify all includes work
}
```

**Test Data:**
- Copy select PoP source files to `tests/data/pop/`
- Known-good assembly outputs (if available)
- Symbol table snapshots for validation

---

## Dependencies and Risks

### Dependencies

**Phase 1:**
- Assembler class with DefineSymbol(), SetOrigin(), EmitByte()
- Expression evaluator (basic arithmetic)

**Phase 2:**
- Symbol table with scope support (may need to add)

**Phase 3:**
- Symbol table with forward reference resolution

**Phase 4:**
- File system API for includes
- Path resolution

**Phase 5-8:**
- All previous phases working

### Technical Risks

**Risk 1: Variable label complexity**
- **Likelihood:** Medium
- **Impact:** High (blocks DUM block usage)
- **Mitigation:** 
  - Reference vasm-ext implementation (lines 82-200)
  - Start simple (no mutations), add complexity incrementally
  - Comprehensive tests

**Risk 2: Expression evaluator limitations**
- **Likelihood:** Medium
- **Impact:** Medium (may need to extend existing Expression class)
- **Mitigation:**
  - Review existing xasm++ Expression implementation
  - Identify gaps vs Merlin needs
  - Add Merlin-specific operators if needed (< > for low/high byte)

**Risk 3: Symbol table scoping**
- **Likelihood:** Low
- **Impact:** High (breaks local labels)
- **Mitigation:**
  - Local labels may need custom tracking outside main symbol table
  - Prefix transformation (e.g., "main:loop" → "main$loop")
  - Test scoping thoroughly

**Risk 4: PoP has unsupported features**
- **Likelihood:** Low (analysis shows focused feature set)
- **Impact:** Medium (may need to add features mid-implementation)
- **Mitigation:**
  - Analysis complete, surprises unlikely
  - Phased approach allows adding features as discovered
  - Mark unsupported features as "future work"

### Process Risks

**Risk 5: Scope creep**
- **Likelihood:** High (227 directives in vasm-ext!)
- **Impact:** High (delays PoP assembly)
- **Mitigation:**
  - **Strict prioritization by PoP usage**
  - Implement only what PoP actually uses
  - Defer rarely-used features (ASC strings, macros)
  - Mark Phase 8 as "must succeed" gate

**Risk 6: TDD discipline**
- **Likelihood:** Medium
- **Impact:** Medium (quality issues)
- **Mitigation:**
  - RED-GREEN-REFACTOR strictly enforced
  - Atomic commits per feature
  - Tester/Reviewer validation required
  - Code review focuses on test coverage

---

## Success Metrics

### Quantitative Metrics

**Code Coverage:**
- Target: ≥85% overall, 95%+ new code
- Measure: `cmake --build build --target coverage`

**Test Pass Rate:**
- Target: 100% passing at each phase
- No phase begins until previous phase 100% passing

**PoP Assembly Success:**
- Phase 8 target: 5/5 files assemble successfully
- Stretch: All 29 files assemble

**Performance:**
- Target: Assemble PoP MASTER.S in <1 second
- Measure: Integration test timing

### Qualitative Metrics

**Code Quality:**
- Zero compiler warnings
- Reviewer approval (APPROVED)
- Follows xasm++ patterns and idioms

**Documentation:**
- Inline comments for complex logic
- Examples in tests
- Work log complete with rationale

**Maintainability:**
- Clear separation of concerns
- Easy to add new directives
- Follows vasm-ext patterns where applicable

---

## Timeline Estimate

**Phase 1-2:** ~1-2 weeks (foundation + local labels)
**Phase 3-4:** ~1 week (DUM blocks + PUT)
**Phase 5-7:** ~1-2 weeks (data directives, conditionals, listing)
**Phase 8:** ~1 week (validation and fixes)

**Total:** 4-6 weeks for full implementation

**First success:** Week 2 (can assemble simple files with local labels)
**PoP assembly:** Week 5-6 (all phases complete)

---

## Open Questions

1. **Does xasm++ Expression class support all operators PoP uses?**
   - Need to review: +, -, *, /, &, |, ^, <, >, =
   - May need to add: < (low byte), > (high byte)

2. **Can Symbol table support scoped labels?**
   - Does it have scope stacks?
   - Or do we prefix labels (main:loop → main$loop)?

3. **How do we handle PoP's bank switching in emulation?**
   - RAMRDaux, RAMWRTaux - are these just labels?
   - Or do we need special handling?

4. **Binary output format?**
   - PoP expects Apple II DOS 3.3 binary format?
   - Or just raw binary for now?

---

## Next Steps

**Immediate (Week 1):**
1. Create merlin_syntax.h/cpp skeleton
2. Implement Phase 1 (foundation)
3. Write tests for comments and basic directives
4. Integrate with build system

**Then:** Continue through phases sequentially, validating at each step.

**Success Criterion:** By Week 6, assemble PoP's MASTER.S successfully.

---

**Plan Approved By:**
- [ ] Orchestrator: [Name] - [Date]
- [ ] Engineer: [Name] - [Date]

**Ready to Execute:** [ ]
