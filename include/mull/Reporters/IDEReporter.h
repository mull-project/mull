#pragma once

#include "mull/Reporters/Reporter.h"
#include "mull/Reporters/SourceCodeReader.h"

struct MullDiagnostics;

namespace mull {

class IDEReporter : public Reporter {
public:
  explicit IDEReporter(const MullDiagnostics &diagnostics, bool showKilled = false,
                       const std::string &reportDir = "", const std::string &reportName = "");
  void reportResults(const Result &result) override;

private:
  const MullDiagnostics &diagnostics;
  bool showKilled;
  SourceCodeReader sourceCodeReader;
  std::string reportFilePath;
};

} // namespace mull
