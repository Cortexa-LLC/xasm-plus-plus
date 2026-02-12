/**
 * @file cpu_constants.h
 * @brief CPU family and variant name constants
 *
 * Defines string constants for CPU names to avoid magic strings throughout
 * the codebase.
 */

#pragma once

#include <string>

namespace xasm {
namespace cpu {

// CPU Family Names
inline constexpr const char *CPU_6502 = "6502";
inline constexpr const char *CPU_65C02 = "65c02";
inline constexpr const char *CPU_65C02_ROCK = "65c02rock";
inline constexpr const char *CPU_65816 = "65816";
inline constexpr const char *CPU_6809 = "6809";
inline constexpr const char *CPU_Z80 = "z80";

// CPU Family Identifiers (for GetCpuFamily())
inline constexpr const char *FAMILY_6502 = "6502";
inline constexpr const char *FAMILY_6809 = "6809";
inline constexpr const char *FAMILY_Z80 = "z80";

} // namespace cpu
} // namespace xasm
