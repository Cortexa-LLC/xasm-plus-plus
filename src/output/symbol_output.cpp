/**
 * @file symbol_output.cpp
 * @brief Symbol table output plugin implementation
 *
 * This file implements the symbol table output plugin, which generates a
 * file containing all defined symbols and their values.
 *
 * @note Phase 3.3: Output Format Plugins - Symbol Table Output
 */

#include "xasm++/output/symbol_output.h"
#include "xasm++/expression.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace xasm {

std::string SymbolOutput::GetName() const { return "symbol"; }

std::string SymbolOutput::GetFileExtension() const { return ".sym"; }

/**
 * @brief Get string representation of symbol type
 * @param type Symbol type
 * @return Type string
 */
static std::string GetSymbolTypeString(SymbolType type) {
  switch (type) {
  case SymbolType::Label:
    return "label";
  case SymbolType::Equate:
    return "equate";
  case SymbolType::Set:
    return "set";
  default:
    return "unknown";
  }
}

/**
 * @brief Format a hex value with padding
 * @param value Value to format
 * @param width Field width
 * @return Formatted string
 */
static std::string FormatHexValue(int32_t value, int width = 4) {
  std::ostringstream oss;
  oss << std::uppercase << std::hex << std::setw(width) << std::setfill('0')
      << value;
  return oss.str();
}

void SymbolOutput::WriteOutput(const std::string &filename,
                               const std::vector<Section *> & /* sections */,
                               const SymbolTable &symbols) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open output file: " + filename);
  }

  // Write header
  file << "Symbol Table\n";
  file << "============\n\n";

  // Structure to hold symbol information
  struct SymbolInfo {
    std::string name;
    int32_t value;
    SymbolType type;
  };

  std::vector<SymbolInfo> symbol_list;

  // Get all symbols from the concrete symbol table
  if (auto *concrete = dynamic_cast<const ConcreteSymbolTable *>(&symbols)) {
    for (const auto &[name, symbol] : concrete->GetAllSymbols()) {
      SymbolInfo info;
      info.name = name;
      info.type = symbol.type;

      // Evaluate the expression to get the value
      if (symbol.value) {
        info.value = symbol.value->Evaluate(symbols);
      } else {
        info.value = 0;
      }

      symbol_list.push_back(info);
    }
  }

  // Sort symbols alphabetically by name
  std::sort(
      symbol_list.begin(), symbol_list.end(),
      [](const SymbolInfo &a, const SymbolInfo &b) { return a.name < b.name; });

  // Write symbols
  for (const auto &sym : symbol_list) {
    file << std::left << std::setw(15) << sym.name << " = $"
         << FormatHexValue(sym.value) << "  (" << GetSymbolTypeString(sym.type)
         << ")\n";
  }

  if (symbol_list.empty()) {
    file << "(No symbols defined)\n";
  }

  file << "\n; End of symbol table\n";
  file.close();
}

} // namespace xasm
