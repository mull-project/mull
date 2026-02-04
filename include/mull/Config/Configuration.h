#pragma once

#include <string>
#include <vector>

#include "mull/Config/ConfigurationOptions.h"
#include "rust/mull-config/lib.rs.h"

namespace mull {

extern int MullDefaultTimeoutMilliseconds;

class Diagnostics;

struct Configuration {
  std::string pathOnDisk;
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

  /// Populate this Configuration from a Rust MullConfig loaded via the FFI bridge.
  void populateFromRustConfig(const MullConfig &mullConfig, const std::string &configPath,
                              Diagnostics &diagnostics);
};

} // namespace mull
