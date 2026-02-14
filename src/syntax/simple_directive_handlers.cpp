/**
 * @file simple_directive_handlers.cpp
 * @brief Simple syntax directive handler implementations
 *
 * Implementations of Simple syntax directive handlers.
 * Extracted from lambdas in simple_syntax.cpp to follow Merlin pattern.
 */

#include "xasm++/directives/simple_directive_handlers.h"
#include "xasm++/atom.h"
#include "xasm++/parse_utils.h"
#include "xasm++/section.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace xasm {
namespace simple {

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Trim whitespace from string
 * @param str Input string
 * @return Trimmed string
 */
static std::string Trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = str.find_last_not_of(" \t");
  return str.substr(start, end - start + 1);
}

// ============================================================================
// Directive Handlers
// ============================================================================

void HandleOrg(const std::string & /*label*/, const std::string &operand,
               DirectiveContext &context) {
  // ORG directive - set assembly origin address
  uint32_t address = ParseHex(operand);
  context.section->atoms.push_back(std::make_shared<OrgAtom>(address));
  *context.current_address = address;
}

void HandleDb(const std::string & /*label*/, const std::string &operand,
              DirectiveContext &context) {
  // DB directive - define byte(s)
  std::vector<uint8_t> bytes;
  std::istringstream ops(operand);
  std::string value;

  while (std::getline(ops, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      bytes.push_back(static_cast<uint8_t>(ParseHex(value)));
    }
  }

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleDw(const std::string & /*label*/, const std::string &operand,
              DirectiveContext &context) {
  // DW directive - define word(s) in little-endian format
  std::vector<uint8_t> bytes;
  std::istringstream ops(operand);
  std::string value;

  while (std::getline(ops, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      uint32_t word = ParseHex(value);
      bytes.push_back(static_cast<uint8_t>(word & 0xFF));        // Low byte
      bytes.push_back(static_cast<uint8_t>((word >> 8) & 0xFF)); // High byte
    }
  }

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

} // namespace simple
} // namespace xasm
