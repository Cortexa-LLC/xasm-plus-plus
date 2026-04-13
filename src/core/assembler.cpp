// Assembler implementation

#include "xasm++/assembler.h"
#include "xasm++/common/expression_parser.h"
#include "xasm++/expression.h"
#include "xasm++/parse_utils.h"
#include "xasm++/symbol.h"
#include "xasm++/util/string_utils.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

namespace xasm {

// Bring util functions into scope for convenience
using xasm::util::Trim;

// Note: ParseHex() consolidated to xasm::ParseHex (from parse_utils.h)



Assembler::Assembler() = default;

void Assembler::SetCpuPlugin(CpuPlugin *cpu) { cpu_ = cpu; }

void Assembler::SetSymbolTable(SymbolTable *symbols) { symbols_ = symbols; }

void Assembler::SetMaxPasses(int max_passes) {
  if (max_passes >= 1) {
    max_passes_ = max_passes;
  }
}

void Assembler::SetExpressionFeatures(ParserFeatures features) {
  expression_features_ = features;
}

void Assembler::AddSection(const Section &section) {
  sections_.push_back(section);
}

size_t Assembler::GetSectionCount() const { return sections_.size(); }

const std::vector<Section> &Assembler::GetSections() const { return sections_; }

void Assembler::Reset() { sections_.clear(); }

std::vector<size_t> Assembler::EncodeInstructions(ConcreteSymbolTable &symbols,
                                                  AssemblerResult &result,
                                                  int pass_number) {
  // Encode instructions using CPU plugin
  std::vector<size_t> current_sizes;
  if (cpu_ != nullptr) {
    for (auto &section : sections_) {
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

      for (auto &atom : section.atoms) {
        // Skip null atoms gracefully
        if (!atom) {
          AssemblerError error;
          error.message = "Null atom encountered - skipping";
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
            error.message =
                "Failed to cast to PhaseAtom - atom corruption detected";
            result.errors.push_back(error);
            result.success = false;
            continue;
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
        } break;
        case AtomType::Org: {
          // Handle .org directive
          auto org = std::dynamic_pointer_cast<OrgAtom>(atom);
          if (!org) {
            // Cast failed - this indicates a corrupted atom
            AssemblerError error;
            error.location = atom->location;
            error.message =
                "Failed to cast to OrgAtom - atom corruption detected";
            result.errors.push_back(error);
            result.success = false;
            continue;
          }
          current_address = org->address;
          virtual_address = org->address;
        } break;
        case AtomType::DummyOrg:
        case AtomType::Label: {
          // DummyOrg: .OR inside .DUMMY/.ED: do NOT change the real program counter.
          // Symbol addresses are fixed up in ResolveSymbols; the emitter just
          // skips this atom so no bytes are written and the PC is unaffected.
          // Labels don't advance address yet, but we track them
          // (address will be finalized in Pass 2)
        } break;
        case AtomType::CpuMode: {
          // XC / XC OFF — replay CPU mode change so subsequent instructions
          // are encoded with the correct feature set.
          auto cm = std::dynamic_pointer_cast<CpuModeAtom>(atom);
          if (cm && cpu_) {
            cpu_->SetCpuModeFromAtom(cm->mode);
          }
        } break;
        case AtomType::MxState: {
          // MX directive — replay M/X flag state so subsequent instructions
          // use the correct accumulator/index register widths.
          auto mx = std::dynamic_pointer_cast<MxAtom>(atom);
          if (mx && cpu_) {
            cpu_->SetMX(mx->m_flag, mx->x_flag);
          }
        } break;
        case AtomType::Equate: {
          // Re-evaluate position-dependent equates (.EQ *) on each pass so
          // they track the correct address after branch relaxation changes
          // code sizes between passes.
          auto eq = std::dynamic_pointer_cast<EquateAtom>(atom);
          if (eq) {
            std::string expr_str = eq->expression_str;
            std::string addr_str = std::to_string(virtual_address);
            size_t star_pos = 0;
            while ((star_pos = expr_str.find('*', star_pos)) !=
                   std::string::npos) {
              bool preceded_by_ident = false;
              if (star_pos > 0) {
                char prev = expr_str[star_pos - 1];
                preceded_by_ident =
                    std::isalnum(static_cast<unsigned char>(prev)) ||
                    prev == '.' || prev == '_' || prev == '?';
              }
              if (preceded_by_ident) {
                star_pos++;
                continue;
              }
              expr_str.replace(star_pos, 1, addr_str);
              star_pos += addr_str.length();
            }
            try {
              std::shared_ptr<Expression> expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
              int64_t value = expr->Evaluate(symbols);
              symbols.Define(eq->label_name, SymbolType::Equate,
                             std::make_shared<LiteralExpr>(
                                 static_cast<uint32_t>(value)));
            } catch (const std::exception &e) {
              (void)e; // Forward reference - ignore this pass, will resolve later
            }
          }
        } break;
        case AtomType::Data: {
          auto data = std::dynamic_pointer_cast<DataAtom>(atom);
          if (!data) {
            // Cast failed - this indicates a corrupted atom
            AssemblerError error;
            error.location = atom->location;
            error.message =
                "Failed to cast to DataAtom - atom corruption detected";
            result.errors.push_back(error);
            result.success = false;
            continue;
          }

          // Re-evaluate expressions on each pass for forward references
          if (!data->expressions.empty()) {
            data->data.clear();

            for (const auto &expr_str_raw : data->expressions) {
              // Replace * with current virtual address, same as EquateAtom
              std::string expr_str = expr_str_raw;
              {
                std::string addr_str = std::to_string(virtual_address);
                size_t star_pos = 0;
                while ((star_pos = expr_str.find('*', star_pos)) !=
                       std::string::npos) {
                  bool preceded_by_ident = false;
                  if (star_pos > 0) {
                    char prev = expr_str[star_pos - 1];
                    preceded_by_ident =
                        std::isalnum(static_cast<unsigned char>(prev)) ||
                        prev == '.' || prev == '_' || prev == '?';
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
                std::shared_ptr<Expression> expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
                int64_t value = expr->Evaluate(symbols);

                if (data->data_size == DataSize::Byte) {
                  // Byte data (DB/DFB)
                  data->data.push_back(static_cast<uint8_t>(value & 0xFF));
                } else if (data->data_size == DataSize::Long) {
                  // Long data (DA in 65816 mode) - 24-bit little-endian
                  auto word = static_cast<uint32_t>(value);
                  data->data.push_back(static_cast<uint8_t>(word & 0xFF));
                  data->data.push_back(
                      static_cast<uint8_t>((word >> 8) & 0xFF));
                  data->data.push_back(
                      static_cast<uint8_t>((word >> 16) & 0xFF));
                } else {
                  // Word data (DW/DA) - little-endian
                  auto word = static_cast<uint32_t>(value);
                  data->data.push_back(static_cast<uint8_t>(word & 0xFF));
                  data->data.push_back(
                      static_cast<uint8_t>((word >> 8) & 0xFF));
                }
              } catch (const UndefinedSymbolError &) {
                // Forward reference — use placeholder 0, resolve next pass
                if (data->data_size == DataSize::Byte) {
                  data->data.push_back(0);
                } else if (data->data_size == DataSize::Long) {
                  data->data.push_back(0);
                  data->data.push_back(0);
                  data->data.push_back(0);
                } else {
                  data->data.push_back(0);
                  data->data.push_back(0);
                }
              }
            }

            data->size = data->data.size();
          }

          // Advance both address counters past this data
          current_address += data->size;
          virtual_address += data->size;
          current_sizes.push_back(data->size);
        } break;
        case AtomType::Instruction: {
          auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
          if (!inst) {
            // Cast failed - this indicates a corrupted atom
            AssemblerError error;
            error.location = atom->location;
            error.message =
                "Failed to cast to InstructionAtom - atom corruption detected";
            result.errors.push_back(error);
            result.success = false;
            continue;
          }
          // Clear previous encoding
          inst->encoded_bytes.clear();

          // Encode the instruction
          std::string mnemonic = inst->mnemonic;
          std::string operand = inst->operand;

          // Strip trailing '!' from mnemonic before table lookup
          // The '!' suffix is used in some assembly dialects to force
          // a specific instruction encoding but should not affect the
          // mnemonic lookup in the opcode table.
          if (!mnemonic.empty() && mnemonic.back() == '!') {
            mnemonic.pop_back();
          }

          // Check if instruction requires special encoding (e.g., branch
          // relaxation, multi-byte instructions)
          //
          // WHY SPECIAL ENCODING?
          // =====================
          // Some instructions need context beyond standard operand values:
          //
          // 1. BRANCH RELAXATION (6502 branches):
          //    - Branches use 8-bit signed relative offsets (-128 to +127
          //    bytes)
          //    - If target is farther, must "relax" into longer sequence:
          //      Short form (2 bytes):  BEQ label
          //      Long form (5 bytes):   BNE skip / JMP label / skip: ...
          //    - Relaxation triggers cascading changes requiring multi-pass
          //
          // 2. MULTI-BYTE INSTRUCTIONS (MVN/MVP):
          //    - 65816 block move instructions take two operands
          //    - Need special parsing for "srcbank,destbank" format
          //
          // CPU plugin handles ALL special cases - core assembler stays
          // agnostic
          if (cpu_->RequiresSpecialEncoding(mnemonic)) {
            try {
              // Resolve labels in operand before passing to CPU plugin
              // Branch instructions need target address, not label name
              std::string resolved_operand = operand;
              std::string trimmed = Trim(operand);

              // Check if operand is a label reference (not starting with $ or
              // #). Skip resolution for multi-operand forms like MVN/MVP
              // (e.g. "0,1" or "$E1,1") — these are bank pairs, not labels.
              if (trimmed.find(',') != std::string::npos) {
                // Multi-operand instruction (MVN/MVP block move): pass as-is
              } else if (trimmed == "*") {
                // * means current PC address (branch to self)
                // Use virtual_address so phased code uses the virtual PC
                std::ostringstream oss;
                oss << "$" << std::hex << virtual_address;
                resolved_operand = oss.str();
              } else if (!trimmed.empty() && trimmed[0] != '$' &&
                         trimmed[0] != '#' && trimmed[0] != '(') {
                // Try to resolve as symbol.  ConcreteSymbolTable::Lookup
                // handles SCMASM uppercase fallback internally
                // (ADR-005 V1: migration complete — no manual toupper here).
                int64_t symbol_value = 0;
                int64_t expr_offset = 0;
                std::string lookup_name = trimmed;
                if (!symbols.Lookup(lookup_name, symbol_value)) {
                  // Try SYMBOL+N or SYMBOL-N form (e.g. "LABEL+5")
                  // Symbol chars are alphanumeric, '.', '_', '@', ':'
                  // Scan for a '+' or '-' that follows at least one symbol char
                  lookup_name = "";
                  for (size_t i = 1; i < trimmed.size(); ++i) {
                    char c = trimmed[i];
                    if (c == '+' || c == '-') {
                      std::string sym_part = trimmed.substr(0, i);
                      std::string off_str  = trimmed.substr(i);
                      int64_t sym_val = 0;
                      if (symbols.Lookup(sym_part, sym_val)) {
                        // Parse the numeric offset (decimal or hex)
                        try {
                          expr_offset = std::stoll(off_str, nullptr, 0);
                        } catch (...) {
                          expr_offset = 0;
                        }
                        symbol_value = sym_val;
                        lookup_name  = sym_part;
                        break;
                      }
                    }
                  }
                }
                if (!lookup_name.empty() && pass_number > 1) {
                  // Symbol resolved — use actual address (plus any expression
                  // offset, e.g. LABEL+5).
                  // "Start short, expand only when necessary": the CPU plugin
                  // (EncodeInstructionSpecial) checks the resolved distance and
                  // emits SHORT (2 bytes) if in range, LONG (3 or 5 bytes) only
                  // if the target is genuinely out of ±127 bytes.  Multiple
                  // passes converge to the minimum set of LONG branches because
                  // expanding a branch only shifts later labels forward, which
                  // is rechecked in the next pass.  No branch is ever made LONG
                  // unless it truly cannot reach its target.
                  std::ostringstream oss;
                  oss << "$" << std::hex << ((symbol_value + expr_offset) & 0xFFFF);
                  resolved_operand = oss.str();
                } else {
                  // Pass 1: always assume SHORT (use current VA as target →
                  // offset = -2, always in range).  Parse-time addresses can
                  // be stale (computed with all branches at 2 bytes), so using
                  // them in pass 1 causes spurious long-branch expansions that
                  // then cascade into later passes, locking in unnecessary 5-byte
                  // branches.  By starting all branches short in pass 1, we let
                  // passes 2+ converge to the minimum expansion set.
                  // Same path is used when symbol is still unresolved.
                  std::ostringstream oss;
                  oss << "$" << std::hex << virtual_address;
                  resolved_operand = oss.str();
                }
              }

              // Delegate to CPU plugin for special encoding
              // Plugin handles branch relaxation, multi-byte instructions, etc.
              // Use virtual_address as the PC so branches in phased code are
              // computed relative to the virtual address.
              inst->encoded_bytes = cpu_->EncodeInstructionSpecial(
                  mnemonic, resolved_operand,
                  static_cast<uint16_t>(virtual_address));

              // Advance both address counters past this instruction
              current_address += inst->encoded_bytes.size();
              virtual_address += inst->encoded_bytes.size();
              current_sizes.push_back(inst->encoded_bytes.size());
              continue; // Skip to next atom
            } catch (const std::exception &e) {
              // Special encoding failed - report error
              AssemblerError error;
              error.location = inst->location;
              error.message =
                  "Special encoding failed for " + mnemonic + ": " + e.what();
              result.errors.push_back(error);
              result.success = false;
              continue;
            }
          }

          // Parse operand value for standard encoding
          // Note: CPU plugin determines addressing mode from operand string
          uint16_t value = 0;

          // Make current PC available to expressions (e.g. `jmp *`)
          symbols.SetCurrentLocation(static_cast<int64_t>(virtual_address));

          // Extract operand value
          if (!operand.empty()) {
            std::string trimmed = Trim(operand);

            // Strip parentheses for indirect modes: ($1234) or ($80,X) or
            // ($80),Y
            std::string value_str = trimmed;
            if (value_str[0] == '(') {
              size_t close_paren = value_str.find(')');
              if (close_paren != std::string::npos) {
                value_str = value_str.substr(1, close_paren - 1);
                value_str = Trim(value_str);
              }
            }

            // Strip index registers (,X or ,Y) for value extraction
            size_t comma_pos = value_str.find(',');
            if (comma_pos != std::string::npos) {
              value_str = Trim(value_str.substr(0, comma_pos));
            }

            // Strip brackets for 65816 indirect long: [$zp] or [$zp],Y
            if (!value_str.empty() && value_str[0] == '[') {
              size_t close_bracket = value_str.find(']');
              if (close_bracket != std::string::npos) {
                value_str = Trim(value_str.substr(1, close_bracket - 1));
              }
            }

            if (value_str[0] == '#') {
              // Immediate: #$42 or #SYMBOL
              // Use shared ExpressionParser to handle both hex literals and symbol
              // references
              std::string expr_str = value_str.substr(1);
              try {
                std::shared_ptr<Expression> expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
                int64_t expr_value = expr->Evaluate(symbols);
                value = static_cast<uint16_t>(expr_value);
              } catch (const UndefinedSymbolError &) {
                value = 0; // Forward reference — resolve next pass
              }
            } else if (value_str[0] == '$') {
              // Absolute/Zero Page: $1234 (or $1234,X after stripping)
              // Use shared ExpressionParser to handle both simple hex and expressions
              // like $528+2
              try {
                std::shared_ptr<Expression> expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(value_str);
                int64_t expr_value = expr->Evaluate(symbols);
                value = static_cast<uint16_t>(expr_value);
              } catch (const UndefinedSymbolError &) {
                value = 0; // Forward reference — resolve next pass
              }
            } else if (value_str[0] == '/') {
              // SCMASM high byte immediate: /expr (equivalent to #>expr)
              // Evaluates expression and takes bits 8-15 as the immediate byte
              std::string expr_str = value_str.substr(1);
              try {
                std::shared_ptr<Expression> expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
                int64_t expr_value = expr->Evaluate(symbols);
                value = static_cast<uint16_t>(
                    (static_cast<uint32_t>(expr_value) >> 8) & 0xFF);
              } catch (const UndefinedSymbolError &) {
                value = 0; // Forward reference — resolve next pass
              }
            } else if (value_str != "A") {
              // Label reference or expression - use shared ExpressionParser to handle
              // both simple symbols and expressions like ZPPTR+1
              // BUG-003 FIX: Support expressions with +, -, <, > operators
              try {
                std::shared_ptr<Expression> expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(value_str);
                int64_t expr_value = expr->Evaluate(symbols);
                value = static_cast<uint16_t>(expr_value);
              } catch (const UndefinedSymbolError &) {
                value = 0; // Forward reference — resolve next pass
              }
            }
          }

          // Use polymorphic CPU plugin interface for instruction encoding
          try {
            // Call polymorphic EncodeInstruction() - CPU plugin handles
            // addressing modes
            inst->encoded_bytes =
                cpu_->EncodeInstruction(mnemonic, value, operand);
          } catch (const std::invalid_argument &e) {
            // Invalid argument (e.g., unsupported addressing mode)
            AssemblerError error;
            error.location = inst->location;
            error.message =
                "Invalid argument for " + mnemonic + ": " + e.what();
            result.errors.push_back(error);
            result.success = false;
          } catch (const std::out_of_range &e) {
            // Value out of range (e.g., branch too far, value too large)
            AssemblerError error;
            error.location = inst->location;
            error.message =
                "Value out of range for " + mnemonic + ": " + e.what();
            result.errors.push_back(error);
            result.success = false;
          } catch (const std::runtime_error &e) {
            // Runtime error (e.g., undefined behavior, internal error)
            AssemblerError error;
            error.location = inst->location;
            error.message =
                "Runtime error encoding " + mnemonic + ": " + e.what();
            result.errors.push_back(error);
            result.success = false;
          } catch (const std::logic_error &e) {
            // Logic error (programming error, shouldn't happen in production)
            AssemblerError error;
            error.location = inst->location;
            error.message =
                "Logic error encoding " + mnemonic + ": " + e.what();
            result.errors.push_back(error);
            result.success = false;
          }

          // Record size for convergence check
          current_sizes.push_back(inst->encoded_bytes.size());

          // Advance both address counters past this instruction
          current_address += inst->encoded_bytes.size();
          virtual_address += inst->encoded_bytes.size();
        } break;
        case AtomType::Space: {
          // SpaceAtom (DS/BS directives) — advance addresses past the reserved
          // bytes so that subsequent instruction virtual_addresses are correct
          // for branch offset calculations.
          auto space = std::dynamic_pointer_cast<SpaceAtom>(atom);
          if (space) {
            // If this SpaceAtom has a PC-relative expression (e.g. "DS $900-*"),
            // re-evaluate it each pass so the count stays correct as branch
            // relaxations shift code sizes between passes.
            if (!space->expression_str.empty()) {
              std::string expr_str = space->expression_str;
              std::string addr_str = std::to_string(virtual_address);
              size_t star_pos = 0;
              while ((star_pos = expr_str.find('*', star_pos)) != std::string::npos) {
                bool before = (star_pos > 0 &&
                    (std::isalnum(static_cast<unsigned char>(expr_str[star_pos - 1])) ||
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
                std::shared_ptr<Expression> expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
                int64_t value = expr->Evaluate(symbols);
                if (value >= 0) {
                  space->count = static_cast<size_t>(value);
                  space->size = space->count;
                }
              } catch (const std::exception &e) {
                (void)e; // Forward reference or error - keep previous count
              }
            }
            current_address += space->size;
            virtual_address += space->size;
            current_sizes.push_back(space->size);
          }
        }
        default: break;
        } // switch (atom->type)
      }
    }
  }
  return current_sizes;
}

void Assembler::RefixupDataAtoms(ConcreteSymbolTable &symbols,
                                 AssemblerResult &result) {
  // Re-evaluate DataAtom and EquateAtom content with the final converged symbol
  // values.  InstructionAtoms are intentionally skipped (their encoded_bytes
  // are left unchanged) to prevent branch-size changes from cascading into
  // address shifts.
  if (cpu_ == nullptr) {
    return;
  }

  for (auto &section : sections_) {
    uint32_t current_address = section.org;
    uint32_t virtual_address = section.org;
    uint32_t phase_real_start = 0;
    uint32_t phase_virtual_start = 0;

    for (auto &atom : section.atoms) {
      if (!atom)
        continue;

      switch (atom->type) {
      case AtomType::Phase: {
        auto phase = std::dynamic_pointer_cast<PhaseAtom>(atom);
        if (phase) {
          if (phase->is_start) {
            phase_real_start = current_address;
            phase_virtual_start = phase->virtual_addr;
            virtual_address = phase->virtual_addr;
          } else {
            uint32_t bytes_emitted = virtual_address - phase_virtual_start;
            current_address = phase_real_start + bytes_emitted;
            virtual_address = current_address;
          }
        }
      } break;
      case AtomType::Org: {
        auto org = std::dynamic_pointer_cast<OrgAtom>(atom);
        if (org) {
          current_address = org->address;
          virtual_address = org->address;
        }
      } break;
      case AtomType::DummyOrg: {
        // .OR inside .DUMMY/.ED: skip — do not move the real PC during
        // instruction encoding.  Addresses were already resolved by
        // ResolveSymbols().
      } break;
      case AtomType::CpuMode: {
        // XC / XC OFF — replay CPU mode change.
        auto cm = std::dynamic_pointer_cast<CpuModeAtom>(atom);
        if (cm && cpu_) {
          cpu_->SetCpuModeFromAtom(cm->mode);
        }
      } break;
      case AtomType::MxState: {
        // MX directive — replay M/X flag state.
        auto mx = std::dynamic_pointer_cast<MxAtom>(atom);
        if (mx && cpu_) {
          cpu_->SetMX(mx->m_flag, mx->x_flag);
        }
      } break;
      case AtomType::Equate: {
        // Re-evaluate position-dependent equates (.EQ *) with the current
        // virtual address so they track the correct value.
        auto eq = std::dynamic_pointer_cast<EquateAtom>(atom);
        if (eq) {
          std::string expr_str = eq->expression_str;
          std::string addr_str = std::to_string(virtual_address);
          size_t star_pos = 0;
          while ((star_pos = expr_str.find('*', star_pos)) !=
                 std::string::npos) {
            bool preceded_by_ident = false;
            if (star_pos > 0) {
              char prev = expr_str[star_pos - 1];
              preceded_by_ident =
                  std::isalnum(static_cast<unsigned char>(prev)) ||
                  prev == '.' || prev == '_' || prev == '?';
            }
            if (preceded_by_ident) {
              star_pos++;
              continue;
            }
            expr_str.replace(star_pos, 1, addr_str);
            star_pos += addr_str.length();
          }
          try {
            std::shared_ptr<Expression> expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
            int64_t value = expr->Evaluate(symbols);
            symbols.Define(eq->label_name, SymbolType::Equate,
                           std::make_shared<LiteralExpr>(
                               static_cast<uint32_t>(value)));
          } catch (const std::exception &e) {
            (void)e; // Should be resolved by now; ignore.
          }
        }
      } break;
      case AtomType::Data: {
        auto data = std::dynamic_pointer_cast<DataAtom>(atom);
        if (data && !data->expressions.empty()) {
          data->data.clear();
          for (const auto &expr_str_raw : data->expressions) {
            // Replace * with current virtual address (same as EquateAtom handling)
            std::string expr_str = expr_str_raw;
            {
              std::string addr_str = std::to_string(virtual_address);
              size_t star_pos = 0;
              while ((star_pos = expr_str.find('*', star_pos)) !=
                     std::string::npos) {
                bool preceded_by_ident = false;
                if (star_pos > 0) {
                  char prev = expr_str[star_pos - 1];
                  preceded_by_ident =
                      std::isalnum(static_cast<unsigned char>(prev)) ||
                      prev == '.' || prev == '_' || prev == '?';
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
              std::shared_ptr<Expression> expr = ExpressionParser(&symbols, nullptr, expression_features_).Parse(expr_str);
              int64_t value = expr->Evaluate(symbols);
              if (data->data_size == DataSize::Byte) {
                data->data.push_back(static_cast<uint8_t>(value & 0xFF));
              } else if (data->data_size == DataSize::Long) {
                auto word = static_cast<uint32_t>(value);
                data->data.push_back(static_cast<uint8_t>(word & 0xFF));
                data->data.push_back(
                    static_cast<uint8_t>((word >> 8) & 0xFF));
                data->data.push_back(
                    static_cast<uint8_t>((word >> 16) & 0xFF));
              } else {
                auto word = static_cast<uint32_t>(value);
                data->data.push_back(static_cast<uint8_t>(word & 0xFF));
                data->data.push_back(
                    static_cast<uint8_t>((word >> 8) & 0xFF));
              }
            } catch (const UndefinedSymbolError &e) {
              // Still undefined after convergence — hard error.
              AssemblerError err;
              err.message = e.what();
              result.errors.push_back(err);
              result.success = false;
              if (data->data_size == DataSize::Byte) {
                data->data.push_back(0);
              } else if (data->data_size == DataSize::Long) {
                data->data.push_back(0);
                data->data.push_back(0);
                data->data.push_back(0);
              } else {
                data->data.push_back(0);
                data->data.push_back(0);
              }
            }
          }
          data->size = data->data.size();
        }
        current_address += data->size;
        virtual_address += data->size;
      } break;
      case AtomType::Instruction: {
        // Do NOT re-encode.  Just advance address past the existing bytes.
        auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
        if (inst) {
          current_address += inst->encoded_bytes.size();
          virtual_address += inst->encoded_bytes.size();
        }
      } break;
      case AtomType::Space: {
        auto space = std::dynamic_pointer_cast<SpaceAtom>(atom);
        if (space) {
          current_address += space->size;
          virtual_address += space->size;
        }
      } break;
      case AtomType::Align: {
        auto align = std::dynamic_pointer_cast<AlignAtom>(atom);
        if (align) {
          uint32_t remainder = current_address % align->alignment;
          if (remainder != 0) {
            uint32_t padding = align->alignment - remainder;
            current_address += padding;
            virtual_address += padding;
          }
        }
      } break;
      // Label, ListingControl: no bytes, no address change.
      default: break;
    } // switch (atom->type)
    } // for (auto &atom : section.atoms)
  } // for (auto &section : sections_)
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
  ConcreteSymbolTable *label_table_ptr = nullptr;

  if (symbols_ != nullptr) {
    // If external symbol table is ConcreteSymbolTable, use it for label updates
    label_table_ptr = dynamic_cast<ConcreteSymbolTable *>(symbols_);
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
    std::vector<size_t> current_sizes =
        EncodeInstructions(*label_table_ptr, result, pass);

    // Pass 2: Extract labels from LabelAtoms
    // (Must happen AFTER encoding so encoded_bytes.size() is correct)
    // ALWAYS do this to ensure correct addresses, even with external symbol
    // tables
    if (label_table_ptr != nullptr) {
      for (auto &section : sections_) {
        ResolveSymbols(section.atoms, *label_table_ptr, section.org, result);
      }
    }

    // Check for convergence
    // WHY: If instruction sizes are identical to previous pass, addresses won't
    // change, so we've reached a stable state and can stop.
    if (pass > 1) {
      converged = CheckConvergence(previous_sizes, current_sizes);
    }
    previous_sizes = current_sizes;
  }

  // Check if we hit the pass limit without converging
  if (!converged) {
    AssemblerError error;
    error.message =
        "Assembly did not converge after " + std::to_string(max_passes_) +
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

void Assembler::ResolveSymbols(std::vector<std::shared_ptr<Atom>> &atoms,
                               ConcreteSymbolTable &symbols,
                               uint32_t org_address, AssemblerResult &result) {
  // Clear only labels (preserve other symbols like EQU/SET)
  // For now, just redefine - this will overwrite parser's placeholder addresses
  uint32_t current_address = org_address;

  // Phase tracking: when inside .PH/.EP block, current_address uses the
  // virtual address so labels get the correct phased value.
  bool in_phase = false;
  uint32_t phase_real_start = 0;
  uint32_t phase_virtual_start = 0;

  // Process atoms to extract label addresses
  for (auto &atom : atoms) {
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
        error.message =
            "Failed to cast to PhaseAtom - atom corruption detected";
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
      (void)in_phase; // suppress unused warning if no further use
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
        error.message =
            "Failed to cast to LabelAtom - atom corruption detected";
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
        error.message =
            "Failed to cast to InstructionAtom - atom corruption detected";
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
        error.message =
            "Failed to cast to SpaceAtom - atom corruption detected";
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
        error.message =
            "Failed to cast to AlignAtom - atom corruption detected";
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
    default: break;
    } // switch (atom->type)
  }
}

bool Assembler::CheckConvergence(
    const std::vector<size_t> &previous_sizes,
    const std::vector<size_t> &current_sizes) {
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
  return current_sizes == previous_sizes;
}

} // namespace xasm
