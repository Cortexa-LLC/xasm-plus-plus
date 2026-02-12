/**
 * @file macro_processor.cpp
 * @brief FLEX ASM09 Macro Processor Implementation
 *
 * Implementation of macro definition storage, parameter substitution,
 * and macro expansion for FLEX ASM09 assembly language.
 */

#include "xasm++/common/macro_processor.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace xasm {

// ============================================================================
// Helper Functions
// ============================================================================

std::string MacroProcessor::ToUpper(const std::string &str) const {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return result;
}

// ============================================================================
// Public API Methods
// ============================================================================

void MacroProcessor::DefineMacro(const std::string &name,
                                 const std::vector<std::string> &parameters,
                                 const std::vector<std::string> &body) {
  MacroDefinition macro;
  macro.name = ToUpper(name); // Store in uppercase for case-insensitive lookup
  macro.parameters = parameters;
  macro.body = body;
  macro.definition_line = 0; // Will be set by caller if needed

  macros_[macro.name] = macro;
}

bool MacroProcessor::IsMacro(const std::string &name) const {
  return macros_.find(ToUpper(name)) != macros_.end();
}

const MacroDefinition *MacroProcessor::GetMacro(const std::string &name) const {
  auto it = macros_.find(ToUpper(name));
  if (it != macros_.end()) {
    return &it->second;
  }
  return nullptr;
}

void MacroProcessor::Clear() {
  macros_.clear();
  expansion_counter_ = 0;
}

std::vector<std::string>
MacroProcessor::ExpandMacro(const std::string &name,
                            const std::vector<std::string> &arguments) {
  // Find the macro definition
  auto it = macros_.find(ToUpper(name));
  if (it == macros_.end()) {
    return {}; // Macro not found
  }

  const MacroDefinition &macro = it->second;
  std::vector<std::string> result;

  // Generate unique expansion ID
  int expansion_id = ++expansion_counter_;

  // Process each line in the macro body
  for (const std::string &line : macro.body) {
    // Step 1: Substitute parameters with arguments
    std::string expanded = SubstituteParameters(line, macro, arguments);

    // Step 2: Make local labels unique
    // Find all labels in the line (labels start with .)
    std::string final_line;
    size_t pos = 0;
    while (pos < expanded.length()) {
      // Look for local label markers (.)
      size_t dot_pos = expanded.find('.', pos);
      if (dot_pos == std::string::npos) {
        // No more dots, append rest of line
        final_line += expanded.substr(pos);
        break;
      }

      // Append everything before the dot
      final_line += expanded.substr(pos, dot_pos - pos);

      // Extract the label name
      size_t label_start = dot_pos;
      size_t label_end = dot_pos + 1;
      while (label_end < expanded.length() &&
             (std::isalnum(static_cast<unsigned char>(expanded[label_end])) ||
              expanded[label_end] == '_')) {
        label_end++;
      }

      // Check if this looks like a local label (starts with . followed by
      // alphanum)
      if (label_end > label_start + 1 &&
          std::isalpha(
              static_cast<unsigned char>(expanded[label_start + 1]))) {
        // This is a local label - make it unique
        std::string label =
            expanded.substr(label_start, label_end - label_start);
        std::string unique_label = MakeLocalLabelUnique(label, expansion_id);
        final_line += unique_label;
        pos = label_end;
      } else {
        // Not a local label, just append the dot and continue
        final_line += '.';
        pos = dot_pos + 1;
      }
    }

    result.push_back(final_line);
  }

  return result;
}

// ============================================================================
// Private Implementation Methods
// ============================================================================

std::string
MacroProcessor::SubstituteParameters(const std::string &line,
                                     const MacroDefinition &macro,
                                     const std::vector<std::string> &arguments) {
  std::string result = line;

  // Replace each parameter with its corresponding argument
  for (size_t i = 0; i < macro.parameters.size(); ++i) {
    const std::string &param = macro.parameters[i];
    const std::string &arg = (i < arguments.size()) ? arguments[i] : "";

    // Find and replace all occurrences of this parameter (word boundaries)
    size_t pos = 0;
    while ((pos = result.find(param, pos)) != std::string::npos) {
      // Check if this is a whole word match
      bool is_start_boundary =
          (pos == 0 ||
           !std::isalnum(static_cast<unsigned char>(result[pos - 1])));
      bool is_end_boundary =
          (pos + param.length() >= result.length() ||
           !std::isalnum(static_cast<unsigned char>(result[pos + param.length()])));

      // Check if this parameter is part of a local label (preceded by '.')
      bool is_local_label = (pos > 0 && result[pos - 1] == '.');

      if (is_start_boundary && is_end_boundary && !is_local_label) {
        // This is a whole word match and not a local label, replace it
        result.replace(pos, param.length(), arg);
        pos += arg.length(); // Move past the replacement
      } else {
        // Not a whole word or is a local label, skip this occurrence
        pos += param.length();
      }
    }
  }

  return result;
}

std::string MacroProcessor::MakeLocalLabelUnique(const std::string &label,
                                                  int expansion_id) {
  // Local labels start with '.' in FLEX ASM09
  if (label.empty() || label[0] != '.') {
    // Not a local label, return unchanged
    return label;
  }

  // Make local label unique by appending expansion ID
  // Format: .LOOP -> .LOOP_001
  std::ostringstream oss;
  oss << label << "_" << std::setfill('0') << std::setw(3) << expansion_id;
  return oss.str();
}

} // namespace xasm
