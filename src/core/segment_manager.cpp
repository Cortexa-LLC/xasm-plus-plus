/**
 * @file segment_manager.cpp
 * @brief Implementation of segment management for Z80 assemblers
 */

#include "xasm++/segment_manager.h"

namespace xasm {

SegmentManager::SegmentManager() : current_segment_(SegmentType::Code) {
  // Initialize all standard segments with address 0
  segments_[SegmentType::Code] = SegmentState{};
  segments_[SegmentType::Data] = SegmentState{};
  segments_[SegmentType::Absolute] = SegmentState{};
}

void SegmentManager::SwitchToSegment(SegmentType type) {
  current_segment_ = type;
  current_common_name_.clear();

  // Ensure segment exists
  if (segments_.find(type) == segments_.end()) {
    segments_[type] = SegmentState{};
  }
}

void SegmentManager::SwitchToCommon(const std::string &name) {
  current_segment_ = SegmentType::Common;
  current_common_name_ = name;

  // Ensure COMMON block exists
  if (common_blocks_.find(name) == common_blocks_.end()) {
    common_blocks_[name] = SegmentState{};
  }
}

void SegmentManager::SetOrigin(uint64_t address) {
  if (current_segment_ == SegmentType::Common) {
    auto &state = common_blocks_[current_common_name_];
    state.address = address;
    state.has_origin = true;
  } else {
    auto &state = segments_[current_segment_];
    state.address = address;
    state.has_origin = true;
  }
}

uint64_t SegmentManager::GetCurrentAddress() const {
  if (current_segment_ == SegmentType::Common) {
    auto it = common_blocks_.find(current_common_name_);
    if (it != common_blocks_.end()) {
      return it->second.address;
    }
    return 0;
  } else {
    auto it = segments_.find(current_segment_);
    if (it != segments_.end()) {
      return it->second.address;
    }
    return 0;
  }
}

void SegmentManager::Advance(size_t bytes) {
  if (current_segment_ == SegmentType::Common) {
    common_blocks_[current_common_name_].address += bytes;
  } else {
    segments_[current_segment_].address += bytes;
  }
}

SegmentType SegmentManager::GetCurrentSegmentType() const {
  return current_segment_;
}

std::string SegmentManager::GetCurrentCommonName() const {
  return current_common_name_;
}

void SegmentManager::Reset() {
  current_segment_ = SegmentType::Code;
  current_common_name_.clear();
  segments_.clear();
  common_blocks_.clear();

  // Re-initialize default segments
  segments_[SegmentType::Code] = SegmentState{};
  segments_[SegmentType::Data] = SegmentState{};
  segments_[SegmentType::Absolute] = SegmentState{};
}

} // namespace xasm
