# vasm-ext SCMASM Implementation Comparison

**Date:** 2026-02-13
**Purpose:** Compare vasm-ext SCMASM implementation with xasm++ to identify missing features
**Source:** ~/Projects/Vintage/tools/vasm-ext/syntax/scmasm/

---

## Executive Summary

Analyzed vasm-ext SCMASM syntax module (2500+ lines) to identify features not yet implemented in xasm++. This reference implementation provides insights into directive semantics, string handling, and advanced features.

**Key Findings:**
1. **P0 Directives** - All 5 P0 directives have reference implementations
2. **Advanced Features** - .AC compression, three-tier labels, macro extensions
3. **String Handling** - Flexible delimiters, escape sequences, high-bit rules
4. **CPU Mode** - Simple CPU type switching via set_cpu_type()

---

## 1. P0 Critical Directives (xasm++ Phase 1 & 2 targets)

### 1.1 .PS (Pascal String) - ✅ IMPLEMENTED in Phase 1

**vasm-ext implementation:** `handle_ps()` (lines 1396-1450)

```c
static void handle_ps(char *s) {
  // Parse string with flexible delimiter
  // Emit length byte (0-255) + string bytes
  // No high-bit manipulation (raw bytes)
  data[0] = (unsigned char)str_len;  // Length prefix
  // Copy string bytes unchanged
}
```

**Key features:**
- Length-prefixed strings (ProDOS convention)
- Flexible delimiter (any non-whitespace character)
- Max 255 bytes (truncates if longer)
- **No high-bit manipulation** (differs from .AS)
- Multiple strings comma-separated

**xasm++ status:** ✅ Implemented in Phase 1 with inverted high-bit logic

---

### 1.2 .INB (Include Binary) - ✅ IMPLEMENTED in Phase 1

**vasm-ext implementation:** `handle_include()` (line 2284 - reuses .IN handler)

```c
// .INB maps to handle_include (same as .IN directive)
// Reads source file, not binary file in vasm!
```

**⚠️ IMPORTANT DIFFERENCE:**
- **vasm-ext:** `.INB` includes **source code** files (same as `.IN`)
- **xasm++:** `.INB` includes **binary data** files (matches SCMASM 3.1 behavior)
- **A2oSX usage:** Confirmed as binary inclusion (font files, graphics data)

**xasm++ implementation is CORRECT** - A2oSX uses .INB for binary data, not source inclusion.

---

### 1.3 .LIST (Listing Control) - ✅ IMPLEMENTED in Phase 1

**vasm-ext implementation:** `handle_list()` (lines 2041-2049)

```c
static void handle_list(char *s) {
  // Enable listing with optional parameters
  // SCASM format: .list [xoff] [moff] [symoff]
  // Parameters: xoff (cross-ref off), moff (macro off), symoff (symbol off)
  set_listing(1);
  // Consume any parameters on the line
}
```

**Features:**
- Toggle listing output (on/off)
- Optional parameters: xoff, moff, symoff (ignored in vasm)
- Related: `.NOLIST`, `.LISTTTL`, `.LISTSUBTTL`, `.LISTPAGE`, `.LISTSPACE`

**xasm++ status:** ✅ Stub implementation in Phase 1 (no-op, accepted for compatibility)

---

### 1.4 .DUMMY (Dummy Section) - 🔄 Phase 2 target

**vasm-ext implementation:** `handle_dsect()` / `handle_dend()` (lines 1621-1646)

```c
static void handle_dsect(char *s) {
  if (!dsect_active) {
    last_alloc_sect = current_section;  // Save current section
    dsect_active = 1;
  }
  else
    syntax_error(35);  /* .DUMMY section already active */

  switch_offset_section(NULL, dsect_offs);  // Switch to dummy section
  eol(s);
}

static void handle_dend(char *s) {
  if (dsect_active) {
    dsect_offs = current_section->pc;   // Save dummy section position
    set_section(last_alloc_sect);        // Restore previous section
    last_alloc_sect = NULL;
    dsect_active = 0;
  }
  else
    syntax_error(21);  /* .ED without .DUMMY */
  eol(s);
}
```

**Key features:**
- **State management:** Save/restore section context
- **Offset tracking:** `dsect_offs` persists across .DUMMY/.ED pairs
- **Nesting detection:** Error if .DUMMY already active
- **No output:** Dummy sections don't emit bytes (structure definitions only)
- **Label creation:** Labels defined in dummy sections get addresses without allocation

**Implementation requirements:**
1. Add `dsect_active` flag to parser state
2. Add `last_alloc_sect` to save section stack
3. Add `dsect_offs` to track dummy section position
4. Implement `switch_offset_section()` to create non-emitting section
5. Validation: .ED without .DUMMY = error, nested .DUMMY = error

**A2oSX usage:** Structure definitions (231 occurrences, 2.4% of directives)

---

### 1.5 .OP (CPU Operation Mode) - 🔄 Phase 2 target

**vasm-ext implementation:** `handle_op()` (lines 1473-1486)

```c
static void handle_op(char *s) {
  /* SCASM .OP directive - change CPU type */
  char *cpuname;

  s = skip(s);
  cpuname = s;

  /* Call CPU module's set_cpu_type() directly */
  if (!set_cpu_type(cpuname)) {
    syntax_error(24);  /* invalid CPU type for .OP directive */
  }
}
```

**Key features:**
- **Simple pass-through:** Calls CPU module's `set_cpu_type()`
- **No parsing:** CPU name is rest of line (whitespace-separated token)
- **Error handling:** Invalid CPU type = syntax error

**Common SCMASM CPU types:**
- `65C02` - 65C02 processor (A2oSX target)
- `6502` - Original 6502
- `65816` - 16-bit 65816

**Implementation requirements:**
1. Parse CPU type operand (token after `.OP`)
2. Validate CPU type (must be supported by xasm++)
3. Call CPU subsystem to switch mode (affects opcode encoding, addressing modes)
4. Consider: Mid-assembly CPU changes may require address resolution changes

**A2oSX usage:** CPU mode selection (148 occurrences, 1.5% of directives)

---

## 2. Missing P1 Directives (Lower Priority)

### 2.1 .CS (C-String with Escapes)

**Implementation:** `handle_cs()` (lines 1256-1341)

```c
// C-style strings with backslash escape sequences
// Escape codes: \a \b \e \f \n \r \t \v \0 \\ \" \' \xHH
// No high-bit manipulation (raw C strings)
```

**Features:**
- Full C escape sequence support
- Hex escapes `\xHH`
- Flexible delimiters
- Multiple strings comma-separated

**Related:** `.CZ` (C-string zero-terminated) = `.CS` + null byte

**Priority:** P1 (used 118 times in A2oSX, 1.2%)

---

### 2.2 .TF (Text File / Title)

**Implementation:** `handle_tf()` (line 2171 - not shown in excerpts)

**Purpose:** Set listing title or include text file (context-dependent)

**Priority:** P1 (used 83 times in A2oSX, 0.9%)

---

### 2.3 .DO / .FIN (Conditional Assembly)

**Purpose:** Conditional assembly blocks (like #if/#endif)

```assembly
.DO expr       ; If expr != 0, assemble following lines
  ...
.FIN           ; End conditional block
```

**Priority:** P1 (used 68 times in A2oSX, 0.7%)

---

### 2.4 .EP (Entry Point)

**Purpose:** Define program entry point for executable formats

**Priority:** P2 (used 21 times in A2oSX, 0.2%)

---

## 3. Advanced String Directives

### 3.1 .AS (ASCII String) - ✅ ALREADY IMPLEMENTED

**vasm-ext implementation:** `handle_as()` (lines 837-909)

```c
// Flexible delimiter
// Optional '-' flag: FORCE high-bit on all characters
// Delimiter rule: if delim < 0x27 (apostrophe), set high bit
// Multiple strings comma-separated
```

**xasm++ status:** Already implemented, matches vasm-ext behavior

---

### 3.2 .AT (ASCII with Toggle Last)

**Implementation:** `handle_at()` (lines 1184+)

```c
// High bit handling depends on delimiter:
// - Delimiter < 0x27: Set high bit on all chars, then toggle last
// - Delimiter >= 0x27: No high bit on chars, then toggle last
// Used for command tables vs display strings
```

**Priority:** P2 (rarely used)

---

### 3.3 .AZ (ASCII Zero-Terminated) - ✅ ALREADY IMPLEMENTED

**vasm-ext implementation:** `handle_az()` (lines 1123-1181)

**xasm++ status:** Already implemented

---

### 3.4 .AC (ASCII Compression)

**Implementation:** `handle_ac()` (lines 929-1119)

```c
// Nibble-based text compression:
// .AC 0              - Initialize compression state
// .AC 1"chars"       - Define table 1 (single nibble, 1-15)
// .AC 2"chars"       - Define table 2 (two nibbles, 0 then 1-15)
// .AC 3"chars"       - Define table 3 (three nibbles, 0, 0, then 0-15)
// .AC /message/      - Compress message using tables
//
// Compression:
// - Table 1 chars at index i emit nibble (i+1)
// - Table 2 chars at index i emit nibbles 0, (i+1)
// - Table 3 chars at index i emit nibbles 0, 0, i
// - Nibbles packed two per byte (high nibble first)
// - State persists across .AC directives
// - Digits not in tables = blank count (0=10 spaces, 1-9=N spaces)
```

**Features:**
- Three-tier compression tables
- State machine (nibble position persists)
- Blank count encoding for digits
- Used in A2oSX SCI (screen control interpreter) modules

**Priority:** P2 (advanced feature, 2 occurrences in A2oSX)

---

## 4. Label System Features

### 4.1 Three-Tier Labels

**vasm-ext implementation:** (lines 79-81)

```c
// Three label tiers:
// 1. Global: Normal labels (visible everywhere)
// 2. Private: :NAME (visible within macro invocation context)
// 3. Local: .N (numeric, scoped to last global label)

static const char *scmasm_last_global_label = NULL;
static int scmasm_private_context = 0;
```

**SCMASM label syntax:**
```assembly
GLOBAL          ; Global label (tier 1)
:PRIVATE        ; Private label (tier 2, macro-scoped)
.1              ; Local label (tier 3, scoped to GLOBAL)
.2              ; Another local label
```

**xasm++ status:**
- ✅ Global labels - Implemented
- ❌ Private labels (:NAME) - NOT implemented
- ❌ Numeric local labels (.N) - NOT implemented

**Priority:** P2 (advanced feature, used in complex macros)

---

### 4.2 Macro Parameter Syntax

**vasm-ext:** (lines 49-52, syntax.h)

```c
// SCMASM supports ]1 through ]9 parameters
#define MAXMACPARAMS 9
char *my_skip_macro_arg(char *);
#define SKIP_MACRO_ARGNAME(p) my_skip_macro_arg(p)
```

**Macro invocation:**
```assembly
.MA MYMACRO              ; Define macro
  LDA ]1                 ; Parameter 1
  STA ]2                 ; Parameter 2
.EM

>MYMACRO $1000,$2000     ; Call: >NAME or _NAME
```

**xasm++ status:**
- ✅ Macro definition (.MA/.EM) - Implemented
- ❌ ]N parameter syntax - NOT verified (may use different syntax)

**Priority:** P1 (critical for A2oSX macros)

---

## 5. Character Constant Rules

**vasm-ext:** (syntax.h lines 20-24)

```c
// SCMASM character constant rule:
// Delimiter < apostrophe (0x27) → set high bit on character
// Applies to expression constants like #"A" or #'A'
#define CHAR_CONST_TRANSFORM(c, delim) \
  ((delim) < '\'' ? ((c) | 0x80) : (c))
```

**Examples:**
```assembly
LDA #'A'        ; Load 0x41 (delimiter 0x27, no high bit)
LDA #-A-        ; Load 0xC1 (delimiter 0x2D < 0x27, high bit set)
```

**xasm++ status:** Unknown (need to verify character constant handling)

**Priority:** P2 (rarely used in A2oSX)

---

## 6. Other Directive Differences

### 6.1 .TA (Target Address)

**vasm-ext implementation:** `handle_ta()` (lines 1453-1470)

```c
// In native SCASM: sets where object code is written in memory
// In cross-assembly: NO EFFECT (warning issued)
// vasm-ext ignores .TA with a warning
```

**Priority:** P3 (no-op in cross-assembler)

---

### 6.2 .ORG with Expression Support

**vasm-ext:** (lines 1510-1551)

```c
// Parse expression, try to evaluate
// Allows: .OR * to reference current PC
// Allows: .OR SYMBOL where SYMBOL = .EQ *
```

**xasm++ status:** Need to verify if .OR supports symbolic expressions

---

## 7. Data Directives (Reference)

**Confirmed in vasm-ext:**
- `.D8` / `.DFB` - 8-bit data (byte)
- `.D16` / `.DA` - 16-bit data (word, little-endian)
- `.D24` - 24-bit data (long address)
- `.D32` - 32-bit data (dword)
- `.HS` - Hex string
- `.BS` - Block storage (reserve bytes)

**xasm++ status:** ✅ All implemented

---

## 8. Implementation Recommendations

### Phase 2 (Current) - P0 Completion
1. **.DUMMY** - Complex state management, critical for A2oSX structures
2. **.OP** - Simple pass-through to CPU module, critical for 65C02 mode

### Phase 3 - P1 Directives
1. **.CS** / **.CZ** - C-string with escapes (118 uses)
2. **.TF** - Text file / title (83 uses)
3. **.DO** / **.FIN** - Conditional assembly (68 uses)

### Phase 4 - Advanced Features
1. **Macro ]N parameters** - Verify or implement (critical for A2oSX macros)
2. **Three-tier labels** - Private (:NAME) and local (.N) scoping
3. **.AC compression** - Advanced feature (2 uses, low priority)

### Phase 5 - Polish
1. **Character constant rules** - Verify delimiter-based high-bit handling
2. **.EP entry point** - Define executable entry point
3. **Expression .ORG** - Verify symbolic expression support

---

## 9. Key Insights for xasm++ Implementation

### 9.1 .DUMMY State Management

**Critical components:**
```cpp
// Add to parser state:
bool dsect_active = false;
Section* last_alloc_sect = nullptr;
uint64_t dsect_offs = 0;

// .DUMMY handler:
void HandleDummy() {
  if (dsect_active) throw "Nested .DUMMY not allowed";
  last_alloc_sect = current_section;
  dsect_active = true;
  // Switch to non-emitting section at dsect_offs
}

// .ED handler:
void HandleEndDummy() {
  if (!dsect_active) throw ".ED without .DUMMY";
  dsect_offs = current_section->pc;  // Save position
  current_section = last_alloc_sect; // Restore
  dsect_active = false;
}
```

### 9.2 .OP CPU Mode Switching

**Critical components:**
```cpp
void HandleOp(const std::string& operand) {
  // Parse CPU type (first token)
  std::string cpu_type = ParseCpuType(operand);

  // Validate and switch CPU mode
  if (cpu_type == "65C02") {
    cpu->SetMode(CPUMode::M65C02);
  } else if (cpu_type == "6502") {
    cpu->SetMode(CPUMode::M6502);
  } else if (cpu_type == "65816") {
    cpu->SetMode(CPUMode::M65816);
  } else {
    throw "Invalid CPU type for .OP: " + cpu_type;
  }
}
```

### 9.3 String Handling Patterns

**All string directives share common pattern:**
```cpp
// 1. Parse delimiter (first non-whitespace character)
// 2. Extract string up to closing delimiter
// 3. Apply high-bit rules based on delimiter value
// 4. Handle multiple comma-separated strings
// 5. Emit data atoms with processed bytes
```

---

## 10. Testing Recommendations

### P0 Directive Tests (Phase 2)

**.DUMMY tests:**
```assembly
; Test 1: Basic dummy section
.DUMMY
STRUCT_A .EQ 0
FIELD1   .BS 2      ; Offset 0-1
FIELD2   .BS 4      ; Offset 2-5
.ED                 ; STRUCT_A should = 6

; Test 2: Nested .DUMMY error
.DUMMY
.DUMMY              ; Error: Already active
.ED

; Test 3: .ED without .DUMMY
.ED                 ; Error: No active dummy section

; Test 4: Labels in dummy section
.DUMMY
POINT .EQ 0
  X .BS 2           ; POINT.X = offset 0
  Y .BS 2           ; POINT.Y = offset 2
.ED                 ; Size = 4 bytes, but no output
```

**.OP tests:**
```assembly
; Test 1: Basic CPU mode switch
.OP 65C02
  BRA LABEL         ; 65C02 instruction

; Test 2: Invalid CPU type
.OP 68000           ; Error: Invalid CPU

; Test 3: Mid-assembly switch
.OR $8000
.OP 6502
  NOP               ; 6502 mode
.OP 65C02
  BRA LABEL         ; 65C02 mode
```

---

## 11. Conclusion

**vasm-ext provides excellent reference implementation** for all P0 directives:
- **.PS** - Already implemented correctly in xasm++ Phase 1
- **.INB** - Already implemented correctly (xasm++ uses binary, vasm uses source)
- **.LIST** - Already implemented (stub, compatible)
- **.DUMMY** - Clear implementation pattern, requires state management
- **.OP** - Simple pass-through to CPU module

**Next steps:**
1. Complete Phase 2 implementation (.DUMMY, .OP)
2. Validate against A2oSX test files
3. Plan Phase 3 for P1 directives (.CS, .TF, .DO/.FIN)

**Key architectural insight:** SCMASM directives require stateful parser (dummy sections, compression state, macro contexts) rather than purely functional approach.

---

**Document version:** 1.0
**Last updated:** 2026-02-13
**Reviewed by:** Orchestrator (vasm-ext analysis)
