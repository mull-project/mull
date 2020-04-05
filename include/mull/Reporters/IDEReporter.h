#pragma once

#include "mull/Reporters/Reporter.h"
#include "mull/Reporters/SourceCodeReader.h"

namespace mull {

class Diagnostics;

class IDEReporter : public Reporter {
public:
  IDEReporter(Diagnostics &diagnostics, bool showKilled = false);
  void reportResults(const Result &result, const Metrics &metrics) override;

private:
  Diagnostics &diagnostics;
  bool showKilled;
  SourceCodeReader sourceCodeReader;

  void reportSurvivedMutants(const Result &result, const Metrics &metrics);
  void reportAllMutants(const Result &result, const Metrics &metrics);
};

} // namespace mull