// OutputPlugin - Base interface for output plugins
// Phase 1: Minimal Viable Assembler - Binary Output Plugin

#pragma once

#include <string>
#include <vector>

namespace xasm {

// Forward declarations
class Section;
class SymbolTable;

// OutputPlugin base interface
class OutputPlugin {
public:
  virtual ~OutputPlugin() = default;

  // Plugin metadata
  virtual std::string GetName() const = 0;
  virtual std::string GetFileExtension() const = 0;

  // Output generation
  virtual void WriteOutput(const std::string& filename,
                           const std::vector<Section*>& sections,
                           const SymbolTable& symbols) = 0;
};

} // namespace xasm
