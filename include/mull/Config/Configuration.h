#pragma once

#include <string>
#include <vector>

#include "mull/Config/ConfigurationOptions.h"

namespace mull {

extern int MullDefaultTimeoutMilliseconds;

class Diagnostics;

struct Configuration {
  bool debugEnabled;
  bool quiet;
  bool silent;
  bool dryRunEnabled;
  bool captureTestOutput;
  bool captureMutantOutput;
  bool includeNotCovered;
  bool junkDetectionDisabled;

  unsigned timeout;

  IDEDiagnosticsKind diagnostics;

  std::vector<std::string> mutators;
  std::vector<std::string> ignoreMutators;

  std::string executable;

  std::string compilationDatabasePath;
  std::vector<std::string> compilerFlags;

  std::vector<std::string> includePaths;
  std::vector<std::string> excludePaths;

  ParallelizationConfig parallelization;

  std::string gitDiffRef;
  std::string gitProjectRoot;

  DebugConfig debug{};

  Configuration();

  static std::string findConfig(Diagnostics &diagnostics);
  static Configuration loadFromDisk(Diagnostics &diagnostics, const std::string &path);
};

} // namespace mull
