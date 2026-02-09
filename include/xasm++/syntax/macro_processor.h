/**
 * @file macro_processor.h
 * @brief FLEX ASM09 Macro Processor
 *
 * This file defines the MacroProcessor class that handles macro definition,
 * storage, and expansion for FLEX ASM09 assembly language.
 *
 * The macro processor provides:
 * - Macro definition storage
 * - Parameter substitution
 * - Local label uniquification
 * - Macro expansion with nested macro support
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace xasm {

/**
 * @brief Macro definition structure
 *
 * Contains all information needed to define and expand a macro.
 */
struct MacroDefinition {
  std::string name;                    ///< Macro name
  std::vector<std::string> parameters; ///< Parameter names (max 8 per FLEX spec)
  std::vector<std::string> body;       ///< Macro body lines
  int definition_line;                 ///< Line number where defined
};

/**
 * @brief FLEX ASM09 Macro Processor
 *
 * Manages macro definitions and provides expansion services for the FLEX
 * ASM09 assembler. Handles parameter substitution and local label
 * uniquification according to FLEX ASM09 specifications.
 *
 * @par Features
 * - Store and retrieve macro definitions
 * - Case-insensitive macro lookup
 * - Parameter substitution with word-boundary checking
 * - Local label uniquification (.LABEL → .LABEL_001)
 * - Support for up to 8 parameters per macro (FLEX spec)
 *
 * @par Usage Example
 * @code
 * MacroProcessor processor;
 * 
 * // Define a macro
 * processor.DefineMacro("MOVB", {"SRC", "DEST"},
 *                       {"        LDA     SRC", "        STA     DEST"});
 * 
 * // Check if macro exists
 * if (processor.IsMacro("MOVB")) {
 *     // Expand macro with arguments
 *     auto lines = processor.ExpandMacro("MOVB", {"$80", "$90"});
 *     // lines[0] = "        LDA     $80"
 *     // lines[1] = "        STA     $90"
 * }
 * @endcode
 */
class MacroProcessor {
public:
  /**
   * @brief Default constructor
   */
  MacroProcessor() = default;

  /**
   * @brief Define a macro
   *
   * Stores a macro definition for later expansion. Macro names are stored
   * in uppercase for case-insensitive lookup.
   *
   * @param name Macro name (will be stored uppercase)
   * @param parameters Parameter names (max 8 per FLEX spec)
   * @param body Macro body lines
   *
   * @note If a macro with the same name exists, it will be replaced
   * @note Parameter names are case-sensitive in the body
   */
  void DefineMacro(const std::string &name,
                   const std::vector<std::string> &parameters,
                   const std::vector<std::string> &body);

  /**
   * @brief Check if a macro is defined
   *
   * @param name Macro name (case-insensitive)
   * @return true if macro exists
   */
  bool IsMacro(const std::string &name) const;

  /**
   * @brief Get a macro definition
   *
   * @param name Macro name (case-insensitive)
   * @return Pointer to macro definition, or nullptr if not found
   */
  const MacroDefinition *GetMacro(const std::string &name) const;

  /**
   * @brief Clear all macro definitions
   *
   * Removes all macros and resets the expansion counter.
   */
  void Clear();

  /**
   * @brief Expand a macro with arguments
   *
   * Expands a macro by:
   * 1. Substituting parameters with provided arguments
   * 2. Making local labels unique (appending expansion ID)
   * 3. Returning the expanded source lines
   *
   * @param name Macro name (case-insensitive)
   * @param arguments Argument values (corresponds to parameters)
   * @return Expanded source lines, or empty vector if macro not found
   *
   * @note Missing arguments are substituted with empty strings
   * @note Each expansion gets a unique ID for local label uniquification
   */
  std::vector<std::string>
  ExpandMacro(const std::string &name,
              const std::vector<std::string> &arguments);

private:
  /**
   * @brief Convert string to uppercase
   *
   * @param str String to convert
   * @return Uppercase string
   */
  std::string ToUpper(const std::string &str) const;

  /**
   * @brief Substitute parameters in a macro line
   *
   * Replaces parameter names with argument values using word-boundary
   * checking to avoid replacing substrings.
   *
   * @param line Macro body line
   * @param macro Macro definition
   * @param arguments Argument values
   * @return Line with parameters substituted
   */
  std::string SubstituteParameters(const std::string &line,
                                   const MacroDefinition &macro,
                                   const std::vector<std::string> &arguments);

  /**
   * @brief Make local label unique for this expansion
   *
   * Local labels (starting with '.') are made unique by appending the
   * expansion ID: .LOOP → .LOOP_001
   *
   * @param label Label name (may start with '.')
   * @param expansion_id Unique expansion ID
   * @return Unique label name
   */
  std::string MakeLocalLabelUnique(const std::string &label,
                                   int expansion_id);

  std::unordered_map<std::string, MacroDefinition> macros_; ///< Defined macros
  int expansion_counter_ = 0; ///< Unique ID for macro expansions
};

} // namespace xasm
