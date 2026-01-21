// SimpleSyntax - Simple assembly syntax parser
// Phase 1: Minimal Viable Assembler - SimpleSyntax Plugin

#pragma once

#include <string>
#include "xasm++/section.h"
#include "xasm++/symbol.h"

namespace xasm {

// SimpleSyntax parser
// Parses simple assembly syntax into Atoms and Symbols
class SimpleSyntaxParser {
public:
  SimpleSyntaxParser() = default;

  // Parse assembly source and populate section with atoms
  void Parse(const std::string& source, Section& section, ConcreteSymbolTable& symbols);
};

} // namespace xasm
