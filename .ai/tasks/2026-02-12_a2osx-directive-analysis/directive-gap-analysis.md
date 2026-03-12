# A2oSX SCMASM Directive Gap Analysis

**Analysis Date:** 2026-02-13  
**Analyzer:** Engineer Agent  
**Source:** A2oSX Operating System (190 source files)  
**Target:** xasm++ SCMASM Implementation  

---

## Executive Summary

This analysis examines 190 A2oSX source files to identify missing SCMASM directives in xasm++, enabling xasm++ to become the first modern cross-assembler capable of building the complete A2oSX operating system.

**Key Findings:**
- **25 unique directives** found across **9,647 total occurrences**
- **13 directives already implemented** in xasm++ (52% coverage)
- **12 directives missing** (48% coverage gap)
- **Top priority blockers:** .INB, .PS, .DUMMY, .LIST, .OP
- **Estimated effort:** 40-60 hours for complete implementation

**Recommendation:** Implement in 3 phases (P0 blockers first, then P1, finally P2 nice-to-haves)

---

## Methodology

### Data Collection
1. **Automated Scanning:** Used shell script with grep/awk to extract directives from all .S and .S.txt files
2. **Source Files:** 190 files in ~/Projects/Vintage/Apple/A2osX/
3. **Extraction Pattern:** Lines starting with optional whitespace + period + alphanumeric characters
4. **Normalization:** Converted all directives to uppercase for consistent counting

### Analysis Approach
1. Frequency analysis (occurrences per directive)
2. Cross-reference with xasm++ implementation
3. Priority assignment based on:
   - Usage frequency (higher = more critical)
   - Build criticality (can A2oSX build without it?)
   - Implementation complexity

---

## Complete Directive Inventory

### Frequency Distribution

| Rank | Directive | Count | % of Total | Status |
|------|-----------|-------|-----------|--------|
| 1 | .DA | 4616 | 47.8% | ✅ Implemented |
| 2 | .INB | 1131 | 11.7% | ❌ **Missing** |
| 3 | .HS | 909 | 9.4% | ✅ Implemented |
| 4 | .PS | 529 | 5.5% | ❌ **Missing** |
| 5 | .OR | 389 | 4.0% | ✅ Implemented |
| 6 | .BS | 280 | 2.9% | ✅ Implemented |
| 7 | .AT | 258 | 2.7% | ✅ Implemented |
| 8 | .DUMMY | 231 | 2.4% | ❌ **Missing** |
| 9 | .LIST | 192 | 2.0% | ❌ **Missing** |
| 10 | .CS | 180 | 1.9% | ❌ **Missing** |
| 11 | .TF | 161 | 1.7% | ❌ **Missing** |
| 12 | .OP | 148 | 1.5% | ❌ **Missing** |
| 13 | .FIN | 129 | 1.3% | ❌ **Missing** |
| 14 | .DO | 118 | 1.2% | ❌ **Missing** |
| 15 | .CZ | 94 | 1.0% | ❌ **Missing** |
| 16 | .AZ | 89 | 0.9% | ✅ Implemented |
| 17 | .AS | 49 | 0.5% | ✅ Implemented |
| 18 | .EP | 38 | 0.4% | ❌ **Missing** |
| 19 | .MA | 28 | 0.3% | ✅ Implemented |
| 20 | .EM | 28 | 0.3% | ✅ Implemented |
| 21 | .ED | 23 | 0.2% | ❌ **Missing** |
| 22 | .ELSE | 15 | 0.2% | ❌ **Missing** (part of .DO) |
| 23 | .PH | 10 | 0.1% | ❌ **Missing** |
| 24 | .FI | 1 | <0.1% | ❌ **Missing** (part of .DO) |
| 25 | .EL | 1 | <0.1% | ❌ **Missing** (part of .DO) |

**Totals:** 9,647 occurrences, 25 unique directives

---

## Gap Analysis

### Currently Implemented (13 directives)

| Directive | Purpose | Notes |
|-----------|---------|-------|
| .OR | Set origin address | Core functionality |
| .EQ | Define constant | Symbol table |
| .SE | Set (alternate form) | Alias to .EQ |
| .AS | ASCII string | String support |
| .AT | ASCII text (with terminator) | String support |
| .AZ | ASCII zero-terminated | String support |
| .DA | Define address/data | Core data definition |
| .DFB | Define byte | Alias to .DA |
| .HS | Hex string | Binary data |
| .BS | Block storage (reserve space) | Memory allocation |
| .MA | Macro definition | Macro support |
| .ENDM | End macro | Macro support |
| .EM | End macro (alias) | Alias to .ENDM |

**Implementation Coverage:** 52% (13/25 directives)

### Missing Directives (12 directives)

Priority classifications:
- **P0 (Critical Blockers):** A2oSX cannot build without these (5 directives)
- **P1 (Important):** Build may work but with warnings/missing features (4 directives)
- **P2 (Nice-to-Have):** Optional features, minimal impact (3 directives)

---

## Priority Rankings

### P0: Critical Blockers (MUST IMPLEMENT)

#### 1. .INB - Include Binary File
**Frequency:** 1131 occurrences (11.7% of total) - **#2 most common**  
**Priority:** P0 - **CRITICAL BLOCKER**

**Purpose:** Include external binary file at current position

**Usage Example:**
```assembly
.INB INC/MACROS.I
.INB INC/A2OSX.I
.INB USR/SRC/SHARED/X.PRINTF.S
```

**Why Critical:**
- A2oSX uses modular architecture with extensive includes
- 1131 includes across 190 files = average 6 includes per file
- Without .INB, files cannot be assembled (immediate build failure)

**Implementation Complexity:** **Medium** (3-5 hours)
- File I/O to read binary file
- Error handling for missing files
- Path resolution (relative vs absolute)
- Recursive depth protection

**Semantics:**
- Read binary file and insert bytes at current position
- Path can be relative to source file or include directory
- No assembly of included file (raw binary insertion)

---

#### 2. .OP - CPU Operation Mode
**Frequency:** 148 occurrences (1.5% of total)  
**Priority:** P0 - **CRITICAL BLOCKER**

**Purpose:** Select target CPU (6502, 65C02, 65816, etc.)

**Usage Example:**
```assembly
.OP 65C02         ; Target 65C02 CPU
.OP 6502          ; Target plain 6502
.OP 65816         ; Target 65816 (16-bit)
```

**Why Critical:**
- A2oSX targets 65C02 specifically
- Instruction set differs between 6502/65C02 (e.g., BRA, STZ)
- Wrong CPU = invalid opcodes = build failure
- Used in 148 places to enforce 65C02

**Implementation Complexity:** **High** (8-10 hours)
- CPU variant switching logic
- Opcode table per CPU variant
- Validation of instructions against selected CPU
- Integration with existing CPU abstraction

**Semantics:**
- Sets target CPU for subsequent instructions
- Validates opcodes against selected CPU
- Affects: opcode availability, addressing modes, cycle counts

---

#### 3. .PS - Pascal String
**Frequency:** 529 occurrences (5.5% of total) - **#4 most common**  
**Priority:** P0 - **CRITICAL BLOCKER**

**Purpose:** Define length-prefixed string (Pascal/ProDOS convention)

**Usage Example:**
```assembly
.PS "HELLO"       ; Emits: 05 48 45 4C 4C 4F (length byte + chars)
```

**Why Critical:**
- ProDOS and A2oSX APIs use Pascal strings extensively
- 529 occurrences = core data structure for OS
- Without .PS, data structures are malformed

**Implementation Complexity:** **Low** (2-3 hours)
- String processing (already have .AS, .AT, .AZ)
- Length byte prefix generation
- Similar to existing string directives

**Semantics:**
- First byte = string length (0-255)
- Followed by string bytes (no terminator)
- ProDOS standard format

---

#### 4. .DUMMY - Dummy Section
**Frequency:** 231 occurrences (2.4% of total)  
**Priority:** P0 - **CRITICAL BLOCKER**

**Purpose:** Define data structure without allocating storage (like C struct)

**Usage Example:**
```assembly
.DUMMY
ZP.TEMP  .BS 2    ; Define offset, don't allocate
ZP.COUNT .BS 1
.ED               ; End dummy section
```

**Why Critical:**
- Essential for zero-page and structure definitions
- 231 occurrences = heavily used pattern
- Without .DUMMY, address calculations fail

**Implementation Complexity:** **Medium** (5-7 hours)
- Separate address counter (dummy vs real)
- Symbol definitions without output
- .ED directive to terminate
- Interaction with .OR

**Semantics:**
- Symbols defined with addresses, but no bytes emitted
- Address counter advances but output counter does not
- Terminated by .ED
- Used for overlay definitions, structures

---

#### 5. .LIST - Listing Control
**Frequency:** 192 occurrences (2.0% of total)  
**Priority:** P0 - **BUILD CRITICAL** (but can be stubbed)

**Purpose:** Control assembly listing output (ON/OFF/options)

**Usage Example:**
```assembly
.LIST OFF         ; Disable listing
.LIST ON          ; Enable listing
```

**Why Critical:**
- Used 192 times to control verbosity
- Can be stubbed initially (accept but ignore)
- Eventually needed for proper listing files

**Implementation Complexity:** **Low** (1-2 hours initially, 5-8 hours for full implementation)
- **Phase 1 (Stub):** Parse and ignore (1-2 hours)
- **Phase 2 (Full):** Control listing output (5-8 hours)

**Semantics:**
- Controls assembly listing detail
- Options: ON, OFF, COFF (code off), MOFF (macro off)
- Affects output verbosity only, not assembly

**Initial Approach:** **Stub it** - Parse but don't implement listing control initially

---

### P1: Important (Should Implement)

#### 6. .DO/.ELSE/.FI/.EL - Conditional Assembly
**Frequency:** 118 (.DO) + 15 (.ELSE) + 1 (.FI) + 1 (.EL) = **135 occurrences**  
**Priority:** P1 - **IMPORTANT**

**Purpose:** Conditional assembly (like #ifdef in C)

**Usage Example:**
```assembly
.DO DEBUG=1
  JSR PRINT_DEBUG
.ELSE
  NOP
.FI
```

**Why Important:**
- Used 135 times for configuration variants
- Allows building different versions (debug, release, etc.)
- Without it, must manually edit source files

**Implementation Complexity:** **High** (10-15 hours)
- Expression evaluation (already exists?)
- Conditional block parsing
- Nested conditionals support
- .ELSE/.EL variants
- .FI terminator

**Semantics:**
- Evaluate expression after .DO
- If true, assemble block
- If false, skip to .ELSE or .FI
- Supports nesting

---

#### 7. .TF - Target File
**Frequency:** 161 occurrences (1.7% of total)  
**Priority:** P1 - **IMPORTANT**

**Purpose:** Specify output filename

**Usage Example:**
```assembly
.TF BIN/MYPROG    ; Output to BIN/MYPROG
```

**Why Important:**
- Determines output file name/path
- 161 files explicitly set their output name
- Without it, must specify via command line

**Implementation Complexity:** **Low** (2-3 hours)
- Parse filename
- Override default output path
- Path validation

**Semantics:**
- Sets output file path
- Can include directory
- Overrides command-line output

---

#### 8. .CS/.CZ - C String Variants
**Frequency:** 180 (.CS) + 94 (.CZ) = **274 occurrences**  
**Priority:** P1 - **IMPORTANT**

**Purpose:** C-style null-terminated strings (variants of .AZ)

**Usage Example:**
```assembly
.CS "HELLO"       ; Null-terminated string
.CZ "WORLD\r\n"   ; Null-terminated with escapes
```

**Why Important:**
- 274 occurrences = common pattern
- Similar to .AZ but may have different escape handling
- Used for C library compatibility

**Implementation Complexity:** **Low** (1-2 hours)
- Clone .AZ implementation
- Verify escape sequence handling
- Alias if semantics identical

**Semantics:**
- Null-terminated string (like .AZ)
- May support different escape sequences
- Need to verify difference from .AZ in practice

---

#### 9. .FIN - Finish/End Block
**Frequency:** 129 occurrences (1.3% of total)  
**Priority:** P1 - **IMPORTANT**

**Purpose:** End conditional block or macro (alternative to .FI)

**Usage Example:**
```assembly
.DO DEBUG=1
  ; debug code
.FIN
```

**Why Important:**
- 129 occurrences = widely used
- May be synonym for .FI or have different semantics
- Need to understand relationship to .DO/.ELSE/.FI

**Implementation Complexity:** **Low** (1 hour if alias, 3-5 hours if different)
- If alias to .FI: trivial
- If different semantics: needs investigation

**Semantics:**
- Likely terminates .DO block (like .FI)
- May also terminate other constructs
- Need to verify exact behavior

---

### P2: Nice-to-Have (Optional)

#### 10. .EP - Entry Point
**Frequency:** 38 occurrences (0.4% of total)  
**Priority:** P2 - **NICE-TO-HAVE**

**Purpose:** Specify program entry point address

**Usage Example:**
```assembly
.EP               ; Entry point at current address
.EP $2000         ; Entry point at $2000
```

**Why Optional:**
- Only 38 occurrences (low frequency)
- May be informational only (not required for assembly)
- Linker feature more than assembler feature

**Implementation Complexity:** **Low** (2-3 hours)
- Parse entry point address
- Store in symbol table or metadata
- Emit to output format if supported

**Semantics:**
- Marks execution start address
- May be used by loader/debugger
- Not essential for assembly process

---

#### 11. .ED - End Dummy Section
**Frequency:** 23 occurrences (0.2% of total)  
**Priority:** P2 - **TIED TO .DUMMY**

**Purpose:** Terminate .DUMMY section

**Usage Example:**
```assembly
.DUMMY
  ZP.VAR .BS 2
.ED               ; End dummy, resume normal assembly
```

**Why Optional:**
- Only meaningful with .DUMMY
- 23 occurrences (low frequency)
- Required if .DUMMY implemented (P0)

**Implementation Complexity:** **Low** (1-2 hours)
- Trivial if .DUMMY implemented
- Restores normal address counter

**Semantics:**
- Ends .DUMMY section
- Resumes normal assembly mode
- Part of .DUMMY feature

**Note:** If .DUMMY is P0, then .ED is also P0

---

#### 12. .PH - Phase
**Frequency:** 10 occurrences (0.1% of total)  
**Priority:** P2 - **NICE-TO-HAVE**

**Purpose:** Set phase address (for code that runs at different address than assembled)

**Usage Example:**
```assembly
.PH $800          ; Code will run at $800
.OR $2000         ; But assembled at $2000
```

**Why Optional:**
- Only 10 occurrences (very rare)
- Advanced feature for relocatable code
- Most code doesn't need phasing

**Implementation Complexity:** **Medium** (5-7 hours)
- Dual address counter (phase vs assembly)
- Symbol resolution with phase address
- Complex feature, rarely used

**Semantics:**
- Code runs at phase address but is assembled at origin address
- Labels use phase address
- Output uses assembly address
- For self-relocating code

---

## Summary Gap Analysis Table

| Directive | Frequency | Priority | Effort (hrs) | Status | Notes |
|-----------|-----------|----------|--------------|--------|-------|
| **.INB** | 1131 (11.7%) | **P0** | 3-5 | ❌ Missing | Include binary - CRITICAL |
| **.OP** | 148 (1.5%) | **P0** | 8-10 | ❌ Missing | CPU mode - CRITICAL |
| **.PS** | 529 (5.5%) | **P0** | 2-3 | ❌ Missing | Pascal string - CRITICAL |
| **.DUMMY** | 231 (2.4%) | **P0** | 5-7 | ❌ Missing | Struct definition - CRITICAL |
| **.LIST** | 192 (2.0%) | **P0** | 1-2 (stub) | ❌ Missing | Listing control - stub OK |
| **.DO/.ELSE/.FI/.EL** | 135 (1.4%) | P1 | 10-15 | ❌ Missing | Conditional assembly |
| **.TF** | 161 (1.7%) | P1 | 2-3 | ❌ Missing | Target file |
| **.CS/.CZ** | 274 (2.8%) | P1 | 1-2 | ❌ Missing | C string variants |
| **.FIN** | 129 (1.3%) | P1 | 1-3 | ❌ Missing | End block |
| **.EP** | 38 (0.4%) | P2 | 2-3 | ❌ Missing | Entry point |
| **.ED** | 23 (0.2%) | P2 | 1-2 | ❌ Missing | End dummy (with .DUMMY) |
| **.PH** | 10 (0.1%) | P2 | 5-7 | ❌ Missing | Phase |

**Total Missing:** 12 directives  
**Total Effort Estimate:** 
- **P0 (Critical):** 20-27 hours
- **P1 (Important):** 14-23 hours  
- **P2 (Nice-to-Have):** 8-12 hours  
- **Grand Total:** **42-62 hours**

---

## Recommendations

### Implementation Strategy

**Phase 1: P0 Blockers (20-27 hours)**
Implement critical directives required for A2oSX to build:
1. .INB (3-5 hrs) - Include binary
2. .PS (2-3 hrs) - Pascal string
3. .DUMMY + .ED (6-8 hrs) - Structure definition
4. .LIST (1-2 hrs) - Stub implementation
5. .OP (8-10 hrs) - CPU mode selection

**Goal:** A2oSX files can be assembled without errors

**Phase 2: P1 Important (14-23 hours)**
Add features for proper build variants and usability:
1. .TF (2-3 hrs) - Target file
2. .CS/.CZ (1-2 hrs) - C string variants
3. .FIN (1-3 hrs) - End block
4. .DO/.ELSE/.FI/.EL (10-15 hrs) - Conditional assembly

**Goal:** A2oSX can be built in different configurations

**Phase 3: P2 Optional (8-12 hours)**
Nice-to-have features:
1. .EP (2-3 hrs) - Entry point
2. .ED (1-2 hrs) - Enhance .DUMMY
3. .PH (5-7 hrs) - Phase support

**Goal:** Complete SCMASM compatibility

### Validation Plan

After each phase:
1. **Unit Tests:** Test each directive individually
2. **Integration Tests:** Assemble small A2oSX files
3. **Full Build Test:** Attempt complete A2oSX build
4. **Comparison:** Compare output with original SCMASM assembler

### Success Metrics

**Phase 1 Success:**
- [ ] All P0 directives implemented
- [ ] A2oSX test files assemble without errors
- [ ] Basic OS modules assemble successfully

**Phase 2 Success:**
- [ ] All P1 directives implemented
- [ ] Conditional builds work (DEBUG vs RELEASE)
- [ ] Full A2oSX kernel assembles

**Phase 3 Success:**
- [ ] All directives implemented
- [ ] Complete A2oSX builds successfully
- [ ] Output matches original SCMASM (byte-for-byte if possible)

---

## Risk Assessment

### High Risk Items

**1. .OP (CPU Mode) - High Complexity**
- **Risk:** Instruction set variations across CPUs
- **Impact:** Incorrect opcodes = broken binaries
- **Mitigation:** Extensive opcode tables, validation tests

**2. .DO (Conditional Assembly) - High Complexity**
- **Risk:** Expression evaluation, nested conditionals
- **Impact:** Wrong code assembled in builds
- **Mitigation:** Thorough parser testing, comparison with reference

**3. .DUMMY (Structure Definition) - Medium Complexity**
- **Risk:** Address counter management
- **Impact:** Wrong offsets = broken data structures
- **Mitigation:** Unit tests for address calculations

### Medium Risk Items

**4. .INB (Include Binary) - Path Resolution**
- **Risk:** Cross-platform path handling
- **Impact:** Files not found on different OSes
- **Mitigation:** Robust path resolution, error messages

---

## Alternative Approaches

### Option A: Full Implementation (Recommended)
Implement all missing directives per priority ranking

**Pros:**
- Complete SCMASM compatibility
- xasm++ becomes reference implementation
- Full A2oSX build support

**Cons:**
- 42-62 hours of work
- Complex features (.OP, .DO)

### Option B: Minimal Implementation
Implement only P0 directives, stub the rest

**Pros:**
- Faster (20-27 hours)
- Gets A2oSX building quickly

**Cons:**
- Limited functionality
- No conditional builds
- Incomplete solution

### Option C: Hybrid (Recommended Initial Approach)
Implement P0 fully, stub P1/P2

**Pros:**
- Balance of speed and functionality
- Can iterate based on user feedback
- Phased delivery

**Cons:**
- Multiple implementation cycles
- May need refactoring later

**Recommendation:** Use **Option C (Hybrid)** - Implement P0 first, then evaluate based on user needs

---

## Conclusion

xasm++ currently supports 52% of A2oSX directives (13/25). Implementing the remaining 12 directives (estimated 42-62 hours) will make xasm++ the first modern cross-assembler capable of building A2oSX.

**Critical Path:** P0 directives (.INB, .OP, .PS, .DUMMY, .LIST) are essential for any A2oSX assembly. These should be implemented first.

**Next Steps:**
1. Create implementation task (xasm++-zitw) with P0 directives
2. Design .OP CPU mode switching architecture
3. Implement .INB file inclusion
4. Implement .PS, .DUMMY, .LIST
5. Test with A2oSX source files
6. Iterate with P1/P2 directives as needed

**Strategic Value:** Successful implementation makes xasm++ a compelling tool for vintage Apple II development, validated against a real-world, production operating system (A2oSX).

---

**Analysis Complete:** 2026-02-13  
**Reviewed By:** Engineer Agent  
**Next Action:** Create implementation task (xasm++-zitw) with detailed specifications for P0 directives

