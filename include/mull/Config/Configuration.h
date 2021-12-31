#pragma once

#include <string>
#include <vector>

#include "mull/Config/ConfigurationOptions.h"

namespace mull {

extern int MullDefaultTimeoutMilliseconds;
extern unsigned MullDefaultLinkerTimeoutMilliseconds;

class Diagnostics;

struct Configuration {
  bool debugEnabled;
  bool dryRunEnabled;
  bool captureTestOutput;
  bool captureMutantOutput;
  bool skipSanityCheckRun;
  bool includeNotCovered;
  bool keepObjectFiles;
  bool keepExecutable;
  bool mutateOnly;
  bool lowerBitcode;

  unsigned timeout;
  unsigned linkerTimeout;

  IDEDiagnosticsKind diagnostics;

  std::vector<std::string> bitcodePaths;
  std::vector<std::string> mutators;

  std::string executable;
  std::string outputFile;
  std::string coverageInfo;

  std::string linker;
  std::vector<std::string> linkerFlags;

  ParallelizationConfig parallelization;

  Configuration();

  static std::string findConfig(Diagnostics &diagnostics);
  static Configuration loadFromDisk(Diagnostics &diagnostics, const std::string &path);
};

} // namespace mull
