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
  bool quiet;
  bool silent;
  bool dryRunEnabled;
  bool captureTestOutput;
  bool captureMutantOutput;
  bool skipSanityCheckRun;
  bool includeNotCovered;
  bool keepObjectFiles;
  bool keepExecutable;
  bool mutateOnly;
  bool lowerBitcode;
  bool junkDetectionDisabled;

  unsigned timeout;
  unsigned linkerTimeout;

  IDEDiagnosticsKind diagnostics;

  std::vector<std::string> bitcodePaths; // TODO: Drop this one
  std::vector<std::string> mutators;

  std::string executable;
  std::string outputFile;
  std::string coverageInfo;

  std::string linker;
  std::vector<std::string> linkerFlags;

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
