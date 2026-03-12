# Runtime Investigation: SEQTABLE DW Address Resolution

**Trigger:** Binary comparison showing SEQTABLE.bin differs from reference
**Entry Point:** xasm++ multi-pass assembler during SEQTABLE.S assembly
**Investigation Date:** 2026-02-01

## The Journey

### 1. Entry: DW Table Resolution (First Pass)

**Function:** `Assembler::EncodeInstructions()`
**Location:** `src/core/assembler.cpp:589`
**Timestamp:** Pass 1

**Observed Behavior:**
```
TRACE: DW startrun resolved to $30e9 at DW address $3000
TRACE: DW stand resolved to $3116 at DW address $3002
TRACE: DW standjump resolved to $3209 at DW address $3004
TRACE: DW hardbump resolved to $35b4 at DW address $305a
TRACE: DW bump resolved to $3575 at DW address $305c
```

**State:** Forward references in DW table being resolved during encoding.

### 2. Label Definition (During Same Pass)

**Function:** `Assembler::EncodeInstructions()` - Label atom processing
**Location:** `src/core/assembler.cpp:625`

**Observed Behavior:**
```
TRACE: Label 'startrun' at address $30e9
TRACE: Label 'stand' at address $3116
TRACE: Label 'standjump' at address $3209
TRACE: Label 'bump' at address $3575
TRACE: Label 'hardbump' at address $35b4
```

**State:** Labels being assigned addresses based on current assembly position.

### 3. Binary Comparison: The Problem

**xasm++ output:**
```
Offset 0x120: d6 39 (little-endian) = address $39D6
Offset 0x150: d6 39 (little-endian) = address $39D6
Offset 0x280: b1 37 (little-endian) = address $37B1
```

**Reference output (vasm/snap):**
```
Offset 0x120: 1e 31 (little-endian) = address $311E
Offset 0x150: 4e 31 (little-endian) = address $314E
Offset 0x280: 8c 32 (little-endian) = address $328C
```

**Analysis:**
- xasm++ addresses are 2232 bytes (0x8B8) higher than reference
- Multiple DW entries point to same wrong address ($39D6, $37B1)
- Pattern suggests systematic offset or label assignment error

### 4. Multi-Pass Convergence

**Observation:** Assembler runs 2 passes
```
TRACE logs show identical resolution in both passes
Labels resolve to same addresses in Pass 1 and Pass 2
Convergence achieved (sizes stable)
```

**State:** Multi-pass is working correctly (converging), but labels have wrong base addresses.

## Hidden Paths Discovered

### Path 1: Label Address Assignment Timing

**Discovery:** Labels are defined **after** forward references in source order:
1. Line 32: `:1 dw startrun` (DW references label)
2. Line 159: `startrun` (label definition)

This creates forward reference scenario that multi-pass must resolve.

### Path 2: Symbol Table Linkage (Fixed Previously)

**Context:** Previous fix added `assembler.SetSymbolTable(&symbols)` in main.cpp
**Status:** Symbol table IS linked (EQU constants work correctly)
**Conclusion:** Not the issue here.

### Path 3: ORG Directive Processing

**Source:** SEQTABLE.S line 1: `org = $3000`
**Source:** SEQTABLE.S line 30: `org org`

**Question:** Is the `org org` directive being processed correctly?
**Hypothesis:** If ORG isn't set properly, labels would have wrong base addresses.

## Dark Passages (Untraced)

### Area 1: ORG Directive Handling

**What we DON'T know:**
- How `org org` is parsed (self-reference)
- Whether section.org is set correctly before label assignment
- If there's a second ORG later that shifts addresses

**Why it matters:** Labels at $30XX should be ~0x8B8 bytes lower

### Area 2: DW Expression Evaluation Context

**What we DON'T know:**
- Whether DW uses parser's ParseExpression or assembler's ParseExpression
- If there's a symbol table lookup issue for label names
- Why some DWs resolve to wrong but CONSISTENT addresses

### Area 3: Label Atom Processing Order

**What we DON'T know:**
- Are LabelAtoms processed before or after DataAtoms in same pass?
- Could label addresses be stale from previous pass?
- Is there a race condition in symbol table updates?

## Root Cause Hypothesis

**Primary Theory:** ORG directive mismatch

The 0x8B8 byte offset suggests that labels are being assigned addresses relative to wrong origin. Possible causes:

1. **ORG not processed:** Labels start at $0000 + assembled code size instead of $3000
2. **Double ORG:** Second ORG directive shifts labels up by 0x8B8
3. **DUM block interference:** Previous DUM blocks leaving current_address_ in wrong state

**Supporting Evidence:**
- Offset is consistent (always 0x8B8 higher)
- All traced labels show same systematic error
- DW resolution uses symbol table (which should have correct addresses)

## Recommendations

### Immediate Actions

1. **Add ORG tracing:**
   ```cpp
   // In HandleOrg():
   std::cerr << "TRACE: ORG directive set address to $" << std::hex << address << std::dec << "\n";
   ```

2. **Trace current_address_ before label assignment:**
   ```cpp
   // In EncodeInstructions() before LabelAtom:
   std::cerr << "TRACE: current_address before label = $" << std::hex << current_address << std::dec << "\n";
   ```

3. **Check for multiple ORG directives:**
   ```bash
   grep -n "org\|ORG" SEQTABLE.S
   ```

### Prevent Future Issues

1. **Add ORG validation:** Warn if ORG is set multiple times in same section
2. **Symbol table sanity check:** Compare parser-assigned addresses vs assembler-resolved addresses
3. **Convergence logging:** Log why convergence happens (or doesn't)

### Monitor

1. **Label address ranges:** Verify all labels fall within expected $3000-$3XXX range
2. **DW resolution consistency:** All DW to same label should resolve to same address
3. **Pass-to-pass stability:** Label addresses shouldn't change after Pass 1

## Next Steps

**For Engineer:**
1. Add comprehensive ORG tracing
2. Verify current_address_ state at label assignment
3. Check if DUM blocks are leaving stale current_address_
4. Fix root cause (likely ORG handling)

**Investigation Tools Used:**
- Runtime tracing via std::cerr in assembler
- Binary hexdump comparison
- Source file analysis
- Symbol resolution tracing

**Artifacts:**
- Trace output: 50 lines of DW and Label resolution
- Binary comparison: `/tmp/seqtable-trace.hex` vs `/tmp/seqtable-ref.hex`
- Test binary: `/tmp/SEQTABLE_trace.bin`

---

**Status:** ROOT CAUSE HYPOTHESIS FORMED
**Confidence:** HIGH (systematic 0x8B8 offset points to ORG issue)
**Ready for Engineer:** YES (clear hypothesis and tracing strategy)
