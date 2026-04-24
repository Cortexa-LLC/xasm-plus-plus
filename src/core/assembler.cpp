/**
 * @file assembler.cpp
 * @brief Assembler implementation
 */

#include "xasm++/assembler.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

#include "xasm++/common/expression_parser.h"
#include "xasm++/expression.h"
#include "xasm++/parse_utils.h"
#include "xasm++/symbol.h"
#include "xasm++/util/string_utils.h"

namespace xasm {

// Bring util functions into scope for convenience
using xasm::util::Trim;

// Note: ParseHex() consolidated to xasm::ParseHex (from parse_utils.h)

// ---------------------------------------------------------------------------
// Helper: replace unqualified '*' with current address string.
// An asterisk is unqualified when it is NOT preceded by an identifier char
// (alphanumeric, '.', '_', '?').  This mirrors the logic used in HandleDataAtom,
// RefixupDataAtoms and HandleEquateAtom.
// ---------------------------------------------------------------------------
static void ReplaceStarWithAddress(std::string& expr_str, const std::string& addr_str) {
  size_t star_pos = 0;
  while ((star_pos = expr_str.find('*', star_pos)) != std::string::npos) {
    bool preceded_by_ident = false;
    if (star_pos > 0) {
      char prev = expr_str[star_pos - 1];
      preceded_by_ident = std::isalnum(static_cast<unsigned char>(prev)) || prev == '.' ||
                          prev == '_' || prev == '?';
    }
    if (preceded_by_ident) {
      star_pos++;
      continue;
    }
    expr_str.replace(star_pos, 1, addr_str);
    star_pos += addr_str.length();
  }
}

// ---------------------------------------------------------------------------
// EncodePass — IAtomVisitor implementation for EncodeInstructions
//
// Replaces the switch/dynamic_pointer_cast dispatch chain.  Each Visit()
// overload receives an already-typed atom reference, calls the matching
// Assembler::Handle* helper, and sets wants_continue when the outer loop
// should skip the current atom (equivalent to the old `continue`).
// ---------------------------------------------------------------------------
class EncodePass final : public IAtomVisitor {
 public:
  Assembler& asm_;
  Assembler::EncodeAtomState& state_;
  bool wants_continue{false};
  std::shared_ptr<Atom> current_atom_ = nullptr;

  EncodePass(Assembler& a, Assembler::EncodeAtomState& st) : asm_(a), state_(st) {}

  void Visit(PhaseAtom&) override { wants_continue = asm_.HandlePhaseAtom(current_atom_, state_); }

  void Visit(OrgAtom&) override { wants_continue = asm_.HandleOrgAtom(current_atom_, state_); }

  void Visit(EquateAtom&) override {
    wants_continue = asm_.HandleEquateAtom(current_atom_, state_);
  }

  void Visit(DataAtom&) override { asm_.HandleDataAtom(current_atom_, state_); }

  void Visit(InstructionAtom&) override { asm_.HandleInstructionAtom(current_atom_, state_); }

  void Visit(SpaceAtom&) override { asm_.HandleSpaceAtom(current_atom_, state_); }

  // Labels resolved in ResolveSymbols; no bytes emitted here.
  void Visit(LabelAtom&) override {}

  // DummyOrg: like Label, no bytes emitted during encode.
  void Visit(DummyOrgAtom&) override {}

  void Visit(CpuModeAtom& cm) override {
    if (asm_.cpu_) {
      asm_.cpu_->SetCpuModeFromAtom(cm.mode);
    }
  }

  void Visit(MxAtom& mx) override {
    if (asm_.cpu_) {
      asm_.cpu_->SetMX(mx.m_flag, mx.x_flag);
    }
  }

  // ListingControlAtom: no encoding action
  void Visit(ListingControlAtom&) override {}

  // AlignAtom: handled in ResolveSymbols, not EncodeInstructions
  void Visit(AlignAtom&) override {}
};

// ---------------------------------------------------------------------------
// RefixupPass — IAtomVisitor implementation for RefixupDataAtoms
//
// Replaces the switch/dynamic_pointer_cast dispatch chain in RefixupDataAtoms.
// Holds the per-section mutable state; caller resets before each section.
// ---------------------------------------------------------------------------
class RefixupPass final : public IAtomVisitor {
 public:
  Assembler& asm_;
  ConcreteSymbolTable& symbols_;
  AssemblerResult& result_;
  uint32_t current_address{0};
  uint32_t virtual_address{0};
  uint32_t phase_real_start{0};
  uint32_t phase_virtual_start{0};

  RefixupPass(Assembler& a, ConcreteSymbolTable& sym, AssemblerResult& res)
      : asm_(a), symbols_(sym), result_(res) {}

  void Visit(PhaseAtom& ph) override {
    if (ph.is_start) {
      phase_real_start = current_address;
      phase_virtual_start = ph.virtual_addr;
      virtual_address = ph.virtual_addr;
    } else {
      uint32_t bytes_emitted = virtual_address - phase_virtual_start;
      current_address = phase_real_start + bytes_emitted;
      virtual_address = current_address;
    }
  }

  void Visit(OrgAtom& org) override {
    current_address = org.address;
    virtual_address = org.address;
  }

  void Visit(DummyOrgAtom&) override {
    // .OR inside .DUMMY/.ED: skip — do not move the real PC during refixup.
  }

  void Visit(CpuModeAtom& cm) override {
    if (asm_.cpu_) {
      asm_.cpu_->SetCpuModeFromAtom(cm.mode);
    }
  }

  void Visit(MxAtom& mx) override {
    if (asm_.cpu_) {
      asm_.cpu_->SetMX(mx.m_flag, mx.x_flag);
    }
  }

  void Visit(EquateAtom& eq) override {
    std::string expr_str = eq.expression_str;
    ReplaceStarWithAddress(expr_str, std::to_string(virtual_address));
    try {
      auto expr = ExpressionParser(&symbols_, nullptr, asm_.expression_features_).Parse(expr_str);
      int64_t value = expr->Evaluate(symbols_);
      symbols_.Define(eq.label_name, SymbolType::Equate,
                      std::make_shared<LiteralExpr>(static_cast<uint32_t>(value)));
    } catch (const std::exception&) {
      // Should be resolved by now; ignore.
    }
  }

  void Visit(DataAtom& data) override {
    if (!data.expressions.empty()) {
      data.data.clear();
      for (const auto& expr_str_raw : data.expressions) {
        std::string expr_str = expr_str_raw;
        ReplaceStarWithAddress(expr_str, std::to_string(virtual_address));
        try {
          auto expr =
              ExpressionParser(&symbols_, nullptr, asm_.expression_features_).Parse(expr_str);
          int64_t value = expr->Evaluate(symbols_);
          if (data.data_size == DataSize::Byte) {
            data.data.push_back(static_cast<uint8_t>(value & 0xFF));
          } else if (data.data_size == DataSize::Long) {
            auto word = static_cast<uint32_t>(value);
            data.data.push_back(static_cast<uint8_t>(word & 0xFF));
            data.data.push_back(static_cast<uint8_t>((word >> 8) & 0xFF));
            data.data.push_back(static_cast<uint8_t>((word >> 16) & 0xFF));
          } else {
            auto word = static_cast<uint32_t>(value);
            data.data.push_back(static_cast<uint8_t>(word & 0xFF));
            data.data.push_back(static_cast<uint8_t>((word >> 8) & 0xFF));
          }
        } catch (const UndefinedSymbolError& e) {
          AssemblerError err;
          err.message = e.what();
          result_.errors.push_back(err);
          result_.success = false;
          if (data.data_size == DataSize::Byte) {
            data.data.push_back(0);
          } else if (data.data_size == DataSize::Long) {
            data.data.push_back(0);
            data.data.push_back(0);
            data.data.push_back(0);
          } else {
            data.data.push_back(0);
            data.data.push_back(0);
          }
        }
      }
      data.size = data.data.size();
    }
    current_address += data.size;
    virtual_address += data.size;
  }

  void Visit(InstructionAtom& inst) override {
    // Do NOT re-encode; just advance address past the existing bytes.
    current_address += inst.encoded_bytes.size();
    virtual_address += inst.encoded_bytes.size();
  }

  void Visit(SpaceAtom& sp) override {
    current_address += sp.size;
    virtual_address += sp.size;
  }

  void Visit(AlignAtom& align) override {
    uint32_t remainder = current_address % align.alignment;
    if (remainder != 0) {
      uint32_t padding = align.alignment - remainder;
      current_address += padding;
      virtual_address += padding;
    }
  }

  // Label, ListingControl: no bytes, no address change.
  void Visit(LabelAtom&) override {}

  void Visit(ListingControlAtom&) override {}
};

Assembler::Assembler() = default;

void Assembler::SetCpuPlugin(CpuPlugin* cpu) {
  cpu_ = cpu;
}

void Assembler::SetSymbolTable(SymbolTable* symbols) {
  symbols_ = symbols;
}

void Assembler::SetMaxPasses(int max_passes) {
  if (max_passes >= 1) {
    max_passes_ = max_passes;
  }
}

void Assembler::SetExpressionFeatures(ParserFeatures features) {
  expression_features_ = features;
}

void Assembler::AddSection(const Section& section) {
  sections_.push_back(section);
}

size_t Assembler::GetSectionCount() const {
  return sections_.size();
}

const std::vector<Section>& Assembler::GetSections() const {
  return sections_;
}

void Assembler::Reset() {
  sections_.clear();
}

std::vector<size_t> Assembler::EncodeInstructions(ConcreteSymbolTable& symbols,
                                                  AssemblerResult& result, int pass_number) {
  // Encode instructions using CPU plugin
  std::vector<size_t> current_sizes;
  if (cpu_ != nullptr) {
    for (auto& section : sections_) {
      // Track current address during encoding.
      // virtual_address tracks the PC seen by the program (virtual during
      // .PH/.EP phase blocks); used for branch offset calculations.
      uint32_t current_address = section.org;
      uint32_t virtual_address = section.org;
      uint32_t phase_real_start = 0;
      uint32_t phase_virtual_start = 0;

      // Reset 65816 register-width state (M/X flags) to 8-bit defaults at
      // the start of each section so REP/SEP instructions re-establish the
      // correct widths from scratch each encoding pass.
      if (cpu_) {
        cpu_->SetMX(true, true);
      }

      for (auto& atom : section.atoms) {
        // Skip null atoms gracefully
        if (!atom) {
          AssemblerError error;
          error.message = "Null atom encountered - skipping";
          result.errors.push_back(error);
          result.success = false;
          continue;
        }

        EncodeAtomState state{current_address,     virtual_address, phase_real_start,
                              phase_virtual_start, symbols,         result,
                              current_sizes,       pass_number};
        EncodePass pass(*this, state);
        pass.current_atom_ = atom;
        atom->Accept(pass);
        if (pass.wants_continue) {
          continue;
        }
      }
    }
  }
  return current_sizes;
}

// ─── Per-atom-type handlers extracted from EncodeInstructions ──────────

bool Assembler::HandlePhaseAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& st) {
  uint32_t& current_address = st.current_address;
  uint32_t& virtual_address = st.virtual_address;
  uint32_t& phase_real_start = st.phase_real_start;
  uint32_t& phase_virtual_start = st.phase_virtual_start;
  (void)st.symbols;
  AssemblerResult& result = st.result;
  (void)st.current_sizes;
  (void)st.pass_number;
  auto phase = std::dynamic_pointer_cast<PhaseAtom>(atom);
  if (!phase) {
    AssemblerError error;
    error.location = atom->location;
    error.message = "Failed to cast to PhaseAtom - atom corruption detected";
    result.errors.push_back(error);
    result.success = false;
    return true;
  }
  if (phase->is_start) {
    phase_real_start = current_address;
    phase_virtual_start = phase->virtual_addr;
    virtual_address = phase->virtual_addr;
  } else {
    uint32_t bytes_emitted = virtual_address - phase_virtual_start;
    current_address = phase_real_start + bytes_emitted;
    virtual_address = current_address;
  }
  return false;
}

bool Assembler::HandleOrgAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& st) {
  uint32_t& current_address = st.current_address;
  uint32_t& virtual_address = st.virtual_address;
  (void)st.phase_real_start;
  (void)st.phase_virtual_start;
  (void)st.symbols;
  AssemblerResult& result = st.result;
  (void)st.current_sizes;
  (void)st.pass_number;
  // Handle .org directive
  auto org = std::dynamic_pointer_cast<OrgAtom>(atom);
  if (!org) {
    // Cast failed - this indicates a corrupted atom
    AssemblerError error;
    error.location = atom->location;
    error.message = "Failed to cast to OrgAtom - atom corruption detected";
    result.errors.push_back(error);
    result.success = false;
    return true;
  }
  current_address = org->address;
  virtual_address = org->address;
  return false;
}

bool Assembler::HandleEquateAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& st) {
  (void)st.current_address;
  uint32_t& virtual_address = st.virtual_address;
  (void)st.phase_real_start;
  (void)st.phase_virtual_start;
  ConcreteSymbolTable& symbols = st.symbols;
  (void)st.result;
  (void)st.current_sizes;
  (void)st.pass_number;
  // Re-evaluate position-dependent equates (.EQ *) on each pass so
  // they track the correct address after branch relaxation changes
  // code sizes between passes.
  auto eq = std::dynamic_pointer_cast<EquateAtom>(atom);
  if (eq) {
    std::string expr_str = eq->expression_str;
    std::string addr_str = std::to_string(virtual_address);
    size_t star_pos = 0;
    while ((star_pos = expr_str.find('*', star_pos)) != std::string::npos) {
      bool preceded_by_ident = false;
      if (star_pos > 0) {
        char prev = expr_str[star_pos - 1];
        preceded_by_ident = std::isalnum(static_cast<unsigned char>(prev)) || prev == '.' ||
                            prev == '_' || prev == '?';
      }
      if (preceded_by_ident) {
        star_pos++;
        continue;
      }
      expr_str.replace(star_pos, 1, addr_str);
      star_pos += addr_str.length();
    }
    try {
      std::shared_ptr<Expression> expr =
          ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
      int64_t value = expr->Evaluate(symbols);
      symbols.Define(eq->label_name, SymbolType::Equate,
                     std::make_shared<LiteralExpr>(static_cast<uint32_t>(value)));
    } catch (const std::exception& e) {
      (void)e;  // Forward reference - ignore this pass, will resolve later
    }
  }
  return false;
}

// ---------------------------------------------------------------------------
// EvaluateDataElement — helper extracted from HandleDataAtom
//
// Evaluates a single expression string from a DataAtom and appends the
// resulting byte(s) to data.data.  All star-substitution and forward-reference
// handling that was previously duplicated inside the per-element loop now lives
// here, keeping HandleDataAtom itself short and low-CC.
// ---------------------------------------------------------------------------
void Assembler::EvaluateDataElement(const std::string& expr_str_raw, uint32_t virtual_address,
                                    ConcreteSymbolTable& symbols, DataAtom& data) {
  // Replace bare '*' with the current virtual address (PC), same as EquateAtom.
  // A '*' that is preceded or followed by an identifier character is treated as
  // multiplication, not the current-location symbol.
  std::string expr_str = expr_str_raw;
  {
    std::string addr_str = std::to_string(virtual_address);
    size_t star_pos = 0;
    while ((star_pos = expr_str.find('*', star_pos)) != std::string::npos) {
      bool preceded_by_ident = false;
      if (star_pos > 0) {
        char prev = expr_str[star_pos - 1];
        preceded_by_ident = std::isalnum(static_cast<unsigned char>(prev)) || prev == '.' ||
                            prev == '_' || prev == '?';
      }
      if (preceded_by_ident) {
        star_pos++;
        continue;
      }
      expr_str.replace(star_pos, 1, addr_str);
      star_pos += addr_str.length();
    }
  }

  try {
    std::shared_ptr<Expression> expr =
        ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
    int64_t value = expr->Evaluate(symbols);

    if (data.data_size == DataSize::Byte) {
      // Byte data (DB/DFB)
      data.data.push_back(static_cast<uint8_t>(value & 0xFF));
    } else if (data.data_size == DataSize::Long) {
      // Long data (DA in 65816 mode) - 24-bit little-endian
      auto word = static_cast<uint32_t>(value);
      data.data.push_back(static_cast<uint8_t>(word & 0xFF));
      data.data.push_back(static_cast<uint8_t>((word >> 8) & 0xFF));
      data.data.push_back(static_cast<uint8_t>((word >> 16) & 0xFF));
    } else {
      // Word data (DW/DA) - little-endian
      auto word = static_cast<uint32_t>(value);
      data.data.push_back(static_cast<uint8_t>(word & 0xFF));
      data.data.push_back(static_cast<uint8_t>((word >> 8) & 0xFF));
    }
  } catch (const UndefinedSymbolError&) {
    // Forward reference — use placeholder 0; resolve in a subsequent pass.
    if (data.data_size == DataSize::Byte) {
      data.data.push_back(0);
    } else if (data.data_size == DataSize::Long) {
      data.data.push_back(0);
      data.data.push_back(0);
      data.data.push_back(0);
    } else {
      data.data.push_back(0);
      data.data.push_back(0);
    }
  }
}

bool Assembler::HandleDataAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& st) {
  uint32_t& current_address = st.current_address;
  uint32_t& virtual_address = st.virtual_address;
  (void)st.phase_real_start;
  (void)st.phase_virtual_start;
  ConcreteSymbolTable& symbols = st.symbols;
  AssemblerResult& result = st.result;
  std::vector<size_t>& current_sizes = st.current_sizes;
  (void)st.pass_number;
  auto data = std::dynamic_pointer_cast<DataAtom>(atom);
  if (!data) {
    // Cast failed - this indicates a corrupted atom
    AssemblerError error;
    error.location = atom->location;
    error.message = "Failed to cast to DataAtom - atom corruption detected";
    result.errors.push_back(error);
    result.success = false;
    return true;
  }

  // Re-evaluate expressions on each pass for forward references.
  if (!data->expressions.empty()) {
    data->data.clear();
    for (const auto& expr_str_raw : data->expressions) {
      EvaluateDataElement(expr_str_raw, virtual_address, symbols, *data);
    }
    data->size = data->data.size();
  }

  // Advance both address counters past this data
  current_address += data->size;
  virtual_address += data->size;
  current_sizes.push_back(data->size);
  return false;
}

// ---------------------------------------------------------------------------
// TrySpecialEncodeInstruction — helper extracted from HandleInstructionAtom
//
// Handles the "special encoding" branch (branch relaxation, block-move
// multi-operand instructions, etc.) via cpu_->EncodeInstructionSpecial().
// Returns true when the instruction has been fully encoded (caller should
// advance address counters and continue), or false when standard encoding
// should be used instead.
// ---------------------------------------------------------------------------
// ResolveBranchOperand — format a branch target operand as "$XXXX" hex string.
// Pass 1 always returns current VA (start short; let passes 2+ expand only when
// necessary).  Pass 2+ resolves via ExpressionParser so SYMBOL, SYMBOL+N, and
// arbitrary expressions are all handled without a hand-rolled lookup loop.
static std::string ResolveBranchOperand(const std::string& trimmed, uint32_t virtual_address,
                                        ConcreteSymbolTable& symbols, ParserFeatures features,
                                        int pass_number) {
  auto hex_va = [&]() {
    std::ostringstream oss;
    oss << "$" << std::hex << virtual_address;
    return oss.str();
  };

  if (trimmed.find(',') != std::string::npos) {
    return trimmed;  // multi-operand (MVN/MVP): pass as-is
  }
  if (trimmed == "*") {
    return hex_va();  // branch-to-self: current VA
  }
  if (pass_number <= 1) {
    return hex_va();  // pass 1: always short to avoid spurious expansions
  }
  // Try ExpressionParser first (handles SYMBOL, SYMBOL+N, and expressions).
  // Some forms (e.g. SCMASM colon-labels like ":2") cannot be parsed by
  // ExpressionParser, so fall back to direct ConcreteSymbolTable::Lookup
  // on any parse/evaluate failure, then to current VA (stay short).
  try {
    auto expr = ExpressionParser(&symbols, nullptr, features).Parse(trimmed);
    std::ostringstream oss;
    oss << "$" << std::hex << (static_cast<uint32_t>(expr->Evaluate(symbols)) & 0xFFFF);
    return oss.str();
  } catch (const std::exception&) {
    int64_t sym_val = 0;
    if (symbols.Lookup(trimmed, sym_val)) {
      std::ostringstream oss;
      oss << "$" << std::hex << (static_cast<uint32_t>(sym_val) & 0xFFFF);
      return oss.str();
    }
    return hex_va();
  }
}

bool Assembler::TrySpecialEncodeInstruction(InstructionAtom& inst, uint32_t current_address,
                                            uint32_t virtual_address, ConcreteSymbolTable& symbols,
                                            AssemblerResult& result, int pass_number) {
  const std::string& mnemonic = inst.mnemonic;

  if (!cpu_->RequiresSpecialEncoding(mnemonic)) {
    return false;
  }

  try {
    std::string trimmed = Trim(inst.operand);
    std::string resolved =
        ResolveBranchOperand(trimmed, virtual_address, symbols, expression_features_, pass_number);
    inst.encoded_bytes =
        cpu_->EncodeInstructionSpecial(mnemonic, resolved, static_cast<uint16_t>(virtual_address));
    (void)current_address;
    return true;
  } catch (const std::exception& e) {
    AssemblerError error;
    error.location = inst.location;
    error.message = "Special encoding failed for " + mnemonic + ": " + e.what();
    result.errors.push_back(error);
    result.success = false;
    return true;
  }
}

// ---------------------------------------------------------------------------
// ParseInstructionOperandValue — helper extracted from HandleInstructionAtom
//
// Parses the operand string and returns a uint16_t value suitable for passing
// to cpu_->EncodeInstruction().  Returns 0 for forward references (which are
// resolved in a later pass) and for accumulator-mode operands ("A").
// ---------------------------------------------------------------------------
uint16_t Assembler::ParseInstructionOperandValue(const std::string& operand,
                                                 uint32_t virtual_address,
                                                 ConcreteSymbolTable& symbols) {
  if (operand.empty()) {
    return 0;
  }

  symbols.SetCurrentLocation(static_cast<int64_t>(virtual_address));

  std::string trimmed = Trim(operand);

  // Strip parentheses for indirect modes: ($1234) or ($80,X) or ($80),Y
  std::string value_str = trimmed;
  if (!value_str.empty() && value_str[0] == '(') {
    size_t close_paren = value_str.find(')');
    if (close_paren != std::string::npos) {
      value_str = Trim(value_str.substr(1, close_paren - 1));
    }
  }

  // Strip index registers (,X or ,Y) for value extraction
  {
    size_t comma_pos = value_str.find(',');
    if (comma_pos != std::string::npos) {
      value_str = Trim(value_str.substr(0, comma_pos));
    }
  }

  // Strip brackets for 65816 indirect long: [$zp] or [$zp],Y
  if (!value_str.empty() && value_str[0] == '[') {
    size_t close_bracket = value_str.find(']');
    if (close_bracket != std::string::npos) {
      value_str = Trim(value_str.substr(1, close_bracket - 1));
    }
  }

  if (value_str.empty()) {
    return 0;
  }

  if (value_str == "A") {
    return 0;  // Accumulator addressing mode — no operand value needed
  }

  // All remaining forms (#immediate, $hex, /high-byte, label/expr) are
  // handled by ExpressionParser.  Strip known prefix characters first,
  // then evaluate; apply a right-shift for the SCMASM /expr high-byte form.
  std::string expr_str = value_str;
  int right_shift = 0;
  if (!expr_str.empty() && expr_str[0] == '#') {
    expr_str = expr_str.substr(1);  // strip # — ExpressionParser handles the rest
  } else if (!expr_str.empty() && expr_str[0] == '/') {
    expr_str = expr_str.substr(1);  // strip / — take bits [15:8] of result
    right_shift = 8;
  }
  // $ and bare symbols are passed through unchanged; ExpressionParser handles both.

  try {
    auto expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
    int64_t val = expr->Evaluate(symbols);
    return static_cast<uint16_t>((static_cast<uint32_t>(val) >> right_shift) & 0xFFFF);
  } catch (const UndefinedSymbolError&) {
    return 0;  // Forward reference — resolve next pass
  }
}

bool Assembler::HandleInstructionAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& st) {
  uint32_t& current_address = st.current_address;
  uint32_t& virtual_address = st.virtual_address;
  (void)st.phase_real_start;
  (void)st.phase_virtual_start;
  ConcreteSymbolTable& symbols = st.symbols;
  AssemblerResult& result = st.result;
  std::vector<size_t>& current_sizes = st.current_sizes;
  int& pass_number = st.pass_number;
  auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
  if (!inst) {
    AssemblerError error;
    error.location = atom->location;
    error.message = "Failed to cast to InstructionAtom - atom corruption detected";
    result.errors.push_back(error);
    result.success = false;
    return true;
  }

  inst->encoded_bytes.clear();

  // Strip trailing '!' from mnemonic before table lookup.
  std::string mnemonic = inst->mnemonic;
  if (!mnemonic.empty() && mnemonic.back() == '!') {
    mnemonic.pop_back();
  }
  // Keep inst->mnemonic in sync so TrySpecialEncodeInstruction sees the
  // cleaned mnemonic.
  inst->mnemonic = mnemonic;

  // -----------------------------------------------------------------------
  // Special encoding path (branch relaxation, MVN/MVP, etc.)
  // -----------------------------------------------------------------------
  if (TrySpecialEncodeInstruction(*inst, current_address, virtual_address, symbols, result,
                                  pass_number)) {
    // Only update address counters if encoding actually produced bytes
    // (on error, encoded_bytes is empty and we skip advancement).
    if (!inst->encoded_bytes.empty() || result.success) {
      current_address += inst->encoded_bytes.size();
      virtual_address += inst->encoded_bytes.size();
      current_sizes.push_back(inst->encoded_bytes.size());
    }
    return true;
  }

  // -----------------------------------------------------------------------
  // Standard encoding path
  // -----------------------------------------------------------------------
  uint16_t value = ParseInstructionOperandValue(inst->operand, virtual_address, symbols);

  try {
    inst->encoded_bytes = cpu_->EncodeInstruction(mnemonic, value, inst->operand);
  } catch (const std::invalid_argument& e) {
    AssemblerError error;
    error.location = inst->location;
    error.message = "Invalid argument for " + mnemonic + ": " + e.what();
    result.errors.push_back(error);
    result.success = false;
  } catch (const std::out_of_range& e) {
    AssemblerError error;
    error.location = inst->location;
    error.message = "Value out of range for " + mnemonic + ": " + e.what();
    result.errors.push_back(error);
    result.success = false;
  } catch (const std::runtime_error& e) {
    AssemblerError error;
    error.location = inst->location;
    error.message = "Runtime error encoding " + mnemonic + ": " + e.what();
    result.errors.push_back(error);
    result.success = false;
  } catch (const std::logic_error& e) {
    AssemblerError error;
    error.location = inst->location;
    error.message = "Logic error encoding " + mnemonic + ": " + e.what();
    result.errors.push_back(error);
    result.success = false;
  }

  current_sizes.push_back(inst->encoded_bytes.size());
  current_address += inst->encoded_bytes.size();
  virtual_address += inst->encoded_bytes.size();
  return false;
}

// ---------------------------------------------------------------------------
// EvaluateSpaceExpression — helper extracted from HandleSpaceAtom
//
// Re-evaluates the PC-relative expression stored in space.expression_str
// (e.g. "DS $900-*") and updates space.count / space.size in place.  The
// '*' token is substituted with virtual_address before parsing.
// ---------------------------------------------------------------------------
void Assembler::EvaluateSpaceExpression(SpaceAtom& space, uint32_t virtual_address,
                                        ConcreteSymbolTable& symbols) {
  std::string expr_str = space.expression_str;
  std::string addr_str = std::to_string(virtual_address);
  size_t star_pos = 0;
  while ((star_pos = expr_str.find('*', star_pos)) != std::string::npos) {
    bool before =
        (star_pos > 0 && (std::isalnum(static_cast<unsigned char>(expr_str[star_pos - 1])) ||
                          expr_str[star_pos - 1] == ')'));
    bool after = (star_pos + 1 < expr_str.size() &&
                  (std::isalnum(static_cast<unsigned char>(expr_str[star_pos + 1])) ||
                   expr_str[star_pos + 1] == '(' || expr_str[star_pos + 1] == '$' ||
                   expr_str[star_pos + 1] == '%'));
    if (before && after) {
      star_pos++;
    } else {
      expr_str.replace(star_pos, 1, addr_str);
      star_pos += addr_str.length();
    }
  }
  try {
    auto expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
    int64_t value = expr->Evaluate(symbols);
    if (value >= 0) {
      space.count = static_cast<size_t>(value);
      space.size = space.count;
    }
  } catch (const std::exception&) {
    // Forward reference or evaluation error — keep previous count.
  }
}

bool Assembler::HandleSpaceAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& st) {
  uint32_t& current_address = st.current_address;
  uint32_t& virtual_address = st.virtual_address;
  (void)st.phase_real_start;
  (void)st.phase_virtual_start;
  ConcreteSymbolTable& symbols = st.symbols;
  (void)st.result;
  std::vector<size_t>& current_sizes = st.current_sizes;
  (void)st.pass_number;
  auto space = std::dynamic_pointer_cast<SpaceAtom>(atom);
  if (!space) {
    return false;
  }

  // Re-evaluate the PC-relative expression (if any) each pass so that the
  // count stays correct as branch relaxations shift code sizes between passes.
  if (!space->expression_str.empty()) {
    EvaluateSpaceExpression(*space, virtual_address, symbols);
  }

  current_address += space->size;
  virtual_address += space->size;
  current_sizes.push_back(space->size);
  return false;
}

void Assembler::RefixupDataAtoms(ConcreteSymbolTable& symbols, AssemblerResult& result) {
  // Re-evaluate DataAtom and EquateAtom content with the final converged symbol
  // values.  InstructionAtoms are intentionally skipped (their encoded_bytes
  // are left unchanged) to prevent branch-size changes from cascading into
  // address shifts.
  if (cpu_ == nullptr) {
    return;
  }

  for (auto& section : sections_) {
    RefixupPass pass(*this, symbols, result);
    pass.current_address = section.org;
    pass.virtual_address = section.org;

    for (auto& atom : section.atoms) {
      if (!atom) {
        continue;
      }
      atom->Accept(pass);
    }  // for (auto &atom : section.atoms)
  }  // for (auto &section : sections_)
}

AssemblerResult Assembler::Assemble() {
  // WHY MULTI-PASS ASSEMBLY?
  // ========================
  // Multi-pass assembly is necessary because of three interdependent
  // challenges:
  //
  // 1. FORWARD REFERENCES: Labels can be used before they're defined
  //    Example: JMP end_loop    ; 'end_loop' address unknown on first pass
  //             ...
  //             end_loop: RTS   ; Now we know the address
  //
  // 2. BRANCH RELAXATION: Branch instructions may change size between passes
  //    Example: BEQ label       ; Initially 2 bytes (short branch)
  //             [lots of code]
  //             label: NOP       ; If >127 bytes away, must expand to:
  //                              ; BNE +3 / JMP label (5 bytes total)
  //
  // 3. INSTRUCTION SIZING: We can't know final instruction sizes until symbols
  // are resolved
  //    - Short branches (2 bytes) vs long branches (5 bytes)
  //    - Zero page addressing ($80) vs absolute addressing ($1234)
  //
  // CONVERGENCE ALGORITHM:
  // ======================
  // We repeat encoding passes until instruction sizes stabilize:
  // - Pass 1: Encode with best guesses, extract label addresses
  // - Pass 2: Re-encode with updated addresses, check if sizes changed
  // - Pass N: Repeat until sizes stop changing (converged)
  //
  // Typically converges in 2-3 passes. MAX_PASSES (10) prevents infinite loops
  // from pathological cases (e.g., oscillating forward references).
  //
  // WHY 10 PASSES?
  // ==============
  // Empirical data from comprehensive test suite:
  // - Typical: 2-3 passes (simple forward references)
  // - Maximum observed: 5 passes (complex forward reference chains)
  // - Real-world code: Always <10 passes
  //
  // 10 is chosen as a safety margin:
  // - 2x the maximum observed pass count (5 passes)
  // - Catches bugs quickly (oscillating branches fail at 10, not 500)
  // - Fast feedback for developers (no 500-iteration hangs)

  AssemblerResult result;

  // Empty assembly
  if (sections_.empty()) {
    result.success = true;
    result.pass_count = 0;
    return result;
  }

  // Create internal symbol table if needed
  // Always extract labels from atoms to get correct addresses
  ConcreteSymbolTable internal_symbols;
  ConcreteSymbolTable* label_table_ptr = nullptr;

  if (symbols_ != nullptr) {
    // If external symbol table is ConcreteSymbolTable, use it for label updates
    label_table_ptr = dynamic_cast<ConcreteSymbolTable*>(symbols_);
  } else {
    // Otherwise use internal table
    label_table_ptr = &internal_symbols;
  }

  // Multi-pass assembly loop
  bool converged = false;
  int pass = 0;
  std::vector<size_t> previous_sizes;

  while (!converged && pass < max_passes_) {
    pass++;

    // Pass 1: Encode instructions using CPU plugin
    std::vector<size_t> current_sizes = EncodeInstructions(*label_table_ptr, result, pass);

    // Pass 2: Extract labels from LabelAtoms
    // (Must happen AFTER encoding so encoded_bytes.size() is correct)
    // ALWAYS do this to ensure correct addresses, even with external symbol
    // tables
    if (label_table_ptr != nullptr) {
      for (auto& section : sections_) {
        ResolveSymbols(section.atoms, *label_table_ptr, section.org, result);
      }
    }

    // Check for convergence
    // WHY: If instruction sizes are identical to previous pass, addresses won't
    // change, so we've reached a stable state and can stop.
    if (pass > 1) {
      converged = CheckConvergence({previous_sizes, current_sizes});
    }
    previous_sizes = current_sizes;
  }

  // Check if we hit the pass limit without converging
  if (!converged) {
    AssemblerError error;
    error.message = "Assembly did not converge after " + std::to_string(max_passes_) +
                    " passes — possible oscillating forward references or too many "
                    "cascading branches";
    result.errors.push_back(error);
    result.success = false;
  }

  // Post-convergence instruction fixup pass.
  //
  // WHY IS THIS NEEDED?
  // ===================
  // During the convergence loop, EquateAtoms (e.g. "MAIN.S .EQ *-MAIN.B") are
  // processed IN ATOM ORDER, which means they are evaluated AFTER the
  // instructions that appear earlier in the source.  Those early instructions
  // therefore encode with the equate value from the END of the PREVIOUS pass.
  //
  // At parse time, position-tracking labels like "MAIN.B" are assigned
  // addresses based on the CPU plugin's size heuristics, which can differ from
  // the actual encoded sizes.  After the first full ResolveSymbols pass the
  // label gets its correct address.  The equate that depends on it (MAIN.S)
  // is then correct from pass 2 onward.  But an early instruction that
  // references MAIN.S still encodes with the pass-1 value of MAIN.S (which
  // used the heuristic-based MAIN.B) rather than the correct pass-2 value.
  //
  // Example (A2osX kernel):
  //   MAIN.B   .PH $800          ; label assigned heuristic file-PC at parse
  //   ...                        ; (large MAIN section)
  //   MAIN.S   .EQ *-MAIN.B      ; computed correctly after pass 1
  //   ...
  //   LDY #$800+MAIN.S           ; appears BEFORE MAIN.S in atom order
  //                              ; → encoded with MAIN.S from previous pass
  //
  // In the last convergence pass sizes are stable so convergence is declared,
  // but MAIN.S has the old (slightly wrong) value in the symbol table at the
  // time the LDY instruction is encoded.  RefixupDataAtoms then updates
  // .DA MAIN.S with the correct final value, creating an inconsistency:
  // the instruction has the wrong byte, the data table has the right one.
  //
  // SOLUTION: Run one additional EncodeInstructions pass AFTER convergence.
  // By this point the symbol table holds the final stable values set at the
  // END of the last convergence pass (including the correct MAIN.S).  All
  // instructions are therefore re-encoded with correct values.
  //
  // SAFETY: All instruction sizes are already stable (convergence guarantees
  // this), so re-encoding cannot change any instruction size.  Immediate-
  // value instructions like "LDY #imm" are always 2 bytes regardless of the
  // immediate value; the extra pass only updates the operand bytes.
  // Pass a pass number > 1 so branches encode normally (not forced-long).
  EncodeInstructions(*label_table_ptr, result);

  // Final data fixup: re-evaluate DataAtoms and EquateAtoms with the converged
  // symbol values, WITHOUT re-encoding instructions.
  //
  // WHY NOT EncodeInstructions ALONE?
  // A full EncodeInstructions call re-encodes branches, which can change branch
  // sizes (e.g. a previously-relaxed 5-byte long branch may tighten back to 2
  // bytes when symbol values shift).  That cascades into address changes and
  // invalidates the label values computed by the last ResolveSymbols call.
  // RefixupDataAtoms avoids this by leaving InstructionAtom.encoded_bytes
  // untouched and simply advancing addresses past them.
  //
  // Together, the extra EncodeInstructions above and RefixupDataAtoms below
  // ensure that both InstructionAtoms and DataAtoms reflect the same final
  // symbol values.
  RefixupDataAtoms(*label_table_ptr, result);

  result.pass_count = pass;
  return result;
}

void Assembler::ResolveSymbols(std::vector<std::shared_ptr<Atom>>& atoms,
                               ConcreteSymbolTable& symbols, uint32_t org_address,
                               AssemblerResult& result) {
  // Clear only labels (preserve other symbols like EQU/SET)
  // For now, just redefine - this will overwrite parser's placeholder addresses
  uint32_t current_address = org_address;

  // Phase tracking: when inside .PH/.EP block, current_address uses the
  // virtual address so labels get the correct phased value.
  bool in_phase = false;
  uint32_t phase_real_start = 0;
  uint32_t phase_virtual_start = 0;

  // Process atoms to extract label addresses
  for (auto& atom : atoms) {
    // Skip null atoms gracefully
    if (!atom) {
      AssemblerError error;
      error.message = "Null atom encountered in symbol resolution - skipping";
      result.errors.push_back(error);
      result.success = false;
      continue;
    }

    switch (atom->type) {
      case AtomType::Phase: {
        auto phase = std::dynamic_pointer_cast<PhaseAtom>(atom);
        if (!phase) {
          AssemblerError error;
          error.location = atom->location;
          error.message = "Failed to cast to PhaseAtom - atom corruption detected";
          result.errors.push_back(error);
          result.success = false;
          continue;
        }
        if (phase->is_start) {
          // .PH: switch current_address to virtual address for label resolution
          phase_real_start = current_address;
          phase_virtual_start = phase->virtual_addr;
          current_address = phase->virtual_addr;
          in_phase = true;
        } else {
          // .EP: compute bytes emitted in phase and restore physical address
          uint32_t bytes_emitted = current_address - phase_virtual_start;
          current_address = phase_real_start + bytes_emitted;
          in_phase = false;
        }
        (void)in_phase;  // suppress unused warning if no further use
      } break;
      case AtomType::Org: {
        // Handle .org directive - updates current address
        auto org = std::dynamic_pointer_cast<OrgAtom>(atom);
        if (!org) {
          // Cast failed - this indicates a corrupted atom
          AssemblerError error;
          error.location = atom->location;
          error.message = "Failed to cast to OrgAtom - atom corruption detected";
          result.errors.push_back(error);
          result.success = false;
          continue;
        }
        current_address = org->address;
      } break;
      case AtomType::DummyOrg: {
        // .OR inside .DUMMY/.ED: skip in ResolveSymbols — do not move the real
        // PC.  Dummy-section symbols (e.g. ZPTR from "ZPTR .BS 2" inside
        // .DUMMY/.ED) are placed in the symbol table at their correct zero-page
        // addresses during parsing (HandleBs in dummy mode calls
        // symbols.Define() directly, and advances the parse-time address
        // counter, but does NOT emit a LabelAtom or DataAtom).
        //
        // Since there are NO LabelAtoms for dummy-section labels (they are
        // suppressed when in_dummy_section_ is true), changing current_address
        // here provides no benefit.  Worse, it permanently corrupts the
        // main-section PC for every atom that follows the .ED close — including
        // forward-reference labels like .1 in:
        //
        //   jmp (.1,x)      ; operand is a forward reference
        //   .DA #$61
        // .1 .DA 0          ; <- LabelAtom address is computed from current_address
        //
        // When current_address is stuck in the zero-page range (e.g. $E0+n),
        // the jmp operand resolves to a ZP address, EncodeInstruction picks
        // IndirectX instead of AbsoluteIndexedIndirect, JMP has no IndirectX
        // encoding, and the instruction is silently dropped (empty encoded_bytes).
        //
        // Fix: treat DummyOrgAtom as a no-op in ResolveSymbols.  The dummy
        // symbols are already correct from parse time; the main PC must remain
        // unaffected so that subsequent LabelAtoms get the right addresses.
      } break;
      case AtomType::Label: {
        auto label = std::dynamic_pointer_cast<LabelAtom>(atom);
        if (!label) {
          // Cast failed - this indicates a corrupted atom
          AssemblerError error;
          error.location = atom->location;
          error.message = "Failed to cast to LabelAtom - atom corruption detected";
          result.errors.push_back(error);
          result.success = false;
          continue;
        }
        // Update label address
        label->address = current_address;
        // Define or redefine label in symbol table
        symbols.Define(label->name, SymbolType::Label,
                       std::make_shared<LiteralExpr>(current_address));
      } break;
      case AtomType::Instruction: {
        // Instructions consume bytes
        auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
        if (!inst) {
          // Cast failed - this indicates a corrupted atom
          AssemblerError error;
          error.location = atom->location;
          error.message = "Failed to cast to InstructionAtom - atom corruption detected";
          result.errors.push_back(error);
          result.success = false;
          continue;
        }
        current_address += inst->encoded_bytes.size();
      } break;
      case AtomType::Data: {
        // Data directives consume bytes
        auto data = std::dynamic_pointer_cast<DataAtom>(atom);
        if (!data) {
          // Cast failed - this indicates a corrupted atom
          AssemblerError error;
          error.location = atom->location;
          error.message = "Failed to cast to DataAtom - atom corruption detected";
          result.errors.push_back(error);
          result.success = false;
          continue;
        }
        current_address += data->size;
      } break;
      case AtomType::Space: {
        // Space directives consume bytes
        auto space = std::dynamic_pointer_cast<SpaceAtom>(atom);
        if (!space) {
          // Cast failed - this indicates a corrupted atom
          AssemblerError error;
          error.location = atom->location;
          error.message = "Failed to cast to SpaceAtom - atom corruption detected";
          result.errors.push_back(error);
          result.success = false;
          continue;
        }
        current_address += space->size;
      } break;
      case AtomType::Align: {
        // Align directives may add padding
        auto align = std::dynamic_pointer_cast<AlignAtom>(atom);
        if (!align) {
          // Cast failed - this indicates a corrupted atom
          AssemblerError error;
          error.location = atom->location;
          error.message = "Failed to cast to AlignAtom - atom corruption detected";
          result.errors.push_back(error);
          result.success = false;
          continue;
        }
        // Calculate padding needed to reach alignment
        uint32_t remainder = current_address % align->alignment;
        if (remainder != 0) {
          current_address += align->alignment - remainder;
        }
      }
      // CpuMode, MxState, Equate, ListingControl: no address change in this pass.
      default:
        break;
    }  // switch (atom->type)
  }
}

bool Assembler::CheckConvergence(ConvergenceSizes sizes) {
  // Convergence achieved when instruction sizes are identical between passes
  //
  // WHY THIS WORKS:
  // ===============
  // If all instruction sizes are stable, then:
  // 1. All addresses are stable (no size changes = no address shifts)
  // 2. All symbol values are stable (addresses determine symbol values)
  // 3. All branches are correctly sized (no more relaxation needed)
  // 4. Further passes would produce identical output (fixed point reached)
  //
  // This is the mathematical definition of convergence: f(x) = x
  // where f is "encode one pass" and x is "instruction sizes vector"
  return sizes.current == sizes.previous;
}

}  // namespace xasm
