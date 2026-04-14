/**
 * @file label_policy.cpp
 * @brief Label definition policy implementation.
 *
 * Shared implementation used by ScmasmSyntaxParser and MerlinSyntaxParser
 * to handle label definition for directive and instruction lines.
 *
 * Pattern 3 refactor: extracted from ~100-line inline blocks.
 */

#include "xasm++/syntax/label_policy.h"
#include "xasm++/atom.h"
#include "xasm++/expression.h"
#include <unordered_set>

namespace xasm {

// ---------------------------------------------------------------------------
// ClassifyLabelPolicy — CC <= 6
// ---------------------------------------------------------------------------

LabelPolicy ClassifyLabelPolicy(std::string_view opcode_upper, bool has_label,
                                bool in_dummy_section) {
  if (!has_label) {
    return LabelPolicy::Skip;
  }
  // Value directives own their own symbol definition — don't pre-define.
  if (opcode_upper == ".EQ" || opcode_upper == ".SE") {
    return LabelPolicy::Defer;
  }
  // Control-flow directives: label definition makes no sense here.
  if (opcode_upper == "DO" || opcode_upper == "LU") {
    return LabelPolicy::Skip;
  }
  // .OR sets a new origin; the label is always defined at the *current*
  // address before the origin changes, and always gets a LabelAtom.
  if (opcode_upper == ".OR") {
    return LabelPolicy::AtAddress;
  }
  (void)in_dummy_section; // caller passes correct address; policy is AtPc
  return LabelPolicy::AtPc;
}

// ---------------------------------------------------------------------------
// DefineLabelForDirective — CC <= 8
// ---------------------------------------------------------------------------

void DefineLabelForDirective(
    const std::string &label, uint32_t address, LabelPolicy policy,
    bool emit_atom, ConcreteSymbolTable &symbols, Section &section,
    std::unordered_map<std::string, uint32_t> &local_labels,
    std::string &last_global_label,
    const std::function<std::string(const std::string &)> &scope_fn,
    const std::function<bool(const std::string &)> &is_local_fn,
    const std::function<void()> &on_global_update) {
  if (policy == LabelPolicy::Skip || policy == LabelPolicy::Defer) {
    return;
  }
  if (label.empty()) {
    return;
  }

  // AtAddress always emits a LabelAtom; AtPc only when the caller requests it.
  bool should_emit = (policy == LabelPolicy::AtAddress) || emit_atom;

  // scope_fn converts the raw label to its canonical stored name (handles
  // both local scoping and global normalization, e.g. ToUpper for SCMASM).
  std::string canonical = scope_fn(label);
  auto expr = std::make_shared<LiteralExpr>(address);
  symbols.Define(canonical, SymbolType::Label, expr);

  if (is_local_fn(label)) {
    // Local label: also update the address cache keyed by the raw label.
    local_labels[label] = address;
  } else {
    // Global label: update the last-seen global label scope.
    last_global_label = canonical;
    if (on_global_update) {
      on_global_update();
    }
  }

  if (should_emit) {
    section.atoms.push_back(std::make_shared<LabelAtom>(canonical, address));
  }
}

} // namespace xasm
