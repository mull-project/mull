#pragma once

#include <memory>
#include <mull/Reporters/Reporter.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "rust/mull-core/core.rs.h"

namespace tool {

struct ReporterParameters {
  std::vector<std::string> reporters;
  std::string reporterName;
  std::string reporterDirectory;
  std::string patchBasePathDir;
  bool compilationDatabaseAvailable;
  bool IDEReporterShowKilled;
  std::unordered_map<std::string, std::string> mullInformation;
};

/// Convert rust::Vec<rust::String> to std::vector<std::string>.
std::vector<std::string> toStdVector(const rust::Vec<rust::String> &v);

/// Build ReporterParameters from CliConfig + caller-provided info map.
ReporterParameters
buildReporterParameters(const CliConfig &cli,
                        std::unordered_map<std::string, std::string> mullInformation);

/// Instantiate reporters from string names in params.
std::vector<std::unique_ptr<mull::Reporter>> createReporters(const MullDiagnostics &diagnostics,
                                                             const ReporterParameters &params);

void dumpMutators(const MullDiagnostics &diagnostics, std::string out);

/// Convert argv into a rust::Vec<rust::String> suitable for Rust CLI parsing.
rust::Vec<rust::String> argsFromArgv(int argc, char **argv);

} // namespace tool
