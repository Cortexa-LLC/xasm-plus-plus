# Work Log: A2osX Macro Requirements Investigation

**Beads Task:** xasm++-bk49
**Status:** IN_PROGRESS
**Started:** 2026-02-16

---

## Session 1: Initial Investigation

### Objectives
1. Analyze inc/macros.i structure
2. Document pseudo-instruction usage (>NAME patterns)
3. Verify .MA/.EM directive support in current codebase
4. Create comprehensive requirements document

### Progress

#### Step 1: Project Structure Analysis
- Verified task packet exists
- Task claimed in Beads (xasm++-bk49)
- Beginning systematic investigation

#### Step 2: Initial Findings
- Found test files with macro usage:
  - `test_a2osx_minimal.s` - Basic A2osX syntax
  - `tests/integration/binary_compat/test_sources/scmasm/6502/04_macros.asm` - SCMASM macro test
  
**Key Discovery: Pseudo-instruction Syntax**
- Macro invocations use `>NAME` prefix (e.g., `>ADDVAL $05`)
- This is called a "pseudo-instruction" in SCMASM
- Example from test:
  ```asm
  .MA ADDVAL
  CLC
  ADC #]1
  .EM
  
  >ADDVAL $05  ; Macro invocation with > prefix
  ```

#### Step 3: Searching for A2osX macros.i
- A2osX is an operating system project for Apple II
- References mention "inc/macros.i" file with macro definitions
- Found A2osX source at: `/Users/bryanw/Projects/Vintage/Apple/A2osX/`

#### Step 4: Analysis of A2osX macros.i
**Location:** `/Users/bryanw/Projects/Vintage/Apple/A2osX/INC/macros.i`

**Key Findings:**
- Total macros defined: 56
- Macro syntax: `.MA MACRONAME` ... `.EM`
- Macro invocation: `>MACRONAME param1,param2` (pseudo-instruction syntax)
- Parameter syntax: `]1`, `]2`, etc. (not `\1`, `\2`)
- Special parameter: `]#` for parameter count
- Heavy use of:
  - Local labels (`:1`, `:2`, etc.)
  - Conditional assembly (`.DO`, `.FIN`)
  - Nested macro calls (macros calling other macros)

**Sample Macros Found:**
1. `>LDYAI` - Load Y and A immediate (word immediate load)
2. `>STYA` - Store Y and A
3. `>PUSHYA` - Push Y and A to stack
4. `>PUSHA` - Push A to stack
5. `>PUSHW` - Push word
6. `>PUSHL` - Push long (32-bit)
7. `>INCW` - Increment word
8. `>DECW` - Decrement word
9. `>LDYA.G` - Load Y/A from global data segment
10. `>MLICALL` - ProDOS MLI call with automatic parameter count

#### Step 5: Current xasm++ Implementation Analysis
**Files:**
- `src/common/macro_processor.cpp` - FLEX ASM macro processor (parameter names, not `]N`)
- `src/syntax/scmasm_syntax.cpp` - SCMASM parser with `.MA`/`.EM` support
- Pseudo-instruction prefix `>` is recognized
- Parameter substitution exists but needs verification for `]N` syntax

**Status:**
- ✅ `.MA`/`.EM` directive support exists
- ✅ `>MACRONAME` pseudo-instruction prefix recognized
- ❓ Parameter syntax: Need to verify `]1`, `]2` vs `\1`, `\2`
- ❓ `]#` parameter count support
- ❓ Nested macro expansion
- ❓ Local label uniqueness in macro expansion

#### Step 6: Requirements Document Creation
- Created comprehensive requirements document: `30-requirements.md`
- Documented all 56 A2osX macros
- Categorized macros by function (syscalls, stack, words, global data)
- Identified parameter syntax: `]1`-`]9` and `]#`
- Created test requirements
- Defined implementation strategy
- Documented current status and gaps

**Key Deliverables:**
1. ✅ Macro structure analysis (56 macros documented)
2. ✅ Parameter syntax verification (`]N` not `\N`)
3. ✅ Pseudo-instruction patterns documented
4. ✅ Test requirements created
5. ✅ Implementation priorities defined
6. ✅ Next steps identified

#### Step 7: Documentation Discovery
**IMPORTANT FINDING:** The SCMASM reference doc in xasm++ incorrectly states `\0`-`\9` for parameters, but actual A2osX code and xasm++ tests use `]1`-`]9`. This is a **documentation error** that needs correction.

---

## Task Complete ✅

**Summary:**
- Investigated A2osX macro requirements
- Found and analyzed `/Users/bryanw/Projects/Vintage/Apple/A2osX/INC/macros.i`
- Documented 56 macros and their patterns
- Created comprehensive requirements document
- Identified implementation gaps and priorities
- Ready for next phase: verification and implementation

