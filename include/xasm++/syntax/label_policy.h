/**
 * @file label_policy.h
 * @brief Label definition policy for directive and instruction lines.
 *
 * Provides a LabelPolicy enum and helper functions used by both
 * ScmasmSyntaxParser::TryHandleDirectiveLine and
 * MerlinSyntaxParser::HandleInstructionLine to decide how a label
 * that precedes a directive or instruction should be defined.
 *
 * Pattern 3 refactor: replaces ~100-line inline label blocks with 4-line calls.
 */

#ifndef XASMPP_SYNTAX_LABEL_POLICY_H
#define XASMPP_SYNTAX_LABEL_POLICY_H

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

#include "xasm++/section.h"
#include "xasm++/symbol.h"

namespace xasm {

/**
 * @brief Decides how a label preceding a directive should be handled.
 *
 * - Skip      : no label present, or control-flow directive — do nothing.
 * - Defer     : value directive (.EQ / .SE) — the handler owns the definition.
 * - AtAddress : define at current address AND emit a LabelAtom.
 * - AtPc      : define at current PC; emit LabelAtom only when
 *               emit_atom=true in DefineLabelForDirective.
 */
enum class LabelPolicy : std::uint8_t { Skip, Defer, AtAddress, AtPc };

/**
 * @brief Classify how the label before @p opcode_upper should be handled.
 *
 * CC <= 6 (five explicit branches, early returns).
 *
 * @param opcode_upper      Directive mnemonic in UPPER CASE (e.g. ".DA", ".OR").
 * @param has_label         True when the line carries a non-empty label field.
 * @param in_dummy_section  True when the parser is inside a dummy / DUM block.
 * @return The appropriate LabelPolicy for this combination.
 */
LabelPolicy ClassifyLabelPolicy(std::string_view opcode_upper, bool has_label,
                                bool in_dummy_section);

/**
 * @brief Define the label in the symbol table (and optionally emit a LabelAtom)
 *        according to @p policy.
 *
 * CC <= 8.  Does nothing when policy is Skip or Defer.
 *
 * @param label             Raw label string (may be local or global).
 * @param address           Assembly address to assign to this label (may be
 *                          current_address_ or dum_address_ for DUM blocks).
 * @param policy            Result of ClassifyLabelPolicy().
 * @param emit_atom         When policy==AtPc, controls whether a LabelAtom is
 *                          emitted. For AtAddress the atom is always emitted.
 * @param symbols           Symbol table to define into.
 * @param section           Section whose atom list receives the LabelAtom.
 * @param local_labels      Parser's local-label address cache (updated here).
 * @param last_global_label Parser's last-seen global label name (updated here).
 * @param scope_fn          Converts a raw label to its scoped/canonical name.
 *                          May apply sequencing (e.g. ]var_1, ]var_2) before
 *                          returning.
 * @param is_local_fn       Returns true when a label string is a local label.
 *                          Local labels update local_labels but NOT
 *                          last_global_label.
 * @param on_global_update  Called after last_global_label is updated; used by
 *                          parsers that need additional state changes (e.g.
 *                          clearing local-label caches).  May be nullptr/empty.
 */
void DefineLabelForDirective(const std::string& label, uint32_t address, LabelPolicy policy,
                             bool emit_atom, ConcreteSymbolTable& symbols, Section& section,
                             std::unordered_map<std::string, uint32_t>& local_labels,
                             std::string& last_global_label,
                             const std::function<std::string(const std::string&)>& scope_fn,
                             const std::function<bool(const std::string&)>& is_local_fn,
                             const std::function<void()>& on_global_update = {});

}  // namespace xasm

#endif  // XASMPP_SYNTAX_LABEL_POLICY_H
