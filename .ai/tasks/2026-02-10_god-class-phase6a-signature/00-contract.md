# Task Contract: God-Class Phase 6a - Signature Unification

**Task ID:** 2026-02-10_god-class-phase6a-signature
**Beads Task:** xasm++-98cb
**Created:** 2026-02-10
**Requestor:** Orchestrator (from escalation xasm++-3mwi)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Migrate all directive handlers across all parsers (Merlin, SCMASM, EDTASM-M80++) to use the DirectiveContext pattern. This unifies directive signatures and prepares for handler extraction in Phase 6b.

### Background and Context

During God-Class Phase 6 analysis, two competing directive signature patterns were discovered:

**OLD Pattern (Currently in use):**
```cpp
DirectiveHandler = std::function<void(
  const std::string& label,
  const std::string& operands,  // plural
  Section& section,
  ConcreteSymbolTable& symbols
)>
```

**NEW Pattern (Defined but not integrated):**
```cpp
DirectiveHandler = std::function<void(
  const std::string& label,
  const std::string& operand,   // singular
  DirectiveContext& context
)>
```

The NEW DirectiveContext pattern is more extensible and forward-compatible with plugin architecture. This task migrates all parsers to use it.

### Current State

- 3 parsers with directive lambdas using OLD signature
- Merlin: 31 directives
- SCMASM: ~25 directives
- EDTASM-M80++: ~20 directives
- Total: ~75 handlers to migrate

### Desired State

All directive lambdas migrated to DirectiveContext signature, enabling Phase 6b extraction.

---

## Success Criteria

```
✓ All Merlin directives use DirectiveContext
✓ All SCMASM directives use DirectiveContext
✓ All EDTASM-M80++ directives use DirectiveContext
✓ All tests passing (97%+)
✓ No behavior changes (pure refactoring)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Update directive_registry_ in merlin_syntax.cpp to use DirectiveContext
□ Update directive_registry_ in scmasm_syntax.cpp to use DirectiveContext
□ Update directive_registry_ in edtasm_m80_plusplus_syntax.cpp to use DirectiveContext
□ Convert all directive lambdas to new signature (~75 total)
□ Package Section& and ConcreteSymbolTable& into DirectiveContext
□ Verify all directive invocations work identically
```

### Quality Requirements
```
□ All tests passing (97%+ target)
□ No warnings
□ TDD followed (test-first for each parser)
□ Zero behavior changes
```

---

## Estimated Complexity

**Complexity:** Medium

**Files:** 6 (3 parser headers, 3 implementations)
**Tokens:** 15K → ✅ SAFE

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 6 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 6 files → ACCEPTABLE
```

### Batch Size Justification

Files: 6 (within acceptable range but requires justification)

**Why not decomposed further:**
- High cohesion - all files are directive signature migrations
- Single concern - converting lambdas to DirectiveContext pattern
- Already minimal viable batch - must touch all 3 parsers together

**Contingency for token limits:**
- If token limit hit, will decompose by parser (Merlin + SCMASM + EDTASM)

**Estimated tokens:** ~6 × 3000 = 18,000 tokens
**Status:** Within 25K-32K limit? Yes ✅

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
6 × 3,000 tokens = 18,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 18,000 tokens → SAFE ✅
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [x] YES - Batch size 6 files AND token budget 18K AND WIP 1

---

## Dependencies

**Parent Task:** xasm++-3mwi (God-Class Phase 6 - BLOCKED)
**Blocks:** xasm++-uqyd (Phase 6b - Core Handler Extraction)
**Reference:** `.ai/tasks/2026-02-09_god-class-phase6-directive/ESCALATION.md`

---

**Contract Approved:** ✓ Orchestrator 2026-02-10
