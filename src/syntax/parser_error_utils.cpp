/**
 * @file parser_error_utils.cpp
 * @brief Implementation of parser error utility functions
 *
 * Provides reusable error formatting and throwing functions that eliminate
 * duplicate error handling patterns across parser implementations.
 */

#include "xasm++/syntax/parser_error_utils.h"
#include <stdexcept>

namespace xasm {

std::string FormatError(const DirectiveContext &ctx,
                        const std::string &message) {
  if (!ctx.current_file.empty() && ctx.current_line > 0) {
    return ctx.current_file + ":" + std::to_string(ctx.current_line) + ": " +
           message;
  }
  return message;
}

void ThrowFormattedError(const DirectiveContext &ctx,
                         const std::string &message) {
  throw std::runtime_error(FormatError(ctx, message));
}

void ThrowRequiresLabel(const DirectiveContext &ctx,
                        const std::string &directive_name) {
  std::string message = directive_name + " requires a label";
  throw std::runtime_error(FormatError(ctx, message));
}

void ThrowRequiresOperand(const DirectiveContext &ctx,
                          const std::string &directive_name) {
  std::string message = directive_name + " requires operand";
  throw std::runtime_error(FormatError(ctx, message));
}

void ThrowMissingMatchingDirective(const DirectiveContext &ctx,
                                   const std::string &closing_directive,
                                   const std::string &opening_directive) {
  std::string message =
      closing_directive + " without matching " + opening_directive;
  throw std::runtime_error(FormatError(ctx, message));
}

void ThrowNotImplemented(const DirectiveContext &ctx,
                         const std::string &directive_name,
                         const std::string &operand) {
  std::string message = directive_name + " directive not yet implemented";
  if (!operand.empty()) {
    message += ": " + operand;
  }
  throw std::runtime_error(FormatError(ctx, message));
}

void ThrowInvalidValue(const DirectiveContext &ctx,
                       const std::string &value_type, const std::string &value,
                       const std::string &reason) {
  std::string message = "Invalid " + value_type + " value: " + value;
  if (!reason.empty()) {
    message += " (" + reason + ")";
  }
  throw std::runtime_error(FormatError(ctx, message));
}

} // namespace xasm
