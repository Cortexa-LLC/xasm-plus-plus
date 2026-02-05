/**
 * @file error_formatter.cpp
 * @brief Implementation of enhanced error formatting and diagnostics
 */

#include "xasm++/core/error_formatter.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unistd.h>

namespace xasm {

// ============================================================================
// ErrorFormatter Implementation
// ============================================================================

ErrorFormatter::ErrorFormatter(ColorMode mode) : color_mode_(mode) {}

std::string
ErrorFormatter::FormatError(const AssemblerError &error,
                            const ConcreteSymbolTable *symbols) const {

  std::ostringstream oss;

  // Error header with color
  if (ShouldUseColors()) {
    oss << "\033[1;31merror:\033[0m ";
  } else {
    oss << "error: ";
  }
  oss << error.message << "\n";

  // Location information
  if (!error.location.filename.empty()) {
    if (ShouldUseColors()) {
      oss << "  \033[1;36m-->\033[0m ";
    } else {
      oss << "  --> ";
    }
    oss << error.location.filename << ":" << error.location.line << ":"
        << error.location.column << "\n";

    // Source context
    std::string source_line =
        ReadSourceLine(error.location.filename, error.location.line);
    if (!source_line.empty()) {
      // Calculate gutter width (line number width)
      int gutter_width = std::to_string(error.location.line).length();
      if (gutter_width < 2) {
        gutter_width = 2;
      }

      // Empty line before context
      if (ShouldUseColors()) {
        oss << "   \033[1;36m|\033[0m\n";
      } else {
        oss << "   |\n";
      }

      // The error line
      if (ShouldUseColors()) {
        oss << " " << std::setw(gutter_width) << error.location.line
            << " \033[1;36m|\033[0m " << source_line << "\n";
      } else {
        oss << " " << std::setw(gutter_width) << error.location.line << " | "
            << source_line << "\n";
      }

      // Column marker (default length of 8 characters)
      size_t marker_length = 8;
      std::string marker =
          GenerateColumnMarker(error.location.column, marker_length, "");
      oss << marker;

      // Empty line after context
      if (ShouldUseColors()) {
        oss << "   \033[1;36m|\033[0m\n";
      } else {
        oss << "   |\n";
      }
    }
  }

  // Symbol suggestions
  if (symbols) {
    std::string symbol_name = ExtractSymbolName(error.message);
    if (!symbol_name.empty()) {
      std::vector<std::string> suggestions =
          FindSimilarSymbols(symbol_name, symbols);
      if (!suggestions.empty()) {
        std::string help = FormatSuggestions(suggestions);
        if (!help.empty()) {
          oss << help;
        }
      }
    }
  }

  return oss.str();
}

size_t ErrorFormatter::CalculateEditDistance(const std::string &s1,
                                             const std::string &s2) {
  const size_t len1 = s1.length();
  const size_t len2 = s2.length();

  // Create distance matrix
  std::vector<std::vector<size_t>> dp(len1 + 1, std::vector<size_t>(len2 + 1));

  // Initialize base cases
  for (size_t i = 0; i <= len1; ++i) {
    dp[i][0] = i;
  }
  for (size_t j = 0; j <= len2; ++j) {
    dp[0][j] = j;
  }

  // Fill matrix
  for (size_t i = 1; i <= len1; ++i) {
    for (size_t j = 1; j <= len2; ++j) {
      size_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
      dp[i][j] = std::min({
          dp[i - 1][j] + 1,       // deletion
          dp[i][j - 1] + 1,       // insertion
          dp[i - 1][j - 1] + cost // substitution
      });
    }
  }

  return dp[len1][len2];
}

bool ErrorFormatter::ShouldUseColors() const {
  // Check NO_COLOR environment variable
  const char *no_color = std::getenv("NO_COLOR");
  if (no_color != nullptr && no_color[0] != '\0') {
    return false;
  }

  switch (color_mode_) {
  case ColorMode::Enabled:
    return true;
  case ColorMode::Disabled:
    return false;
  case ColorMode::Auto:
    // Check if stdout is a TTY
    return isatty(STDOUT_FILENO) != 0;
  }

  return false;
}

std::string ErrorFormatter::Colorize(const std::string &text,
                                     const std::string &color) const {
  if (!ShouldUseColors()) {
    return text;
  }
  return "\033[" + color + "m" + text + "\033[0m";
}

std::string ErrorFormatter::ReadSourceLine(const std::string &filename,
                                           size_t line_number) const {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return "";
  }

  std::string line;
  size_t current_line = 1;

  while (std::getline(file, line) && current_line <= line_number) {
    if (current_line == line_number) {
      return line;
    }
    ++current_line;
  }

  return "";
}

std::string
ErrorFormatter::GenerateColumnMarker(size_t column, size_t length,
                                     const std::string &message) const {
  std::ostringstream oss;

  if (ShouldUseColors()) {
    oss << "   \033[1;36m|\033[0m ";
  } else {
    oss << "   | ";
  }

  // Spaces to align marker
  for (size_t i = 1; i < column; ++i) {
    oss << " ";
  }

  // Marker (carets)
  if (ShouldUseColors()) {
    oss << "\033[1;31m";
  }
  for (size_t i = 0; i < length; ++i) {
    oss << "^";
  }
  if (ShouldUseColors()) {
    oss << "\033[0m";
  }

  // Optional message
  if (!message.empty()) {
    oss << " " << message;
  }

  oss << "\n";

  return oss.str();
}

std::vector<std::string>
ErrorFormatter::FindSimilarSymbols(const std::string &typo,
                                   const ConcreteSymbolTable *symbols) const {

  if (!symbols) {
    return {};
  }

  std::vector<std::string> all_symbols = symbols->GetAllSymbolNames();
  if (all_symbols.empty()) {
    return {};
  }

  // Find symbols with small edit distance
  std::vector<std::pair<std::string, size_t>> candidates;

  for (const auto &symbol : all_symbols) {
    size_t distance = CalculateEditDistance(typo, symbol);

    // Only consider symbols with edit distance <= 2
    if (distance <= 2) {
      candidates.push_back({symbol, distance});
    }
  }

  // Sort by distance (best matches first)
  std::sort(candidates.begin(), candidates.end(),
            [](const auto &a, const auto &b) { return a.second < b.second; });

  // Return up to 3 best matches
  std::vector<std::string> suggestions;
  for (size_t i = 0; i < std::min(size_t(3), candidates.size()); ++i) {
    suggestions.push_back(candidates[i].first);
  }

  return suggestions;
}

std::string
ErrorFormatter::ExtractSymbolName(const std::string &message) const {
  // Look for patterns like "symbol 'NAME'" or "'NAME' not defined"
  size_t start = message.find('\'');
  if (start == std::string::npos) {
    return "";
  }

  size_t end = message.find('\'', start + 1);
  if (end == std::string::npos) {
    return "";
  }

  return message.substr(start + 1, end - start - 1);
}

std::string ErrorFormatter::FormatSuggestions(
    const std::vector<std::string> &suggestions) const {

  if (suggestions.empty()) {
    return "";
  }

  std::ostringstream oss;

  if (suggestions.size() == 1) {
    // Single suggestion
    if (ShouldUseColors()) {
      oss << "   \033[1;32mhelp:\033[0m did you mean '" << suggestions[0]
          << "'?\n";
    } else {
      oss << "   help: did you mean '" << suggestions[0] << "'?\n";
    }
  } else {
    // Multiple suggestions
    if (ShouldUseColors()) {
      oss << "   \033[1;32mhelp:\033[0m did you mean one of these?\n";
    } else {
      oss << "   help: did you mean one of these?\n";
    }

    for (const auto &suggestion : suggestions) {
      oss << "         - " << suggestion << "\n";
    }
  }

  return oss.str();
}

} // namespace xasm
