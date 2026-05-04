/**
 * @file error_formatter.cpp
 * @brief Implementation of enhanced error formatting and diagnostics
 */

#include "xasm++/core/error_formatter.h"

#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace xasm {

// ============================================================================
// ErrorFormatter Implementation
// ============================================================================

ErrorFormatter::ErrorFormatter(ColorMode mode) : color_mode_(mode) {}

std::string ErrorFormatter::FormatError(const AssemblerError& error,
                                        const ConcreteSymbolTable* symbols) const {
  std::ostringstream oss;

  oss << Colorize("error:", "1;31") << " " << error.message << "\n";

  if (!error.location.filename.empty()) {
    oss << "  " << Colorize("-->", "1;36") << " "
        << error.location.filename << ":" << error.location.line << ":" << error.location.column
        << "\n";

    std::string source_line = ReadSourceLine(error.location.filename, error.location.line);
    if (!source_line.empty()) {
      int gutter_width = std::max(static_cast<int>(std::to_string(error.location.line).length()), 2);
      const std::string pipe = Colorize("|", "1;36");

      oss << "   " << pipe << "\n";
      oss << " " << std::setw(gutter_width) << error.location.line << " " << pipe << " "
          << source_line << "\n";
      oss << GenerateColumnMarker(error.location.column, 8, "");
      oss << "   " << pipe << "\n";
    }
  }

  if (symbols) {
    std::string symbol_name = ExtractSymbolName(error.message);
    if (!symbol_name.empty()) {
      oss << FormatSuggestions(FindSimilarSymbols(symbol_name, symbols));
    }
  }

  return oss.str();
}

size_t ErrorFormatter::CalculateEditDistance(const std::string& s1, const std::string& s2) {
  const size_t kLen1 = s1.length();
  const size_t kLen2 = s2.length();

  // Create distance matrix
  std::vector<std::vector<size_t>> dp(kLen1 + 1, std::vector<size_t>(kLen2 + 1));

  // Initialize base cases
  for (size_t i = 0; i <= kLen1; ++i) {
    dp[i][0] = i;
  }
  for (size_t j = 0; j <= kLen2; ++j) {
    dp[0][j] = j;
  }

  // Fill matrix
  for (size_t i = 1; i <= kLen1; ++i) {
    for (size_t j = 1; j <= kLen2; ++j) {
      size_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
      dp[i][j] = std::min({
          dp[i - 1][j] + 1,        // deletion
          dp[i][j - 1] + 1,        // insertion
          dp[i - 1][j - 1] + cost  // substitution
      });
    }
  }

  return dp[kLen1][kLen2];
}

bool ErrorFormatter::ShouldUseColors() const {
  // Check NO_COLOR environment variable
  const char* no_color = std::getenv("NO_COLOR");
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

std::string ErrorFormatter::Colorize(const std::string& text, const std::string& color) const {
  if (!ShouldUseColors()) {
    return text;
  }
  return "\033[" + color + "m" + text + "\033[0m";
}

std::string ErrorFormatter::ReadSourceLine(const std::string& filename, size_t line_number) {
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

std::string ErrorFormatter::GenerateColumnMarker(size_t column, size_t length,
                                                 const std::string& message) const {
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

std::vector<std::string> ErrorFormatter::FindSimilarSymbols(const std::string& typo,
                                                            const ConcreteSymbolTable* symbols) {
  if (!symbols) {
    return {};
  }

  std::vector<std::string> all_symbols = symbols->GetAllSymbolNames();
  if (all_symbols.empty()) {
    return {};
  }

  // Find symbols with small edit distance
  std::vector<std::pair<std::string, size_t>> candidates;

  for (const auto& symbol : all_symbols) {
    size_t distance = CalculateEditDistance(typo, symbol);

    // Only consider symbols with edit distance <= 2
    if (distance <= 2) {
      candidates.emplace_back(symbol, distance);
    }
  }

  // Sort by distance (best matches first)
  std::sort(candidates.begin(), candidates.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

  // Return up to 3 best matches
  std::vector<std::string> suggestions;
  for (size_t i = 0; i < std::min(static_cast<size_t>(3), candidates.size()); ++i) {
    suggestions.emplace_back(candidates[i].first);
  }

  return suggestions;
}

std::string ErrorFormatter::ExtractSymbolName(const std::string& message) {
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

std::string ErrorFormatter::FormatSuggestions(const std::vector<std::string>& suggestions) const {
  if (suggestions.empty()) {
    return "";
  }

  std::ostringstream oss;

  if (suggestions.size() == 1) {
    // Single suggestion
    if (ShouldUseColors()) {
      oss << "   \033[1;32mhelp:\033[0m did you mean '" << suggestions[0] << "'?\n";
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

    for (const auto& suggestion : suggestions) {
      oss << "         - " << suggestion << "\n";
    }
  }

  return oss.str();
}

}  // namespace xasm
