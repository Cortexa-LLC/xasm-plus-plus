/**
 * @file conditional_assembler.cpp
 * @brief Implementation of ConditionalAssembler
 *
 * Implements the shared conditional assembly logic extracted from god-class
 * syntax parsers.
 */

#include "xasm++/common/conditional_assembler.h"
#include <stdexcept>

namespace xasm {

ConditionalAssembler::ConditionalAssembler() {
  // Start with empty stack (unconditional emission)
}

void ConditionalAssembler::BeginIf(bool condition) {
  // Determine if code should be emitted based on:
  // 1. Current condition
  // 2. Parent block's should_emit state (if any)
  bool parent_should_emit = stack_.empty() ? true : stack_.back().should_emit;
  bool should_emit = parent_should_emit && condition;

  stack_.push_back({
      condition,  // condition
      false,      // in_else_block (not in ELSE yet)
      should_emit // should_emit
  });
}

void ConditionalAssembler::BeginElse() {
  if (stack_.empty()) {
    throw std::runtime_error("ELSE without matching IF");
  }

  ConditionalBlock &block = stack_.back();
  if (block.in_else_block) {
    throw std::runtime_error("Multiple ELSE in same IF block");
  }

  block.in_else_block = true;

  // Determine if code in ELSE branch should be emitted:
  // - Parent must be emitting
  // - Original condition must be false (so ELSE is taken)
  bool parent_should_emit =
      stack_.size() > 1 ? stack_[stack_.size() - 2].should_emit : true;
  block.should_emit = parent_should_emit && !block.condition;
}

void ConditionalAssembler::EndIf() {
  if (stack_.empty()) {
    throw std::runtime_error("ENDIF without matching IF");
  }

  stack_.pop_back();
}

bool ConditionalAssembler::ShouldEmit() const {
  // If stack is empty, emit unconditionally
  if (stack_.empty()) {
    return true;
  }

  // Otherwise, check if current block allows emission
  return stack_.back().should_emit;
}

bool ConditionalAssembler::IsBalanced() const { return stack_.empty(); }

void ConditionalAssembler::Reset() { stack_.clear(); }

size_t ConditionalAssembler::GetDepth() const { return stack_.size(); }

} // namespace xasm
