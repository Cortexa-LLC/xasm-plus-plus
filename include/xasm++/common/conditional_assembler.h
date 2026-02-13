/**
 * @file conditional_assembler.h
 * @brief Shared conditional assembly logic for all syntax parsers
 *
 * This component extracts duplicated conditional assembly logic (IF/ELSE/ENDIF,
 * IFDEF/IFNDEF, etc.) from the Z80Universal, Merlin, and SCMASM syntax parsers
 * into a single reusable implementation. It reduces ~350 LOC of duplication per
 * parser and centralizes conditional block management.
 *
 * @note Part of god-class refactoring Phase 4
 */

#pragma once

#include <cstddef>
#include <vector>

namespace xasm {

/**
 * @brief Manages conditional assembly blocks (IF/ELSE/ENDIF)
 *
 * ConditionalAssembler maintains a stack of conditional blocks to support
 * nested conditional assembly directives. It tracks whether code should be
 * emitted based on the evaluation of conditional expressions and the state
 * of parent conditional blocks.
 *
 * Supports common conditional assembly patterns:
 * - IF condition / ENDIF
 * - IF condition / ELSE / ENDIF
 * - IFDEF symbol / ENDIF
 * - IFNDEF symbol / ENDIF
 * - Nested conditionals (properly inherits parent state)
 *
 * @par Usage Example (Merlin DO/ELSE/FIN)
 * @code
 * ConditionalAssembler cond;
 *
 * // DO 1              ; True condition
 * cond.BeginIf(true);
 * assert(cond.ShouldEmit());  // Code emitted
 *
 * //   LDA #$42        ; Emitted
 * if (cond.ShouldEmit()) {
 *   // Generate code
 * }
 *
 * // ELSE
 * cond.BeginElse();
 * assert(!cond.ShouldEmit()); // Code not emitted
 *
 * //   LDA #$99        ; Not emitted
 * if (cond.ShouldEmit()) {
 *   // Skip
 * }
 *
 * // FIN
 * cond.EndIf();
 * assert(cond.ShouldEmit());  // Back to emitting
 * @endcode
 *
 * @par Nested Conditionals
 * Nested conditionals correctly inherit the parent block's emit state:
 * @code
 * cond.BeginIf(false);        // Outer: false (don't emit)
 * assert(!cond.ShouldEmit());
 *
 * cond.BeginIf(true);         // Inner: true, but parent is false
 * assert(!cond.ShouldEmit()); // Still don't emit (parent overrides)
 *
 * cond.EndIf();               // End inner
 * assert(!cond.ShouldEmit()); // Back to outer (still false)
 *
 * cond.EndIf();               // End outer
 * assert(cond.ShouldEmit());  // Back to normal
 * @endcode
 *
 * @par Thread Safety
 * Not thread-safe. Each parser should have its own ConditionalAssembler
 * instance.
 */
class ConditionalAssembler {
public:
  /**
   * @brief Construct a conditional assembler (initially emitting)
   */
  ConditionalAssembler();

  /**
   * @brief Begin a conditional block (IF/IFDEF/IFNDEF/DO)
   *
   * Pushes a new conditional block onto the stack. Code within the block
   * is emitted only if:
   * 1. The condition is true (condition != 0), AND
   * 2. The parent block (if any) is also emitting
   *
   * @param condition True if condition is met, false otherwise
   *
   * @par Examples
   * @code
   * // IF 1        ; Condition true
   * cond.BeginIf(EvaluateExpression("1") != 0);
   *
   * // IFDEF FOO   ; Check if symbol FOO is defined
   * cond.BeginIf(symbols.IsDefined("FOO"));
   *
   * // IFNDEF BAR  ; Check if symbol BAR is NOT defined
   * cond.BeginIf(!symbols.IsDefined("BAR"));
   * @endcode
   */
  void BeginIf(bool condition);

  /**
   * @brief Begin ELSE block (switch to alternative branch)
   *
   * Switches to the ELSE branch of the current conditional block.
   * Code in the ELSE branch is emitted only if:
   * 1. The original condition was false, AND
   * 2. The parent block (if any) is emitting
   *
   * @throws std::runtime_error if:
   * - Called without a matching IF (stack empty)
   * - Called twice in the same IF block (multiple ELSE)
   *
   * @par Example
   * @code
   * cond.BeginIf(false);      // Condition false
   * assert(!cond.ShouldEmit()); // Don't emit IF block
   *
   * cond.BeginElse();         // Switch to ELSE
   * assert(cond.ShouldEmit());  // Emit ELSE block
   * @endcode
   */
  void BeginElse();

  /**
   * @brief End conditional block (ENDIF/FIN)
   *
   * Pops the current conditional block from the stack, returning to the
   * previous block's state (or unconditional emission if stack is empty).
   *
   * @throws std::runtime_error if stack is empty (ENDIF without IF)
   *
   * @par Example
   * @code
   * cond.BeginIf(false);
   * // ... code in IF block ...
   * cond.EndIf();             // Pop IF block
   * assert(cond.ShouldEmit()); // Back to unconditional emission
   * @endcode
   */
  void EndIf();

  /**
   * @brief Check if code should be emitted at current position
   *
   * Returns true if all active conditional blocks allow code emission.
   * This is the primary query method used by parsers to decide whether
   * to generate atoms for the current line.
   *
   * @return true if code should be emitted, false otherwise
   *
   * @par Usage
   * @code
   * void Parser::ParseLine(const std::string& line) {
   *   // Process conditional directives
   *   if (directive == "IF") {
   *     cond.BeginIf(EvaluateCondition(operands));
   *     return;
   *   }
   *
   *   // Skip non-conditional directives if not emitting
   *   if (!cond.ShouldEmit()) {
   *     return;
   *   }
   *
   *   // Generate code (only if emitting)
   *   section.AddAtom(...);
   * }
   * @endcode
   */
  bool ShouldEmit() const;

  /**
   * @brief Check if all conditional blocks are balanced
   *
   * Returns true if the conditional stack is empty (all IFs have matching
   * ENDIFs). Useful for detecting unmatched conditionals at end of parse.
   *
   * @return true if stack is empty (balanced), false otherwise
   *
   * @par Usage
   * @code
   * parser.Parse(source);
   * if (!cond.IsBalanced()) {
   *   throw std::runtime_error("Unmatched IF directive");
   * }
   * @endcode
   */
  bool IsBalanced() const;

  /**
   * @brief Reset conditional state (clear all blocks)
   *
   * Clears the conditional stack, returning to unconditional emission.
   * Typically called at the start of parsing a new file or when recovering
   * from parse errors.
   *
   * @par Usage
   * @code
   * void Parser::Parse(const std::string& source) {
   *   cond.Reset();  // Clear state from previous parse
   *   // ... parse source ...
   * }
   * @endcode
   */
  void Reset();

  /**
   * @brief Get current nesting depth
   *
   * Returns the number of active conditional blocks. Useful for debugging
   * and for limiting maximum nesting depth.
   *
   * @return Number of active conditional blocks (0 = no conditionals)
   *
   * @par Usage
   * @code
   * if (cond.GetDepth() > 32) {
   *   throw std::runtime_error("Conditional nesting too deep");
   * }
   * @endcode
   */
  size_t GetDepth() const;

private:
  /**
   * @brief State of a single conditional block
   */
  struct ConditionalBlock {
    bool condition;     ///< True if condition was met
    bool in_else_block; ///< True if currently in ELSE branch
    bool should_emit;   ///< True if code should be emitted in this block
  };

  /**
   * @brief Stack of nested conditional blocks
   *
   * Each element represents an IF/ELSE/ENDIF block. The stack grows with
   * nested conditionals and shrinks as ENDIFs are processed.
   */
  std::vector<ConditionalBlock> stack_;
};

} // namespace xasm
