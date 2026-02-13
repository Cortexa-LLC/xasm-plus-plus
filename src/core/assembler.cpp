// Assembler implementation

#include "xasm++/assembler.h"
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

// Helper: Simple expression parser for data directives
// This is a simplified version that handles:
// - Hex literals: $1234
// - Decimal literals: 42
// - Symbol references: label_name
// - Simple expressions: $528+2, label-$80
static std::shared_ptr<Expression>
ParseExpression(const std::string &str, ConcreteSymbolTable &symbols) {
  std::string trimmed = Trim(str);

  // Strip outer parentheses for grouping: (EXPR)
  // BUG-003 FIX: Support parentheses in complex expressions like <(MESSAGE+$10)
  if (!trimmed.empty() && trimmed[0] == '(' && trimmed[trimmed.length() - 1] == ')') {
    // Check if these are matching outer parentheses
    int depth = 0;
    bool is_outer = true;
    for (size_t i = 0; i < trimmed.length(); ++i) {
      if (trimmed[i] == '(') depth++;
      if (trimmed[i] == ')') depth--;
      // If depth hits 0 before the end, these aren't outer parens
      if (depth == 0 && i < trimmed.length() - 1) {
        is_outer = false;
        break;
      }
    }
    if (is_outer && depth == 0) {
      // Strip outer parentheses and recurse
      trimmed = Trim(trimmed.substr(1, trimmed.length() - 2));
    }
  }

  // Check for low byte operator (# or <) BEFORE checking for operators
  if (!trimmed.empty() && (trimmed[0] == '#' || trimmed[0] == '<')) {
    if (trimmed.length() < 2) {
      throw std::runtime_error("Low byte operator (#/<) requires an operand");
    }
    std::string operand = Trim(trimmed.substr(1));
    if (operand.empty()) {
      throw std::runtime_error("Low byte operator (#/<) has empty operand");
    }
    // Recursively parse the operand (might be expression like SHIFT0-$80)
    auto operand_expr = ParseExpression(operand, symbols);
    int64_t value = operand_expr->Evaluate(symbols);
    return std::make_shared<LiteralExpr>(value & 0xFF); // Low byte
  }

  // Check for high byte operator (>)
  if (!trimmed.empty() && trimmed[0] == '>') {
    if (trimmed.length() < 2) {
      throw std::runtime_error("High byte operator (>) requires an operand");
    }
    std::string operand = Trim(trimmed.substr(1));
    if (operand.empty()) {
      throw std::runtime_error("High byte operator (>) has empty operand");
    }
    // Recursively parse the operand (might be expression like SHIFT0-$80)
    auto operand_expr = ParseExpression(operand, symbols);
    int64_t value = operand_expr->Evaluate(symbols);
    return std::make_shared<LiteralExpr>((value >> 8) & 0xFF); // High byte
  }

  // Check for addition first (before trying to parse as single value)
  size_t plus_pos = trimmed.find('+');
  if (plus_pos != std::string::npos && plus_pos > 0) {
    std::string left = Trim(trimmed.substr(0, plus_pos));
    std::string right = Trim(trimmed.substr(plus_pos + 1));

    // Recursively parse both sides
    auto left_expr = ParseExpression(left, symbols);
    auto right_expr = ParseExpression(right, symbols);

    // Return BinaryOpExpr for addition
    return std::make_shared<BinaryOpExpr>(BinaryOp::Add, left_expr, right_expr);
  }

  // Check for subtraction (skip first char - could be negative sign)
  size_t minus_pos = trimmed.find('-', 1);
  if (minus_pos != std::string::npos && minus_pos > 0) {
    std::string left = Trim(trimmed.substr(0, minus_pos));
    std::string right = Trim(trimmed.substr(minus_pos + 1));

    // Recursively parse both sides
    auto left_expr = ParseExpression(left, symbols);
    auto right_expr = ParseExpression(right, symbols);

    // Return BinaryOpExpr for subtraction
    return std::make_shared<BinaryOpExpr>(BinaryOp::Subtract, left_expr,
                                          right_expr);
  }

  // Hex literal: $1234 (may have addressing mode suffix like $200,x)
  if (!trimmed.empty() && trimmed[0] == '$') {
    // Strip addressing mode suffix (,X ,Y ,S) before parsing
    std::string hex_str = trimmed;
    size_t comma_pos = hex_str.find(',');
    if (comma_pos != std::string::npos) {
      hex_str = hex_str.substr(0, comma_pos);
    }
    uint32_t value = ParseHex(hex_str);
    return std::make_shared<LiteralExpr>(value);
  }

  // Binary literal: %10101010
  if (!trimmed.empty() && trimmed[0] == '%') {
    std::string bin_part = trimmed.substr(1);
    if (bin_part.empty()) {
      throw std::runtime_error("Invalid binary number: '" + trimmed +
                               "' (no digits after %)");
    }

    // Validate binary digits BEFORE calling stoul
    for (char c : bin_part) {
      if (c != '0' && c != '1') {
        throw std::runtime_error("Invalid binary digit '" + std::string(1, c) +
                                 "' in binary number: '" + trimmed + "'");
      }
    }

    try {
      uint32_t value = std::stoul(bin_part, nullptr, 2);
      return std::make_shared<LiteralExpr>(value);
    } catch (const std::invalid_argument &e) {
      throw std::runtime_error("Invalid binary number: '" + trimmed + "' - " +
                               e.what());
    } catch (const std::out_of_range &e) {
      throw std::runtime_error("Binary number out of range: '" + trimmed +
                               "' - " + e.what());
    }
  }

  // Negative number: -1, -128 (check BEFORE general decimal to avoid symbol
  // lookup)
  if (!trimmed.empty() && trimmed[0] == '-') {
    // Check if rest is all digits (valid negative number)
    bool is_neg_number = true;
    for (size_t i = 1; i < trimmed.length(); ++i) {
      if (!std::isdigit(static_cast<unsigned char>(trimmed[i]))) {
        is_neg_number = false;
        break;
      }
    }
    if (is_neg_number && trimmed.length() > 1) {
      // Valid negative number
      int64_t value = std::stoll(trimmed);
      return std::make_shared<LiteralExpr>(value);
    }
    // Otherwise fall through - might be subtraction or symbol starting with '-'
  }

  // Decimal literal: 42
  bool is_number = true;
  for (char c : trimmed) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      is_number = false;
      break;
    }
  }
  if (is_number && !trimmed.empty()) {
    int64_t value = std::stoll(trimmed);
    return std::make_shared<LiteralExpr>(value);
  }

  // Symbol reference
  return std::make_shared<SymbolExpr>(trimmed);
}

Assembler::Assembler() {
  // CPU plugin handles instruction encoding - no handlers needed
}

void Assembler::SetCpuPlugin(CpuPlugin *cpu) { cpu_ = cpu; }

void Assembler::SetSymbolTable(SymbolTable *symbols) { symbols_ = symbols; }

void Assembler::AddSection(const Section &section) {
  sections_.push_back(section);
}

size_t Assembler::GetSectionCount() const { return sections_.size(); }

const std::vector<Section> &Assembler::GetSections() const { return sections_; }

void Assembler::Reset() { sections_.clear(); }

std::vector<size_t> Assembler::EncodeInstructions(ConcreteSymbolTable &symbols,
                                                  AssemblerResult &result) {
  // Encode instructions using CPU plugin
  std::vector<size_t> current_sizes;
  if (cpu_ != nullptr) {
    for (auto &section : sections_) {
      // Track current address during encoding
      uint32_t current_address = section.org;

      for (auto &atom : section.atoms) {
        // Skip null atoms gracefully
        if (!atom) {
          AssemblerError error;
          error.message = "Null atom encountered - skipping";
          result.errors.push_back(error);
          result.success = false;
          continue;
        }

        if (atom->type == AtomType::Org) {
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
        } else if (atom->type == AtomType::Label) {
          // Labels don't advance address yet, but we track them
          // (address will be finalized in Pass 2)
        } else if (atom->type == AtomType::Data) {
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

            for (const auto &expr_str : data->expressions) {
              try {
                auto expr = ParseExpression(expr_str, symbols);
                int64_t value = expr->Evaluate(symbols);

                if (data->data_size == DataSize::Byte) {
                  // Byte data (DB/DFB)
                  data->data.push_back(static_cast<uint8_t>(value & 0xFF));
                } else {
                  // Word data (DW/DA) - little-endian
                  uint32_t word = static_cast<uint32_t>(value);
                  data->data.push_back(static_cast<uint8_t>(word & 0xFF));
                  data->data.push_back(
                      static_cast<uint8_t>((word >> 8) & 0xFF));
                }
              } catch (const std::exception &e) {
                // Check if this is a parse error or an undefined symbol
                std::string msg(e.what());
                if (msg.find("Undefined symbol") != std::string::npos) {
                  // Symbol undefined (forward reference) - use placeholder
                  // Multi-pass assembly will resolve on subsequent passes
                  // If still undefined after convergence, will be caught
                  // elsewhere
                  if (data->data_size == DataSize::Byte) {
                    data->data.push_back(0);
                  } else {
                    data->data.push_back(0);
                    data->data.push_back(0);
                  }
                } else {
                  // Parse error - propagate the exception
                  throw;
                }
              }
            }

            data->size = data->data.size();
          }

          // Advance address past this data
          current_address += data->size;
          current_sizes.push_back(data->size);
        } else if (atom->type == AtomType::Instruction) {
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
              // #)
              if (!trimmed.empty() && trimmed[0] != '$' && trimmed[0] != '#' &&
                  trimmed[0] != '(') {
                // Try to resolve as symbol
                int64_t symbol_value;
                if (symbols.Lookup(trimmed, symbol_value)) {
                  // Convert resolved address to hex string format expected by
                  // CPU plugin
                  std::ostringstream oss;
                  oss << "$" << std::hex << symbol_value;
                  resolved_operand = oss.str();
                } else {
                  // Label not yet defined - use placeholder $0000 for first
                  // pass Multi-pass assembly will resolve on subsequent passes
                  resolved_operand = "$0000";
                }
              }

              // Delegate to CPU plugin for special encoding
              // Plugin handles branch relaxation, multi-byte instructions, etc.
              inst->encoded_bytes = cpu_->EncodeInstructionSpecial(
                  mnemonic, resolved_operand,
                  static_cast<uint16_t>(current_address));

              // Advance current address past this instruction
              current_address += inst->encoded_bytes.size();
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

            if (value_str[0] == '#') {
              // Immediate: #$42 or #SYMBOL
              // Use ParseExpression to handle both hex literals and symbol
              // references
              std::string expr_str = value_str.substr(1);
              try {
                auto expr = ParseExpression(expr_str, symbols);
                int64_t expr_value = expr->Evaluate(symbols);
                value = static_cast<uint16_t>(expr_value);
              } catch (const std::exception &e) {
                // Check if this is a parse error or an undefined symbol
                std::string msg(e.what());
                if (msg.find("Undefined symbol") != std::string::npos) {
                  // Symbol undefined (forward reference) - use placeholder 0
                  // Multi-pass assembly will resolve on subsequent passes
                  value = 0;
                } else {
                  // Parse error - propagate the exception
                  throw;
                }
              }
            } else if (value_str[0] == '$') {
              // Absolute/Zero Page: $1234 (or $1234,X after stripping)
              // Use ParseExpression to handle both simple hex and expressions
              // like $528+2
              try {
                auto expr = ParseExpression(value_str, symbols);
                int64_t expr_value = expr->Evaluate(symbols);
                value = static_cast<uint16_t>(expr_value);
              } catch (const std::exception &e) {
                // Check if this is a parse error or an undefined symbol
                std::string msg(e.what());
                if (msg.find("Undefined symbol") != std::string::npos) {
                  // Symbol undefined (forward reference) - use placeholder 0
                  value = 0;
                } else {
                  // Parse error - propagate the exception
                  throw;
                }
              }
            } else if (value_str != "A") {
              // Label reference or expression - use ParseExpression to handle
              // both simple symbols and expressions like ZPPTR+1
              // BUG-003 FIX: Support expressions with +, -, <, > operators
              try {
                auto expr = ParseExpression(value_str, symbols);
                int64_t expr_value = expr->Evaluate(symbols);
                value = static_cast<uint16_t>(expr_value);
              } catch (const std::exception &e) {
                // Check if this is a parse error or an undefined symbol
                std::string msg(e.what());
                if (msg.find("Undefined symbol") != std::string::npos) {
                  // Symbol undefined (forward reference) - use placeholder 0
                  // Multi-pass assembly will resolve on subsequent passes
                  value = 0;
                } else {
                  // Parse error - propagate the exception
                  throw;
                }
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

          // Advance current address past this instruction
          current_address += inst->encoded_bytes.size();
        }
      }
    }
  }
  return current_sizes;
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
  // Typically converges in 2-3 passes. MAX_PASSES (500) prevents infinite loops
  // in pathological cases (though such cases are rare in practice).
  //
  // WHY 500 PASSES?
  // ===============
  // Empirically, real-world code converges quickly (<10 passes).
  // 500 is a safety limit chosen to be:
  // - Large enough for any realistic code
  // - Small enough to catch bugs (e.g., oscillating branches)
  // - Fast enough not to annoy users (even 500 passes takes <1 second)

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

  while (!converged && pass < MAX_PASSES) {
    pass++;

    // Pass 1: Encode instructions using CPU plugin
    std::vector<size_t> current_sizes =
        EncodeInstructions(*label_table_ptr, result);

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

  result.pass_count = pass;
  return result;
}

void Assembler::ResolveSymbols(std::vector<std::shared_ptr<Atom>> &atoms,
                               ConcreteSymbolTable &symbols,
                               uint32_t org_address, AssemblerResult &result) {
  // Clear only labels (preserve other symbols like EQU/SET)
  // For now, just redefine - this will overwrite parser's placeholder addresses
  uint32_t current_address = org_address;

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

    if (atom->type == AtomType::Org) {
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
    } else if (atom->type == AtomType::Label) {
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
    } else if (atom->type == AtomType::Instruction) {
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
    } else if (atom->type == AtomType::Data) {
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
    } else if (atom->type == AtomType::Space) {
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
    } else if (atom->type == AtomType::Align) {
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
  }
}

bool Assembler::CheckConvergence(
    const std::vector<size_t> &previous_sizes,
    const std::vector<size_t> &current_sizes) const {
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
