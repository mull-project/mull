#pragma once

#include "Reporter.h"

namespace mull {

class Diagnostics;

class IDEReporter : public Reporter {
public:
  IDEReporter(Diagnostics &diagnostics, bool showKilled = false);
  void reportResults(const Result &result, const Metrics &metrics) override;

private:
  Diagnostics &diagnostics;
  bool showKilled;

  void reportSurvivedMutants(const Result &result, const Metrics &metrics);
  void reportAllMutants(const Result &result, const Metrics &metrics);
};

} // namespace mull