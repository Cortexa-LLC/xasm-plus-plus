/**
 * @file segment_manager.h
 * @brief Segment management for Z80 assemblers (CSEG/DSEG/ASEG/COMMON)
 *
 * This file defines segment management for Z80 assemblers, supporting:
 * - CSEG: Code segment
 * - DSEG: Data segment
 * - ASEG: Absolute segment
 * - COMMON /name/: Named common blocks
 *
 * @note Phase 5: Segment Management
 */

#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace xasm {

/**
 * @brief Segment types supported by Z80 assemblers
 */
enum class SegmentType {
  Code,     ///< CSEG - Code segment
  Data,     ///< DSEG - Data segment
  Absolute, ///< ASEG - Absolute segment
  Common    ///< COMMON - Named common block
};

/**
 * @brief Manages multiple segments and their address counters
 *
 * The SegmentManager tracks multiple segments (CSEG, DSEG, ASEG) and named
 * COMMON blocks. When switching between segments, each segment maintains its
 * own address counter and origin, allowing the assembler to generate code and
 * data in separate address spaces.
 *
 * @par Example Usage
 * @code
 * SegmentManager mgr;
 *
 * // Start in CSEG at 0x1000
 * mgr.SwitchToSegment(SegmentType::Code);
 * mgr.SetOrigin(0x1000);
 *
 * // Advance by 10 bytes
 * mgr.Advance(10);  // Now at 0x100A
 *
 * // Switch to DSEG at 0x2000
 * mgr.SwitchToSegment(SegmentType::Data);
 * mgr.SetOrigin(0x2000);
 * mgr.Advance(5);  // Now at 0x2005
 *
 * // Switch back to CSEG
 * mgr.SwitchToSegment(SegmentType::Code);
 * // Still at 0x100A (preserved)
 * @endcode
 */
class SegmentManager {
public:
  /**
   * @brief Default constructor - starts in CSEG at address 0
   */
  SegmentManager();

  /**
   * @brief Switch to a specific segment type
   *
   * Changes the current active segment. The address counter of the previous
   * segment is preserved. If switching to a segment for the first time, it
   * starts at address 0.
   *
   * @param type Segment type to switch to
   */
  void SwitchToSegment(SegmentType type);

  /**
   * @brief Switch to a named COMMON block
   *
   * Changes the current active segment to a COMMON block with the given name.
   * COMMON blocks are independent address spaces that can be referenced
   * multiple times. Each reference to the same COMMON block continues from
   * where it left off.
   *
   * @param name COMMON block name (can be empty for anonymous COMMON)
   */
  void SwitchToCommon(const std::string &name);

  /**
   * @brief Set the origin (starting address) for the current segment
   *
   * Sets the address counter for the current segment. This is typically
   * used after an ORG directive.
   *
   * @param address New origin address
   */
  void SetOrigin(uint64_t address);

  /**
   * @brief Get the current address counter
   *
   * Returns the current address for the active segment. This is used when
   * defining labels or assembling data/instructions.
   *
   * @return Current address counter value
   */
  uint64_t GetCurrentAddress() const;

  /**
   * @brief Advance the address counter
   *
   * Increments the address counter by the given number of bytes. This is
   * called after assembling data or instructions.
   *
   * @param bytes Number of bytes to advance
   */
  void Advance(size_t bytes);

  /**
   * @brief Get the current segment type
   *
   * Returns the type of the currently active segment.
   *
   * @return Current segment type
   */
  SegmentType GetCurrentSegmentType() const;

  /**
   * @brief Get the current COMMON block name
   *
   * If the current segment is a COMMON block, returns its name.
   * Otherwise returns an empty string.
   *
   * @return Current COMMON block name, or empty if not in COMMON
   */
  std::string GetCurrentCommonName() const;

  /**
   * @brief Reset all segments to initial state
   *
   * Resets all segment address counters to 0 and clears COMMON blocks.
   * Returns to CSEG as the active segment.
   */
  void Reset();

private:
  /// Segment state: tracks address for a single segment
  struct SegmentState {
    uint64_t address = 0;    ///< Current address counter
    bool has_origin = false; ///< True if ORG has been set for this segment
  };

  SegmentType current_segment_;     ///< Currently active segment type
  std::string current_common_name_; ///< Current COMMON block name (if any)

  /// Address counters for each standard segment type
  std::map<SegmentType, SegmentState> segments_;

  /// Address counters for named COMMON blocks
  std::map<std::string, SegmentState> common_blocks_;
};

} // namespace xasm
