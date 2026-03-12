# Task Contract

**Task ID:** 2026-01-24_merlin-syntax-support
**Created:** 2026-01-24
**Requestor:** Bryan Woodruff
**Assigned Role:** Orchestrator → Engineer
**Workflow:** Feature

---

## Task Description

Implement Merlin/Merlin 32 assembler syntax support in xasm++ to enable assembling real-world Apple II/IIgs code, specifically Prince of Persia source code.

### Background and Context

**Why:** The Prince of Persia source code uses Merlin assembler syntax. To validate xasm++ works with real-world code, we need full Merlin syntax compatibility.

**Problem:** xasm++ currently has a basic syntax plugin but lacks Merlin-specific features:
- Three-tier label system (global, :local, ]variable)
- Merlin directives (no dot prefix: ORG not .ORG)
- String literal high-bit rules (delimiter < 0x27 = high bit set)
- Macro system (MAC...<<<, ]0-]8 parameters, semicolon separators)
- CPU mode directives (MX, XC, LONGA, LONGI for 65816)
- Merlin-specific data directives (ASC, DCI, INV, FLS, REV, HEX, etc.)

**Context:** This builds on Phase 2.5 (complete 65C02/65816 CPU support). We have a proven reference implementation in vasm-ext that we can adapt.

### Current State

**What Exists:**
- ✅ Complete CPU plugin (6502/65C02/65816) with mode switching
- ✅ Basic syntax plugin (syntax/simple_syntax.cpp)
- ✅ Core assembler engine (Assembler, Section, Symbol, Expression)
- ✅ Reference implementation: ~/Projects/Vintage/tools/vasm-ext/syntax/merlin/

**What's Missing:**
- ❌ Merlin label syntax (:local, ]variable)
- ❌ Merlin directives (ORG, EQU, DA, DFB, ASC, etc.)
- ❌ Merlin string handling (high-bit rules)
- ❌ Merlin macro system (MAC...<<<, ]N parameters)
- ❌ CPU mode directives (MX, XC, LONGA, LONGI)
- ❌ Comment styles (* in column 1, ; inline)

### Desired State

**After Completion:**
- ✅ Full Merlin syntax compatibility
- ✅ Can assemble Prince of Persia source code
- ✅ Three-tier label system working
- ✅ All common Merlin directives supported
- ✅ String literals with correct high-bit handling
- ✅ Basic macro support (MAC...<<<)
- ✅ CPU mode directives (MX for 65816)
- ✅ Comprehensive test coverage

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ Assembles at least one Prince of Persia source file successfully
✓ Label system: global, :local, ]variable all working
✓ Core directives: ORG, EQU, DA, DFB, DW, ASC working
✓ String directives: ASC, DCI, INV, FLS, HEX working
✓ Comments: * and ; both supported
✓ Basic macros: MAC...<<< with ]0-]8 parameters
✓ CPU mode: MX directive for 65816
✓ Test coverage ≥ 85% overall, 95%+ for new code
✓ Zero compiler warnings
✓ Tester validation: APPROVED
✓ Reviewer validation: APPROVED
```

---

## Reference Materials

### Primary References

**vasm-ext Implementation (Complete Reference):**
- `~/Projects/Vintage/tools/vasm-ext/syntax/merlin/syntax.h` - Interface
- `~/Projects/Vintage/tools/vasm-ext/syntax/merlin/syntax.c` - 4,219 lines of working code
- Lines 2737-2967: Complete directive table (227 directives!)

**Test Files:**
- `~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/` - Real-world validation

---

**Version:** 1.0
**Last Updated:** 2026-01-24
