#pragma once

#include <string>
#include <vector>

namespace mull {

enum class IDEDiagnosticsKind { None, Survived, Killed, All };

struct ParallelizationConfig {
  unsigned workers;
  unsigned testExecutionWorkers;
  unsigned mutantExecutionWorkers;
  ParallelizationConfig();
  static ParallelizationConfig defaultConfig();
  void normalize();
  bool exceedsHardware();
};

struct DebugConfig {
  bool printIR = false;
  bool printIRBefore = false;
  bool printIRAfter = false;
  bool traceMutants = false;
};

} // namespace mull
