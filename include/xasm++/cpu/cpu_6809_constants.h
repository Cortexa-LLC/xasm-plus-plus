/**
 * @file cpu_6809_constants.h
 * @brief Motorola 6809 CPU-specific constants
 *
 * Defines constants for 6809 CPU architecture limits and addressing modes.
 */

#pragma once

#include <cstdint>

namespace xasm {
namespace cpu6809 {

// ============================================================================
// Addressing Mode Limits
// ============================================================================

/// Direct page address limit (0x00-0xFF)
/// Addresses below this value can use direct page addressing mode
constexpr uint32_t DIRECT_PAGE_MAX = 0x100;

} // namespace cpu6809
} // namespace xasm
