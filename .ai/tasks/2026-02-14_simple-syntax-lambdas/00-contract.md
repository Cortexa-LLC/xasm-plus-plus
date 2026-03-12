# Task Contract: Convert simple_syntax lambdas to free functions

**Beads Task:** xasm++-vngq

**Created:** 2026-02-14  
**Status:** Planning  
**Priority:** Normal  
**Estimated Effort:** 1 hour

---

## Overview

Convert 3 lambda-based directive registrations in `simple_syntax.cpp` to free functions following the Merlin pattern. This improves code maintainability, testability, and consistency with other syntax parsers.

---

## Problem Statement

**Current State:**
- `SimpleSyntaxParser::InitializeDirectives()` registers 3 directives (ORG, DB, DW) using inline lambdas
- Lambdas are anonymous, making them hard to test individually
- Pattern inconsistent with MerlinSyntaxParser which uses free functions in separate file

**Desired State:**
- Create `simple_directive_handlers.cpp` and `.h` following Merlin pattern
- Extract 3 lambda bodies to named free functions in `simple` namespace
- Register free functions in `InitializeDirectives()`
- Match signature: `void Handler(const std::string& label, const std::string& operand, DirectiveContext& ctx)`

---

## Scope

### In Scope
1. Create `src/syntax/simple_directive_handlers.cpp`
2. Create `include/xasm++/directives/simple_directive_handlers.h`
3. Extract 3 directive handlers:
   - `HandleOrg()` - Set assembly origin address
   - `HandleDb()` - Define byte(s)
   - `HandleDw()` - Define word(s) in little-endian
4. Update `simple_syntax.cpp` to use free functions
5. Update CMakeLists.txt to include new files
6. Verify existing tests still pass

### Out of Scope
- Adding new directives
- Changing directive behavior/logic
- Refactoring other parsers
- Adding new tests (existing tests validate correctness)

---

## Requirements

### Functional Requirements

**FR1: Handler Extraction**
- Extract lambda bodies to free functions with signature:
  ```cpp
  void HandlerName(const std::string& label, 
                   const std::string& operand,
                   DirectiveContext& context)
  ```

**FR2: Namespace Organization**
- Place handlers in `xasm::simple` namespace (mirrors `xasm::merlin`)
- Header: `include/xasm++/directives/simple_directive_handlers.h`
- Implementation: `src/syntax/simple_directive_handlers.cpp`

**FR3: Directive Registration**
- Update `InitializeDirectives()` to register free functions:
  ```cpp
  directive_registry_.Register("ORG", simple::HandleOrg);
  directive_registry_.Register("DB", simple::HandleDb);
  directive_registry_.Register("DW", simple::HandleDw);
  ```

### Non-Functional Requirements

**NFR1: Backward Compatibility**
- All existing tests must pass without modification
- Directive behavior must remain identical

**NFR2: Code Quality**
- Follow existing code style (see `merlin_directive_handlers.cpp`)
- Include Doxygen documentation
- Maintain helper function implementations (Trim, ToUpper, etc.)

**NFR3: Build Integration**
- Add new source files to CMakeLists.txt
- Verify clean build with no warnings

---

## Acceptance Criteria

### AC1: File Structure
- [ ] `include/xasm++/directives/simple_directive_handlers.h` exists
- [ ] `src/syntax/simple_directive_handlers.cpp` exists
- [ ] Files follow Merlin pattern structure
- [ ] Proper namespace (`xasm::simple`)

### AC2: Handler Implementation
- [ ] `HandleOrg()` implemented (parses hex address, creates OrgAtom)
- [ ] `HandleDb()` implemented (comma-separated bytes, creates DataAtom)
- [ ] `HandleDw()` implemented (comma-separated words, little-endian)
- [ ] All handlers have Doxygen comments

### AC3: Integration
- [ ] `simple_syntax.cpp` imports new header
- [ ] `InitializeDirectives()` calls `simple::HandleOrg`, etc.
- [ ] No lambdas remain in `InitializeDirectives()`
- [ ] CMakeLists.txt includes new source file

### AC4: Verification
- [ ] Build succeeds with zero warnings
- [ ] All existing tests pass
- [ ] `SimpleSyntaxTest` suite validates directive behavior

---

## Implementation Notes

### Reference Pattern
See `merlin_directive_handlers.cpp` for the target pattern:
- Header guards and includes
- Namespace structure (`xasm::simple`)
- Helper functions at file scope (static)
- Handler implementations with Doxygen comments
- Error handling via `std::runtime_error`

### Handler Details

**HandleOrg:**
```cpp
// Input: operand = "$8000" or "8000"
// Output: OrgAtom(address), update current_address
```

**HandleDb:**
```cpp
// Input: operand = "$01,$02,$03" or "1,2,3"
// Output: DataAtom([0x01, 0x02, 0x03]), update current_address
```

**HandleDw:**
```cpp
// Input: operand = "$1234,$5678"
// Output: DataAtom([0x34,0x12, 0x78,0x56]), update current_address
// Note: Little-endian byte order (low byte first)
```

---

## Dependencies

**Blocked By:**
- None

**Blocks:**
- None (independent refactoring)

---

## Risks and Mitigations

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Test failures | High | Low | Run tests immediately, fix any issues |
| Build errors | Medium | Low | Follow exact Merlin pattern |
| Behavior changes | High | Very Low | Extract exact lambda bodies, no logic changes |

---

## Resources

### Reference Files
- `src/syntax/merlin_directive_handlers.cpp` - Target pattern
- `include/xasm++/directives/merlin_directive_handlers.h` - Header pattern
- `src/syntax/merlin_syntax.cpp` - Registration pattern
- `include/xasm++/syntax/directive_registry.h` - Registry interface

### Helper Functions Needed
- `ParseHex(std::string)` - Already available in parse_utils.h
- `Trim(std::string)` - Copy from simple_syntax.cpp or use util/string_utils.h
- `ToUpper(std::string)` - Copy from simple_syntax.cpp or use util/string_utils.h

---

## Success Metrics

- Zero test failures
- Zero build warnings
- Code review approval (follows established patterns)
- Cleaner, more maintainable directive registration

---

**Stakeholders:**
- **Requester:** System maintainability requirement
- **Engineer:** TBD
- **Reviewer:** TBD

**Sign-off:** TBD
