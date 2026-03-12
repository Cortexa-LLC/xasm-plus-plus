# Task Escalation: God-Class Phase 6

**Task ID:** 2026-02-09_god-class-phase6-directive
**Beads ID:** xasm++-3mwi
**Escalated By:** Engineer
**Date:** 2026-02-10
**Status:** BLOCKED - Awaiting Architectural Guidance

---

## Escalation Summary

This task was assessed as **architecturally complex** and requires architectural decisions before implementation can proceed safely. It is NOT a simple extraction task as initially scoped.

---

## Problem Statement

### What Was Requested
Extract 20+ directive methods from parsers into DirectiveHandler registry pattern using the existing DirectiveRegistry infrastructure.

### What Was Discovered
1. **Two Competing Patterns Exist:**
   - **OLD Pattern** (Currently in use): 
     ```cpp
     DirectiveHandler = std::function<void(
       const std::string& label,
       const std::string& operands,
       Section& section,
       ConcreteSymbolTable& symbols
     )>
     ```
     - Used by all parsers (Merlin, SCMASM, EDTASM)
     - Lambda functions in `directive_registry_` map
     - 31 directives in Merlin alone
   
   - **NEW Pattern** (Already exists but not integrated):
     ```cpp
     DirectiveHandler = std::function<void(
       const std::string& label,
       const std::string& operand,  // Note: singular, not plural
       DirectiveContext& context
     )>
     ```
     - Defined in `include/xasm++/syntax/directive_registry.h`
     - Has DirectiveContext struct (section, symbols, current_address, parser_state)
     - Not used by any parser yet

2. **Signature Incompatibility:**
   - Parameters differ: `operands` vs `operand` (plural vs singular)
   - Context bundling: separate params vs DirectiveContext struct
   - Cannot directly extract handlers without unifying signatures first

3. **Large Scope:**
   - Merlin: 31 directives
   - SCMASM: ~25 directives
   - EDTASM: ~20 directives
   - Total: ~75-100 handler extractions needed
   - Affects 10+ files across codebase

---

## Complexity Assessment (Engineer Role Section 0.75)

### Assessment Criteria

| Criterion | Assessment | Details |
|-----------|-----------|---------|
| **Requirements clear?** | ⚠️ PARTIAL | Goal clear (extract directives) but approach uncertain due to signature mismatch |
| **Scope bounded?** | ❌ NO | 75-100 handlers across 10+ files, multiple parsers |
| **Approach obvious?** | ❌ NO | Multiple architectural patterns possible, unclear which to follow |
| **Architecture concerns?** | ⚠️ YES | Two competing patterns, need unification before extraction |

### Decision: ESCALATE

Per Engineer Role guidelines (Section 0.75), this meets criteria for "Architectural Issue" escalation:
- Multiple implementations detected (two signature patterns)
- Architectural refactoring required (not simple extraction)
- Cross-component impact (all parsers affected)

---

## Architectural Questions Requiring Guidance

### Question 1: Signature Unification Strategy

**Which signature should be the standard?**

**Option A: Migrate to NEW DirectiveContext pattern**
```cpp
// Pros:
+ DirectiveContext already defined
+ More extensible (add fields without changing all handlers)
+ Cleaner signature (fewer parameters)
+ Forward-compatible with plugin architecture

// Cons:
- Requires modifying all existing directive lambdas
- Parser state needs to be packaged into context
- Migration cost: ~75-100 lambda updates
```

**Option B: Keep OLD separate parameter pattern**
```cpp
// Pros:
+ No migration needed for existing code
+ Explicit parameters (clearer what's being passed)
+ No context struct overhead

// Cons:
- Adding new context requires changing all signatures
- Less extensible long-term
- DirectiveRegistry infrastructure becomes unused
```

**Option C: Adapter Pattern (support both)**
```cpp
// Pros:
+ Incremental migration possible
+ Both patterns work during transition

// Cons:
- Additional complexity (adapter layer)
- Temporary solution (still need to unify eventually)
- Two maintenance paths
```

**Recommendation:** Option A (migrate to DirectiveContext) for long-term extensibility, but needs architectural approval.

---

### Question 2: Handler Extraction Pattern

**Should handlers be classes or functions?**

**Option A: Handler Classes (Inheritance)**
```cpp
class IDirectiveHandler {
public:
  virtual void Execute(const std::string& label,
                      const std::string& operand,
                      DirectiveContext& context) = 0;
  virtual ~IDirectiveHandler() = default;
};

class OrgDirectiveHandler : public IDirectiveHandler {
  void Execute(...) override { /* ORG implementation */ }
};

// Pros:
+ Object-oriented, clear hierarchy
+ Easy to add state if needed
+ Testable in isolation
+ Supports polymorphism

// Cons:
- More boilerplate (class definitions)
- Heap allocation for handler instances
- Requires factory pattern for registration
```

**Option B: Handler Functions (Composition)**
```cpp
using DirectiveHandler = std::function<void(...)>;

DirectiveHandler CreateOrgHandler() {
  return [](const std::string& label, ...) {
    // ORG implementation
  };
}

// Pros:
+ Lightweight (just functions)
+ Matches current lambda style
+ No object lifetime management
+ Simple registration

// Cons:
- Harder to add state
- Less explicit structure
- Testing requires function injection
```

**Option C: Hybrid (Free functions wrapped in classes)**
```cpp
namespace handlers {
  void HandleOrg(const std::string& label, ...);
  void HandleEqu(const std::string& label, ...);
}

// Registered as:
registry.Register("ORG", handlers::HandleOrg);

// Pros:
+ Simple free functions
+ Easy to test
+ No class hierarchy overhead

// Cons:
- No polymorphism
- State management unclear
- Namespace organization critical
```

**Recommendation:** Option C (free functions) for simplicity, but needs architectural approval.

---

### Question 3: Migration Strategy

**How to migrate without breaking existing functionality?**

**Option A: Big-Bang Migration**
```
1. Unify all signatures to DirectiveContext
2. Extract all handlers at once
3. Update all parsers simultaneously
4. Run full test suite

Pros: Clean cut, no temporary code
Cons: High risk, large changeset, difficult to review
Estimated Time: 2-3 days
```

**Option B: Incremental Migration (Recommended)**
```
Phase 6a: Signature Unification (1 day)
  - Migrate all parsers to DirectiveContext pattern
  - Keep handlers as lambdas
  - Verify tests pass (97%+ target)

Phase 6b: Extract Core Handlers (1 day)
  - Extract 5-10 common directives (ORG, EQU, DB, DW, DS)
  - Proof-of-concept in Merlin parser
  - Verify tests pass

Phase 6c: Extract Remaining Handlers (1-2 days)
  - Extract remaining Merlin directives
  - Apply pattern to SCMASM, EDTASM
  - Full test verification

Pros: Lower risk, reviewable increments, early verification
Cons: Temporary intermediate states
Estimated Time: 3-4 days total
```

**Recommendation:** Option B (incremental) to minimize risk and allow continuous verification.

---

## Recommendations

### Primary Recommendation: Task Decomposition

Break this task into three subtasks with clear deliverables:

**Subtask 1: Signature Unification (xasm++-3mwi-a)**
- **Goal:** Migrate all parsers to DirectiveContext pattern
- **Scope:** Merlin, SCMASM, EDTASM parsers
- **Deliverable:** All directives use DirectiveContext signature
- **Acceptance:** 97%+ tests pass
- **Estimated Time:** 1 day
- **Risk:** Low (signature change only, no extraction)

**Subtask 2: Core Handler Extraction (xasm++-3mwi-b)**
- **Goal:** Extract 5-10 common directives as proof-of-concept
- **Scope:** ORG, EQU, DB, DW, DS in Merlin parser
- **Deliverable:** Free function handlers + registration
- **Acceptance:** Tests pass, handlers work identically
- **Estimated Time:** 1 day
- **Risk:** Low (small scope, reversible)
- **Depends On:** Subtask 1

**Subtask 3: Full Handler Extraction (xasm++-3mwi-c)**
- **Goal:** Extract remaining directives across all parsers
- **Scope:** All 75-100 directives
- **Deliverable:** Complete extraction, all parsers using registry
- **Acceptance:** 97%+ tests pass, all handlers extracted
- **Estimated Time:** 1-2 days
- **Risk:** Medium (large scope, but pattern proven)
- **Depends On:** Subtask 2

**Total Estimated Time:** 3-4 days (vs original estimate of unknown)

### Alternative Recommendation: Architect Review

If task decomposition is not acceptable, request:
- Architect review of signature unification approach
- Architect approval of handler extraction pattern
- Detailed implementation plan from Architect
- Clear acceptance criteria for architectural decisions

---

## Impact Assessment

### If Proceeding Without Guidance

**Risks:**
1. **Wrong Pattern Choice:** May need to redo entire extraction if pattern doesn't scale
2. **Technical Debt:** Inconsistent patterns across parsers
3. **Maintenance Burden:** Multiple signature styles to maintain
4. **Test Failures:** Large refactoring increases breakage risk
5. **Review Difficulty:** Large changeset hard to review thoroughly

**Impact on Timeline:**
- Potential rework: +2-3 days if pattern choice wrong
- Integration issues: +1-2 days resolving parser conflicts
- Test fixes: +1 day if breakage occurs
- Total Risk: +4-6 days potential overhead

### If Proceeding With Guidance

**Benefits:**
1. **Clear Direction:** Architectural decisions made upfront
2. **Incremental Progress:** Verifiable at each phase
3. **Reduced Risk:** Smaller changesets, continuous testing
4. **Reviewable:** Each phase independently reviewable
5. **Reversible:** Can stop after Phase 6a if issues arise

**Impact on Timeline:**
- Planning: +0.5 days (architectural decisions)
- Implementation: 3-4 days (as decomposed)
- Total Time: 3.5-4.5 days (controlled, predictable)

---

## Current Status

**Beads Task:** xasm++-3mwi - BLOCKED
**Blocker:** "Architectural complexity: Two directive signatures exist, need unification strategy before extraction"
**Work Log:** `.ai/tasks/2026-02-09_god-class-phase6-directive/20-work-log.md`
**Progress:** 5% (analysis complete, design pending)

**Awaiting:**
- [ ] Architectural guidance on signature unification
- [ ] Pattern approval (classes vs functions vs free functions)
- [ ] Migration strategy decision (big-bang vs incremental)
- [ ] Task decomposition approval (optional but recommended)

---

## References

- **Engineer Role:** `.ai/roles/engineer.md` Section 0.75 (Complexity Assessment)
- **DirectiveRegistry Header:** `include/xasm++/syntax/directive_registry.h`
- **DirectiveRegistry Implementation:** `src/syntax/directive_registry.cpp`
- **Merlin Parser:** `src/syntax/merlin/merlin_syntax.cpp` (31 directives)
- **SCMASM Parser:** `src/syntax/scmasm/scmasm_syntax.cpp`
- **EDTASM Parser:** `src/syntax/edtasm/edtasm_syntax.cpp`

---

## Contact

**Escalated By:** Engineer Agent
**Date:** 2026-02-10 12:45
**Next Action:** Orchestrator review and architectural guidance
**Urgency:** Medium (blocks Phase 6, but other phases can proceed)

---

**Decision Required From Orchestrator:**
1. Approve task decomposition into 3 subtasks (recommended)
2. OR: Request Architect review for guidance
3. OR: Provide direct architectural decisions on signature/pattern
