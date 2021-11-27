#pragma once

#include <string>
#include <vector>

namespace mull {

enum class IDEDiagnosticsKind { None, Survived, Killed, All };

struct ParallelizationConfig {
  int workers;
  int testExecutionWorkers;
  int mutantExecutionWorkers;
  ParallelizationConfig();
  static ParallelizationConfig defaultConfig();
  void normalize();
  bool exceedsHardware();
};

} // namespace mull
