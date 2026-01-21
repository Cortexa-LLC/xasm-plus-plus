// BinaryOutput - Binary output plugin
// Phase 1: Minimal Viable Assembler - Binary Output Plugin

#pragma once

#include "xasm++/output/output_plugin.h"
#include <string>
#include <vector>

namespace xasm {

// Forward declarations
class Section;
class SymbolTable;

// BinaryOutput plugin - writes raw binary output
class BinaryOutput : public OutputPlugin {
public:
  BinaryOutput() = default;

  // OutputPlugin interface
  std::string GetName() const override;
  std::string GetFileExtension() const override;
  void WriteOutput(const std::string& filename,
                   const std::vector<Section*>& sections,
                   const SymbolTable& symbols) override;
};

} // namespace xasm
