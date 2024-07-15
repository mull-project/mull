#pragma once

namespace mull {

#include "GeneratedDebugConfig.h"
#include "GeneratedParallelizationConfig.h"

struct ParallelizationConfig : public GeneratedParallelizationConfig {
  static ParallelizationConfig defaultConfig();
  void normalize();
  bool exceedsHardware();
};

struct DebugConfig : public GeneratedDebugConfig {};

} // namespace mull
