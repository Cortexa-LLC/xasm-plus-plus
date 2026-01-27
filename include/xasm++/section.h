/**
 * @file section.h
 * @brief Section management for multi-section assembly
 * 
 * This file defines sections, which group related atoms together and can
 * have different attributes (code, data, bss) and origin addresses.
 * 
 * @note Phase 1: Minimal Viable Assembler - Core Abstractions
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "xasm++/atom.h"

namespace xasm {

/**
 * @brief Section attribute flags
 * 
 * These flags describe the characteristics of a section and can be
 * combined using bitwise OR to specify multiple attributes.
 * 
 * @par Example
 * @code
 * // Create a readable, executable code section
 * uint32_t attrs = static_cast<uint32_t>(SectionAttributes::Code) |
 *                  static_cast<uint32_t>(SectionAttributes::Read) |
 *                  static_cast<uint32_t>(SectionAttributes::Execute);
 * @endcode
 */
enum class SectionAttributes {
    Code    = 0x01,    ///< Code section (contains executable instructions)
    Data    = 0x02,    ///< Data section (contains initialized data)
    Bss     = 0x04,    ///< BSS section (contains uninitialized data)
    Read    = 0x08,    ///< Readable section
    Write   = 0x10,    ///< Writable section
    Execute = 0x20,    ///< Executable section
};

/**
 * @brief Represents a single output section
 * 
 * A section is a logical grouping of atoms that share common attributes
 * and an origin address. Different sections can be assembled independently
 * and combined in the output.
 * 
 * @par Typical Sections
 * - CODE: Contains executable instructions (.text)
 * - DATA: Contains initialized data (.data)
 * - BSS: Contains uninitialized data (.bss)
 * 
 * @par Example
 * @code
 * Section code("CODE", 0x01, 0x8000);  // Code section at 0x8000
 * code.atoms.push_back(std::make_shared<LabelAtom>("start", 0x8000));
 * code.atoms.push_back(std::make_shared<InstructionAtom>("LDA", "#$42"));
 * @endcode
 */
class Section {
public:
    std::string name;                           ///< Section name (e.g., "CODE", "DATA")
    uint32_t attributes = 0;                    ///< Section attributes (bitwise OR of SectionAttributes)
    uint64_t org = 0;                           ///< Origin address (starting address)
    std::vector<std::shared_ptr<Atom>> atoms;   ///< List of atoms in this section
    size_t current_offset = 0;                  ///< Current offset within section

    /**
     * @brief Default constructor - creates an empty section
     */
    Section() = default;

    /**
     * @brief Construct a named section with attributes and origin
     * 
     * @param n Section name
     * @param attr Section attributes (bitwise OR of SectionAttributes)
     * @param origin Starting address for this section (default: 0)
     */
    Section(const std::string& n, uint32_t attr, uint64_t origin = 0)
        : name(n), attributes(attr), org(origin), current_offset(0) {}
};

} // namespace xasm
