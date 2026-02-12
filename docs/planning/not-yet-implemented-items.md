# Not Yet Implemented Items - Work Required

**Generated:** 2026-02-11
**Source:** Code scan of src/ and include/ directories

---

## Summary

**Total Items:** 49 not-yet-implemented items found
**Categories:** 7 major categories

---

## 1. EDTASM-M80++ Conditional Directives (13 items)

**Priority:** High (conditional assembly is fundamental)
**Location:** `src/syntax/edtasm_m80_plusplus/edtasm_directive_handlers.cpp`

### Directives to Implement

```
□ IFDEF   - If symbol defined
□ IFNDEF  - If symbol not defined
□ IFEQ    - If equal
□ IFNE    - If not equal
□ IFLT    - If less than
□ IFGT    - If greater than
□ IFLE    - If less than or equal
□ IFGE    - If greater than or equal
□ IF1     - If pass 1
□ IF2     - If pass 2
□ IFB     - If blank
□ IFNB    - If not blank
□ IFIDN   - If identical
□ IFDIF   - If different
```

### Implementation Approach

- Create conditional evaluation engine
- Support nested conditionals
- Track conditional state per pass
- Expression evaluation for comparison operators

### Estimated Effort
- Medium-Large (6-8 hours)
- **Files:** 3-4 (handler, conditional evaluator, tests)
- **Tokens:** ~18K

### Reference
- Similar to Merlin IF/ELSE/ENDIF (already implemented)
- M80/ZMAC assembler documentation

---

## 2. EDTASM-M80++ Macro Directives (7 items)

**Priority:** High (macros are essential for real code)
**Location:** `src/syntax/edtasm_m80_plusplus/edtasm_directive_handlers.cpp`

### Directives to Implement

```
□ MACRO   - Define macro
□ ENDM    - End macro definition
□ LOCAL   - Define local symbols
□ EXITM   - Exit macro early
□ REPT    - Repeat block
□ IRP     - Iterate over parameters
□ IRPC    - Iterate over characters
```

### Implementation Approach

- Reference FLEX ASM macro processor (already implemented in xasm++-684)
- Adapt macro processor for EDTASM-M80++ syntax
- Parameter substitution (& and %)
- Local label uniquification
- Nested macro support

### Estimated Effort
- Large (8-12 hours)
- **Files:** 5-6 (macro processor, handlers, tests)
- **Tokens:** ~24K
- **Note:** Similar implementation exists for FLEX ASM - can reuse pattern

### Reference
- Phase 12 contract: `.ai/tasks/2026-02-10_z80-phase12-macros/00-contract.md`
- FLEX ASM macro processor: `src/syntax/motorola/flex_syntax.cpp`

---

## 3. EDTASM-M80++ Listing Directives (3 items)

**Priority:** Medium (listing output exists, directives incomplete)
**Location:** `src/syntax/edtasm_m80_plusplus/edtasm_directive_handlers.cpp`

### Directives to Implement

```
□ LIST    - Enable listing output
□ NOLIST  - Disable listing output
□ *LIST   - Special listing control (verify exact behavior)
```

### Implementation Approach

- Integrate with existing listing output (`listing_output.cpp`)
- Toggle listing state in directive handlers
- Macro expansion control (see TODO in `listing_output.cpp`)

### Estimated Effort
- Small (2-3 hours)
- **Files:** 2-3 (handler, listing integration, tests)
- **Tokens:** ~9K

### Reference
- Listing output: `src/output/listing_output.cpp`
- Phase 13 contract: `.ai/tasks/2026-02-10_z80-phase13-listing/00-contract.md`

---

## 4. EDTASM-M80++ Segment Directives (3 items)

**Priority:** Medium (segment management for relocatable code)
**Location:** `src/syntax/edtasm_m80_plusplus/edtasm_directive_handlers.cpp`

### Directives to Implement

```
□ ASEG    - Absolute segment (fixed address)
□ CSEG    - Code segment (relocatable code)
□ DSEG    - Data segment (relocatable data)
```

### Implementation Approach

- Implement segment switching mechanism
- Track current segment type
- Support relocatable vs absolute addressing
- Object file format integration (may be deferred)

### Estimated Effort
- Medium (4-6 hours)
- **Files:** 3-4 (segment manager, handlers, tests)
- **Tokens:** ~15K
- **Note:** May require object file format design (out of current scope)

### Reference
- M80/ZMAC relocatable code documentation
- Segment manager: `include/xasm++/segment_manager.h`

---

## 5. Merlin LUP Directive (1 item)

**Priority:** Low (loop directive, less common)
**Location:** `src/directives/merlin_directives.cpp`

### Directive to Implement

```
□ LUP     - Loop directive (repeat code N times)
```

### Implementation Approach

- Similar to REPT in macros
- Expand loop body N times
- Support nested loops
- Local label generation

### Estimated Effort
- Small (2-3 hours)
- **Files:** 2 (handler, tests)
- **Tokens:** ~6K

### Reference
- Merlin assembler documentation
- Similar to REPT implementation

---

## 6. 6809 Indexed Addressing Modes (11 items)

**Priority:** High (fundamental for 6809 code)
**Location:** `src/cpu/m6809/cpu_6809.cpp`

### Missing Implementations

```
□ Indexed addressing for all 11 instruction variants
  - Post-byte encoding
  - Offset calculation
  - Register selection
  - Indirect modes
```

### Implementation Approach

- Implement indexed mode post-byte generation
- Support all variants: ,X / ,Y / ,U / ,S / ,PCR
- Offset encoding (5-bit, 8-bit, 16-bit)
- Indirect modes

### Estimated Effort
- Medium (4-6 hours)
- **Files:** 2 (cpu_6809.cpp, tests)
- **Tokens:** ~12K

### Reference
- 6809 architecture: `docs/reference/cpu/6809-levanthal.txt`
- Opcode verification: `docs/architecture/verification-6809-opcodes.md`

---

## 7. Expression Parser Integration (2 items)

**Priority:** Medium (improves expression handling)
**Location:** `src/syntax/core_directive_handlers.cpp`

### TODOs

```
□ Full expression parser integration
□ Handle arithmetic expressions (1+2, SIZE*2, etc.)
```

### Implementation Approach

- Complete ExpressionParser integration (Phase 2 partially done)
- Support all arithmetic operators
- Symbol resolution in expressions
- Nested expression evaluation

### Estimated Effort
- Small (2-3 hours)
- **Files:** 2 (handlers, tests)
- **Tokens:** ~6K
- **Note:** Phase 2 already integrated ExpressionParser, may just need refinement

### Reference
- Phase 2 contracts: `.ai/tasks/2026-02-10_phase2*-integration/`
- Expression parser: `include/xasm++/expression.h`

---

## 8. Miscellaneous Items (9 items)

### 8.1. Listing Macro Expansion Control (1 item)
**Location:** `src/output/listing_output.cpp`
```
□ TODO: Macro expansion control
```
**Effort:** Small (1-2 hours) - Part of listing directive implementation

### 8.2. Instruction Size Calculation (1 item)
**Location:** `src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.cpp`
```
□ TODO: Calculate instruction size (for now, assume 1 byte minimum)
```
**Effort:** Small (1-2 hours) - Integrate with CPU encoders

### 8.3. Symbol Visibility Tracking (1 item)
**Location:** `src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.cpp`
```
□ TODO: Track public/private distinction for symbol visibility
```
**Effort:** Small (1-2 hours) - Object file format related

### 8.4. Expression Evaluation in Pass 2 (1 item)
**Location:** `src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.cpp`
```
□ TODO: Evaluate expression in pass 2
```
**Effort:** Small (1 hour) - Complete two-pass assembly

### 8.5. Word-Boundary Checking (1 item)
**Location:** `src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.cpp`
```
□ TODO: More sophisticated word-boundary checking
```
**Effort:** Trivial (30 min) - Improve parameter substitution

### 8.6. 6809 LBRA Encoding (1 item)
**Location:** `src/cpu/m6809/cpu_6809.cpp`
```
□ TODO: Implement Relative16 (LBRA) in future
```
**Effort:** Small (1 hour) - Add long branch support

### 8.7. Handler Registration (1 item)
**Location:** `src/syntax/edtasm_m80_plusplus/edtasm_directive_handlers.cpp`
```
□ TODO: Register all handlers
```
**Effort:** Trivial (30 min) - Complete DirectiveRegistry registration

### 8.8. ORG Atom Creation (1 item)
**Location:** `src/syntax/motorola/flex_syntax.cpp`
```
□ TODO: Revisit this - should ORG create an atom?
```
**Effort:** Trivial (30 min) - Design decision

### 8.9. Line Number Tracking (1 item)
**Location:** `src/syntax/motorola/flex_syntax.cpp`
```
□ TODO: Track line numbers
```
**Effort:** Trivial (30 min) - Error reporting improvement

---

## Prioritized Implementation Roadmap

### Phase 1: High Priority (Foundation)
1. **EDTASM-M80++ Conditionals** (6-8 hours) - Essential for conditional assembly
2. **EDTASM-M80++ Macros** (8-12 hours) - Essential for code reuse (can reuse FLEX pattern)
3. **6809 Indexed Modes** (4-6 hours) - Essential for 6809 programs

**Total:** 18-26 hours

### Phase 2: Medium Priority (Enhancement)
4. **EDTASM Segment Directives** (4-6 hours) - Relocatable code support
5. **EDTASM Listing Directives** (2-3 hours) - Complete listing support
6. **Expression Parser Refinement** (2-3 hours) - Improve arithmetic

**Total:** 8-12 hours

### Phase 3: Low Priority (Nice-to-Have)
7. **Merlin LUP Directive** (2-3 hours) - Loop directive
8. **Miscellaneous Items** (6-7 hours) - Small improvements

**Total:** 8-10 hours

### Grand Total: 34-48 hours

---

## Decomposition for Execution

Given Lean Flow principles (max 14 files per task), these should be broken into tasks:

1. **Task: EDTASM Conditionals** (xasm++-????)
   - 13 conditional directives
   - Files: 3-4
   - Tokens: ~18K ✅ SAFE

2. **Task: EDTASM Macros** (xasm++-????)
   - 7 macro directives
   - Files: 5-6
   - Tokens: ~24K ⚠️ APPROACHING LIMIT
   - **Consider:** Reuse FLEX macro processor code

3. **Task: 6809 Indexed Modes** (xasm++-????)
   - 11 indexed addressing variants
   - Files: 2
   - Tokens: ~12K ✅ SAFE

4. **Task: EDTASM Segments & Listing** (xasm++-????)
   - 6 directives total (3 segment + 3 listing)
   - Files: 5-6
   - Tokens: ~21K ⚠️ APPROACHING LIMIT

5. **Task: Miscellaneous Cleanup** (xasm++-????)
   - 10 small items
   - Files: 6-8
   - Tokens: ~12K ✅ SAFE

---

## Magic Number/String Elimination

**CRITICAL:** When implementing these items, ensure:
- ✅ NO magic numbers - use named constants from opcodes_*.h
- ✅ NO magic strings - use directive_constants.h
- ✅ All constants in appropriate headers
- ✅ constexpr for compile-time constants

---

**Document Status:** Complete
**Next Action:** Create Beads tasks for prioritized items
